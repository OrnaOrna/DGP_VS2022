#include "harmonicFlatteningCmd.h"
#include "stdafx.h"
#include "Utils/Maya_Macros.h"
#include <Utils/Maya_Utils.h>
#include "curvature.h"
#include <queue>

harmonicFlatteningCmd::harmonicFlatteningCmd() {

}

void* harmonicFlatteningCmd::creator() {
	return new harmonicFlatteningCmd();
}

MString harmonicFlatteningCmd::commandName() {
	return "topologyStatisticsCmd";
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

	//num of vertices
	int n = meshFn.numVertices(&stat);
	//num of boundary vertices
	int m;
	GMMCompressedRowMatrix sparseMatrix(n-m, n-m);
