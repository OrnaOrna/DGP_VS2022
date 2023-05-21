#pragma once
#include "stdafx.h"
#include <list>

double getAngleBetweenVertices(const MPoint& left, const MPoint& center, const MPoint& right);

void getGaussianCurvature(const MFnMesh& meshFn, std::map<int, double>& curvature);

int connectedComponents(const MFnMesh &meshFn, bool onlyBoundaries);

void getBoundary(const MFnMesh& meshFn, std::list<int> vertices);