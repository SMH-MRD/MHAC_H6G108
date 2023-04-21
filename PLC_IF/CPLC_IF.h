#pragma once
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "CPushButton.h"
#include "PLC_IO_DEF.h"

#define PLC_IF_PLC_IO_MEM_NG        0x8000
#define PLC_IF_CRANE_MEM_NG         0x4000
#define PLC_IF_SIM_MEM_NG           0x2000
#define PLC_IF_AGENT_MEM_NG         0x1000
#define PLC_IF_CS_MEM_NG            0x0800

typedef struct st_PLCreadB_tag {                //今回未使用
    INT16 spare[PLC_IF_SPARE_B_BUFSIZE];
    INT16 PB[PLC_IF_PB_B_BUFSIZE];
    INT16 NA[PLC_IF_NA_B_BUFSIZE];
}ST_PLC_READ_B, * LPST_PLC_READ_B;

typedef struct st_PLCreadW_tag{
    short main_x_buf[PLC_IF_MAIN_X_BUFSIZE];    //MAINPLC X
    short main_y_buf[PLC_IF_MAIN_Y_BUFSIZE];    //MAINPLC Y
    short gnt_x_buf[PLC_IF_GNT_X_BUFSIZE];      //走行PLC X
    short gnt_y_buf[PLC_IF_GNT_Y_BUFSIZE];      //走行PLC Y
    short ope_x_buf[PLC_IF_OPE_X_BUFSIZE];      //運転室PLC X
    short ope_y_buf[PLC_IF_OPE_Y_BUFSIZE];      //運転室PLC Y
    short cc_x_buf[PLC_IF_CC_X_BUFSIZE];        //CC　LINK X
    short cc_y_buf[PLC_IF_CC_Y_BUFSIZE];        //CC　LINK Y
    short padding_buf[2];                       //予備(DI,AI境界
    short cc_w_buf[PLC_IF_CC_W_BUFSIZE];        //CC　LINK W
    short abso_dw_buf[PLC_IF_ABS_DW_BUFSIZE];   //アブソコーダdata
    short sensor_buf[PLC_IF_SENS_W_BUFSIZE];    //その他アナログ信号
}ST_PLC_READ_W, * LPST_PLC_READ_W;

typedef struct st_PLCwriteB_tag {
    INT16 pc_com_buf[PLC_IF_PC_B_WRITE_COMSIZE];
    INT16 pc_sim_buf[PLC_IF_PC_B_WRITE_SIMSIZE];
}ST_PLC_WRITE_B, * LPST_PLC_WRITE_B;

typedef struct st_PLCwriteW_tag {
    INT16 pc_com_buf[PLC_IF_PC_W_WRITE_COMSIZE];
    INT16 pc_sim_buf[PLC_IF_PC_W_WRITE_SIMSIZE];
}ST_PLC_WRITE_W, * LPST_PLC_WRITE_W;

#define MELSEC_NET_CH               51      //MELSECNET/HボードのチャネルNo.
#define MELSEC_NET_NW_NO            2       //MELSECNET/Hネットワーク番号
#define MELSEC_NET_MY_NW_NO         0       //MELSECNET/Hボード  自NW指定 0（ボード設定値とは異なる）
#define MELSEC_NET_MY_STATION       255     //MELSECNET/Hボード」自局番 255（ボード設定値とは異なる）
#define MELSEC_NET_SOURCE_STATION   1       //PLC局番
#define MELSEC_NET_B_WRITE_START    0x0600  //書き込み開始アドレス（PCボードはLBのアドレス指定）
#define MELSEC_NET_W_WRITE_START    0x0600  //書き込み開始アドレス（PCボードはLWのアドレス指定）
#define MELSEC_NET_B_READ_START     0x0900  //読み込み開始アドレス（PLC MAPしたBのアドレス指定）
#define MELSEC_NET_W_READ_START     0x089C  //読み込み開始アドレス（PLC MAPしたWのアドレス指定）

#define MELSEC_NET_OK               1
#define MELSEC_NET_SEND_ERR         -1
#define MELSEC_NET_RECEIVE_ERR      -2
#define MELSEC_NET_CLOSE            0

#define MELSEC_NET_RETRY_CNT        100 //エラー時Retryカウント周期

#define MELSEC_NET_CODE_LW          24  //デバイスコード
#define MELSEC_NET_CODE_LB          23  //デバイスコード
#define MELSEC_NET_CODE_SM          5   //デバイスコード
#define MELSEC_NET_CODE_SB          5   //デバイスコード
#define MELSEC_NET_CODE_SD          14  //デバイスコード
#define MELSEC_NET_CODE_SW          14  //デバイスコード

#define N_PLC_W_OUT_WORD            128   //PLC LINK PLC出力WORD数
#define N_PLC_B_OUT_WORD            16    //PLC LINK PLC出力WORD数
#define N_PC_W_OUT_WORD             80   //PLC LINK PLC出力WORD数
#define N_PC_B_OUT_WORD             16    //PLC LINK PLC出力WORD数

#define MEL_FORCE_PLC_B             0    //IO強制セットID
#define MEL_FORCE_PLC_W             1    //IO強制セットID
#define MEL_FORCE_PC_B              2    //IO強制セットID
#define MEL_FORCE_PC_W              3    //IO強制セットID
#define MEL_FORCE_RESET             4    //IO強制セットID

typedef struct st_MelsecNet_tag {
    short chan=0;                         //通信回線のチャネルNo.
    short mode=0;                         //ダミー
    long  path;                         //オープンされた回線のパス　回線クローズ時に必要
    long err;                           //エラーコード
    short status;                       //回線の状態　0:回線未確立　0より上：正常　0より下：異常
    short retry_cnt;                    //回線オープンリトライカウント マルチメディアタイマ周期の倍数が時間間隔

    long write_size_w;                  //PC LW書き込みサイズ
    long write_size_b;                  //PC LB書き込みサイズ
    INT16 pc_w_out[sizeof(ST_PLC_READ_W)];    //PC書込バッファW
    INT16 pc_b_out[sizeof(ST_PLC_READ_B)];    //PC書込バッファB

    long read_size_w;                   //PLC LW書き込みサイズ
    long read_size_b;                   //PLC LB書き込みサイズ
    INT16 plc_w_out[sizeof(ST_PLC_WRITE_W)];  //PLC書込バッファW
    INT16 plc_b_out[sizeof(ST_PLC_WRITE_B)];  //PLC書込バッファB
  
    ST_PLC_OUT_BMAP plc_b_map;          //PLC LB書き込みバッファMAP情報
    ST_PLC_OUT_WMAP plc_w_map;          //PLC LW書き込みバッファMAP情報
    ST_PC_OUT_BMAP  pc_b_map;           //PC LB書き込みバッファMAP情報
    ST_PC_OUT_WMAP  pc_w_map;           //PC LW書き込みバッファMAP情報

    //BI,WI,BO,WO強制セット
    bool is_force_set_active[4];        //IO強制セットフラグ
    WORD forced_dat[4];                 //強制セット値
    int forced_index[4];                 //強制セット値

    //PC CTRL, PLC emulate強制セット

    BOOL is_forced_pc_ctrl;						//強制出力有効フラグ
    BOOL is_forced_emulate;						//強制出力有効フラグ

}ST_MELSEC_NET, * LPST_MELSEC_NET;

class CPLC_IF :    public CBasicControl
{
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
    int set_sim_status();   //デバッグモード時にSimulatorからの入力で出力内容を上書き
    int closeIF();
 
    ST_MELSEC_NET   melnet;

    LPST_MELSEC_NET get_melnet() { return &melnet; }

    void set_debug_mode(int id) {
        if (id) mode |= PLC_IF_PC_DBG_MODE;
        else    mode &= ~PLC_IF_PC_DBG_MODE;
        return;
    }

    void set_pc_ctrl_forced(bool b) {
        if (b) melnet.is_forced_pc_ctrl = true;
        else melnet.is_forced_pc_ctrl = false;
        return;
    }
    void set_plc_emu_forced(bool b) {
        if (b) melnet.is_forced_emulate = true;
        else melnet.is_forced_emulate = false;
        return;
    }

    int is_debug_mode() { return(mode & PLC_IF_PC_DBG_MODE); }
    int mel_set_force(int id, bool bset, int index, WORD value);

private:
    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pPLCioObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
    CSharedMem* pAgentInfObj;
    CSharedMem* pCSInfObj;

 
    ST_PLC_IO plc_io_workbuf;   //共有メモリへの出力セット作業用バッファ

    LPST_SIMULATION_STATUS pSim;    //シミュレータステータス
    LPST_CRANE_STATUS pCrane;
    LPST_AGENT_INFO pAgentInf;
    LPST_CS_INFO pCSInf;

    int parse_notch_com();
    int parce_brk_status();
    int parse_ope_com();
    int parse_sensor_fb();
    int set_notch_ref();
    int set_bit_coms();
    int set_ao_coms();
 };
