#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス


#define SWAY_IF_SWAY_IO_MEM_NG        0x8000
#define SWAY_IF_CRANE_MEM_NG         0x4000
#define SWAY_IF_SIM_MEM_NG           0x2000


class CSwayIF :
    public CBasicControl
{
private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pSwayIOObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
 
    ST_SWAY_IO sway_io_workbuf;   //共有メモリへの出力セット作業用バッファ

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

};

