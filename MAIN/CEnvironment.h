#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

class CEnvironment :public CTaskObj
{
public:
    CEnvironment() {}
    ~CEnvironment() {}

    ST_SPEC spec;
    int update_crane_status(LPST_CRANE_STATUS);
};

