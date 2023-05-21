#pragma once
#include "stdafx.h"

class harmonicFlatteningCmd : public MPxCommand {
public:
    harmonicFlatteningCmd();
    virtual MStatus doIt(const MArgList& argList);
    static void* creator();
    static MSyntax syntax();
    static MString commandName();
    virtual bool isUndoable() const;
};