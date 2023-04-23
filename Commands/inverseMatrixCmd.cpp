#include "inverseMatrixCmd.h"

#include "stdafx.h"
#include "Utils/STL_Macros.h"
#include "Utils/Maya_Macros.h"
#include <Utils/Maya_Utils.h>
#include "Utils/MatlabInterface.h"
#include "Utils/MatlabGMMDataExchange.h"


inverseMatrixCmd::inverseMatrixCmd() {

}

void* inverseMatrixCmd::creator()
{
    return new inverseMatrixCmd();
}

MString inverseMatrixCmd::commandName() {
    return "inverseMatrixCmd";
};

bool inverseMatrixCmd::isUndoable() const
{
    return false;
}

MStatus inverseMatrixCmd::doIt(const MArgList& argList)
{
	MSyntax commandSyntax = syntax();
    for (int i = 0; i < 9; ++i) {
	    commandSyntax.addArg(MSyntax::kDouble);
    }

	GMMDenseColMatrix M(3, 3);
    for (int i = 0; i < 9; ++i)
    {
	    M(i / 3, i % 3) = argList.asDouble(i);
    }

    int result = MatlabGMMDataExchange::SetEngineDenseMatrix("M", M);
    MatlabInterface::GetEngine().EvalToCout("Q = inv(M)");

    

    GMMDenseColMatrix Q;
    result = MatlabGMMDataExchange::GetEngineDenseMatrix("Q", Q);
    cout << "printing the GMM Matrix: " <<  Q << endl;

    return MS::kSuccess;
}
