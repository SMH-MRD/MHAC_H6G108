#pragma once

#include <Windows.h>
#include <commctrl.h>
#include <time.h>
#include <string>

#define DIALOG_WND_TYPE

#define WORK_WND_X							600			//MAP表示位置X
#define WORK_WND_Y							425			//MAP表示位置Y
#define WORK_WND_W							480		    //MAP WINDOW幅
#define WORK_WND_H							320			//MAP WINDOW高さ

#define WORK_SCAN_TIME						200			// 表示更新周期msec


//コントロールID
#define ID_WORK_WND_BASE                    10600
#define ID_WORK_WND_CLOSE_PB				ID_WORK_WND_BASE+1


//起動タイマーID
#define ID_WORK_WND_TIMER					100

//操作卓パネル管理構造体
#define SLW_SLIDAR_0_NOTCH	5
#define SLW_SLIDAR_MAX	10
#define SLW_SLIDAR_MIN	0
#define BH_SLIDAR_0_NOTCH	5
#define BH_SLIDAR_MAX	10
#define BH_SLIDAR_MIN	0
#define MH_SLIDAR_0_NOTCH	5
#define MH_SLIDAR_MAX	10
#define MH_SLIDAR_MIN	0
#define GT_SLIDAR_0_NOTCH	5
#define GT_SLIDAR_MAX	10
#define GT_SLIDAR_MIN	0

#define LABEL_BUF_SIZE 32

typedef struct stPLCDbugPanelTag {
	int slider_slew = 0;							// IDC_SLIDER_SLEW                 1000
	bool check_estop = false;						// IDC_CHECK_ESTOP                 1001
	int slider_bh = 0;								// IDC_SLIDER_BH                   1002
													// IDC_SPIN_SLEW                   1003
	WCHAR static_slew_label[LABEL_BUF_SIZE] = L"";	// IDC_STATIC_SLEW_LABEL           1004
	WCHAR static_bh_label[LABEL_BUF_SIZE] = L"";	// IDC_STATIC_BH_LABEL             1005
													// IDC_SPIN_BH                     1006
	bool button_slew_0 = false;						// IDC_BUTTON_SLEW_0               1007
	bool button_bh_0 = false;						// IDC_BUTTON_BH_0                 1008
	int slider_gt = 0;								// IDC_SLIDER_GT                   1009
	WCHAR static_gt_label[LABEL_BUF_SIZE]=L"";		// IDC_STATIC_GT_LABEL             1010
													// IDC_SPIN_GT                     1011
	bool button_gt_0 = false;						// IDC_BUTTON_GT_0                 1012
	int slider_mh = 0;								// IDC_SLIDER_MH                   1013
	WCHAR static_mh_label[LABEL_BUF_SIZE] = L"";	// IDC_STATIC_MH_LABEL             1014
													// IDC_SPIN_MH                     1015
	bool button_mh_0 = false;						// IDC_BUTTON_MH_0                 1016
	bool button_source1_on = false;					// IDC_BUTTON_SOURCE1_ON           1017
	bool button_source1_off = false;				// IDC_BUTTON_SOURCE1_OFF          1018
	bool button_source2_on = false;					// IDC_BUTTON_SOURCE2_ON           1019
	bool button_source2_off = false;				// IDC_BUTTON_SOURCE2_OFF          1020
													// IDC_STATIC_SOURCE1_LABEL        1021
	bool check_antisway = false;					// IDC_CHECK_ANTISWAY              1022
													// IDC_STATIC_SOURCE2_LABEL        1023
	bool button_auto_start = false;					// IDC_BUTTON_AUTO_START           1025
	bool button_from1 = false;						// IDC_BUTTON_FROM1                1026
	bool button_auto_reset = false;					// IDC_BUTTON_AUTO_RESET	       1028
	bool button_from2 = false;						// IDC_BUTTON_FROM2                1029
	bool button_from3 = false;						// IDC_BUTTON_FROM3                1030
	bool button_from4 = false;						// IDC_BUTTON_FROM4                1031
	bool button_to1 = false;						// IDC_BUTTON_TO1                  1036
	bool button_to2 = false;						// IDC_BUTTON_TO2                  1040
	bool button_to3 = false;						// IDC_BUTTON_TO3                  1041
	bool button_to4 = false;						// IDC_BUTTON_TO4                  1042
}ST_PLC_DEBUG_PANEL, * LPST_PLC_DEBUG_PANEL;

class CWorkWindow_PLC
{
public:
	CWorkWindow_PLC();
	~CWorkWindow_PLC();

	std::wstring wstr;
	static HWND hWorkWnd;

	static ST_PLC_DEBUG_PANEL stOpePaneStat;

	static int update_all_controls(HWND);

	virtual HWND open_WorkWnd(HWND hwnd_parent);
	static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
	int close_WorkWnd();
};

