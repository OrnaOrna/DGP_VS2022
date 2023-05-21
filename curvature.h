#pragma once
#include "stdafx.h"

double getAngleBetweenVertices(const MPoint& left, const MPoint& center, const MPoint& right);

void getGaussianCurvature(const MFnMesh& meshFn, std::map<int, double>& curvature);
