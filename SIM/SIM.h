#pragma once

#include "resource.h"
///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec

//-Main Windowの配置設定
#define MAIN_WND_INIT_SIZE_W		620		//-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		420		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			680		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			20		//-Main Windowの初期位置設定　Y

//-ID定義 Mainスレッド用　WM_USER + 1000 + 100 +α
#define ID_STATUS					WM_USER + 1101