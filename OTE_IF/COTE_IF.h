#pragma once

#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "Spec.h"
#include "Opeterminal.h"

#include <commctrl.h>
#include <time.h>
#include <string>

#define OTE_IF_OTE_IO_MEM_NG              0x8000
#define OTE_IF_CRANE_MEM_NG                0x4000
#define OTE_IF_SIM_MEM_NG                  0x2000
#define OTE_IF_DBG_MODE                0x00000010

#define ID_SOCK_CODE_U  			        0
#define ID_SOCK_CODE_TE			            1
#define ID_SOCK_CODE_CR			            2

#define ID_UDP_EVENT_U  			        10604
#define ID_UDP_EVENT_M_TE			        10605
#define ID_UDP_EVENT_M_CR			        10606

#define ID_STATIC_OTE_IF_LABEL_COM          10607

#define ID_STATIC_OTE_IF_VIEW_STAT_U        10609
#define ID_STATIC_OTE_IF_VIEW_RCV_U         10609
#define ID_STATIC_OTE_IF_VIEW_SND_U         10610
#define ID_STATIC_OTE_IF_VIEW_INF_U         10611

#define ID_STATIC_OTE_IF_VIEW_STAT_TE       10611
#define ID_STATIC_OTE_IF_VIEW_RCV_TE        10612
#define ID_STATIC_OTE_IF_VIEW_SND_TE        10613
#define ID_STATIC_OTE_IF_VIEW_INF_TE        10614

#define ID_STATIC_OTE_IF_VIEW_STAT_CR       10615
#define ID_STATIC_OTE_IF_VIEW_RCV_CR        10616
#define ID_STATIC_OTE_IF_VIEW_SND_CR        10617
#define ID_STATIC_OTE_IF_VIEW_INF_CR        10618


//起動タイマーID
#define ID_WORK_WND_TIMER					100
#define MULTI_SND_SCAN_TIME				    1000		// マルチキャスト IF送信周期msec


#define WORK_WND_X							1050		//メンテパネル表示位置X
#define WORK_WND_Y							20			//メンテパネル表示位置Y
#define WORK_WND_W							540		    //メンテパネルWINDOW幅
#define WORK_WND_H							370			//メンテパネルWINDOW高さ


class COteIF :  public CBasicControl
{
private:

    //# 出力用共有メモリオブジェクトポインタ:
  CSharedMem* pOteIOObj;
    //# 入力用共有メモリオブジェクトポインタ:
  CSharedMem* pCraneStatusObj;
  CSharedMem* pSimulationStatusObj;


    HINSTANCE hInst;

    void init_rcv_msg();

    static void tweet2rcvMSG(const std::wstring& srcw,int code);
    static void tweet2sndMSG(const std::wstring& srcw,int code);
    static void tweet2infMSG(const std::wstring& srcw, int code);
    static void tweet2statusMSG(const std::wstring& srcw, int code);

    //IF用ソケット
    static WSADATA wsaData;
    static SOCKET s_u;                              //ユニキャスト受信ソケット
    static SOCKET s_m_te, s_m_cr;                   //マルチキャスト受信ソケット
    static SOCKADDR_IN addrin_u;                    //ユニキャスト受信アドレス
    static SOCKADDR_IN addrin_ote_u;                //ユニキャスト送信アドレス
    static SOCKADDR_IN addrin_m_te, addrin_m_cr;    //マルチキャスト受信アドレス
    static u_short port_u;                          //ユニキャスト受信ポート
    static u_short port_m_te, port_m_cr ;           //マルチキャスト受信ポート

public:
    COteIF();
    ~COteIF();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;

    static ST_UOTE_SND_MSG snd_msg_u;
    static ST_UOTE_RCV_MSG rcv_msg_u;

    static ST_MOTE_SND_MSG snd_msg_m;
    static ST_MOTE_RCV_MSG rcv_msg_m_te;
    static ST_MOTE_SND_MSG rcv_msg_m_cr;
 
    //Work Window表示用
    static HWND hwndSTAT_U;
    static HWND hwndRCVMSG_U;
    static HWND hwndSNDMSG_U;
    static HWND hwndINFMSG_U;

    static HWND hwndSTAT_M_TE;
    static HWND hwndRCVMSG_M_TE;
    static HWND hwndSNDMSG_M_TE;
    static HWND hwndINFMSG_M_TE;

    static HWND hwndSTAT_M_CR;
    static HWND hwndRCVMSG_M_CR;
    static HWND hwndSNDMSG_M_CR;
    static HWND hwndINFMSG_M_CR;


    void set_debug_mode(int id) {
        if (id) mode |= OTE_IF_DBG_MODE;
        else    mode &= ~OTE_IF_DBG_MODE;
    }

    int is_debug_mode() { return(mode & OTE_IF_DBG_MODE); }

    //オーバーライド
    int set_outbuf(LPVOID);     //出力バッファセット
    int init_proc();            //初期化処理
    int input();                //入力処理
    int parse();                //メイン処理
    int output();               //出力処理

    static int send_msg_u();   //ユニキャスト送信
    static int send_msg_m();   //マルチキャスト送信

    virtual HWND open_WorkWnd(HWND hwnd_parent);
    static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
    static int close_WorkWnd();
    static int init_sock_u(HWND hwnd);
    static int init_sock_m_te(HWND hwnd);
    static int init_sock_m_cr(HWND hwnd);

    static std::wostringstream woMSG;
    static std::wstring wsMSG;

};
