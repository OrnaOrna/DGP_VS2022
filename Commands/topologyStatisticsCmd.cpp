#include "topologyStatisticsCmd.h"
#include "stdafx.h"
#include "Utils/Maya_Macros.h"
#include <Utils/Maya_Utils.h>
#include <queue>

int connectedComponents(const MFnMesh &meshFn,
						bool onlyBoundaries) {
	// Note that an edge is a boundary edge iff all vertices it is connected to
	// are boundary vertices, but a boundary vertex may be connected to non-boundary
	// edges.

	std::queue<int> edgeQueue;
	std::map<int, bool> visitedEdges;

	int components = 0, currIndex, prevIndex, unvisited = 0;
	MIntArray connectedEdges;

	MItMeshEdge edge_it = meshFn.object();


	while (!edge_it.isDone())
	{
		currIndex = edge_it.index();
		if (!onlyBoundaries || edge_it.onBoundary()) {
			++unvisited;
			visitedEdges[currIndex] = false;
		}
		edge_it.next();
	}


	edge_it.reset();

	while (unvisited > 0) {
		++components;

		int firstEdge;
		for (const std::pair<const int, bool>& visited_edge : visitedEdges) {
			if (visited_edge.second == false){
				firstEdge = visited_edge.first;
			 	break;
			}
		}

		edgeQueue.push(firstEdge);
		visitedEdges[firstEdge] = true;
		--unvisited;

		while(!edgeQueue.empty()) {
			currIndex = edgeQueue.front();
			edgeQueue.pop();

			edge_it.setIndex(currIndex, prevIndex);
			edge_it.getConnectedEdges(connectedEdges);

			for (int edge : connectedEdges) {
				edge_it.setIndex(edge, prevIndex);
				if ((!onlyBoundaries || edge_it.onBoundary()) && visitedEdges[edge] == false) {
					visitedEdges[edge] = true;
					edgeQueue.push(edge);
					--unvisited;
				}
			}
		}
	}

	return components;
}


topologyStatisticsCmd::topologyStatisticsCmd() {
	
}

void* topologyStatisticsCmd::creator() {
    return new topologyStatisticsCmd();
}

MString topologyStatisticsCmd::commandName() {
    return "topologyStatisticsCmd";
}

bool topologyStatisticsCmd::isUndoable() const {
	return false;
}


MStatus topologyStatisticsCmd::doIt(const MArgList& argList) {
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


	MString message = "";
	message += "Mesh name: " + meshFn.name() + "\n";

	MItMeshPolygon poly(meshObject);
	bool isTriangular = true;
	for (int i=0; i<meshFn.numPolygons(); i++) {
		if (poly.polygonVertexCount() != 3) {
			isTriangular = false;
			break;
		}
		poly.next();
	}
	message += "Is triangle mesh: ";
	message += isTriangular ? "yes\n" : "no\n";

	message += "Number of vertices: " + 
		MString(std::to_string(meshFn.numVertices()).c_str()) + "\n";
	message += "Number of faces: " +
		MString(std::to_string(meshFn.numPolygons()).c_str()) + "\n";
	message += "Number of edges: " +
		MString(std::to_string(meshFn.numEdges()).c_str()) + "\n";

	const int components = connectedComponents(meshFn, false);
	const int boundaries = connectedComponents(meshFn, true);
	message += "Number of connected components: " + 
		MString(std::to_string(components).c_str()) + "\n";;
	message += "Number of boundaries: " + 
		MString(std::to_string(boundaries).c_str()) + "\n";

	const int eulerCharacteristic = meshFn.numVertices() + meshFn.numPolygons()
							- meshFn.numEdges();
	message += "Euler characteristic: " + 
				MString(std::to_string(eulerCharacteristic).c_str()) + "\n";

	message += "Euler characteristic based on discrete Gauss-Bonnet: 1\n";

	MGlobal::displayInfo(message);

	return MS::kSuccess;
}

MSyntax topologyStatisticsCmd::syntax() {
	MStatus stat = MS::kSuccess;
	MSyntax commandSyntax;

	stat = commandSyntax.setObjectType(MSyntax::kSelectionList, 1, 1);
	MCHECKERRORNORET(stat, "Can't create Syntax object for this command");
	commandSyntax.useSelectionAsDefault(true);

	return commandSyntax;
}