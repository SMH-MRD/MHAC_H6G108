#pragma once

#include "resource.h"
// #Touchタッチポイント
//Windowsタッチ
#ifndef WINVER                  // Specifies that the minimum required platform is Windows 7.
#define WINVER 0x0601           // Change this to the appropriate value to target other versions of Windows.
#endif

#ifndef _WIN32_WINNT            // Specifies that the minimum required platform is Windows 7.
#define _WIN32_WINNT 0x0601     // Change this to the appropriate value to target other versions of Windows.
#endif     

#ifndef _WIN32_WINDOWS          // Specifies that the minimum required platform is Windows 98.
#define _WIN32_WINDOWS 0x0410 // Change this to the appropriate value to target Windows Me or later.
#endif

#ifndef _WIN32_IE                       // Specifies that the minimum required platform is Internet Explorer 7.0.
#define _WIN32_IE 0x0700        // Change this to the appropriate value to target other versions of IE.
#endif

#include <windows.h>    // included for Windows Touch
#include <windowsx.h>   // included for point conversion

#define MAXPOINTS 10

// You will use this array to track touch points
int points[MAXPOINTS][2];

// You will use this array to switch the color / track ids
int idLookup[MAXPOINTS];


// You can make the touch points larger
// by changing this radius value
static int radius = 50;

// There should be at least as many colors
// as there can be touch points so that you
// can have different colors for each point
COLORREF colors[] = { RGB(153,255,51),
					  RGB(153,0,0),
					  RGB(0,153,0),
					  RGB(255,255,0),
					  RGB(255,51,204),
					  RGB(0,0,0),
					  RGB(0,153,0),
					  RGB(153, 255, 255),
					  RGB(153,153,255),
					  RGB(0,51,153)
};

// #Touchタッチポイント

///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec

//タイマーID
#define ID_MAIN_WINDOW_UPDATE_TIMER				10702
#define ID_MAIN_WINDOW_UPDATE_TICK_ms			1000


//-Main Windowの配置設定
#define MAIN_WND_INIT_SIZE_W		380		//-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		195		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			670		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			20		//-Main Windowの初期位置設定　Y

//-ID定義 Mainスレッド用　2000 + 300 +α
#define ID_STATUS					2400
#define IDC_STATIC_0				2401
#define IDC_PB_EXIT					2402
#define IDC_PB_DEBUG				2403
#define IDC_PB_COMWIN				2404
#define IDC_PB_SIM_OTE				2405

#define IDC_STATIC_1				2415
#define IDC_PB_SENSOR_1				2416
#define IDC_PB_SENSOR_2				2417
#define IDC_PB_0SET_CAMERA			2418
#define IDC_PB_0SET_TILT			2419
#define IDC_PB_RESET_CAMERA			2420
#define IDC_PB_RESET_TILT			2421
#define IDC_PB_PC_RESET				2422
#define IDC_PB_SCREEN_SHOT			2424

#define ID_CHECK_SWAY_CAL_NO_OFFSET 2424
#define ID_CHECK_SWAY_CAL_NO_TILT   2425

#define MSG_SENSOR_SOCK				2410	//WinSockイベントメッセージ


//メインウィンドウ管理構造体
typedef struct stMainWndTag {
	HWND hWnd_status_bar = NULL;	//ステータスバーのハンドル
	HWND h_static0 = NULL;			//スタティックオブジェクトのハンドル
	HWND h_pb_exit = NULL;			//ボタンオブジェクトのハンドル
	HWND h_pb_debug = NULL;			//ボタンオブジェクトのハンドル
	HWND h_pb_comwin = NULL;		//ボタンオブジェクトのハンドル

	HWND h_static1 = NULL;			//スタティックオブジェクトのハンドル
	HWND h_pb_sel_sensor1 = NULL;	//ボタンオブジェクトのハンドル
	HWND h_pb_sel_sensor2 = NULL;	//ボタンオブジェクトのハンドル
	HWND h_pb_0set_sensor = NULL;	//ボタンオブジェクトのハンドル
	HWND h_pb_0set_tilt = NULL;		//ボタンオブジェクトのハンドル
	HWND h_pb_reset_sensor = NULL;	//ボタンオブジェクトのハンドル
	HWND h_pb_reset_tilt = NULL;	//ボタンオブジェクトのハンドル
	HWND h_pb_pc_reset = NULL;		//ボタンオブジェクトのハンドル
	HWND h_pb_img_save = NULL;		//ボタンオブジェクトのハンドル


	HWND hWorkWnd = NULL;			//ワークウィンドウのハンドル

}ST_MAIN_WND, * LPST_MAIN_WND;

//マルチメディアタイマー管理構造体
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec
#define INITIAL_TASK_STACK_SIZE		16384	//タスクオブジェクトスレッド用スタックサイズ

typedef struct stKnlManageSetTag {
	WORD mmt_resolution = TARGET_RESOLUTION;			//マルチメディアタイマーの分解能
	unsigned int cycle_base = SYSTEM_TICK_ms;			//マルチメディアタイマーの分解能
	WORD KnlTick_TimerID = 0;							//マルチメディアタイマーのID
	unsigned int num_of_task = 0;						//アプリケーションで利用するスレッド数
	unsigned long sys_counter = 0;						//マルチメディア起動タイマカウンタ
	SYSTEMTIME Knl_Time = { 0,0,0,0,0,0,0,0 };			//アプリケーション開始からの経過時間
	unsigned int stackSize = INITIAL_TASK_STACK_SIZE;	//タスクの初期スタックサイズ
}ST_KNL_MANAGE_SET, * LPST_KNL_MANAGE_SET;