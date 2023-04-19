#include "stdafx.h"

#include "colorMeshVerticesCmd.h"

#include "Utils/STL_Macros.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"
#include "Utils/MatlabInterface.h"
#include "Utils/GMM_Macros.h"
#include "Utils/MatlabGMMDataExchange.h"


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
	meshFn.deleteColorSet("ExampleColorSet");
	MString s1 = meshFn.createColorSetWithName("ExampleColorSet");
	meshFn.setCurrentColorSetName( s1 );

	MItMeshVertex vertex_it(meshFn.object());
	MIntArray vertexList;
	MColorArray colors;

	int curIndex,mod;

	while ( !vertex_it.isDone() )
	{
		curIndex = vertex_it.index();
		mod = curIndex%3;
		switch ( mod )
		{
		case 0:
			colors.append( 1.0f , 0.0f , 0.0f );
			break;
		case 1:
			colors.append( 0.0f , 1.0f , 0.0f );
			break;
		case 2: 
			colors.append( 0.0f , 0.0f , 1.0f  );
			break;
		}
		vertexList.append( curIndex );
		vertex_it.next();
	}

	meshFn.setVertexColors ( colors , vertexList );	
	meshFn.setDisplayColors( true );
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
