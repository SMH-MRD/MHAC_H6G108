#pragma once

#include <winsock.h>

#define SWAY_SENSOR_N_TARGET    4
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_TG3         2
#define SWAY_SENSOR_TG4         3

typedef struct SwayComRcvHead { //振れセンサ受信メッセージヘッダ部
    char	id[4];			//機器個体情報
    UINT16	pix_x;			//カメラ画素数x軸
    UINT16	pix_y;			//カメラ画素数y軸
    UINT16	pixlrad_x;	    //カメラ分解能　PIX/rad
    UINT16	pixlrad_y;	    //カメラ分解能　PIX/rad
    UINT16	l0_x;			//カメラ取付パラメータ㎜
    UINT16	l0_y;			//カメラ取付パラメータ㎜
    UINT16	ph_x;			//カメラ取付パラメータx1000000rad
    UINT16	ph_y;			//カメラ取付パラメータx1000000rad
    TIMEVAL time;			//タイムスタンプ
    char	mode[16];		//モード
    char	status[16];		//ステータス
    char	error[16];		//ステータス
    UINT32	tilt_x;			//カメラ傾斜角x　x1000000rad
    UINT32	tilt_y;			//カメラ傾斜角y　x1000000rad
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SwayComRcvDATA { //振れセンサ受信メッセージデータ構成部
    UINT32	th_x;			//振角xPIX
    UINT32	th_y;			//振角yPIX
    UINT32	dth_x;			//振角速度x　PIX/s
    UINT32	dth_y;			//振角速度y　PIX/s
    UINT32	th_x0;			//振角0点xPIX
    UINT32	th_y0;			//振角0点yPIX
    UINT16	tg1_size;		//ターゲット１サイズ
    UINT16	tr2_size;		//ターゲット２サイズ
    UINT32	skew;			//スキュー角　PIX
}ST_SWAY_RCV_DATA, * LPST_SWAY_RCV_DATA;

typedef struct SwayComRcvBody { //振れセンサ受信メッセージボディ部
    ST_SWAY_RCV_DATA data[4];
    UINT16 info[4];
}ST_SWAY_RCV_BODY, * LPST_SWAY_RCV_BODY;

typedef struct SwayComRcvMsg { //振れセンサ受信メッセージ
    ST_SWAY_RCV_HEAD head;
    ST_SWAY_RCV_BODY body;
}ST_SWAY_RCV_MSG, * LPST_SWAY_RCV_MSG;

typedef struct SwayComSndHead { //振れセンサ送信メッセージヘッダ部
    char	id[4];			//機器個体情報
    sockaddr_in addr;       //送信元IPアドレス
}ST_SWAY_SND_HEAD, * LPST_SWAY_SND_HEAD;

typedef struct SwayComSndBody { //振れセンサ送信メッセージボディ部
    char command[2];
    char mode[40];
    UINT16 freq;    //最小受信周期       
    UINT32 d;       //カメラ-ターゲット間距離
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //振れセンサ受信メッセージ
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;
