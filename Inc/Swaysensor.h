#pragma once

#include <winsock.h>
#include <time.h>

#define SWAY_SENSOR_N_CAM       2
#define SWAY_SENSOR_CAM1        0
#define SWAY_SENSOR_CAM2        1
#define SWAY_SENSOR_TIL_X       0
#define SWAY_SENSOR_TIL_Y       1
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_N_TARGET    2
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_TG3         2
#define SWAY_SENSOR_TG4         3

typedef struct TargetStatus {
    char	mode[4];		//モード
    char	status[4];		//ステータス
    char	error[4];		//ステータス
} ST_TARGET_STAT, *LPST_TARGET_STAT;

typedef struct SyaCamSetting {
    INT32	pix_x;			//カメラ画素数x軸
    INT32	pix_y;			//カメラ画素数y軸
    INT32	pixlrad_x;	    //カメラ分解能　PIX/rad
    INT32	pixlrad_y;	    //カメラ分解能　PIX/rad
    INT32	l0_x;			//カメラ取付パラメータ㎜
    INT32	l0_y;			//カメラ取付パラメータ㎜
    INT32	ph_x;			//カメラ取付パラメータx1000000rad
    INT32	ph_y;			//カメラ取付パラメータx1000000rad
}ST_SWAY_CAM_SETTING, * LPST_SWAY_CAM_SETTING;
typedef struct SwayComRcvHead { //振れセンサ受信メッセージヘッダ部
    char	id[4];			                                            //PC ID
    ST_SWAY_CAM_SETTING cam_setting[SWAY_SENSOR_N_CAM];                 //カメラ配置情報
    SYSTEMTIME time;		                                            //タイムスタンプ
    ST_TARGET_STAT tg_stat[SWAY_SENSOR_N_CAM][SWAY_SENSOR_N_TARGET];    //カメラ２ｘターゲット２
    INT32	tilt[SWAY_SENSOR_N_CAM][2];			                        //カメラ傾斜角x　x1000000rad
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SwayComMainData { //振れセンサ受信メッセージデータ構成部
    INT32	th_x;			//振角xPIX
    INT32	th_y;			//振角yPIX
    INT32	dth_x;			//振角速度x　PIX/s
    INT32	dth_y;			//振角速度y　PIX/s
    INT32	th_x0;			//振角0点xPIX
    INT32	th_y0;			//振角0点yPIX
    INT32	dpx_tgs;		//ターゲット間距離X方向
    INT32	dpy_tgs;		//ターゲット間距離Y方向
    INT32	tg_size;		//ターゲットサイズ
}ST_SWAY_MAIN_DATA, * LPST_SWAY_MAIN_DATA;

typedef struct SwayComRcvBody { //振れセンサ受信メッセージボディ部
    ST_SWAY_MAIN_DATA data[SWAY_SENSOR_N_CAM][SWAY_SENSOR_N_TARGET];
    char info[SWAY_SENSOR_N_CAM][32];
}ST_SWAY_RCV_BODY, * LPST_SWAY_RCV_BODY;



typedef struct SwayComRcvMsg { //振れセンサ受信メッセージ
    ST_SWAY_RCV_HEAD head;
    ST_SWAY_RCV_BODY body;
}ST_SWAY_RCV_MSG, * LPST_SWAY_RCV_MSG;

typedef struct SwayComSndHead { //振れセンサ送信メッセージヘッダ部
    char	id[4];			//機器個体情報
    sockaddr_in sockaddr;       //送信元IPアドレス
}ST_SWAY_SND_HEAD, * LPST_SWAY_SND_HEAD;

typedef struct SwayComSndBody { //振れセンサ送信メッセージボディ部
    char command[2];
    char mode[40];
    INT16 freq;    //最小受信周期       
    INT32 d;       //カメラ-ターゲット間距離
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //振れセンサ受信メッセージ
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;
