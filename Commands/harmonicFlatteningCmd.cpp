#include "harmonicFlatteningCmd.h"
#include "stdafx.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"
#include "Utils/GMM_Macros.h"
#include "helpers.h"

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