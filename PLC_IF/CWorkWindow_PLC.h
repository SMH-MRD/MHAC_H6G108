#pragma once

#include <Windows.h>
#include <commctrl.h>
#include <time.h>
#include <string>

#define DIALOG_WND_TYPE

#define WORK_WND_X							1050		//Window表示位置X
#define WORK_WND_Y							20 			//Window表示位置Y
#define WORK_WND_W							480		    //WindowWINDOW幅
#define WORK_WND_H							320			//WindowWINDOW高さ

#define IO_WND_X							1050		//Window表示位置X
#define IO_WND_Y							340 			//Window表示位置Y
#define IO_WND_W							320		    //WindowWINDOW幅
#define IO_WND_H							320			//WindowWINDOW高さ

#define WORK_SCAN_TIME						200			// 表示更新周期msec


//コントロールID
#define ID_WORK_WND_BASE                    10600
#define ID_WORK_WND_CLOSE_PB				ID_WORK_WND_BASE+1

//IO CHECK 表示更新タイマーID
#define ID_IO_CHK_UPDATE_TIMER				10700

//表示更新周期(msec)
#define IO_CHK_TIMER_PRIOD        			100


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
	bool button_remote = false;						// IDC_BUTTON_REMOTE               1027
	bool button_auto_reset = false;					// IDC_BUTTON_AUTO_RESET	       1028
	bool button_from2 = false;						// IDC_BUTTON_FROM2                1029
	bool button_from3 = false;						// IDC_BUTTON_FROM3                1030
	bool button_from4 = false;						// IDC_BUTTON_FROM4                1031
	bool button_to1 = false;						// IDC_BUTTON_TO1                  1036
	bool button_to2 = false;						// IDC_BUTTON_TO2                  1040
	bool button_to3 = false;						// IDC_BUTTON_TO3                  1041
	bool button_to4 = false;						// IDC_BUTTON_TO4                  1042
}ST_PLC_DEBUG_PANEL, * LPST_PLC_DEBUG_PANEL;

//コモンコントロールID
#define ID_PLCIO_PB_PLUS			2250
#define ID_PLCIO_PB_MINUS			2251
#define ID_PLCIO_PB_RESET			2252
#define ID_PLCIO_PB_DATSET			2253
#define ID_PLCIO_PB_DEC				2254
#define ID_PLCIO_PB_HEX				2255
#define ID_PLCIO_RADIO_BI			2256
#define ID_PLCIO_RADIO_BO			2257
#define ID_PLCIO_RADIO_WI			2258
#define ID_PLCIO_RADIO_WO			2259
#define ID_PLCIO_CHK_FORCE			2260
#define ID_PLCIO_CHK_PAUSE			2261


#define PLCIO_IO_DISP_NUM			8

#define ID_PLCIO_STATIC_DI0			2270
#define ID_PLCIO_STATIC_DI1			2271
#define ID_PLCIO_STATIC_DI2			2272
#define ID_PLCIO_STATIC_DI3			2273
#define ID_PLCIO_STATIC_DI4			2274
#define ID_PLCIO_STATIC_DI5			2275
#define ID_PLCIO_STATIC_DI6			2276
#define ID_PLCIO_STATIC_DI7			2277

#define ID_PLCIO_STATIC_AI0			2280
#define ID_PLCIO_STATIC_AI1			2281
#define ID_PLCIO_STATIC_AI2			2282
#define ID_PLCIO_STATIC_AI3			2283
#define ID_PLCIO_STATIC_AI4			2284
#define ID_PLCIO_STATIC_AI5			2285
#define ID_PLCIO_STATIC_AI6			2286
#define ID_PLCIO_STATIC_AI7			2287

#define ID_PLCIO_STATIC_DO0			2290
#define ID_PLCIO_STATIC_DO1			2291
#define ID_PLCIO_STATIC_DO2			2292
#define ID_PLCIO_STATIC_DO3			2293
#define ID_PLCIO_STATIC_DO4			2294
#define ID_PLCIO_STATIC_DO5			2295
#define ID_PLCIO_STATIC_DO6			2296
#define ID_PLCIO_STATIC_DO7			2297

#define ID_PLCIO_STATIC_AO0			2300
#define ID_PLCIO_STATIC_AO1			2301
#define ID_PLCIO_STATIC_AO2			2302
#define ID_PLCIO_STATIC_AO3			2303
#define ID_PLCIO_STATIC_AO4			2304
#define ID_PLCIO_STATIC_AO5			2305
#define ID_PLCIO_STATIC_AO6			2306
#define ID_PLCIO_STATIC_AO7			2307

#define ID_PLCIO_STATIC_DI_ADDR		2308
#define ID_PLCIO_STATIC_AI_ADDR		2309
#define ID_PLCIO_STATIC_DO_ADDR		2310
#define ID_PLCIO_STATIC_AO_ADDR		2311

#define ID_PLCIO_EDIT_OFFSET		2312
#define ID_PLCIO_EDIT_VALUE			2313

#define ID_PLCIO_STATIC_LABEL_ADDR	2320
#define ID_PLCIO_STATIC_LABEL_0		2321
#define ID_PLCIO_STATIC_LABEL_1		2322
#define ID_PLCIO_STATIC_LABEL_2		2323
#define ID_PLCIO_STATIC_LABEL_3		2324
#define ID_PLCIO_STATIC_LABEL_4		2325
#define ID_PLCIO_STATIC_LABEL_5		2326
#define ID_PLCIO_STATIC_LABEL_6		2327
#define ID_PLCIO_STATIC_LABEL_7		2328
#define ID_PLCIO_STATIC_LABEL_OFFSET 2329

#define ID_PLCIO_STATIC_LABEL_MEL_STAT	2330
#define ID_PLCIO_STATIC_LABEL_MEL_ERR	2331
#define ID_PLCIO_STATIC_MEL_STAT		2332
#define ID_PLCIO_STATIC_MEL_ERR			2333

#define PLCIO_CHK_SEL_WI			0
#define PLCIO_CHK_SEL_WO			1
#define PLCIO_CHK_SEL_BI			2
#define PLCIO_CHK_SEL_BO			3



//IO CHECK画面コモンコントロール管理構造体
typedef struct _stIOCheckComObj {

	HWND hwnd_iochk_plusPB;						//+PBのハンドル
	HWND hwnd_iochk_minusPB;					//-PBのハンドル
	HWND hwnd_iochk_resetPB;					//リセットPBのハンドル
	HWND hwnd_iochk_datsetPB;					//リセットPBのハンドル
	HWND hwnd_iochk_decPB;						//10進PBのハンドル
	HWND hwnd_iochk_hexPB;						//16進PBのハンドル

	HWND hwnd_radio_bi;							//diラジオボタンのハンドル
	HWND hwnd_radio_wi;							//aiラジオボタンのハンドル
	HWND hwnd_radio_bo;							//doラジオボタンのハンドル
	HWND hwnd_radio_wo;							//aoラジオボタンのハンドル

	HWND hwnd_chk_forceset;						//強制セットチェックボックス
	HWND hwnd_chk_pause;						//表示更新ポーズチェックボックス

	HWND hwnd_edit_forceset;					//強制セットエディットボックス
	HWND hwnd_edit_offset;						//強制セットエディットボックス

	HWND hwnd_label_addr;						//ラベルのハンドル
	HWND hwnd_label_offset;						//ラベルのハンドル
	HWND hwnd_label_no[PLCIO_IO_DISP_NUM];		//ラベルのハンドル

	HWND hwnd_bi_dat_static[PLCIO_IO_DISP_NUM];	//スタティックテキストのハンドル
	HWND hwnd_wi_dat_static[PLCIO_IO_DISP_NUM];	//スタティックテキストのハンドル
	HWND hwnd_bo_dat_static[PLCIO_IO_DISP_NUM];	//スタティックテキストのハンドル
	HWND hwnd_wo_dat_static[PLCIO_IO_DISP_NUM];	//スタティックテキストのハンドル
	HWND hwnd_bi_addr_static;					//スタティックテキストのハンドル
	HWND hwnd_wi_addr_static;					//スタティックテキストのハンドル
	HWND hwnd_bo_addr_static;					//スタティックテキストのハンドル
	HWND hwnd_wo_addr_static;					//スタティックテキストのハンドル
	HWND hwnd_mel_status_static;				//スタティックテキストのハンドル
	HWND hwnd_mel_err_static;					//スタティックテキストのハンドル
	
	int IO_selected;							//操作選択中ＩＯ
	WORD IO_offset;								//操作選択中ＩＯ
	
	BOOL is_bi_hex;								//16進モード選択
	BOOL is_wi_hex;								//16進モード選択
	BOOL is_bo_hex;								//16進モード選択
	BOOL is_wo_hex;								//16進モード選択

	BOOL is_pause_update;						//表示更新保留フラグ
	BOOL is_forced_out_active;					//強制出力有効フラグ

	WORD bi_addr;
	WORD bo_addr;
	WORD wi_addr;
	WORD wo_addr;


} ST_IOCHECK_COM_OBJ, * LPST_IOCHECK_COM_OBJ;


class CWorkWindow_PLC
{
public:
	CWorkWindow_PLC();
	~CWorkWindow_PLC();

	std::wstring wstr;
	static HWND hWorkWnd;
	static HWND hIOWnd;

	static ST_PLC_DEBUG_PANEL stOpePaneStat;
	static ST_IOCHECK_COM_OBJ stIOCheckObj;

	static int update_all_controls(HWND);
	static int update_IOChk(HWND);

	static HWND open_WorkWnd(HWND hwnd_parent);
	static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);

	HWND open_IO_Wnd(HWND hwnd_parent);
	static LRESULT CALLBACK IOWndProc(HWND, UINT, WPARAM, LPARAM);

	int close_WorkWnd();
};

