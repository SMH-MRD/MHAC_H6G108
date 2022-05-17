#pragma once

#include "resource.h"

///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec

//-Main Windowの配置設定
#define MAIN_WND_INIT_SIZE_W		400		//-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		200		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			680		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			450		//-Main Windowの初期位置設定　Y

//-ID定義 Mainスレッド用　WM_USER + 1000 + 100 +α
#define ID_STATUS					WM_USER + 1201

///# タスク起動管理用構造体
//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec
#define MAX_APP_TASK				8		//タスクオブジェクトスレッド最大数
#define INITIAL_TASK_STACK_SIZE		16384	//タスクオブジェクトスレッド用スタックサイズ

typedef struct stKnlManageSetTag {
	WORD mmt_resolution = TARGET_RESOLUTION;			//マルチメディアタイマーの分解能
	unsigned int cycle_base = SYSTEM_TICK_ms;			//マルチメディアタイマーの分解能
	WORD KnlTick_TimerID = 0;							//マルチメディアタイマーのID
	unsigned int num_of_task = 0;						//アプリケーションで利用するスレッド数
	unsigned long sys_counter = 0;						//マルチメディア起動タイマカウンタ
	SYSTEMTIME Knl_Time = { 0,0,0,0,0,0,0,0 };			//アプリケーション開始からの経過時間
	unsigned int stackSize = INITIAL_TASK_STACK_SIZE;	//タスクの初期スタックサイズ
}ST_KNL_MANAGE_SET, * PSTT_KNL_MANAGE_SET;