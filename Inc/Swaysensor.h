#pragma once

#include <winsock.h>
#include <time.h>

#define SWAY_SENSOR_N_CAM       2
#define SWAY_SENSOR_CAM1        0
#define SWAY_SENSOR_CAM2        1

#define SWAY_SENSOR_N_TARGET    2
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1

#define SWAY_SENSOR__MSG_SEND_COM         10600

typedef struct SwayComRcvHead { //振れセンサ受信メッセージヘッダ部
    char	id[4];			//PC ID
    SYSTEMTIME time;		//タイムスタンプ
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SyayCamSpec {
    INT32	pix_x;			//カメラ画素数x軸
    INT32	pix_y;			//カメラ画素数y軸
    INT32	pixlrad_x;	    //カメラ分解能　PIX/rad
    INT32	pixlrad_y;	    //カメラ分解能　PIX/rad
    INT32	l0_x;			//カメラ取付パラメータ㎜
    INT32	l0_y;			//カメラ取付パラメータ㎜
    INT32	ph_x;			//カメラ取付パラメータx1000000rad
    INT32	ph_y;			//カメラ取付パラメータx1000000rad
}ST_SWAY_CAM_SPEC, * LPST_SWAY_CAM_SPEC;

typedef struct TargetStatus {
    //機器状態情報
    INT32	mode;		    //モード
    INT32	error;		    //エラーステータス
    INT32	status;		    //検出ステータス
    //傾斜計情報  
    INT32	tilt_x;         //傾斜計X
    INT32	tilt_y;         //傾斜計Y
    //振れ検出情報  
    INT32	th_x;		    //振角xPIX
    INT32	th_y;		    //振角yPIX
    INT32	dth_x;		    //振角速度x　PIX/s
    INT32	dth_y;			//振角速度y　PIX/s
    INT32	th_x0;			//振角0点xPIX
    INT32	th_y0;			//振角0点yPIX
    INT32	dpx_tgs;		//ターゲット間距離X方向
    INT32	dpy_tgs;		//ターゲット間距離Y方向
    INT32	tg_size;		//ターゲットサイズ
} ST_TARGET_STAT, * LPST_TARGET_STAT;

typedef struct SwayComRcvBody { //振れセンサ受信メッセージボディ部
    ST_SWAY_CAM_SPEC cam_spec;
    ST_TARGET_STAT tg_stat[SWAY_SENSOR_N_TARGET];           //カメラ２ｘターゲット２
    char info[32];
}ST_SWAY_RCV_BODY, * LPST_SWAY_RCV_BODY;

typedef struct SwayComRcvMsg { //振れセンサ受信メッセージ
    ST_SWAY_RCV_HEAD head;
    ST_SWAY_RCV_BODY body[SWAY_SENSOR_N_CAM];
}ST_SWAY_RCV_MSG, * LPST_SWAY_RCV_MSG;


typedef struct SwayComSndHead { //振れセンサ送信メッセージヘッダ部
    char	id[4];			    //機器個体情報
    sockaddr_in sockaddr;       //送信元IPアドレス
}ST_SWAY_SND_HEAD, * LPST_SWAY_SND_HEAD;

typedef struct SwayComSndBody { //振れセンサ送信メッセージボディ部
    INT32 command;
    INT32 mode;
    INT32 freq;                         //最小受信周期       
    INT32 d[SWAY_SENSOR_N_CAM];         //カメラ-ターゲット間距離
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //振れセンサ受信メッセージ
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;

//送信データコード
#define SW_SND_COM_ONCE_DATA        0x00000000
#define SW_SND_COM_CONST_DATA       0x00000001
#define SW_SND_COM_STOP_DATA        0x00000008
#define SW_SND_COM_CAMERA1_0SET     0x00000010
#define SW_SND_COM_CAMERA2_0SET     0x00000020
#define SW_SND_COM_TILT1_0SET       0x00000040
#define SW_SND_COM_TILT2_0SET       0x00000080
#define SW_SND_COM_CAMERAR1_RESET   0x00000100
#define SW_SND_COM_CAMERAR2_RESET   0x00000200
#define SW_SND_COM_TILT1_RESET      0x00000400
#define SW_SND_COM_TILT2_RESET      0x00000800
#define SW_SND_COM_SAVE_IMG         0x00001000
#define SW_SND_COM_PC_RESET         0x10000000

#define SW_SND_DEFAULT_SCAN         100         //100msec

#define SW_SND_MODE_NORMAL          0x000000001         //100msec


//受信データコード
//エラー状態
#define SW_RCV_ERR_CODE_CAMERA      0x00000001
#define SW_RCV_ERR_CODE_TILT        0x00000002
#define SW_RCV_ERR_CODE_DETECT_TG1  0x00000010
#define SW_RCV_ERR_CODE_DETECT_TG2  0x00000020

//検出状態
#define SW_RCV_STAT_TG1_DETECTED    0x00000001
#define SW_RCV_STAT_TG2_DETECTED    0x00000002
#define SW_RCV_STAT_INIT_SENSOR     0x00000010
#define SW_RCV_STAT_INIT_TILT       0x00000020
#define SW_RCV_STAT_0SET_SENSOR     0x00000010
#define SW_RCV_STAT_0SET_TILT       0x00000020

//メッセージテキスト
//エラー状態
#define SW_TXT_ERR_CODE_CAMERA      L"カメラ異常"
#define SW_TXT_ERR_CODE_TILT        L"傾斜計異常"
#define SW_TXT_ERR_CODE_DETECT_TG1  L"ターゲット１検出異常"
#define SW_TXT_ERR_CODE_DETECT_TG2  L"ターゲット２検出異常"

//検出状態
#define SW_TXT_STAT_TG1_DETECTED    L"ターゲット１検出中"
#define SW_TXT_STAT_TG2_DETECTED    L"ターゲット２検出中"
#define SW_TXT_STAT_INIT_SENSOR     L"センサ準備中"
#define SW_TXT_STAT_INIT_TILT       L"傾斜計準備中"
#define SW_TXT_STAT_0SET_SENSOR     L"センサ０セット中"
#define SW_TXT_STAT_0SET_TILT       L"傾斜計０セット中"