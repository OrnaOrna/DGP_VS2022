#include "stdafx.h"

#include "colorMeshVerticesCmd.h"

#include "Utils/STL_Macros.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"
#include "Utils/MatlabInterface.h"
#include "Utils/GMM_Macros.h"
#include "Utils/MatlabGMMDataExchange.h"


void colorVertexByValence(const unsigned int valence, float* vertexColor)
{
	if (valence >= 9) {
		vertexColor[0] = 1.0f;
		vertexColor[1] = 0.0f;
		vertexColor[2] = 0.0f;
	}
	else if (valence <= 3) {
		vertexColor[0] = 0.5f;
		vertexColor[1] = 0.0f;
		vertexColor[2] = 1.0f;
	}
	else switch (valence) {
	case 8: {
		vertexColor[0] = 0.0f;
		vertexColor[1] = 0.0f;
		vertexColor[2] = 1.0f;
		break;
	}
	case 7: {
		vertexColor[0] = 1.0f;
		vertexColor[1] = 1.0f;
		vertexColor[2] = 0.5f;
		break;
	}
	case 6: {
		vertexColor[0] = 0.0f;
		vertexColor[1] = 1.0f;
		vertexColor[2] = 0.0f;
		break;
	}
	case 5: {
		vertexColor[0] = 1.0f;
		vertexColor[1] = 0.0;
		vertexColor[2] = 1.0f;
		break;
	}
	case 4: {
		vertexColor[0] = 0.0f;
		vertexColor[1] = 1.0f;
		vertexColor[2] = 1.0f;
		break;
	}
	default:
		break;
	}
}

colorMeshVerticesCmd::colorMeshVerticesCmd()
{

}

void* colorMeshVerticesCmd::creator()
{
	return new colorMeshVerticesCmd;
}

MString colorMeshVerticesCmd::commandName()
{
	return "colorMeshVerticesCmd";
}

bool colorMeshVerticesCmd::isUndoable() const
{
	return false;
}

MStatus	colorMeshVerticesCmd::doIt(const MArgList& argList)
{
	MStatus stat = MS::kSuccess;

	//This code is here just as an example of how to use the Matlab interface.
	//You code for inverting a matrix should be written as part of a new Maya command with the name "inverseMatrixCmd”.
	//test Matlab engine
	if (0)
	{
		MatlabInterface::GetEngine().EvalToCout("My_Matrix = [1 2 3; 4 5 6]"); //creates a 2x3 matrix with name My_Matrix
		GMMDenseColMatrix M(2, 4);
		M(0, 0) = 8.0;
		M(1, 2) = -4.0;
		int result = MatlabGMMDataExchange::SetEngineDenseMatrix("M", M);

		GMMDenseColMatrix My_Matrix;
		result = MatlabGMMDataExchange::GetEngineDenseMatrix("My_Matrix", My_Matrix);
		cout << "printing the GMM Matrix: " <<  My_Matrix << endl;
	}


	MSyntax commandSyntax = syntax();
	commandSyntax.addArg(MSyntax::kString);
	commandSyntax.addFlag("-min", "-min", MSyntax::kDouble);
	commandSyntax.addFlag("-max", "-max", MSyntax::kDouble);

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

	int numVerticesInMeshCreated = 0;
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
	
	/***************** this part should be changed ****************/
	// Delete the color sets if they exist
	meshFn.deleteColorSet("ExampleColorSet");
	meshFn.deleteColorSet("ValenceColorSet");
	meshFn.deleteColorSet("CurvatureColorSet");

	MString colorBy = argList.asString(0);

	if (colorBy == "valence" || colorBy == "Valence") {
		MString s1 = meshFn.createColorSetWithName("ValenceColorSet");
		meshFn.setCurrentColorSetName(s1);

		MItMeshVertex vertex_it(meshFn.object());
		MIntArray vertexList;
		MColorArray colors;

		int curIndex, mod;

		while (!vertex_it.isDone())
		{
			int curIndex = vertex_it.index();

			MIntArray connectedVertices;
			stat = vertex_it.getConnectedVertices(connectedVertices);
			MCHECKERROR(stat, "Error getting connected vertices");

			unsigned int valence = connectedVertices.length();

			float vertexColor[3];
			colorVertexByValence(valence, vertexColor);
			colors.append(vertexColor[0], vertexColor[1], vertexColor[2]);

			vertexList.append(curIndex);
			vertex_it.next();
		}

		meshFn.setVertexColors(colors, vertexList);
		meshFn.setDisplayColors(true);
	}


	/**************************************************************/

	return MS::kSuccess;}


MSyntax colorMeshVerticesCmd::syntax()
{
	MStatus stat = MS::kSuccess;
	MSyntax commandSyntax;

	// Hint - you need to use here the addFlag method of MSyntax class

	stat = commandSyntax.setObjectType(MSyntax::kSelectionList, 1, 1); //expect exactly one object
	MCHECKERRORNORET(stat, "Can't create Syntax object for this command");

	commandSyntax.useSelectionAsDefault(true);
	return commandSyntax;}
