#pragma once

#include "resource.h"
#include "CTaskObj.h"
#include "CEnvironment.h"
#include "CClientService.h"
#include "CPolicy.h"
#include "CAgent.h"
#include "CSCADA.h"

///# ベース設定

//-タスク設定
#define TARGET_RESOLUTION			1		//マルチメディアタイマー分解能 msec
#define SYSTEM_TICK_ms				25		//メインスレッド周期 msec
#define MAX_APP_TASK				8		//タスクオブジェクトスレッド最大数
#define INITIAL_TASK_STACK_SIZE		16384	//タスクオブジェクトスレッド用スタックサイズ
#define TASK_NUM					5		//このアプリケーションのタスク数

//タスクスキャンタイムmsec
#define AGENT_SCAN_MS				50
#define ENV_SCAN_MS					50
#define POLICY_SCAN_MS				50
#define SCADA_SCAN_MS				200
#define CS_SCAN_MS					200

//-Main Windowの
#define MAIN_WND_INIT_SIZE_W		620		//-Main Windowの初期サイズ　W
#define MAIN_WND_INIT_SIZE_H		420		//-Main Windowの初期サイズ　H
#define MAIN_WND_INIT_POS_X			20		//-Main Windowの初期位置設定　X
#define MAIN_WND_INIT_POS_Y			20		//-Main Windowの初期位置設定　Y

//-Tweet Message表示用Static Windowのサイズ設定値
#define MSG_WND_H					20		//Height of tweet window
#define MSG_WND_W					590		//Width of  tweet window
#define MSG_WND_ORG_X				35		//Origin X of tweet window position 
#define MSG_WND_ORG_Y				10		//Origin Y of tweet window position 
#define MSG_WND_Y_SPACE				12		//Space between tweet windows Y direction 
#define TWEET_IMG_ORG_X				5		//Origin X of bitmap tweet icon position

//-各タスクアイコン用イメージリスト設定値
#define ICON_IMG_W					32		//アイコンイメージ幅
#define ICON_IMG_H					32		//アイコンイメージ高さ

//-タスク設定タブ配置設定
#define TAB_DIALOG_W				620
#define TAB_DIALOG_H				300
#define TAB_POS_X					10
#define TAB_POS_Y					MSG_WND_ORG_Y+(MSG_WND_H+MSG_WND_Y_SPACE)*TASK_NUM+10
#define TAB_SIZE_H					35

//-ID定義 Mainスレッド用　WM_USER + 1000 + α
#define ID_STATUS					WM_USER + 1001
#define ID_STATIC_MAIN				WM_USER + 1002
#define ID_TASK_SET_TAB				WM_USER + 1003


//-.ini 関連設定


//#define PATH_OF_INIFILE					L"./app.ini"	//オプショナルパス

#define NAME_OF_INIFILE					L"app"			//iniファイルファイル名
#define EXT_OF_INIFILE					L"ini"			//iniファイル拡張子
#define PATH_OF_INIFILE					pszInifile		//iniファイルパス


#define OPATH_SECT_OF_INIFILE			L"PATH"			//オプショナルパスセクション
#define OBJ_NAME_SECT_OF_INIFILE		L"OBJECT_NAME"	//タスクオブジェクトフルネームセクション
#define OBJ_SNAME_SECT_OF_INIFILE		L"OBJECT_SNAME"	//タスクオブジェクト略称セクション
#define NETWORK_SECT_OF_INIFILE			L"NW_PARAM"		//Communicatorタスクセクション
#define SMEM_SECT_OF_INIFILE			L"SMEM"			//共有メモリセクション

//各セクションの各タスク用の情報を参照するためのキー
#define MAIN_KEY_OF_INIFILE				L"MAIN"
#define CLIENT_SERVICE_KEY_OF_INIFILE	L"CLIENT"
#define SCADA_KEY_OF_INIFILE			L"SCADA"
#define ENVIRONMENT_KEY_OF_INIFILE		L"ENV"
#define POLICY_KEY_OF_INIFILE			L"POLICY"
#define AGENT_KEY_OF_INIFILE			L"AGENT"
#define DEFAULT_KEY_OF_INIFILE			L"DEFAULT"
#define HELPER_KEY_OF_INIFILE			L"HELPER"
#define DUMMY_KEY_OF_INIFILE			L"DUMMY"

///# タスクスレッド起動管理用構造体
typedef struct stKnlManageSetTag {
	WORD mmt_resolution = TARGET_RESOLUTION;			//マルチメディアタイマーの分解能
	unsigned int cycle_base = SYSTEM_TICK_ms;			//マルチメディアタイマーの分解能
	WORD KnlTick_TimerID = 0;							//マルチメディアタイマーのID
	unsigned int num_of_task = 0;						//アプリケーションで利用するスレッド数
	unsigned long sys_counter = 0;						//マルチメディア起動タイマカウンタ
	SYSTEMTIME Knl_Time = {0,0,0,0,0,0,0,0};			//アプリケーション開始からの経過時間
	unsigned int stackSize = INITIAL_TASK_STACK_SIZE;	//タスクの初期スタックサイズ
}ST_KNL_MANAGE_SET, * LPST_KNL_MANAGE_SET;
