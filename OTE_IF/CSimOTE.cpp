#include "CSimOTE.h"
#include <windowsx.h>       //# コモンコントロール

#include <winsock2.h>

#include <iostream>
#include <iomanip>
#include <sstream>

#include "OTEmsglist.h"

HWND CSimOTE::hWorkWnd = NULL;

CSharedMem* CSimOTE::pOteIOObj;
LPST_OTE_IO CSimOTE::pOTEio;

//Work Window表示用
HWND CSimOTE::hwndSTAT_U;
HWND CSimOTE::hwndINFMSG_U;

HWND CSimOTE::hwndSTAT_M_TE;
HWND CSimOTE::hwndINFMSG_M_TE;

HWND CSimOTE::hwndSTAT_M_CR;
HWND CSimOTE::hwndINFMSG_M_CR;

HWND CSimOTE::h_chkMsgSnd;
HWND CSimOTE::h_radio_disp_monOTE;
HWND CSimOTE::h_radio_disp_monSIM;

HWND CSimOTE::hwndMON_U_OTE,CSimOTE::hwndMON_U_CR,CSimOTE::hwndMON_M_OTE,CSimOTE::hwndMON_M_CR,CSimOTE::hwndMON_U_OTE_LABEL,CSimOTE::hwndMON_U_CR_LABEL,CSimOTE::hwndMON_M_OTE_LABEL,CSimOTE::hwndMON_M_CR_LABEL;

HWND CSimOTE::hwnd_monbuf[32],CSimOTE::hwnd_simbuf[128];
HWND CSimOTE::hwnd_lamp[N_UI_LAMP];

HWND CSimOTE::hwnd_notch_radio[MOTION_ID_MAX][12], CSimOTE::hwnd_pb_chk[N_UI_PB];
INT16 CSimOTE::sim_pb[N_UI_PB];

ST_OTE_IO CSimOTE::ote_io_workbuf;

int CSimOTE::is_ote_msg_snd = L_OFF;
int CSimOTE::panel_disp_mode = OTE_SIM_CODE_MON_OTE;


//IF用ソケット
WSADATA CSimOTE::wsaData;
SOCKET CSimOTE::s_u;                                         //ユニキャスト受信ソケット
SOCKET CSimOTE::s_m_te, CSimOTE::s_m_cr;                     //マルチキャスト受信ソケット
SOCKET CSimOTE::s_m_snd, CSimOTE::s_m_snd_dbg;                                      //マルチキャスト送信ソケット
SOCKADDR_IN CSimOTE::addrin_u;                               //ユニキャスト受信アドレス
SOCKADDR_IN CSimOTE::addrin_m_te, CSimOTE::addrin_m_cr;       //マルチキャスト受信アドレス
SOCKADDR_IN CSimOTE::addrin_ote_u;                           //ユニキャスト送信アドレス
SOCKADDR_IN CSimOTE::addrin_m_snd;                          //マルチキャスト送信アドレス

u_short CSimOTE::port_u = OTE_IF_IP_UNICAST_PORT_C;          //ユニキャスト受信ポート
u_short CSimOTE::port_m_te = OTE_IF_IP_MULTICAST_PORT_TE;
u_short CSimOTE::port_m_cr = OTE_IF_IP_MULTICAST_PORT_CR;    //マルチキャスト受信ポート

int CSimOTE::n_active_ote = 1;
int CSimOTE::connect_no_onboad = 0;
int CSimOTE::connect_no_remorte = 0;
int CSimOTE::my_connect_no = 0;
INT32 CSimOTE::sim_notchpos[MOTION_ID_MAX];


CSimOTE::CSimOTE() {
    pOteIOObj = new CSharedMem;

    for(int i=0;i<32;i++) hwnd_monbuf[i]=NULL;
    for (int i = 0;i < 128;i++) hwnd_simbuf[i]=NULL;
    for (int i = 0;i < N_UI_PB;i++) hwnd_pb_chk[i] = NULL;
    for (int i = 0;i < N_UI_LAMP;i++) hwnd_lamp[i] = NULL;
};
CSimOTE::~CSimOTE() {
    delete pOteIOObj;
};

int CSimOTE::set_outbuf(LPVOID pbuf) { 
    poutput = pbuf;return 0;
}    //出力バッファセット

/*****************************************************************************/
/*初期化処理                                                                 */
/*****************************************************************************/
int CSimOTE::init_proc() {
 
    //デバッグモード　ON　製番ではOFFで初期化
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif

    // 出力用共有メモリ取得
    out_size = sizeof(ST_OTE_IO);
    if (OK_SHMEM != pOteIOObj->create_smem(SMEM_OTE_IO_NAME, out_size, MUTEX_OTE_IO_NAME)) {
        mode |= OTE_IF_OTE_IO_MEM_NG;
    }
    pOTEio = (LPST_OTE_IO)pOteIOObj->get_pMap();

   return 0;
}



int CSimOTE::input() { return 0; }               //入力処理
int CSimOTE::parse() { return 0; }               //メイン処理
int CSimOTE::output() {                          //出力処理

    return 0; 
} 

std::wostringstream CSimOTE::woMSG;
std::wstring CSimOTE::wsMSG;

static struct ip_mreq mreq_te, mreq_cr;          //マルチキャスト受信設定用構造体
static int addrlen, nEvent;
static int nRtn = 0, nRcv_u = 0, nSnd_u = 0, nRcv_te = 0, nSnd_te = 0, nRcv_cr = 0;
static int lRcv_u = 0, lSnd_u = 0, lRcv_te = 0, lSnd_te = 0, lRcv_cr = 0;

static char szBuf[512];



//*********************************************************************************************
/*モニタ用ウィンドウ生成関数*/
HWND CSimOTE::open_WorkWnd(HWND hwnd_parent) {
    InitCommonControls();//コモンコントロール初期化

    WNDCLASSEX wc;

    hInst = GetModuleHandle(0);

    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OteSimWndProc;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("OteSimWnd");
    wc.hIconSm = NULL;
    ATOM fb = RegisterClassExW(&wc);

    hWorkWnd = CreateWindow(TEXT("OteSimWnd"),
        TEXT("OTE SIMURATION"),
        WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, SIM_WORK_WND_X, SIM_WORK_WND_Y, SIM_WORK_WND_W, SIM_WORK_WND_H,
        hwnd_parent,
        0,
        hInst,
        NULL);

    ShowWindow(hWorkWnd, SW_SHOW);
    UpdateWindow(hWorkWnd);

    return hWorkWnd;
}
//*********************************************************************************************
int CSimOTE::close_WorkWnd() {
    closesocket(s_u);
    closesocket(s_m_te);
    closesocket(s_m_cr);
    closesocket(s_m_snd);
    closesocket(s_m_snd_dbg);
    WSACleanup();
    DestroyWindow(hWorkWnd);  //ウィンドウ破棄
    hWorkWnd = NULL;
    return 0;
}
/*********************************************************************************************/
/*   ソケット,送信アドレスの初期化　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*********************************************************************************************/
/************ クレーンユニチキャスト受信ソケット初期化　***********/
int CSimOTE::init_sock_u(HWND hwnd) {    //ユニキャスト
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {    //WinSockの初期化
        perror("WSAStartup Error\n");
        return -1;
    }
 
     //# 受信ソケット処理
    s_u = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_u < 0) {
        perror("socket失敗\n");
        return -2;
    }
 
    memset(&addrin_u, 0, sizeof(addrin_u));
    addrin_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_C);        //端末側受信ポート
    addrin_u.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_u.sin_addr.s_addr);
  
    //# 送信先アドレスdefault設定
    memset(&addrin_ote_u, 0, sizeof(addrin_ote_u));
    addrin_ote_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_S);    //クレーン側側受信ポート
    addrin_ote_u.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_ote_u.sin_addr.s_addr);


    nRtn = bind(s_u, (LPSOCKADDR)&addrin_u, (int)sizeof(addrin_u)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_u);
        WSACleanup();
        return -3;
    }

    nRtn = WSAAsyncSelect(s_u, hwnd, ID_UDP_EVENT_U_SIM, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_u);
        WSACleanup();
        return -4;
    }
    return 0;
}
/************ 端末マルチキャスト受信ソケット初期化　***********/
int CSimOTE::init_sock_m_te(HWND hwnd) {

    //マルチキャスト用ソケット
        //ターミナル情報受信用
    s_m_te = socket(AF_INET, SOCK_DGRAM, 0);                                //Socketオープン
    if (s_m_te < 0) {
        perror("socket失敗\n");
        return -5;
    }
    memset(&addrin_m_te, 0, sizeof(addrin_m_te));                           //ソケットに名前を付ける
    addrin_m_te.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_TE);              //端末情報用ポート
    addrin_m_te.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_m_te.sin_addr.s_addr);

    nRtn = bind(s_m_te, (LPSOCKADDR)&addrin_m_te, (int)sizeof(addrin_m_te)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_m_te);
        WSACleanup();
        return -6;
    }
    nRtn = WSAAsyncSelect(s_m_te,hwnd, ID_UDP_EVENT_M_TE_SIM, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_m_te);
        WSACleanup();
        return -7;
    }

    //マルチキャストグループ参加登録
    memset(&mreq_te, 0, sizeof(mreq_te));
    mreq_te.imr_interface.S_un.S_addr = inet_addr(OTE_DEFAULT_IP_ADDR);     //パケット出力元IPアドレス
    mreq_te.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);       //マルチキャストIPアドレス
    if (setsockopt(s_m_te, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_te, sizeof(mreq_te)) != 0) {
        perror("setopt受信設定失敗\n");
        return -8;
    }

    //# 送信先アドレス設定
    memset(&addrin_m_snd, 0, sizeof(addrin_m_snd));
    addrin_m_snd.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_TE);
    addrin_m_snd.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_MULTI_IP_ADDR, &addrin_m_snd.sin_addr.s_addr);

    //# 送信ソケット設定
    s_m_snd = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_snd < 0) {
        perror("socket失敗\n");
        return -33;
    }
    DWORD ipaddr = inet_addr(CTRL_PC_IP_ADDR_OTE);
    //出力インターフェイス指定
    if (setsockopt(s_m_snd, IPPROTO_IP, IP_MULTICAST_IF, (char*)&ipaddr, sizeof(ipaddr)) != 0) {
        printf("setsockopt : %d\n", WSAGetLastError());
        return -13;
    }
    
    s_m_snd_dbg = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_snd_dbg < 0) {
        perror("socket失敗\n");
        return -34;
    }
    ipaddr = inet_addr(OTE_DEFAULT_IP_ADDR);
    //出力インターフェイス指定
    if (setsockopt(s_m_snd_dbg, IPPROTO_IP, IP_MULTICAST_IF, (char*)&ipaddr, sizeof(ipaddr)) != 0) {
        printf("setsockopt : %d\n", WSAGetLastError());
        return -13;
    }

     return 0;
}
/************ クレーンマルチキャスト受信ソケット初期化　***********/
int CSimOTE::init_sock_m_cr(HWND hwnd) {

    //マルチキャスト用ソケット
       //制御PC情報受信用
    s_m_cr = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_cr < 0) {
        perror("socket失敗\n");
        return -9;
    }
    memset(&addrin_m_cr, 0, sizeof(addrin_m_cr));
    addrin_m_cr.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_CR);
    addrin_m_cr.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_m_cr.sin_addr.s_addr);

    nRtn = bind(s_m_cr, (LPSOCKADDR)&addrin_m_cr, (int)sizeof(addrin_m_cr)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_m_cr);
        WSACleanup();
        return -10;
    }

    nRtn = WSAAsyncSelect(s_m_cr, hwnd, ID_UDP_EVENT_M_CR_SIM, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_m_cr);
        WSACleanup();
        return -11;
    }

    //マルチキャストグループ参加登録
    memset(&mreq_cr, 0, sizeof(mreq_cr));
 //   mreq_cr.imr_interface.S_un.S_addr = INADDR_ANY;                         //利用ネットワーク
    mreq_cr.imr_interface.S_un.S_addr = inet_addr(OTE_DEFAULT_IP_ADDR);    //利用ネットワーク
    mreq_cr.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);      //マルチキャストIPアドレス
    if (setsockopt(s_m_cr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_cr, sizeof(mreq_cr)) != 0) {
        perror("setopt受信設定失敗\n");
        return -12;
    }


    return 0;
}
//*********************************************************************************************
int CSimOTE::send_msg_u() {
    //クレーン側受信メッセージ
    int n = sizeof(ST_UOTE_RCV_MSG);
    nRtn = sendto(s_u, reinterpret_cast<const char*> (&ote_io_workbuf.rcv_msg_u), n, 0, (LPSOCKADDR)&addrin_ote_u, sizeof(addrin_ote_u));

    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_u++;lSnd_u = n;

        woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L" Snd n:" << nSnd_u << L" l:" << lSnd_u;
 
    }
    else if (nRtn == SOCKET_ERROR) {
       
        woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L" SND_ERR CODE ->" << WSAGetLastError();
    }
    else {
        
        woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L" sendto size ERROR" ;
     }
    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
    return nRtn;
}

int CSimOTE::send_msg_m_te() {

    int n = sizeof(ST_MOTE_RCV_MSG);

    nRtn = sendto(s_m_snd, reinterpret_cast<const char*> (&ote_io_workbuf.rcv_msg_m_te), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));

    nRtn = sendto(s_m_snd_dbg, reinterpret_cast<const char*> (&ote_io_workbuf.rcv_msg_m_te), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));

    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_te++;lSnd_te = n;

        woMSG << L"Rcv n:" << nRcv_te << L" l:" << lRcv_te << L" Snd n:" << nSnd_te << L" l:" << lSnd_te;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L"Rcv n:" << nRcv_te << L" l:" << lRcv_te << L" SND_ERR CODE ->" << WSAGetLastError();

    }
    else {
        woMSG << L"Rcv n:" << nRcv_te << L" l:" << lRcv_te << L" sendto size ERROR";
    }
    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();

  
    return nRtn;

}

int CSimOTE::set_msg_m_te(int mode, INT32 code, INT32 status) {
    ote_io_workbuf.rcv_msg_m_te.head.myid = 1;
    ote_io_workbuf.rcv_msg_m_te.head.code = ID_OTE_EVENT_CODE_CONST;
    ote_io_workbuf.rcv_msg_m_te.head.addr = addrin_m_te;
    ote_io_workbuf.rcv_msg_m_te.head.status = ID_TE_CONNECT_STATUS_OFF_LINE;
    ote_io_workbuf.rcv_msg_m_te.head.tgid = 0;
    for (int i = 0;i < N_CRANE_PC_MAX;i++) ote_io_workbuf.rcv_msg_m_te.body.pc_enable[i] = ID_PC_CONNECT_CODE_ENABLE;
    ote_io_workbuf.rcv_msg_m_te.body.n_remote_wait = n_active_ote;
    ote_io_workbuf.rcv_msg_m_te.body.onbord_seqno = n_active_ote;
    ote_io_workbuf.rcv_msg_m_te.body.remote_seqno = n_active_ote;
    ote_io_workbuf.rcv_msg_m_te.body.my_seqno = n_active_ote;

    return 0;
}

int CSimOTE::set_msg_u(int mode, INT32 code, INT32 status) {
        ote_io_workbuf.rcv_msg_u.head.myid = 1;
        ote_io_workbuf.rcv_msg_u.head.code = ID_OTE_EVENT_CODE_CONST;
        ote_io_workbuf.rcv_msg_u.head.addr = addrin_u;
        ote_io_workbuf.rcv_msg_u.head.status = ID_TE_CONNECT_STATUS_OFF_LINE;
        ote_io_workbuf.rcv_msg_u.head.tgid = 2;

        ote_io_workbuf.rcv_msg_u.body.tg_pos1[0] = 15;
        for (int i = 0;i < MOTION_ID_MAX;i++) {
            ote_io_workbuf.rcv_msg_u.body.notch_pos[i] = sim_notchpos[i];
        }
        for (int i = 0;i < N_UI_PB;i++) {
            ote_io_workbuf.rcv_msg_u.body.pb[i] = sim_pb[i];
        }

    return 0;
}
//*********************************************************************************************

static int timer_count=0;
LRESULT CALLBACK CSimOTE::OteSimWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    HDC hdc;
    switch (msg) {
    case WM_DESTROY: {
        hWorkWnd = NULL;
        LPST_OTE_IO pOTEio = (LPST_OTE_IO)pOteIOObj->get_pMap();
        pOTEio->OTEsim_status = L_OFF;          //端末シミュレーションモードOFF
    }return 0;
    case WM_CREATE: {

        InitCommonControls();//コモンコントロール初期化
        HINSTANCE hInst = GetModuleHandle(0);

        LPST_OTE_IO pOTEio = (LPST_OTE_IO)pOteIOObj->get_pMap();
        pOTEio->OTEsim_status = L_ON;          //端末シミュレーションモードON

        //通信バッファモニタSTATIC生成
        {
        hwndSTAT_U = CreateWindowW(TEXT("STATIC"), L"UNI",
            WS_CHILD | WS_VISIBLE | SS_LEFT,10, 5, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_U, hInst, NULL);
        hwndINFMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 25, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_U, hInst, NULL);
        hwndSTAT_M_TE = CreateWindowW(TEXT("STATIC"), L"M-TE",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 265, 5, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_TE, hInst, NULL);
        hwndINFMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            265, 25, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_TE, hInst, NULL);
        hwndSTAT_M_CR = CreateWindowW(TEXT("STATIC"), L"M-CR",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 520, 5, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_CR, hInst, NULL);
        hwndINFMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            520, 25, 250, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_CR, hInst, NULL);

        hwndMON_U_OTE_LABEL = CreateWindowW(TEXT("STATIC"), L"#FROM OTE UNI",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 50, 800, 20, hwnd, (HMENU)ID_STATIC_MON_OTE_U_LABEL, hInst, NULL);
        hwndMON_U_OTE = CreateWindowW(TEXT("STATIC"), L"-", 
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 75, 800, 55, hwnd, (HMENU)ID_STATIC_MON_OTE_U, hInst, NULL);

        hwndMON_U_CR_LABEL = CreateWindowW(TEXT("STATIC"), L"#TO OTE UNI",
           WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 135, 800, 20, hwnd, (HMENU)ID_STATIC_MON_CR_U_LABEL, hInst, NULL);
        hwndMON_U_CR = CreateWindowW(TEXT("STATIC"), L"-",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 160, 800, 140, hwnd, (HMENU)ID_STATIC_MON_CR_U, hInst, NULL);

        hwndMON_M_OTE_LABEL = CreateWindowW(TEXT("STATIC"), L"#FROM OTE MULTI",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 320, 800, 20, hwnd, (HMENU)ID_STATIC_MON_OTE_M_LABEL, hInst, NULL);
        hwndMON_M_OTE = CreateWindowW(TEXT("STATIC"), L"-",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 345, 800, 55, hwnd, (HMENU)ID_STATIC_MON_OTE_M, hInst, NULL);

        hwndMON_M_CR_LABEL = CreateWindowW(TEXT("STATIC"), L"#TO OTE MULTI",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 405, 800, 20, hwnd, (HMENU)ID_STATIC_MON_CR_M_LABEL, hInst, NULL);
        hwndMON_M_CR = CreateWindowW(TEXT("STATIC"), L"-",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 10, 430, 800, 55, hwnd, (HMENU)ID_STATIC_MON_CR_M, hInst, NULL);
        }
        
        //機能設定ラジオボタン,チェックボタン
        {
        h_chkMsgSnd = CreateWindow(L"BUTTON", L"SND MSG", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
            20, 520, 100, 25, hwnd, (HMENU)ID_CHK_OTE_SIM_MSG_SND, hInst, NULL);
        h_radio_disp_monOTE = CreateWindow(L"BUTTON", L"MON BUF", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
            140, 520, 80, 25, hwnd, (HMENU)IDC_RADIO_DISP_MON_OTE, hInst, NULL);
        h_radio_disp_monSIM = CreateWindow(L"BUTTON", L"MON SIM", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
            220, 520, 80, 25, hwnd, (HMENU)IDC_RADIO_DISP_MON_SIM, hInst, NULL);
        SendMessage(h_radio_disp_monSIM, BM_SETCHECK, BST_CHECKED, 0L);
        }

        hwnd_monbuf[0] = hwndMON_U_OTE; hwnd_monbuf[1] = hwndMON_U_CR; hwnd_monbuf[2] = hwndMON_M_OTE; hwnd_monbuf[3] = hwndMON_U_OTE_LABEL;hwnd_monbuf[4] = hwndMON_M_CR;
        hwnd_monbuf[5] = hwndMON_U_CR_LABEL; hwnd_monbuf[6] = hwndMON_M_OTE_LABEL; hwnd_monbuf[7] = hwndMON_M_CR_LABEL;
        for (int i = 0;i < 32;i++) {if (hwnd_monbuf[i] == NULL) break; else ShowWindow(hwnd_monbuf[i], SW_HIDE); }

        //ノッチ入力ラジオボタン
        {
            
            int mid = ID_HOIST;
 
            int pos_adjust = 0;
            LONGLONG idc;
            for (int j = 0;j < MOTION_ID_MAX;j++) {
                if (be_create[j] == false) {
                     continue;
                }
                int notch = 5;//ボタン表示テキスト
                for (int i = 0;i < 11;i++) {
                    idc = ID_RADIO_HST_NOTCH_5 + 20*j +i;
                    woMSG.str(L"");woMSG << notch;
                    if (i == 0) {
                        hwnd_notch_radio[j][i] = CreateWindow(L"BUTTON", woMSG.str().c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
                            80 + i * 40, 60 + pos_adjust * 35, 40, 25, hwnd, (HMENU)idc, hInst, NULL);
                    }
                    else {
                        hwnd_notch_radio[j][i] = CreateWindow(L"BUTTON", woMSG.str().c_str(), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
                            80 + i * 40, 60 + pos_adjust * 35, 40, 25, hwnd, (HMENU)idc, hInst, NULL);
                    }
                    notch--;
                }

                hwnd_notch_radio[j][11] = CreateWindowW(TEXT("STATIC"), motion_text[j], WS_CHILD | WS_VISIBLE | SS_LEFT, 
                    25 , 60 + pos_adjust * 35, 40, 25, hwnd, (HMENU)(++idc), hInst, NULL);

                SendMessage(hwnd_notch_radio[j][OTE_SIM_NOTCH_ARR_0], BM_SETCHECK, BST_CHECKED, 0L);
                sim_notchpos[j] = 0;
                pos_adjust++;
                woMSG.str(L"");
            }
 
            //自動目標位置調整ボタン
            for (int i = 0;i < 4;i++) {
                hwnd_pb_chk[ID_PB_MH_P1 + i] = CreateWindow(L"BUTTON", pb_text[ID_PB_MH_P1 + i], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                    535 + i * 60, 60, 58, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_MH_P1 + i), hInst, NULL);
            }
            for (int i = 0;i < 4;i++) {
                hwnd_pb_chk[ID_PB_BH_P1 + i] = CreateWindow(L"BUTTON", pb_text[ID_PB_BH_P1 + i], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                    535 + i * 60, 130, 58, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_BH_P1 + i), hInst, NULL);
            }
            for (int i = 0;i < 4;i++) {
                hwnd_pb_chk[ID_PB_SL_P1 + i] = CreateWindow(L"BUTTON", pb_text[ID_PB_SL_P1 + i], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                    535 + i * 60, 165, 58, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_SL_P1 + i), hInst, NULL);
            }
        }

        //操作入力チェックボタン + ランプ表示スタティック
        {
            int pos_x_offset = 0;
      
            hwnd_lamp[ID_PB_ESTOP] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_ESTOP), hInst, NULL);
            hwnd_pb_chk[ID_PB_ESTOP] = CreateWindow(L"BUTTON", pb_text[ID_PB_ESTOP], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_ESTOP), hInst, NULL); pos_x_offset += 105;

            hwnd_lamp[ID_PB_ANTISWAY_ON] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_ANTISWAY_ON), hInst, NULL);
            hwnd_pb_chk[ID_PB_ANTISWAY_ON] = CreateWindow(L"BUTTON", pb_text[ID_PB_ANTISWAY_ON], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20 + pos_x_offset, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_ANTISWAY_ON), hInst, NULL);pos_x_offset += 105;

            hwnd_lamp[ID_PB_AUTO_START] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_AUTO_START), hInst, NULL);
            hwnd_pb_chk[ID_PB_AUTO_START] = CreateWindow(L"BUTTON", pb_text[ID_PB_AUTO_START], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_AUTO_START), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_AUTO_MODE] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_AUTO_MODE), hInst, NULL);
            hwnd_pb_chk[ID_PB_AUTO_MODE] = CreateWindow(L"BUTTON", pb_text[ID_PB_AUTO_MODE], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_AUTO_MODE), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_AUTO_SET_Z] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_AUTO_SET_Z), hInst, NULL);
            hwnd_pb_chk[ID_PB_AUTO_SET_Z] = CreateWindow(L"BUTTON", pb_text[ID_PB_AUTO_SET_Z], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_AUTO_SET_Z), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_AUTO_SET_XY] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 355, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_AUTO_SET_XY), hInst, NULL);
            hwnd_pb_chk[ID_PB_AUTO_SET_XY] = CreateWindow(L"BUTTON", pb_text[ID_PB_AUTO_SET_XY], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 380, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_AUTO_SET_XY), hInst, NULL); pos_x_offset = 0;

            hwnd_lamp[ID_PB_REMOTE_MODE] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 410, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_REMOTE_MODE), hInst, NULL);
            hwnd_pb_chk[ID_PB_REMOTE_MODE] = CreateWindow(L"BUTTON", pb_text[ID_PB_REMOTE_MODE], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 435, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_REMOTE_MODE), hInst, NULL); pos_x_offset += 105;

            hwnd_lamp[ID_PB_CTRL_SOURCE_ON] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 410, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_CTRL_SOURCE_ON), hInst, NULL);
            hwnd_pb_chk[ID_PB_CTRL_SOURCE_ON] = CreateWindow(L"BUTTON", pb_text[ID_PB_CTRL_SOURCE_ON], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 435, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE_ON), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_CTRL_SOURCE2_ON] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 410, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_CTRL_SOURCE2_ON), hInst, NULL);
            hwnd_pb_chk[ID_PB_CTRL_SOURCE2_ON] = CreateWindow(L"BUTTON", pb_text[ID_PB_CTRL_SOURCE2_ON], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 435, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE2_ON), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_AUTO_RESET] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 410, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_ESTOP), hInst, NULL);
            hwnd_pb_chk[ID_PB_AUTO_RESET] = CreateWindow(L"BUTTON", pb_text[ID_PB_AUTO_RESET], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 435, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_AUTO_RESET), hInst, NULL); pos_x_offset+=105;

            hwnd_lamp[ID_PB_FAULT_RESET] = CreateWindowW(TEXT("STATIC"), L"-",
                WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + pos_x_offset, 410, 100, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_FAULT_RESET), hInst, NULL);
            hwnd_pb_chk[ID_PB_FAULT_RESET] = CreateWindow(L"BUTTON", pb_text[ID_PB_FAULT_RESET], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                20+ pos_x_offset, 435, 100, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_FAULT_RESET), hInst, NULL); pos_x_offset+=105;

            for (int i = 0;i < 6;i++) {
                hwnd_lamp[ID_PB_SEMI_AUTO_S1 + i] = CreateWindowW(TEXT("STATIC"), L"-",
                    WS_CHILD | WS_VISIBLE | SS_CENTER, 20 + i * 60, 465, 58, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_SEMI_AUTO_S1 + i), hInst, NULL);
                hwnd_pb_chk[ID_PB_SEMI_AUTO_S1 + i] = CreateWindow(L"BUTTON", pb_text[ID_PB_SEMI_AUTO_S1 + i], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                    20 + i * 60, 490, 58, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S1 + i), hInst, NULL);
            }
            for (int i = 0;i < 3;i++) {
                hwnd_lamp[ID_PB_PARK + i] = CreateWindowW(TEXT("STATIC"), L"-",
                    WS_CHILD | WS_VISIBLE | SS_CENTER, 400 + i * 60, 465, 58, 20, hwnd, (HMENU)(ID_STATIC_LAMP_0 + ID_PB_PARK + i), hInst, NULL);
                hwnd_pb_chk[ID_PB_PARK + i] = CreateWindow(L"BUTTON", pb_text[ID_PB_PARK + i], WS_CHILD | WS_VISIBLE | BS_PUSHLIKE | BS_AUTOCHECKBOX,
                    400 + i * 60, 490, 58, 25, hwnd, (HMENU)(ID_CHECK_PB_0 + ID_PB_PARK + i), hInst, NULL);
            }
        }
        
        //ソケット初期化処理
         if (init_sock_u(hwnd) == 0) {
            woMSG << L"UNI SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();
         }
        else {
            woMSG << L"UNI SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L""); woMSG.clear();
            wsMSG = L"-";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();
         }
        if (init_sock_m_te(hwnd) == 0) {
            woMSG << L"M-TE SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L""); woMSG.clear();
            wsMSG = L"-";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();
        }
        else {
            woMSG << L"M-TE SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L""); woMSG.clear();
            wsMSG = L"-";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();
         }
        if (init_sock_m_cr(hwnd) == 0) {
            woMSG << L"M-CR SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L""); woMSG.clear();
            wsMSG = L"-";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
        }
        else {
            woMSG << L"M-CR SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L""); woMSG.clear();
            wsMSG = L"-";
            tweet2infMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
           }

         //マルチキャスト送信電文初期値セット
        set_msg_m_te(0,0,0);
        set_msg_u(0, 0, 0);

        //マルチキャスト送信タイマ起動
        SetTimer(hwnd, ID_OTE_SIM_TIMER, OTE_SIM_SCAN_TIME, NULL);

    }break;
    case WM_TIMER: {
        timer_count++;
        if (is_ote_msg_snd) {
            set_msg_m_te(1,1,1);
            send_msg_m_te();
            set_msg_u(1,1,1);
            send_msg_u();
        }

        //モニタ表示　UNI　OTE-＞CR
        {
            woMSG << L"#FROM OTE UNI    ID:" << pOTEio->rcv_msg_u.head.myid << L"  EVENT:" << pOTEio->rcv_msg_u.head.code;
            sockaddr_in sockaddr = (sockaddr_in)pOTEio->rcv_msg_u.head.addr;

            woMSG << L" IP:" << sockaddr.sin_addr.S_un.S_un_b.s_b1 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b2 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b3 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b4;
            woMSG << L" PORT: " << htons(sockaddr.sin_port);
            woMSG << L" CODE:" << pOTEio->rcv_msg_u.head.status << L"  PC" << pOTEio->rcv_msg_u.head.tgid;
            SetWindowText(hwndMON_U_OTE_LABEL, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();

            woMSG << L"PB :";  
            for (int i = 0;i < 45;i++) {
                woMSG << pOTEio->rcv_msg_u.body.pb[i]; woMSG << L",";
              }
            woMSG << L"  NOTCH :";
            for (int i = 0;i < 5;i++) {
                woMSG << pOTEio->rcv_msg_u.body.notch_pos[i]; woMSG << L",";
            }
            woMSG << L"\nTG1p :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->rcv_msg_u.body.tg_pos1[i]; woMSG << L",";
            }
            woMSG << L" TG1d :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->rcv_msg_u.body.tg_dist1[i]; woMSG << L",";
            }
            woMSG << L"    TG2p :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->rcv_msg_u.body.tg_pos2[i]; woMSG << L",";
            }
            woMSG << L" TG2d :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->rcv_msg_u.body.tg_dist2[i]; woMSG << L",";
            }
            SetWindowText(hwndMON_U_OTE, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();
        }
        //モニタ表示　MULTI　OTE-＞CR
        {
            woMSG << L"#FROM OTE MULTI  ID:" << pOTEio->rcv_msg_m_te.head.myid << L"  EVENT:" << pOTEio->rcv_msg_m_te.head.code;
            sockaddr_in sockaddr = (sockaddr_in)pOTEio->rcv_msg_m_te.head.addr;

            woMSG << L" IP:" << sockaddr.sin_addr.S_un.S_un_b.s_b1 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b2 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b3 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b4;
            woMSG << L" PORT: " << htons(sockaddr.sin_port);
            woMSG << L" CODE:" << pOTEio->rcv_msg_m_te.head.status << L"  PC" << pOTEio->rcv_msg_m_te.head.tgid;
            SetWindowText(hwndMON_M_OTE_LABEL, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();

            woMSG << L"PC ENABLE :";
            for (int i = 0;i < 32;i++) {
                woMSG << pOTEio->rcv_msg_m_te.body.pc_enable[i]; woMSG << L",";
            }
            woMSG << L"\nSEQ ONBOARD :" << pOTEio->rcv_msg_m_te.body.onbord_seqno;
            woMSG << L"  SEQ REMOTE :" << pOTEio->rcv_msg_m_te.body.remote_seqno;
            woMSG << L"  SEQ MINE :" << pOTEio->rcv_msg_m_te.body.my_seqno;
 
            SetWindowText(hwndMON_M_OTE, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();
        }

        //モニタ表示　UNI　CR->OTE
        {
            woMSG << L"#TO OTE UNI ID:" << pOTEio->snd_msg_u.head.myid << L"  EVENT:" << pOTEio->snd_msg_u.head.code;
            sockaddr_in sockaddr = (sockaddr_in)pOTEio->snd_msg_u.head.addr;
            woMSG << L" IP:" << sockaddr.sin_addr.S_un.S_un_b.s_b1 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b2 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b3 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b4;
            woMSG << L" PORT: " << htons(sockaddr.sin_port);
            woMSG << L" CODE:" << pOTEio->snd_msg_u.head.status << L"  PC" << pOTEio->snd_msg_u.head.tgid;
            SetWindowText(hwndMON_U_CR_LABEL, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();

            woMSG << L"POS :";
            for (int i = 0;i < 8;i++) {
                woMSG << pOTEio->snd_msg_u.body.pos[i]; woMSG << L",";
            }
            woMSG << L"  VFB :";
            for (int i = 0;i < 8;i++) {
                woMSG << pOTEio->snd_msg_u.body.v_fb[i]; woMSG << L",";
            }
            woMSG << L"  VREF :";
            for (int i = 0;i < 8;i++) {
                woMSG << pOTEio->snd_msg_u.body.v_ref[i]; woMSG << L",";
            }
            woMSG << L"\nLPOS :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.ld_pos[i]; woMSG << L",";
            }
            woMSG << L"  LVFB :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.ld_v_fb[i]; woMSG << L",";
            }
            woMSG << L"  AUTO TG :";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos[i]; woMSG << L",";
            }

            woMSG << L"\nSEMI S1:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[0][i]; woMSG << L",";
            }
            woMSG << L" S2:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[1][i]; woMSG << L",";
            }
            woMSG << L" S3:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[2][i]; woMSG << L",";
            }

            woMSG << L"\nSEMI L1:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[3][i]; woMSG << L",";
            }
            woMSG << L" L2:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[4][i]; woMSG << L",";
            }
            woMSG << L" L3:";
            for (int i = 0;i < 4;i++) {
                woMSG << pOTEio->snd_msg_u.body.tg_pos_semi[5][i]; woMSG << L",";
            }
            woMSG << L"\nPLC CTRL:";
            for (int i = 0;i < 30;i++) {
                woMSG << pOTEio->snd_msg_u.body.plc_data[i]; woMSG << L",";
            }
            woMSG << L"\nPLC FAULT:";
            for (int i = 67;i < 99;i++) {
                woMSG << pOTEio->snd_msg_u.body.plc_data[i]; woMSG << L",";
            }


            woMSG << L"\nLAMP :";
            for (int i = 0;i < 30;i++) {
                woMSG << pOTEio->snd_msg_u.body.lamp[i]; woMSG << L",";
            }
            woMSG << L"  NOTCH :";
            for (int i = 0;i < 5;i++) {
                woMSG << pOTEio->snd_msg_u.body.notch_pos[i]; woMSG << L",";
            }
            SetWindowText(hwndMON_U_CR, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();
        }

        //モニタ表示　MULTI　CR->OTE
        {
            woMSG << L"#TO OTE MULTI  ID:" << pOTEio->snd_msg_m.head.myid << L"  EVENT:" << pOTEio->snd_msg_m.head.code;
            sockaddr_in sockaddr = (sockaddr_in)pOTEio->snd_msg_m.head.addr;
            woMSG << L" IP:" << sockaddr.sin_addr.S_un.S_un_b.s_b1 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b2 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b3 << L"." << sockaddr.sin_addr.S_un.S_un_b.s_b4;
            woMSG << L" PORT: " << htons(sockaddr.sin_port);
            woMSG << L" CODE:" << pOTEio->snd_msg_m.head.status << L"  PC" << pOTEio->snd_msg_m.head.tgid;
            SetWindowText(hwndMON_M_CR_LABEL, woMSG.str().c_str());woMSG.str(L""); woMSG.clear();
        }

        //ランプ表示
        {
            for (int i = 0;i < N_UI_LAMP;i++) {
                if (hwnd_lamp[i] == NULL)continue;
                else {
                    if(pOTEio->snd_msg_u.body.lamp[i]>0)    SetWindowText(hwnd_lamp[i], L"●"); 
                    else                                    SetWindowText(hwnd_lamp[i], L"〇");
                }
            }
        }

    }break;

    case ID_UDP_EVENT_U_SIM: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_u++;

            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_u, (char*)&ote_io_workbuf.snd_msg_u, sizeof(ST_UOTE_SND_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR   Snd n:"<< nSnd_u << L" l:" << lSnd_u;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
            }
            else {
                lRcv_u = nRtn ;
                woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L" Snd n:" << nSnd_u << L" l:" << lSnd_u;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

                //woMSG << L"\n IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                //woMSG << L" PORT: " << psockaddr->sin_port;
            }

        }break;
        case FD_WRITE: {

        }break;
        case FD_CLOSE: {
            ;
        }break;

        }
     }break;
    case ID_UDP_EVENT_M_TE_SIM: {
 
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_te++;
            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_m_te, (char*)&ote_io_workbuf.rcv_msg_m_te, sizeof(ST_MOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

           if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR   Snd n:" << nSnd_te << L" l:" << lSnd_te;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
            }
           else {
               lRcv_te = nRtn;
               woMSG << L"Rcv n:" << nRcv_te << L" l:" << lRcv_te << L" Snd n:" << nSnd_te << L" l:" << lSnd_te;
               tweet2infMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
           }

        }break;
        case FD_WRITE: {

        }break;
        case FD_CLOSE: {
            ;
        }break;

        }
    }break;
    case ID_UDP_EVENT_M_CR_SIM: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_cr++;
            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_m_cr, (char*)&ote_io_workbuf.rcv_msg_m_cr, sizeof(ST_MOTE_SND_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

  
            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
            }
            else {
                lRcv_cr = nRtn;
                woMSG << L"Rcv n:" << nRcv_cr << L" l:" << lRcv_cr ;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
            }


        }break;
        case FD_WRITE: {

        }break;
        case FD_CLOSE: {
            ;
        }break;

        }
    }break;

    case SWAY_SENSOR__MSG_SEND_COM:
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        hdc = BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
    }break;
     
    case WM_COMMAND: {
        int wmId = LOWORD(wp);
        switch (wmId)
        {
        case  ID_CHK_OTE_SIM_MSG_SND: {
            if (IsDlgButtonChecked(hwnd, ID_CHK_OTE_SIM_MSG_SND) == BST_CHECKED) is_ote_msg_snd = L_ON;
            else  is_ote_msg_snd = L_OFF;
        }break;

        case  IDC_RADIO_DISP_MON_OTE: {
            panel_disp_mode = OTE_SIM_CODE_MON_OTE;
            ShowWindow(hwndSTAT_M_TE, SW_RESTORE);
            for (int i = 0;i < 32;i++) {
                if (hwnd_monbuf[i] == NULL) break;
                else ShowWindow(hwnd_monbuf[i], SW_RESTORE);
            }

            for (int j = 0;j < MOTION_ID_MAX;j++) {
                for (int i = 0;i < 12;i++) {
                    if (hwnd_notch_radio[j][i] == NULL) break;
                    else ShowWindow(hwnd_notch_radio[j][i], SW_HIDE);
                }
 
            }
            for (int i = 0;i < 64;i++) {
                if (hwnd_pb_chk[i] == NULL) continue;
                else ShowWindow(hwnd_pb_chk[i], SW_HIDE);
            }

        }break;

        case  IDC_RADIO_DISP_MON_SIM: {
            panel_disp_mode = OTE_SIM_CODE_MON_SIM;
            for (int i = 0;i < 32;i++) {
                if (hwnd_monbuf[i] == NULL) break;
                else ShowWindow(hwnd_monbuf[i], SW_HIDE);
            }
            for (int j = 0;j < MOTION_ID_MAX;j++) {
                for (int i = 0;i < 12;i++) {
                    if (hwnd_notch_radio[j][i] == NULL) break;
                    else ShowWindow(hwnd_notch_radio[j][i], SW_RESTORE);
                }
            }
            for (int i = 0;i < 64;i++) {
                if (hwnd_pb_chk[i] == NULL) continue;
                else ShowWindow(hwnd_pb_chk[i], SW_RESTORE);
            }
        }break;
            //ノッチ入力
        case  ID_RADIO_HST_NOTCH_5 + 0: sim_notchpos[ID_HOIST] = 5;break;
        case  ID_RADIO_HST_NOTCH_5 + 1: sim_notchpos[ID_HOIST] = 4;break;
        case  ID_RADIO_HST_NOTCH_5 + 2: sim_notchpos[ID_HOIST] = 3;break;
        case  ID_RADIO_HST_NOTCH_5 + 3: sim_notchpos[ID_HOIST] = 2;break;
        case  ID_RADIO_HST_NOTCH_5 + 4: sim_notchpos[ID_HOIST] = 1;break;
        case  ID_RADIO_HST_NOTCH_5 + 5: sim_notchpos[ID_HOIST] = 0;break;
        case  ID_RADIO_HST_NOTCH_5 + 6: sim_notchpos[ID_HOIST] = -1;break;
        case  ID_RADIO_HST_NOTCH_5 + 7: sim_notchpos[ID_HOIST] = -2;break;
        case  ID_RADIO_HST_NOTCH_5 + 8: sim_notchpos[ID_HOIST] = -3;break;
        case  ID_RADIO_HST_NOTCH_5 + 9: sim_notchpos[ID_HOIST] = -4;break;
        case  ID_RADIO_HST_NOTCH_5 + 10: sim_notchpos[ID_HOIST] = -5;break;

        case  ID_RADIO_GNT_NOTCH_5 + 0: sim_notchpos[ID_GANTRY] = 5;break;
        case  ID_RADIO_GNT_NOTCH_5 + 1: sim_notchpos[ID_GANTRY] = 4;break;
        case  ID_RADIO_GNT_NOTCH_5 + 2: sim_notchpos[ID_GANTRY] = 3;break;
        case  ID_RADIO_GNT_NOTCH_5 + 3: sim_notchpos[ID_GANTRY] = 2;break;
        case  ID_RADIO_GNT_NOTCH_5 + 4: sim_notchpos[ID_GANTRY] = 1;break;
        case  ID_RADIO_GNT_NOTCH_5 + 5: sim_notchpos[ID_GANTRY] = 0;break;
        case  ID_RADIO_GNT_NOTCH_5 + 6: sim_notchpos[ID_GANTRY] = -1;break;
        case  ID_RADIO_GNT_NOTCH_5 + 7: sim_notchpos[ID_GANTRY] = -2;break;
        case  ID_RADIO_GNT_NOTCH_5 + 8: sim_notchpos[ID_GANTRY] = -3;break;
        case  ID_RADIO_GNT_NOTCH_5 + 9: sim_notchpos[ID_GANTRY] = -4;break;
        case  ID_RADIO_GNT_NOTCH_5 + 10: sim_notchpos[ID_GANTRY] = -5;break;

        case  ID_RADIO_BH_NOTCH_5 + 0: sim_notchpos[ID_BOOM_H] = 5;break;
        case  ID_RADIO_BH_NOTCH_5 + 1: sim_notchpos[ID_BOOM_H] = 4;break;
        case  ID_RADIO_BH_NOTCH_5 + 2: sim_notchpos[ID_BOOM_H] = 3;break;
        case  ID_RADIO_BH_NOTCH_5 + 3: sim_notchpos[ID_BOOM_H] = 2;break;
        case  ID_RADIO_BH_NOTCH_5 + 4: sim_notchpos[ID_BOOM_H] = 1;break;
        case  ID_RADIO_BH_NOTCH_5 + 5: sim_notchpos[ID_BOOM_H] = 0;break;
        case  ID_RADIO_BH_NOTCH_5 + 6: sim_notchpos[ID_BOOM_H] = -1;break;
        case  ID_RADIO_BH_NOTCH_5 + 7: sim_notchpos[ID_BOOM_H] = -2;break;
        case  ID_RADIO_BH_NOTCH_5 + 8: sim_notchpos[ID_BOOM_H] = -3;break;
        case  ID_RADIO_BH_NOTCH_5 + 9: sim_notchpos[ID_BOOM_H] = -4;break;
        case  ID_RADIO_BH_NOTCH_5 + 10: sim_notchpos[ID_BOOM_H] = -5;break;

        case  ID_RADIO_SLW_NOTCH_5 + 0: sim_notchpos[ID_SLEW] = 5;break;
        case  ID_RADIO_SLW_NOTCH_5 + 1: sim_notchpos[ID_SLEW] = 4;break;
        case  ID_RADIO_SLW_NOTCH_5 + 2: sim_notchpos[ID_SLEW] = 3;break;
        case  ID_RADIO_SLW_NOTCH_5 + 3: sim_notchpos[ID_SLEW] = 2;break;
        case  ID_RADIO_SLW_NOTCH_5 + 4: sim_notchpos[ID_SLEW] = 1;break;
        case  ID_RADIO_SLW_NOTCH_5 + 5: sim_notchpos[ID_SLEW] = 0;break;
        case  ID_RADIO_SLW_NOTCH_5 + 6: sim_notchpos[ID_SLEW] = -1;break;
        case  ID_RADIO_SLW_NOTCH_5 + 7: sim_notchpos[ID_SLEW] = -2;break;
        case  ID_RADIO_SLW_NOTCH_5 + 8: sim_notchpos[ID_SLEW] = -3;break;
        case  ID_RADIO_SLW_NOTCH_5 + 9: sim_notchpos[ID_SLEW] = -4;break;
        case  ID_RADIO_SLW_NOTCH_5 + 10: sim_notchpos[ID_SLEW] = -5;break;

            //PB入力
        case (ID_CHECK_PB_0 + ID_PB_ESTOP): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_ESTOP), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_ESTOP] = L_ON;
            else sim_pb[ID_PB_ESTOP] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_ANTISWAY_ON): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_ANTISWAY_ON), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_ANTISWAY_ON] = L_ON;
            else sim_pb[ID_PB_ANTISWAY_ON] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_AUTO_START): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_AUTO_START), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_AUTO_START] = L_ON;
            else sim_pb[ID_PB_AUTO_START] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_AUTO_MODE): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_AUTO_MODE), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_AUTO_MODE] = L_ON;
            else sim_pb[ID_PB_AUTO_MODE] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_AUTO_SET_Z): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_AUTO_SET_Z), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_AUTO_SET_Z] = L_ON;
            else sim_pb[ID_PB_AUTO_SET_Z] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_AUTO_SET_XY): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_AUTO_SET_XY), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_AUTO_SET_XY] = L_ON;
            else sim_pb[ID_PB_AUTO_SET_XY] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE_ON): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE_ON), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_CTRL_SOURCE_ON] = L_ON;
            else sim_pb[ID_PB_CTRL_SOURCE_ON] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE2_ON): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_CTRL_SOURCE2_ON), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_CTRL_SOURCE2_ON] = L_ON;
            else sim_pb[ID_PB_CTRL_SOURCE2_ON] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_AUTO_RESET): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_AUTO_RESET), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_AUTO_RESET] = L_ON;
            else sim_pb[ID_PB_AUTO_RESET] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_FAULT_RESET): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_FAULT_RESET), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_FAULT_RESET] = L_ON;
            else sim_pb[ID_PB_FAULT_RESET] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_S1] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_S1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_S2] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_S2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S3): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_S3), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_S3] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_S3] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_L1] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_L1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_L2] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_L2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L3): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SEMI_AUTO_L3), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SEMI_AUTO_L3] = L_ON;
            else sim_pb[ID_PB_SEMI_AUTO_L3] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_MH_P1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_MH_P1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_MH_P1] = L_ON;
            else sim_pb[ID_PB_MH_P1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_MH_P2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_MH_P2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_MH_P2] = L_ON;
            else sim_pb[ID_PB_MH_P2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_MH_M1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_MH_M1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_MH_M1] = L_ON;
            else sim_pb[ID_PB_MH_M1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_MH_M2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_MH_M2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_MH_M2] = L_ON;
            else sim_pb[ID_PB_MH_M2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_BH_P1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_BH_P1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_BH_P1] = L_ON;
            else sim_pb[ID_PB_BH_P1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_BH_P2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_BH_P2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_BH_P2] = L_ON;
            else sim_pb[ID_PB_BH_P2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_BH_M1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_BH_M1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_BH_M1] = L_ON;
            else sim_pb[ID_PB_BH_M1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_BH_M2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_BH_M2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_BH_M2] = L_ON;
            else sim_pb[ID_PB_BH_M2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SL_P1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SL_P1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SL_P1] = L_ON;
            else sim_pb[ID_PB_SL_P1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SL_P2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SL_P2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SL_P2] = L_ON;
            else sim_pb[ID_PB_SL_P2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SL_M1): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SL_M1), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SL_M1] = L_ON;
            else sim_pb[ID_PB_SL_M1] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_SL_M2): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_SL_M2), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_SL_M2] = L_ON;
            else sim_pb[ID_PB_SL_M2] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_PARK): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_PARK), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_PARK] = L_ON;
            else sim_pb[ID_PB_PARK] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_GRND): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_GRND), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_GRND] = L_ON;
            else sim_pb[ID_PB_ANTISWAY_ON] = L_OFF;
        }break;
        case (ID_CHECK_PB_0 + ID_PB_PICK): {
            if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_CHECK_PB_0 + ID_PB_PICK), BM_GETCHECK, 0, 0)) sim_pb[ID_PB_PICK] = L_ON;
            else sim_pb[ID_PB_PICK] = L_OFF;
        }break;
        default:break;
        }
    }
    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    return 0;
}


//# ウィンドウへのメッセージ表示　wstring
void CSimOTE::tweet2statusMSG(const std::wstring& srcw, int code) {
    switch (code) {
    case ID_SOCK_CODE_U:
        SetWindowText(hwndSTAT_U, srcw.c_str());
        break;
    case ID_SOCK_CODE_TE:
        SetWindowText(hwndSTAT_M_TE, srcw.c_str());
        break;
    case ID_SOCK_CODE_CR:
        SetWindowText(hwndSTAT_M_CR, srcw.c_str());
        break;
    default: break;
    }
    return;
};

void CSimOTE::tweet2infMSG(const std::wstring& srcw, int code) {
    switch (code) {
    case ID_SOCK_CODE_U:
        SetWindowText(hwndINFMSG_U, srcw.c_str());
        break;
    case ID_SOCK_CODE_TE:
        SetWindowText(hwndINFMSG_M_TE, srcw.c_str());
        break;
    case ID_SOCK_CODE_CR:
        SetWindowText(hwndINFMSG_M_CR, srcw.c_str());
        break;
    default: break;
    }
    return;
};

void CSimOTE::tweet2static(const std::wstring& srcw, HWND hwnd) {
     SetWindowText(hwnd, srcw.c_str());
    return;
};