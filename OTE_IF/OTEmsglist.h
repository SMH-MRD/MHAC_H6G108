#pragma once
#include "CSharedMem.h"

WCHAR pb_text[64][128]={
L"îÒí‚",   // ID_PB_ESTOP				0
L"êUé~",    // ID_PB_ANTISWAY_ON		1
L"",        // ID_PB_ANTISWAY_OFF		2
L"ãNìÆ",      // ID_PB_AUTO_START		3
L"é©ìÆ",   // ID_PB_AUTO_MODE			4
L"SETz",    // ID_PB_AUTO_SET_Z		    5
L"SETxy",   // ID_PB_AUTO_SET_XY		6
L"",L"",L"",L"",L"",

L"ã@è„",   // ID_PB_CRANE_MODE	        12
L"âìäu",   // ID_PB_REMOTE_MODE		    13
L"éÂ1ON",   // ID_PB_CTRL_SOURCE_ON	    14
L"éÂ1OFF",   // ID_PB_CTRL_SOURCE_OFF	15
L"éÂ2ON",   // ID_PB_CTRL_SOURCE2_ON	16
L"éÂ2OFF",   // ID_PB_CTRL_SOURCE2_OFF	17
L"AUTORST",  // ID_PB_AUTO_RESET        18
L"åÃè·RST",  // ID_PB_FAULT_RESET		19
L"S1",        // ID_PB_SEMI_AUTO_S1		20
L"S2",        // ID_PB_SEMI_AUTO_S2		21
L"S3",        // ID_PB_SEMI_AUTO_S3		22
L"L1",        // ID_PB_SEMI_AUTO_L1		23
L"L2",        // ID_PB_SEMI_AUTO_L2		24
L"L3",        // ID_PB_SEMI_AUTO_L3		25
L"",L"",L"",L"",

L"MH+",         // ID_PB_MH_P1          30
L"MH++",        // ID_PB_MH_P2          31
L"MH-",         // ID_PB_MH_M1          32
L"MH--",        // ID_PB_MH_M2          33
L"SL+",         // ID_PB_SL_P1          34
L"SL++",        // ID_PB_SL_P2          35
L"SL-",         // ID_PB_SL_M1          36
L"SL--",        // ID_PB_SL_M2          37
L"BH+",         // ID_PB_BH_P1          38
L"BH++",        // ID_PB_BH_P2          39
L"BH-",         // ID_PB_BH_M1          40
L"BH--",        // ID_PB_BH_M2          41
L"PARK",        // ID_PB_PARK           42
L"GRND",        // ID_PB_GRND           43
L"PICK",        // ID_PB_PICK           44
L"",L"",L"",L"",L"",L"",L"",L"",
L"DIST_MODE",  //ID_LAMP_OTE_NOTCH_MODE 53

};

WCHAR motion_text[MOTION_ID_MAX][128] = {
L"HST",
L"GNT",
L"TRY",
L"BH",
L"SLW",
L"-",    // ID_PB_AUTO_SET_Z		    5
L"-",   // ID_PB_AUTO_SET_XY		6
L""
};

bool be_create[MOTION_ID_MAX] = { true,true,false,true,true,false,false,false };