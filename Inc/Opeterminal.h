#pragma once

#include <winsock.h>
#include <time.h>

#define ID_OTE_EVENT_CODE_CONST             0
#define ID_OTE_EVENT_CODE_STAT_REPORT       1
#define ID_OTE_EVENT_CODE_CONNECTED         1

#define ID_TE_CONNECT_STATUS_OFF_LINE       0
#define ID_TE_CONNECT_STATUS_STANDBY        3
#define ID_TE_CONNECT_STATUS_ACTIVE         1
#define ID_TE_CONNECT_STATUS_WAITING        2

#define ID_PC_CONNECT_CODE_ENABLE           1
#define ID_PC_CONNECT_CODE_DISABLE          0

/******* 操作端末IF 共通メッセージヘッダ部                 ***********/
typedef struct OteComHead { 
    INT32       myid;
    INT32       code;
    SOCKADDR_IN addr;
    INT32       status;
    INT32       tgid;
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

#define PLC_IO_MONT_WORD_NUM		112
/******* 操作端末IF ユニチキャスト通信送信メッセージ構造体 ***********/

typedef struct UOteSndBody {
    INT32      pos[8];                         //位置FB
    INT32      v_fb[8];                        //速度FB
    INT32      v_ref[8];                       //速度指令
    INT32      ld_pos[4];                      //吊荷位置FB
    INT32      ld_v_fb[4];                     //吊荷速度FB
    INT32      tg_pos[4];                      //目標位置座標1
    INT32      tg_pos_semi[6][4];              //半自動目標位置座標S1-L3
    INT16      lamp[64];                       //ランプ表示
    INT16      notch_pos[8];                   //ノッチランプ表示
    INT16	   plc_data[PLC_IO_MONT_WORD_NUM]; //PLCモニタリングデータ
}ST_UOTE_SND_BODY, * LPST_OTE_SND_BODY;

typedef struct UOteSndMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_SND_BODY    body;
}ST_UOTE_SND_MSG, * LPST_UOTE_SND_MSG;

/******* 操作端末IF ユニチキャスト通信受信メッセージ構造体 ***********/
typedef struct UOteRcvBody {
    INT32     tg_pos1[4];         //目標位置座標1
    INT32     tg_dist1[4];        //目標までの距離1
    INT32     tg_pos2[4];         //目標位置座標2
    INT32     tg_dist2[4];        //目標までの距離2
    INT16     pb[64];             //ランプ表示
    INT16     notch_pos[8];       //ノッチ入力位置
}ST_UOTE_RCV_BODY, * LPST_UOTE_RCV_BODY;

typedef struct UOteRcvdMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_RCV_BODY    body;
}ST_UOTE_RCV_MSG, * LPST_UOTE_RCV_MSG;

