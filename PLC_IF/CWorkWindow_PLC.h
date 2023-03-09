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
#define IO_WND_W							480		    //WindowWINDOW幅
#define IO_WND_H							320			//WindowWINDOW高さ

#define WORK_SCAN_TIME						200			// 表示更新周期msec


//コントロールID
#define ID_WORK_WND_BASE                    10600
#define ID_WORK_WND_CLOSE_PB				ID_WORK_WND_BASE+1

//IO CHECK 表示更新タイマーID
#define ID_IO_CHK_UPDATE_TIMER				10700
#define ID_WORK_UPDATE_TIMER				10701

//表示更新周期(msec)
#define IO_CHK_TIMER_PRIOD        			100


//起動タイマーID
#define ID_WORK_WND_TIMER					100

//操作卓パネル管理構造体
#define SLW_SLIDAR_0_NOTCH					5
#define SLW_SLIDAR_MAX						10
#define SLW_SLIDAR_MIN						0
#define BH_SLIDAR_0_NOTCH					5
#define BH_SLIDAR_MAX						10
#define BH_SLIDAR_MIN						0
#define MH_SLIDAR_0_NOTCH					5
#define MH_SLIDAR_MAX						10
#define MH_SLIDAR_MIN						0
#define GT_SLIDAR_0_NOTCH					5
#define GT_SLIDAR_MAX						10
#define GT_SLIDAR_MIN						0

#define LABEL_BUF_SIZE						32

#define PLC_IF_PB_HOLD 						100

#define SET_ON								5
#define SET_OFF								0

typedef struct stPLCDbugPanelTag {

	HDC hdc_mem_gr = NULL;					    //グラフィック部メモリデバイスコンテキスト


	int slider_slew = 0;							// #define IDC_SLIDER_SLEW                 1000
	int check_estop = 0;							// #define IDC_CHECK_ESTOP                 1001
	int slider_bh = 0;								// #define IDC_SLIDER_BH                   1002
													// #define IDC_SPIN_SLEW                   1003
													// #define IDC_SLIDER_BH                   1002
													// #define IDC_SPIN_SLEW                   1003
	WCHAR static_slew_label[LABEL_BUF_SIZE] = L"";	// #define IDC_STATIC_SLEW_LABEL           1004
	WCHAR static_bh_label[LABEL_BUF_SIZE] = L"";	// #define IDC_STATIC_BH_LABEL             1005
													// #define IDC_SPIN_BH                     1006
	int button_slew_0 = 0;							// #define IDC_BUTTON_SLEW_0               1007
	int button_bh_0 = 0;							// #define IDC_BUTTON_BH_0                 1008
	int slider_gt = 0;								// #define IDC_SLIDER_GT                   1009
	WCHAR static_gt_label[LABEL_BUF_SIZE] = L"";	// #define IDC_STATIC_GT_LABEL             1010
													// #define IDC_SPIN_GT                     1011
	int button_gt_0 = 0;							// #define IDC_BUTTON_GT_0                 1012
	int slider_mh = 0;								// #define IDC_SLIDER_MH                   1013
	WCHAR static_mh_label[LABEL_BUF_SIZE] = L"";	// #define IDC_STATIC_MH_LABEL             1014
													// #define IDC_SPIN_MH                     1015
	int button_mh_0 = 0;							// #define IDC_BUTTON_MH_0                 1016
	int check_source1_on = 0;						// #define IDC_CHECK_SOURCE1_ON            1017
	int check_source1_off = 0;						// #define IDC_CHECK_SOURCE1_OFF           1018
	int check_source2_on = 0;						// #define IDC_CHECK_SOURCE2_ON            1019
	int check_source2_off = 0;						// #define IDC_CHECK_SOURCE2_OFF           1020
													// #define IDC_STATIC_SOURCE1_LABEL        1021
	int button_antisway = 0;						// #define IDC_BUTTON_ANTISWAY             1022
													// #define IDC_STATIC_SOURCE2_LABEL        1023
													// #define IDC_STATIC_MH_LABEL2            1024
	int check_auto_start = 0;						// #define IDC_BUTTON_AUTO_START           1025
	int check_s1 = 0;								// #define IDC_CHECK_S1                    1026
	int button_remote = 0;							// #define IDC_CHECK_RMOTE                 1027
	int button_auto_reset = 0;						// #define IDC_BUTTON_AUTO_RESET           1028
	int check_s2 = 0;								// #define IDC_CHECK_S2                    1029
	int check_s3 = 0;								// #define IDC_CHECK_S3                    1030
	int button_set_z = 0;							// #define IDC_BUTTON_SET_Z                1031
													// #define IDC_STATIC_S1_LAMP              1032
													// #define IDC_STATIC_S2_LAMP              1033
													// #define IDC_STATIC_S3_LAMP              1034
													// #define IDC_STATIC_SET_Z_LAMP           1035
	int check_l1 = 0;								// #define IDC_CHECK_L1                    1036
													// #define IDC_STATIC_L1_LAMP              1037
													// #define IDC_STATIC_L2_LAMP              1038
													// #define IDC_STATIC_L3_LAMP              1039
	int check_l2 = 0;								// #define IDC_CHECK_L2                    1040
	int check_l3 = 0;								// #define IDC_CHECK_L3                    1041
	int button_set_xy = 0;							// #define IDC_BUTTON_SET_XY               1042
													// #define IDC_STATIC_SET_XY_LAMP          1043
													// #define IDC_ANTISWAY_LAMP               1044
													// #define IDC_AUTOSTART_LAMP              1045
													// #define IDC_SEMI_RESET_LAMP             1046
													// #define IDC_ESTOP_LAMP                  1047
													// #define IDC_SOURCE1_LAMP                1048
													// #define IDC_SOURCE2_LAMP                1049
	int button_fault_reset = 0;						// #define IDC_BUTTON_FAULT_RESET          1050
	int chk_PC_ref_spd = 0;							// #define IDC_CHECK_SPD_MODE              1051
	int chk_sim_fb = 0;								// #define IDC_CHECK_SIM_FB                1052
	int chk_input_disable = 0;						// #define IDC_DISABLE_PANEL_INPUT         1053

	int button_auto_mode = 0;						// #define IDC_BUTTON_AUTO_MODE            1054
													// #define IDC_AUTO_MODE_LAMP              1055
													// #define IDC_STATIC_SLEW_LABEL2          1056
													// #define IDC_STATIC_BH_LABEL2            1057
	int button_mh_p1 = 0;							// #define IDC_BUTTON_MH_P1                1058
	int button_mh_p2 = 0;							// #define IDC_BUTTON_MH_P2                1059
	int button_mh_m1 = 0;							// #define IDC_BUTTON_MH_M1                1060
	int button_mh_m2 = 0;							// #define IDC_BUTTON_MH_M2                1061
	int button_sl_p1 = 0;							// #define IDC_BUTTON_SL_P1                1062
	int button_sl_p2 = 0;							// #define IDC_BUTTON_SL_P2                1063
	int button_sl_m1 = 0;							// #define IDC_BUTTON_SL_M1                1064
	int button_sl_m2 = 0;							// #define IDC_BUTTON_SL_M2                1065
	int button_bh_p1 = 0;							// #define IDC_BUTTON_BH_P1                1066
	int button_bh_p2 = 0;							// #define IDC_BUTTON_BH_P2                1067
	int button_bh_m1 = 0;							// #define IDC_BUTTON_BH_M1                1068
	int button_bh_m2 = 0;							// #define IDC_BUTTON_BH_M2                1069
	int button_park = 0;							// #define IDC_BUTTON_PARK                 1070
	int button_grnd = 0;							// #define IDC_BUTTON_GRND                 1071
	int button_pick = 0;							// #define IDC_BUTTON_PICK                 1072
													// #define IDC_PARK_LAMP                   1073
													// #define IDC_GRND_LAMP                   1074
													// #define IDC_PICK_LAMP                   1075
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

#define ID_PLCIO_CHK_PC_CTRL		2262
#define ID_PLCIO_CHK_EMULATE		2263

#define ID_PLCIO_STATIC_MH_NOTCH	2264
#define ID_PLCIO_STATIC_BH_NOTCH	2265
#define ID_PLCIO_STATIC_SLW_NOTCH	2266
#define ID_PLCIO_STATIC_LABEL_NOCH	2267


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
	HWND hwnd_chk_pc_ctrl;						//PC制御ビット強制出力チェックボックス
	HWND hwnd_chk_plc_emulate;					//表示更新ポーズチェックボックス

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

	HWND hwnd_notch_out_lavel;					//スタティックテキストのハンドル
	HWND hwnd_mh_notch_out_static;				//スタティックテキストのハンドル
	HWND hwnd_bh_notch_out_static;				//スタティックテキストのハンドル
	HWND hwnd_slw_notch_out_static;				//スタティックテキストのハンドル
	
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
	static int update_Work(HWND);

	static HWND open_WorkWnd(HWND hwnd_parent);
	static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);

	HWND open_IO_Wnd(HWND hwnd_parent);
	static LRESULT CALLBACK IOWndProc(HWND, UINT, WPARAM, LPARAM);

	int close_WorkWnd();

private:


};

