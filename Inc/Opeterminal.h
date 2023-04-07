#pragma once

#include <winsock.h>
#include <time.h>

#define ID_OTE_EVENT_CODE_CONST             0
#define ID_OTE_EVENT_CODE_REQ_CONNECT       1
#define ID_OTE_EVENT_CODE_STAT_CONNECT      2

#define ID_OTE_CONNECT_CODE_NO_OPERATION    0
#define ID_OTE_CONNECT_CODE_AUTO_STANDBY    1
#define ID_OTE_CONNECT_CODE_MANU_STANDBY    2
#define ID_OTE_CONNECT_CODE_RESERVED        3
#define ID_OTE_CONNECT_CODE_CONNECTED       4

#define ID_PC_CONNECT_CODE_ENABLE           1
#define ID_PC_CONNECT_CODE_DISABLE          0

/******* 操作端末IF 共通メッセージヘッダ部                 ***********/
typedef struct OteComHead { 
    INT32       myid;
    INT32       code;
    SOCKADDR_IN addr;
    INT32       status;
    INT32       nodeid;
}ST_OTE_HEAD, * LPST_OTE_HEAD;

typedef struct MOteSndMsg {
    ST_OTE_HEAD         head;
}ST_MOTE_SND_MSG, * LPST_MOTE_SND_MSG;

/******* 操作端末IF マルチキャスト通信受信メッセージ構造体 ***********/
#define N_CRANE_PC_MAX      32
typedef struct MOteRcvBody {
    UCHAR       pc_enable[N_CRANE_PC_MAX];	//接続可能端末フラグ
    INT32	    n_remote_wait;  //接続待ち遠隔操作卓台数
    INT32	    onbord_seqno;   //機側接続シーケンス番号
    INT32	    remote_seqno;   //遠隔卓接続シーケンス番号
    INT32	    my_seqno;       //自身の接続シーケンス番号
}ST_MOTE_RCV_BODY, * LPST_MOTE_RCV_BODY;

typedef struct MOteRcvMsg {
    ST_OTE_HEAD         head;
    ST_MOTE_RCV_BODY    body;
}ST_MOTE_RCV_MSG, * LPST_MOTE_RCV_MSG;


/******* 操作端末IF ユニチキャスト通信送信メッセージ構造体 ***********/

typedef struct UOteSndBody {
    char        pad_ao[4];           //パディング
    double      pos[7];             //位置FB
    double      v_fb[6];            //速度FB
    double      v_ref[4];           //速度指令
    double      tg_pos1[3];         //目標位置座標1
    double      tg_dist1[3];        //目標までの距離1
    double      tg_pos2[3];         //目標位置座標2
    double      tg_dist2[3];        //目標までの距離2
    double      tg_pos_semi[6][3];  //半自動目標位置座標S1-L3
    char        pad_lamp[4];            //パディング
    UCHAR       lamp[64];           //ランプ表示
    INT16       notch_pos[4];       //ノッチランプ表示
    char        pad_plc[4];         //パディング
    INT16	    plc_data[99];       //PLCモニタリングデータ
}ST_UOTE_SND_BODY, * LPST_OTE_SND_BODY;

typedef struct UOteSndMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_SND_BODY    body;
}ST_UOTE_SND_MSG, * LPST_UOTE_SND_MSG;

/******* 操作端末IF ユニチキャスト通信受信メッセージ構造体 ***********/
typedef struct UOteRcvBody {
    char        pad_ao[4];          //パディング
    double      tg_pos1[3];         //目標位置座標1
    double      tg_dist1[3];        //目標までの距離1
    double      tg_pos2[3];         //目標位置座標2
    double      tg_dist2[3];        //目標までの距離2
    char        pad_pb[4];          //パディング
    UCHAR       pb[64];             //ランプ表示
    INT16       notch_pos[4];       //ノッチランプ表示
}ST_UOTE_RCV_BODY, * LPST_UOTE_RCV_BODY;

typedef struct UOteRcvdMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_RCV_BODY    body;
}ST_UOTE_RCV_MSG, * LPST_UOTE_RCV_MSG;

