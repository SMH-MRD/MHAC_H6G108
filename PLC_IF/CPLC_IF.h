#pragma once
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス

#define PLC_IF_PLC_IO_MEM_NG        0x8000
#define PLC_IF_CRANE_MEM_NG         0x4000
#define PLC_IF_SIM_MEM_NG           0x2000
#define PLC_IF_AGENT_MEM_NG          0x1000


//PLC IOインターフェースのデータサイズ
//PLC LINKデバイスの割付マップサイズ
#define PLC_IF_MAIN_X_BUFSIZE       14
#define PLC_IF_MAIN_Y_BUFSIZE       6
#define PLC_IF_GNT_X_BUFSIZE        8
#define PLC_IF_GNT_Y_BUFSIZE        2
#define PLC_IF_OPE_X_BUFSIZE        10
#define PLC_IF_OPE_Y_BUFSIZE        3
#define PLC_IF_CC_X_BUFSIZE         12
#define PLC_IF_CC_Y_BUFSIZE         12
#define PLC_IF_CC_W_BUFSIZE         43
#define PLC_IF_ABS_DW_BUFSIZE       8   //WORD数　DWORDx2
#define PLC_IF_SENS_W_BUFSIZE       5

#define PLC_IF_READ_B_BUFSIZE       32


typedef struct st_PLCreadB_tag {                //今回未使用
    short dummy_buf[PLC_IF_READ_B_BUFSIZE];     
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

#define PLC_IF_PC_B_WRITE_COMSIZE         16    //PCコマンド出力部サイズ
#define PLC_IF_PC_B_WRITE_SIMSIZE         4     //PCシミュレーション出力部サイズ
#define PLC_IF_PC_W_WRITE_COMSIZE         16    //PCコマンド出力部サイズ
#define PLC_IF_PC_W_WRITE_SIMSIZE         PLC_IF_CC_W_BUFSIZE + PLC_IF_ABS_DW_BUFSIZE + PLC_IF_SENS_W_BUFSIZE //PCシミュレーション出力部サイズ

typedef struct st_PLCwriteB_tag {
    short pc_com_buf[PLC_IF_PC_B_WRITE_COMSIZE];
    short pc_sim_buf[PLC_IF_PC_B_WRITE_SIMSIZE];
}ST_PLC_WRITE_B, * LPST_PLC_WRITE_B;

typedef struct st_PLCwriteW_tag {
    short pc_com_buf[PLC_IF_PC_W_WRITE_COMSIZE];
    short pc_sim_buf[PLC_IF_PC_W_WRITE_SIMSIZE];
}ST_PLC_WRITE_W, * LPST_PLC_WRITE_W;

#define MELSEC_NET_CH               51      //MELSECNET/HボードのチャネルNo.
#define MELSEC_NET_NW_NO            2       //MELSECNET/Hネットワーク番号
#define MELSEC_NET_MY_NW_NO         0       //MELSECNET/Hボード  自NW指定 0（ボード設定値とは異なる）
#define MELSEC_NET_MY_STATION       255     //MELSECNET/Hボード」自局番 255（ボード設定値とは異なる）
#define MELSEC_NET_SOURCE_STATION   1       //PLC局番
#define MELSEC_NET_B_WRITE_START    0x0A00  //書き込み開始アドレス（PCボードはLBのアドレス指定）
#define MELSEC_NET_W_WRITE_START    0x0A00  //書き込み開始アドレス（PCボードはLWのアドレス指定）
#define MELSEC_NET_B_READ_START     0x0000  //読み込み開始アドレス（PLC MAPしたBのアドレス指定）
#define MELSEC_NET_W_READ_START     0x0000  //読み込み開始アドレス（PLC MAPしたWのアドレス指定）

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

typedef struct st_MelsecNet_tag {
    short chan;                 //通信回線のチャネルNo.
    short mode;                 //ダミー
    long  path;                 //オープンされた回線のパス　回線クローズ時に必要
    long err;                  //エラーコード
    short status;               //回線の状態　0:回線未確立　0より上：正常　0より下：異常
    short retry_cnt;            //回線オープンリトライカウント マルチメディアタイマ周期の倍数が時間間隔
    
    long write_size_w;          //LW書き込みサイズ
    long write_size_b;          //LB書き込みサイズ
    ST_PLC_WRITE_B plc_w_buf_B; //PLC出力元バッファ
    ST_PLC_WRITE_W plc_w_buf_W; //PLC出力元バッファ

    long read_size_w;           //LW読み込みサイズ
    long read_size_b;           //LB読み込みサイズ
    ST_PLC_READ_B  plc_r_buf_B; //PLC入力バッファ
    ST_PLC_READ_W  plc_r_buf_W; //PLC入力バッファ

}ST_MELSEC_NET, * LPST_MELSEC_NET;


class CPLC_IF :    public CBasicControl
{

private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pPLCioObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
    CSharedMem* pAgentInfObj;

    ST_MELSEC_NET   melnet;
    ST_PLC_IO plc_io_workbuf;   //共有メモリへの出力セット作業用バッファ

    LPST_SIMULATION_STATUS pSim;    //シミュレータステータス
    LPST_CRANE_STATUS pCrane;

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
    int set_debug_status(LPST_PLC_IO pworkbuf); //デバッグモード時にデバッグパネルウィンドウからの入力で出力内容を上書き
    int set_sim_status(LPST_PLC_IO pworkbuf);   //デバッグモード時にSimulatorからの入力で出力内容を上書き
    int closeIF();
    
    void set_debug_mode(int id) {
        if (id) mode |= PLC_IF_PC_DBG_MODE;
        else    mode &= ~PLC_IF_PC_DBG_MODE;
    }

    int is_debug_mode() { return(mode & PLC_IF_PC_DBG_MODE); }
};
