#pragma once

#include "resource.h"
///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec

//-Main Windowの配置設定
#define MAIN_WND_INIT_SIZE_W		380		//-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		200		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			680		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			20		//-Main Windowの初期位置設定　Y

//-ID定義 Mainスレッド用　2000 + 100 +α
#define ID_STATUS					2100
#define IDC_STATIC_0				2101
#define IDC_PB_EXIT					2102
#define IDC_PB_ACTIVE				2103

//メインウィンドウ管理構造体
typedef struct stMainWndTag {
	HWND hWnd_status_bar;	//ステータスバーのハンドル
	HWND h_static0;			//スタティックオブジェクトのハンドル
	HWND h_pb_exit;			//ボタンオブジェクトのハンドル
	HWND h_pb_debug;		//ボタンオブジェクトのハンドル
}ST_MAIN_WND, * LPST_MAIN_WND;

//SIMULATION管理構造体
#define IDM_SIM_PAUSE		0x0000
#define IDM_SIM_ACTIVE		0x0001
typedef struct stSimCtrlTag {
	DWORD mode = IDM_SIM_ACTIVE;			//モード
}ST_SIM_CTRL, * LPST_SIM_CTRL;

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