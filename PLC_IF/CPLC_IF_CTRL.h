#pragma once
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス

#define PLC_IF_PLC_IO_MEM_NG        0x8000
#define PLC_IF_CRANE_MEM_NG         0x4000
#define PLC_IF_SIM_MEM_NG           0x2000
#define PLC_IF_EXE_MEM_NG           0x1000


//PLC IOインターフェースのデータサイズ
#define PLC_IF_MAIN_X_BUFSIZE       14
#define PLC_IF_MAIN_Y_BUFSIZE       6
#define PLC_IF_GNT_X_BUFSIZE        8
#define PLC_IF_GNT_Y_BUFSIZE        2
#define PLC_IF_OPE_X_BUFSIZE        10
#define PLC_IF_OPE_Y_BUFSIZE        3
#define PLC_IF_CC_X_BUFSIZE         12
#define PLC_IF_CC_Y_BUFSIZE         12
#define PLC_IF_CC_W_BUFSIZE         43
#define PLC_IF_ABS_DW_BUFSIZE       4
#define PLC_IF_SENS_W_BUFSIZE       5
#define PLC_IF_PC_B_BUFSIZE         16
#define PLC_IF_PC_W_BUFSIZE         32

typedef struct st_PLClink_tag{
    WORD main_x_buf[PLC_IF_MAIN_X_BUFSIZE];
    WORD main_y_buf[PLC_IF_MAIN_Y_BUFSIZE];
    WORD gnt_x_buf[PLC_IF_GNT_X_BUFSIZE];
    WORD gnt_y_buf[PLC_IF_GNT_Y_BUFSIZE];
    WORD ope_x_buf[PLC_IF_OPE_X_BUFSIZE];
    WORD ope_y_buf[PLC_IF_OPE_Y_BUFSIZE];
    WORD cc_x_buf[PLC_IF_CC_X_BUFSIZE];
    WORD cc_y_buf[PLC_IF_CC_Y_BUFSIZE];
    WORD padding_buf[2];
    WORD cc_w_buf[PLC_IF_CC_W_BUFSIZE];
    DWORD abso_dw_buf[PLC_IF_ABS_DW_BUFSIZE];
    WORD sensor_buf[PLC_IF_SENS_W_BUFSIZE];
    WORD pc_b_buf[PLC_IF_PC_B_BUFSIZE];
    WORD pc_w_buf[PLC_IF_PC_W_BUFSIZE];
}ST_PLC_LINK, * LPST_PLC_LINK;


class CPLC_IF :
    public CBasicControl
{

private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pPLCioObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
    CSharedMem* pExecStatusObj;

    ST_PLC_LINK plc_link;       //PLCリンクバッファの内容
    ST_PLC_IO plc_io_workbuf;   //共有メモリへの出力セット作業用バッファ

public:
    CPLC_IF();
    ~CPLC_IF();
 
    WORD helthy_cnt=0;

    //オーバーライド
    int set_outbuf(LPVOID); //出力バッファセット
    int init_proc();        //初期化処理
    int input();            //入力処理
    int parse();            //メイン処理
    int output();           //出力処理

    //追加メソッド
    int set_debug_status(); //デバッグモード時にデバッグパネルウィンドウからの入力で出力内容を上書き
    
    void set_debug_mode(int id) {
        if (id) mode |= PLC_IF_PLC_DBG_MODE;
        else    mode &= ~PLC_IF_PLC_DBG_MODE;
    }

    int is_debug_mode() { return(mode & PLC_IF_PLC_DBG_MODE); }
};
