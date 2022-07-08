#pragma once

#include "resource.h"

#include "CSHAREDMEM.H"
#include "COMMON_DEF.H"
#include "CVector3.h"
#include "Spec.h"

///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				50		//メインスレッド周期 msec

//-Main Windowの配置設定
#define MAIN_WND_INIT_SIZE_W		1030    //-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		480		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			20		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			585		//-Main Windowの初期位置設定　Y

//表示更新タイマーID
#define ID_UPDATE_TIMER				100

//表示更新周期(msec)
#define TIMER_PRIOD        			200

#define ID_STATUS					WM_USER + 600

