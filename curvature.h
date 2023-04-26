#pragma once
#include "stdafx.h"

#include "Utils/Utilities.h"

inline double getAngleBetweenVertices(const MPoint& left, const MPoint& center, const MPoint& right) {
	const MVector u = center - left;
	const MVector v = center - right;

	const double cosine = u * v / (u.length() * v.length());
	return acos(cosine);
}

inline void getGaussianCurvature(const MFnMesh& meshFn, std::map<int, double>& curvature)
{
	MItMeshVertex vertex_it = meshFn.object();
	MItMeshPolygon face_it = meshFn.object();
	//std::map<int, double> curvature;
	while (!vertex_it.isDone())
	{
		if (vertex_it.onBoundary())
		{
			curvature[vertex_it.index()] = M_PI;
		}
		else
		{
			curvature[vertex_it.index()] = 2 * M_PI;
		}
		vertex_it.next();
	}

	MIntArray vertices;
	MPointArray points;
	while (!face_it.isDone())
	{
		face_it.getVertices(vertices);
		int numVertices = vertices.length();
		points.setLength(numVertices);
		for (int i = 0; i < numVertices; ++i)
		{
			meshFn.getPoint(vertices[i], points[i]);
		}
		for (int i = 0; i < numVertices; ++i)
		{
			curvature[vertices[i]] -= getAngleBetweenVertices(points[(i - 1) % numVertices],
				points[i], points[(i + 1) % numVertices]);
		}
		face_it.next();
	}
}
