#include "stdafx.h"

#include "Utils/Utilities.h"

#include "colorMeshVerticesCmd.h"

#include "Utils/STL_Macros.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"
#include "Utils/GMM_Macros.h"


#define MINARG "-min"
#define MAXARG "-max"



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

double getAngleBetweenVertices(const MPoint& left, const MPoint& center, const MPoint& right) {
	const MVector u = center - left;
	const MVector v = center - right;

	const double cosine = u * v / (u.length() * v.length());
	return acos(cosine);
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



	MSyntax commandSyntax = syntax();
	commandSyntax.addFlag(MINARG, MINARG, MSyntax::kDouble);
	commandSyntax.addFlag(MAXARG, MAXARG, MSyntax::kDouble);

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


	MString s1 = meshFn.createColorSetWithName("ValenceColorSet");
	MString s2 = meshFn.createColorSetWithName("CurvatureColorSet");

	MItMeshVertex vertex_it(meshFn.object());
	MIntArray valenceVertexList, curvatureVertexList;
	MColorArray valenceColors, curvatureColors;

	while (!vertex_it.isDone()) {
		int curIndex = vertex_it.index();

		MIntArray connectedVertices;
		vertex_it.getConnectedVertices(connectedVertices);
		unsigned int valence = connectedVertices.length();

		float vertexColor[3];
		colorVertexByValence(valence, vertexColor);
		valenceColors.append(vertexColor[0], vertexColor[1], vertexColor[2]);

		valenceVertexList.append(curIndex);
		vertex_it.next();
	}

	meshFn.setVertexColors(valenceColors, valenceVertexList);
	meshFn.setDisplayColors(true);

	// Get values of min, max flags
	double minCurvature = -M_PI, maxCurvate = M_PI;
	if (argData.isFlagSet(MINARG)) {
		minCurvature = argData.flagArgumentDouble(MINARG, 1);
	}
	if (argData.isFlagSet(MAXARG)) {
		maxCurvate = argData.flagArgumentDouble(MAXARG, 1);
	}


	vertex_it = meshFn.object();
	while (!vertex_it.isDone())
	{
		int curIndex = vertex_it.index();

		double curvature = 0;

		// Calculate Gaussian curvature from non-closed neighboring points
		MPoint pos = vertex_it.position();
		MIntArray connectedVertices;
		vertex_it.getConnectedVertices(connectedVertices);
		for (unsigned i = 0; i < connectedVertices.length() - 1; ++i) {
			MPoint left, right;
			meshFn.getPoint(connectedVertices[i], left);
			meshFn.getPoint(connectedVertices[i + 1], right);
			curvature -= getAngleBetweenVertices(left, pos, right);
		}

		if (vertex_it.onBoundary()) {
			curvature += M_PI;
		} else {
			curvature += 2*M_PI;
			MPoint left, right;
			meshFn.getPoint(connectedVertices[connectedVertices.length() - 1], left);
			meshFn.getPoint(connectedVertices[0], right);
			curvature -= getAngleBetweenVertices(left, pos, right);
		}


		float R, G, B;
		mapColor(curvature, R, G, B, minCurvature, maxCurvate);
		curvatureColors.append(R, G, B);

		curvatureVertexList.append(curIndex);
		vertex_it.next();
	}

	meshFn.setVertexColors(curvatureColors, curvatureVertexList);
	meshFn.setDisplayColors(true);
	/**************************************************************/

	return MS::kSuccess;
}


MSyntax colorMeshVerticesCmd::syntax()
{
	MStatus stat = MS::kSuccess;
	MSyntax commandSyntax;

	// Hint - you need to use here the addFlag method of MSyntax class

	stat = commandSyntax.setObjectType(MSyntax::kSelectionList, 1, 1); //expect exactly one object
	MCHECKERRORNORET(stat, "Can't create Syntax object for this command");

	commandSyntax.useSelectionAsDefault(true);
	return commandSyntax;}
