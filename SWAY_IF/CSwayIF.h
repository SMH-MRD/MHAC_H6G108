#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "Spec.h"
#include "Swaysensor.h"

#define SWAY_IF_SWAY_IO_MEM_NG      0x8000
#define SWAY_IF_CRANE_MEM_NG        0x4000
#define SWAY_IF_SIM_MEM_NG          0x2000

#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

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
    int i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
    int i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //共有メモリへの出力セット作業用バッファ
    double SwayCamParam[N_SWAY_SENSOR][SWAY_SENSOR_N_AXIS][CAM_SET_PARAM_N_PARAM]; //振れセンサカメラ設置パラメータ[カメラNo.][軸方向XY][a,b]

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

