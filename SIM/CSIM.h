#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "CMob.h"

#define SIM_IF_PLC_IO_MEM_NG        0x8000
#define SIM_IF_CRANE_MEM_NG         0x4000
#define SIM_IF_SIM_MEM_NG           0x2000
#define SIM_IF_AGENT_MEM_NG         0x1000
#define SIM_IF_SWAY_MEM_NG          0x0800

class CSIM :
    public CBasicControl
{

private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pSimulationStatusObj;
 
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pPLCioObj;
    CSharedMem* pAgentInfObj;
 
    ST_SIMULATION_STATUS sim_stat_workbuf;   //共有メモリへの出力セット作業用バッファ

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC;
    LPST_AGENT_INFO pAgent;

    double dt;//スキャン時間

    CCrane* pCrane; //クレーンのモデル
    CLoad* pLoad;   //吊荷のモデル

  public:
    CSIM();
    ~CSIM();

    //オーバーライド
    int set_outbuf(LPVOID); //出力バッファセット
    int init_proc();        //初期化処理
    int input();            //入力処理
    int parse();            //メイン処理
    int output();           //出力処理
    void set_dt(double _dt) { dt = _dt; return; }   //起動間隔セット（MM Timerの関数で呼び出し時にセット）
    
    LPST_SIMULATION_STATUS pSIM_work;

    //追加メソッド
 
    void set_mode(int id) {
        if (id) mode |= SIM_ACTIVE_MODE;
        else    mode &= ~SIM_ACTIVE_MODE;
    }

    int is_sim_active_mode() { return(mode & SIM_ACTIVE_MODE); }

private:
   
    double SwayCamParam[SWAY_SENSOR_N_AXIS][CAM_SET_PARAM_N_PARAM];
    
    int set_cran_motion();
    int set_sway_io();

};

