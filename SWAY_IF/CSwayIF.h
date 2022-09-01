#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "Spec.h"
#include <winsock.h>

#define SWAY_IF_SWAY_IO_MEM_NG      0x8000
#define SWAY_IF_CRANE_MEM_NG        0x4000
#define SWAY_IF_SIM_MEM_NG          0x2000

#define CAM_SET_PARAM_N_CAM         2
#define CAM_SET_PARAM_N_AXIS        2
#define CAM_SET_PARAM_X_AXIS        0
#define CAM_SET_PARAM_Y_AXIS        1
#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

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

#define N_SWAY_SENSOR           2   //振れセンサの数
#define SWAY_SENSOR1            0   //振れセンサID1
#define SWAY_SENSOR2            1   //振れセンサID2
#define N_SWAY_SENSOR_RCV_BUF   10  //受信データのバッファ数
#define N_SWAY_SENSOR_SND_BUF   10  //送信データのバッファ数

class CSwayIF :
    public CBasicControl
{
private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pSwayIOObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;

    ST_SWAY_RCV_MSG rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
    ST_SWAY_SND_MSG snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
    int i_rcv_msg[N_SWAY_SENSOR] = { 0,0 };
    int i_snd_msg[N_SWAY_SENSOR] = { 0,0 };

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //共有メモリへの出力セット作業用バッファ
    double SwayCamParam[CAM_SET_PARAM_N_CAM][CAM_SET_PARAM_N_AXIS][CAM_SET_PARAM_N_PARAM]; //振れセンサカメラ設置パラメータ[カメラNo.][軸方向XY][a,b]

    LPST_SIMULATION_STATUS pSim;    //シミュレータステータス

    int parse_sway_stat(int ID);

public:
    CSwayIF();
    ~CSwayIF();

    WORD helthy_cnt = 0;

    //オーバーライド
    int set_outbuf(LPVOID); //出力バッファセット
    int init_proc();        //初期化処理
    int input();            //入力処理
    int parse();            //メイン処理
    int output();           //出力処理

    void set_debug_mode(int id) {
        if (id) mode |= SWAY_IF_SIM_DBG_MODE;
        else    mode &= ~SWAY_IF_SIM_DBG_MODE;
    }

    int is_debug_mode() { return(mode & SWAY_IF_SIM_DBG_MODE); }



    //追加メソッド
     int set_sim_status(LPST_SWAY_IO pworkbuf);   //デバッグモード時にSimulatorからの入力で出力内容を上書き
};

