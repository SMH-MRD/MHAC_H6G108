#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス

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

    LPST_CRANE_STATUS pCrane;
    LPST_PLC_IO pPLC;
    LPST_AGENT_INFO pAgent;

public:
    CSIM();
    ~CSIM();

    WORD helthy_cnt = 0;

    //オーバーライド
    int set_outbuf(LPVOID); //出力バッファセット
    int init_proc();        //初期化処理
    int input();            //入力処理
    int parse();            //メイン処理
    int output();           //出力処理

    //追加メソッド
 
    void set_mode(int id) {
        if (id) mode |= SIM_ACTIVE_MODE;
        else    mode &= ~SIM_ACTIVE_MODE;
    }

    int is_act_mode() { return(mode & SIM_ACTIVE_MODE); }

};

