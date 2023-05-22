#include "harmonicFlatteningCmd.h"
#include "stdafx.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"
#include "Utils/GMM_Macros.h"
#include "helpers.h"
#include "Utils/MatlabGMMDataExchange.h"
#include "Utils/MatlabInterface.h"

harmonicFlatteningCmd::harmonicFlatteningCmd() = default;

void* harmonicFlatteningCmd::creator() {
	return new harmonicFlatteningCmd();
}

MString harmonicFlatteningCmd::commandName() {
	return "harmonicFlatteningCmd";
}

bool harmonicFlatteningCmd::isUndoable() const {
	return false;
}


MStatus harmonicFlatteningCmd::doIt(const MArgList& argList) {
	// Checks to see if everything is alright

	MStatus stat = MS::kSuccess;

	const MSyntax commandSyntax = syntax();

	MArgDatabase argData(commandSyntax, argList, &stat);
	MCHECKERROR(stat, "Wrong syntax for command " + commandName());

	MSelectionList objectsList;
	stat = argData.getObjects(objectsList);
	MCHECKERROR(stat, "Can't access object list");

	MObject object;
	stat = objectsList.getDependNode(0, object);
	MCHECKERROR(stat, "Can't access object");

	MObject meshObject;
	stat = Maya_Utils::getMe_a_Mesh(object, meshObject);
	MCHECKERROR(stat, "Object is not a mesh");

	MFnMesh meshFn(meshObject, &stat);
	MCHECKERROR(stat, "Can't access mesh");

	// Check that the mesh is indeed a triangle mesh
		int numPolygons = meshFn.numPolygons(&stat);

	MItMeshPolygon poly(meshObject);
	if(!poly.isPlanar(&stat) || poly.isLamina(&stat) || poly.isHoled(&stat))
	{
		
		MCHECKERROR(MS::kFailure, "The given polygon shape is either self intersecting, holed or non-planar which are not supported");
	}

	unsigned int temp; 
	for (int i=0; i<numPolygons; i++)
	{
		temp=poly.polygonVertexCount();
		if ( 3 != temp )
			MCHECKERROR(MS::kFailure, "this is not a triangle mesh!");
		poly.next();
	}

	// Check that the mesh is a topological disk
	const int components = connectedComponents(meshFn, false);
	const int boundaries = connectedComponents(meshFn, true);
	const int eulerCharacteristic = meshFn.numVertices() + meshFn.numPolygons()
		- meshFn.numEdges();
	const int genus = components - (eulerCharacteristic + boundaries) / 2;

	if (components != 1 || boundaries != 1 || genus != 0) {
		MGlobal::displayError("mesh is not a topological disk");
		return MS::kFailure;
	}

	MFloatArray u, v;
	u.setLength(meshFn.numVertices());
	v.setLength(meshFn.numVertices());

	std::list<int> boundary;
	getBoundary(meshFn, boundary);

	// Get the differences between the coords of pairs of vertices on the boundary
	std::list<MPoint> boundaryCoords = {};
	MPoint coords;
	for (int vertex : boundary) {
		meshFn.getPoint(vertex, coords);
		boundaryCoords.emplace_back(coords);
	}
	std::list<MVector> differences = {};
	std::adjacent_difference(boundaryCoords.begin(), boundaryCoords.end(), 
		std::back_inserter(differences));


	// Sum the lengths of the differences to get the total boundary length
	float totalBoundaryLength = 0;
	auto i = differences.begin();
	for (++i; i != differences.end(); ++i) {
		totalBoundaryLength += (*i).length();
	}

	// Initialize iterators to go over the boundaries except the first and last
	auto j = boundary.begin(), j_end = boundary.end();
	auto i_end = differences.end();
	--j_end;
	--i_end;
	i = differences.begin();

	// Set the boundary coords, at last
	u[*j] = 1;
	v[*j] = 0;
	float cumulativeBoundaryLength = 0; 
	for (++i, ++j ; i != i_end && j != j_end; ++i, ++j) {
		cumulativeBoundaryLength += (*i).length() / totalBoundaryLength;
		u[*j] = cosf(2 * M_PI * cumulativeBoundaryLength);
		v[*j] = sinf(2 * M_PI * cumulativeBoundaryLength);
	}

	int n = meshFn.numVertices(), m = boundary.size() - 1;
	GMMSparseRowMatrix weight_matrix(n - m, n - m);
	GMMDenseColMatrix rhs(n - m, 2);

	MItMeshVertex vertex_it(meshFn.object());

	// Initialize a map between rows in the matrix and indexes in the mesh,
	// and the other way around
	int row = 0;
	MIntArray rowMap(n - m);
	std::map<int, int> indexMap;
	while (!vertex_it.isDone()) {
		if (!vertex_it.onBoundary()) {
			rowMap[row] = vertex_it.index();
			indexMap[vertex_it.index()] = row;
			++row;
		}
		vertex_it.next();
	}

	// Fill in the weight matrix and the rhs vector, for now with uniform weights
	int col, _, rowSum;
	MIntArray connectedVertices;
	for (int currRow = 0; currRow < n - m; ++currRow) {
		rowSum = 0;
		vertex_it.setIndex(rowMap[currRow], _);
		vertex_it.getConnectedVertices(connectedVertices);
		for (const int vertex : connectedVertices) {
			vertex_it.setIndex(vertex, _);
			++rowSum;

			if (vertex_it.onBoundary()) {
				rhs(currRow, 0) = - u[vertex];
				rhs(currRow, 1) = - v[vertex];
			} else {
				col = indexMap[vertex];
				weight_matrix(currRow, col) = 1;
			}
			if (false);
		}
		weight_matrix(currRow, currRow) = -rowSum;
	}

	if (false);

	// Transfer matrices to MatLab
	int result = MatlabGMMDataExchange::SetEngineDenseMatrix("rhs", rhs);
	result = MatlabGMMDataExchange::SetEngineSparseMatrix("weights", weight_matrix);

	// Solve for the coordinates
	MatlabInterface::GetEngine().Eval("weights = weights * -1");
	MatlabInterface::GetEngine().Eval("rhs = rhs * -1");
	MatlabInterface::GetEngine().Eval("coords = solve_linear_system_with_cholesky(weights, rhs)");

	// Get the coords back from MatLab
	GMMDenseColMatrix coord_matrix(n - m, 2);
	MatlabGMMDataExchange::GetEngineDenseMatrix("coords", coord_matrix);

	// Set the coords to u, v
	int index;
	for (int currRow = 0; currRow < n - m; ++currRow) {
		index = rowMap[currRow];
		double uValue = coord_matrix(currRow, 0);
		double vValue = coord_matrix(currRow, 1);
		u[index] = uValue;
		v[index] = vValue;
	}

	for (int i = 0; i < n; i++) {
		cout << u[i] << " " << v[i] << endl;
	}

	// Create the UV set and set it properly
	MString uvName = "HarmonicUV";
	meshFn.createUVSetWithName(uvName);
	meshFn.setUVs(u, v, &uvName);
	MIntArray uvCounts, uvIds;
	meshFn.getVertices(uvCounts, uvIds);
	meshFn.assignUVs(uvCounts, uvIds, &uvName);

	return MS::kSuccess;
}

MSyntax harmonicFlatteningCmd::syntax() {
	MSyntax commandSyntax;

	const MStatus stat = commandSyntax.setObjectType(MSyntax::kSelectionList, 1, 1);
	MCHECKERRORNORET(stat, "Can't create Syntax object for this command");
	commandSyntax.useSelectionAsDefault(true);
	return commandSyntax;
}