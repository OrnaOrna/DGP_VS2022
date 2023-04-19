#include "stdafx.h"

#include "SpaceDeformer2D.h"
#include "Utils/STL_Macros.h"
#include "Utils/Maya_Macros.h"
#include "Utils/Maya_Utils.h"

const MTypeId SpaceDeformer2D::mTypeId(0x6723c);
const MString SpaceDeformer2D::mTypeName("SpaceDeformer2D");

MObject SpaceDeformer2D::mCageAttr;
MObject SpaceDeformer2D::mCoordinateTypeAttr;



SpaceDeformer2D::SpaceDeformer2D() : mIsFirstTime(true)
{

}

SpaceDeformer2D::~SpaceDeformer2D()
{

}

void* SpaceDeformer2D::creator()
{
	return new SpaceDeformer2D();
}

MStatus SpaceDeformer2D::initialize()
{
	MStatus stat;

	MFnTypedAttribute cageAttr;
	mCageAttr = cageAttr.create("cage" ,"cage", MFnData::kMesh, MObject::kNullObj, &stat);
	CHECK_MSTATUS(addAttribute(mCageAttr));
	CHECK_MSTATUS(attributeAffects(mCageAttr, outputGeom));

	MFnEnumAttribute coordinateTypeAttr;
	mCoordinateTypeAttr = coordinateTypeAttr.create("coordinateType" ,"coordinateType", 0, &stat);
	CHECK_MSTATUS(coordinateTypeAttr.setKeyable(true));
	CHECK_MSTATUS(addAttribute(mCoordinateTypeAttr));
	CHECK_MSTATUS(coordinateTypeAttr.addField("Cauchy", 0));
	CHECK_MSTATUS(coordinateTypeAttr.addField("Cauchy Interpolation", 1));
	CHECK_MSTATUS(coordinateTypeAttr.addField("Point to point", 2));
	CHECK_MSTATUS(attributeAffects(mCoordinateTypeAttr, outputGeom));

	return MStatus::kSuccess;
}


MStatus SpaceDeformer2D::deform(MDataBlock& block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex)
{
	MStatus stat;

	MDataHandle coordHandle = block.inputValue(mCoordinateTypeAttr, &stat);
	short coordinateType = coordHandle.asShort();

	MDataHandle handle = block.inputValue(mCageAttr, &stat);
	CHECK_MSTATUS_AND_RETURN_IT(stat);
	MObject cageMesh = handle.asMesh();

	MFnMesh cageMeshFn(cageMesh, &stat);
	if(stat != MS::kSuccess)
	{
		return stat;
	}

	updateCage(cageMeshFn);

	if(mIsFirstTime)
	{
		stat = doSetup(iter);
		CHECK_MSTATUS_AND_RETURN_IT(stat);
		mIsFirstTime = false;
	}

	///// add your code here //////
	///////////////////////////////


	//compute the deformation of all the internal points. This is done by simply multiplying the coordinate matrix by the cage vertices vector


	///////////////////////////////
	///////////////////////////////
	
	static double test = 0.0;

	test += 0.001;


	//update the new deformed position of all the internal vertices
	for(iter.reset(); !iter.isDone(); iter.next())
	{
		int i = iter.index();

		MPoint pt = iter.position();


		///// add your code here //////
		///////////////////////////////

		//update c to be the deformed position of the i'th vertex

		///////////////////////////////
		///////////////////////////////


		Complex c(pt[0], pt[1]);

		iter.setPosition(MPoint(c.real() + test, c.imag(), 0.0));
	}

	return stat;
}


MStatus SpaceDeformer2D::updateCage(MFnMesh& cageMeshFn)
{
	MStatus stat;

	int numFaces = cageMeshFn.numPolygons(&stat);

	assert(numFaces == 1);

	MIntArray vertexIndices;
	cageMeshFn.getPolygonVertices(0, vertexIndices);
	int numV = vertexIndices.length();
	assert(numV >= 3);

	gmm::clear(mCageVertices);
	gmm::resize(mCageVertices, numV, 1);

	MPointArray vertexArray;
	stat = cageMeshFn.getPoints(vertexArray);

	assert(numV == vertexArray.length());

	for(int i = 0; i < numV; i++)
	{
		MPoint p = vertexArray[i];
		Complex c(p[0], p[1]);
		mCageVertices(i, 0) = c;
	}
	return MS::kSuccess;
}


MStatus SpaceDeformer2D::doSetup(MItGeometry& iter)
{
	MStatus stat;

	int m = iter.count(&stat); //num internal points
	int n = mCageVertices.nrows(); //num vertices in the cage

	gmm::clear(mCoordinates);
	gmm::resize(mCoordinates, m, n);

	gmm::clear(mInternalPoints);
	gmm::resize(mInternalPoints, m, 1);

	for(iter.reset(); !iter.isDone(); iter.next())
	{
		int i = iter.index();
		MPoint pt = iter.position();

		Complex z0(pt[0], pt[1]); //internal point

		mInternalPoints(i, 0) = z0;

		for(int j = 0; j < n; j++)
		{
			Complex K(0.0, 0.0);

			///// add your code here //////
			///////////////////////////////

			//update K to be value of the j'th coordinate at the i'th internal point

			///////////////////////////////
			///////////////////////////////
			mCoordinates(i, j) = K;
		}
	}

	return MS::kSuccess;
}
