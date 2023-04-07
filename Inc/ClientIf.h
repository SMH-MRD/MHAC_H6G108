#pragma once

#include <winsock.h>
#include <time.h>

/*#########################################################################*/
/*      　　         CLIENT　PCとの通信メッセージフォーマット　　　　　　　*/
/*#########################################################################*/

#define CLIENT_CODE_JOB_REQUEST             0x80000001
#define CLIENT_CODE_CONTROL_SOURCE          0x00000001
#define CLIENT_CODE_CONTROL_ON              0x00000003
#define CLIENT_CODE_ANTISWAY_OFF            0x00000004
#define CLIENT_CODE_ANTISWAY_ON             0x0000000C
#define CLIENT_CODE_AUTO_OFF                0x00000010
#define CLIENT_CODE_AUTO_ON                 0x00000030

#define CLIENT_COM_OPT_PB_ON                 0x00000001
#define CLIENT_COM_OPT_PB_OFF                0x00000000
#define CLIENT_COM_OPT_JOB_PICK              0x00000001
#define CLIENT_COM_OPT_JOB_GRND              0x00000002
#define CLIENT_COM_OPT_JOB_PARK              0x00000003


typedef struct ClientComHead {              //端末IFメッセージヘッダ部
    INT32	        myid;		            //PC ID
    INT32	        code;		            //送信内容コード
    SOCKADDR_IN     addr_in;                //ユニキャスト受信IP,ポート
    INT32           option;
    INT32           node_id;                // 接続先ID
    SYSTEMTIME      time;                   // Time Stamp
}ST_CLIENT_COM_HEAD, * LPST_CLIENT_COM_HEAD;

typedef struct ClientComRcvBody {              //端末IFメッセージヘッダ部
    double auto_tg_pos[8];              //自動目標位置
}ST_CLIENT_RCV_BODY, * LPST_CLIENT_RCV_BODY;

typedef struct ClientComRcvMsg {      //振れセンサ受信メッセージ
    ST_CLIENT_COM_HEAD         head;
    ST_CLIENT_RCV_BODY           body;
}ST_CLIENT_COM_RCV_MSG, * LPST_CLIENT_COM_RCV_MSG;

typedef struct ClientComSndMsg {      //振れセンサ受信メッセージ
    ST_CLIENT_COM_HEAD         head;
  }ST_CLIENT_COM_SND_MSG, * LPST_CLIENT_COM_SND_MSG;

#if 0

/*#########################################################################*/
/*      　　         操作端末との通信メッセージフォーマット　　　　　　　　*/
/*#########################################################################*/

typedef struct OteComHead {             //端末IFメッセージヘッダ部
    INT32	        myid;		        //PC ID
    INT32	        code;		        //送信内容コード
    SOCKADDR_IN     addr_in;            //ユニキャスト受信IP,ポート
    INT32           status;
    INT32           node_id;            // 接続先ID
}ST_OTE_COM_HEAD, * LPST_OTE_COM_HEAD;

typedef struct OteMultiComRcvBody {     //振れセンサ受信メッセージボディ部
    INT32 n_standby_ote;
    INT32 rmt_connect_seq_no;           //遠隔端末接続シーケンス番号
    INT32 site_connect_seq_no;          //機側端末接続シーケンス番号
    INT32 my_latest_seq_no;             //自端末接続シーケンス番号
}ST_OTE_MULTI_RCV_BODY, * LPST_OTE_MULTI_RCV_BODY;

typedef struct OteUniComRcvBody {       //端末からの受信メッセージボディ部
    char paddintA1[4];
    double auto_tg_pos[8];
    double auto_tg_dist[8];
    char paddintD1[4];
    UCHAR PB[64];
    INT16 notch[8];
}ST_OTE_UNI_RCV_BODY, * LPST_OTE_UNI_RCV_BODY;

typedef struct OteUniComSndBody {       //端末への送信メッセージボディ部
    char paddintA1[4];
    double pos_fb[8];
    double v_fb[8];
    double v_ref[8];
    double load_pos_fb[8];              //吊荷位置
    double load_v_fb[8];                //吊荷速度
    double auto_tg_pos[8];              //自動目標位置
    double auto_tg_dist[8];             //自動目標距離
    double auto_semi_set_tg[8][3];      //半自動設定目標位置
    char paddintD1[4];
    UCHAR PB[64];
    INT16 notch[8];
}ST_OTE_UNI_SND_BODY, * LPST_OTE_UNI_SND_BODY;

typedef struct OteComMultiRcvMsg {      //振れセンサ受信メッセージ
    ST_OTE_COM_HEAD         head;
    ST_OTE_MULTI_RCV_BODY   body;
}ST_OTE_MULTI_RCV_MSG, * LPST_OTE_MULTI_RCV_MSG;

typedef struct OteComUniRcvMsg {        //振れセンサ受信メッセージ
    ST_OTE_COM_HEAD         head;
    ST_OTE_UNI_RCV_BODY   body;
}ST_OTE_UNI_RCV_MSG, * LPST_OTE_UNI_RCV_MSG;

typedef struct OteComMultiSndMsg {      //振れセンサ受信メッセージ
    ST_OTE_COM_HEAD         head;
}ST_OTE_MULTI_SND_MSG, * LPST_OTE_MULTI_SND_MSG;

typedef struct OteComUniSndMsg {        //振れセンサ受信メッセージ
    ST_OTE_COM_HEAD         head;
    ST_OTE_UNI_SND_BODY   body;
}ST_OTE_UNI_SND_MSG, * LPST_OTE_UNI_SND_MSG;

#endif