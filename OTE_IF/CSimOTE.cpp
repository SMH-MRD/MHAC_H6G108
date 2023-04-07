#include "CSimOTE.h"
#include <windowsx.h>       //# コモンコントロール

#include <winsock2.h>

#include <iostream>
#include <iomanip>
#include <sstream>

HWND CSimOTE::hWorkWnd = NULL;

//Work Window表示用
HWND CSimOTE::hwndSTAT_U;
HWND CSimOTE::hwndRCVMSG_U;
HWND CSimOTE::hwndSNDMSG_U;
HWND CSimOTE::hwndINFMSG_U;

HWND CSimOTE::hwndSTAT_M_TE;
HWND CSimOTE::hwndRCVMSG_M_TE;
HWND CSimOTE::hwndSNDMSG_M_TE;
HWND CSimOTE::hwndINFMSG_M_TE;

HWND CSimOTE::hwndSTAT_M_CR;
HWND CSimOTE::hwndRCVMSG_M_CR;
HWND CSimOTE::hwndSNDMSG_M_CR;
HWND CSimOTE::hwndINFMSG_M_CR;

HWND CSimOTE::hwndCNT_U;
HWND CSimOTE::hwndCNT_M_TE;
HWND CSimOTE::hwndCNT_M_CR;

ST_UOTE_SND_MSG CSimOTE::snd_msg_u;
ST_UOTE_RCV_MSG CSimOTE::rcv_msg_u;

ST_MOTE_SND_MSG CSimOTE::rcv_msg_m;
ST_MOTE_RCV_MSG CSimOTE::rcv_msg_m_te;
ST_MOTE_SND_MSG CSimOTE::rcv_msg_m_cr;


//IF用ソケット
WSADATA CSimOTE::wsaData;
SOCKET CSimOTE::s_u;                                         //ユニキャスト受信ソケット
SOCKET CSimOTE::s_m_te, CSimOTE::s_m_cr;                      //マルチキャスト受信ソケット
SOCKADDR_IN CSimOTE::addrin_u;                               //ユニキャスト受信アドレス
SOCKADDR_IN CSimOTE::addrin_m_te, CSimOTE::addrin_m_cr;       //マルチキャスト受信アドレス
SOCKADDR_IN CSimOTE::addrin_ote_u;                           //ユニキャスト送信アドレス

u_short CSimOTE::port_u = OTE_IF_IP_UNICAST_PORT_C;          //ユニキャスト受信ポート
u_short CSimOTE::port_m_te = OTE_IF_IP_MULTICAST_PORT_TE;
u_short CSimOTE::port_m_cr = OTE_IF_IP_MULTICAST_PORT_CR;    //マルチキャスト受信ポート

int CSimOTE::n_active_ote = 1;
int CSimOTE::connect_no_onboad = 0;
int CSimOTE::connect_no_remorte = 0;
int CSimOTE::my_connect_no = 0;


CSimOTE::CSimOTE() {
    ;
};
CSimOTE::~CSimOTE() {
;
};

int CSimOTE::set_outbuf(LPVOID) { return 0; }    //出力バッファセット

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
    return 0;
}
int CSimOTE::input() { return 0; }               //入力処理
int CSimOTE::parse() { return 0; }               //メイン処理
int CSimOTE::output() { return 0; }              //出力処理

std::wostringstream CSimOTE::woMSG;
std::wstring CSimOTE::wsMSG;

static struct ip_mreq mreq_te, mreq_cr;                     //マルチキャスト受信設定用構造体
static int addrlen, nEvent;
static int nRtn = 0, nRcv_u = 0, nSnd_u = 0, nRcv_te = 0, nSnd_te = 0, nRcv_cr = 0;

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
    WSACleanup();
    DestroyWindow(hWorkWnd);  //ウィンドウ破棄
    hWorkWnd = NULL;
    return 0;
}
/*********************************************************************************************/
/*   ソケット,送信アドレスの初期化　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　　*/
/*********************************************************************************************/
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
    inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_u.sin_addr.s_addr);

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
int CSimOTE::init_sock_m_te(HWND hwnd) {

    //マルチキャスト用ソケット
    {
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

        nRtn = WSAAsyncSelect(s_m_te, hwnd, ID_UDP_EVENT_M_TE_SIM, FD_READ | FD_WRITE | FD_CLOSE);

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
    }
    return 0;
}
int CSimOTE::init_sock_m_cr(HWND hwnd) {

    //マルチキャスト用ソケット
    {
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
        mreq_cr.imr_interface.S_un.S_addr = inet_addr(OTE_DEFAULT_IP_ADDR);     //パケット出力元IPアドレス
        mreq_cr.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);      //マルチキャストIPアドレス
        if (setsockopt(s_m_cr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_cr, sizeof(mreq_cr)) != 0) {
            perror("setopt受信設定失敗\n");
            return -12;
        }
    }
    return 0;
}
//*********************************************************************************************
int CSimOTE::send_msg_u() {

   int n = sizeof(ST_UOTE_RCV_MSG);

    nRtn = sendto(s_u, reinterpret_cast<const char*> (&rcv_msg_u), n, 0, (LPSOCKADDR)&addrin_ote_u, sizeof(addrin_ote_u));

    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_u++;
        woMSG << L"SNDlen: " << nRtn;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L"ERR CODE ->" << WSAGetLastError();
    }
    else {
        woMSG << L" sendto size ERROR ";
    }
    tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
   

    woMSG.str(L"");
    woMSG << L"SND" << nSnd_u;
    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

    return nRtn;
}

int CSimOTE::send_msg_m_te() {

    int n = sizeof(ST_MOTE_RCV_MSG);

    nRtn = sendto(s_m_te, reinterpret_cast<const char*> (&rcv_msg_m_te), n, 0, (LPSOCKADDR)&addrin_m_te, sizeof(addrin_m_te));
    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_te++;
        woMSG << L"SNDlen: " << nRtn ;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L"ERR CODE ->" << WSAGetLastError();
    }
    else {
        woMSG << L" sendto size ERROR ";
    }
    tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();

    woMSG.str(L""); 
    woMSG<< L"SND" <<  nSnd_te;
    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();

    return nRtn;

}

int CSimOTE::set_msg_m_te(int mode, INT32 code, INT32 status) {
    if (mode == ID_MULTI_MSG_SET_MODE_INIT) {
        rcv_msg_m_te.head.myid = 1;
        rcv_msg_m_te.head.code = ID_OTE_EVENT_CODE_CONST;
        rcv_msg_m_te.head.addr = addrin_u;
        rcv_msg_m_te.head.status = ID_OTE_CONNECT_CODE_NO_OPERATION;
        rcv_msg_m_te.head.nodeid = 0;

        for (int i = 0;i < N_CRANE_PC_MAX;i++) rcv_msg_m_te.body.pc_enable[i] = ID_PC_CONNECT_CODE_ENABLE;
    }
    else {
        rcv_msg_m_te.head.code = code;
        rcv_msg_m_te.head.status = status;
    }
    rcv_msg_m_te.body.n_remote_wait = n_active_ote;
    rcv_msg_m_te.body.onbord_seqno = n_active_ote;
    rcv_msg_m_te.body.remote_seqno = n_active_ote;
    rcv_msg_m_te.body.my_seqno = n_active_ote;

    return 0;
}

int CSimOTE::set_msg_m_te(int mode) {
    if (mode == ID_MULTI_MSG_SET_MODE_INIT) {
        rcv_msg_m_te.head.myid = 1;
        rcv_msg_m_te.head.code = ID_OTE_EVENT_CODE_CONST;
        rcv_msg_m_te.head.addr = addrin_u;
        rcv_msg_m_te.head.status = ID_OTE_CONNECT_CODE_NO_OPERATION;
        rcv_msg_m_te.head.nodeid = 0;

        for (int i = 0;i < N_CRANE_PC_MAX;i++) rcv_msg_m_te.body.pc_enable[i] = ID_PC_CONNECT_CODE_ENABLE;
    }
    rcv_msg_m_te.body.n_remote_wait = n_active_ote;
    rcv_msg_m_te.body.onbord_seqno = n_active_ote;
    rcv_msg_m_te.body.remote_seqno = n_active_ote;
    rcv_msg_m_te.body.my_seqno = n_active_ote;

    return 0;
}
//*********************************************************************************************
LRESULT CALLBACK CSimOTE::OteSimWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    HDC hdc;
    switch (msg) {
    case WM_DESTROY: {
        hWorkWnd = NULL;
    }return 0;
    case WM_CREATE: {

        InitCommonControls();//コモンコントロール初期化
        HINSTANCE hInst = GetModuleHandle(0);

        CreateWindowW(TEXT("STATIC"), L"STAT",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 25, 40, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"RCV",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 50, 40, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"SND",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 75, 40, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"Info",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 100, 40, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"Count",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 5, 130, 40, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);


        CreateWindowW(TEXT("STATIC"), L"UNI", 
            WS_CHILD | WS_VISIBLE | SS_LEFT,50, 5, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        hwndSTAT_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            50, 25, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_U, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"M-TE",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 205, 5, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        hwndSTAT_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            205, 25, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_TE, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"M-CR",
            WS_CHILD | WS_VISIBLE | SS_LEFT, 360, 5, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_LABEL_COM, hInst, NULL);
        hwndSTAT_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            360, 25, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_STAT_CR, hInst, NULL);

        hwndRCVMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            50, 50, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_RCV_U, hInst, NULL);
        hwndRCVMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            205, 50, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_RCV_TE, hInst, NULL);
        hwndRCVMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            360, 50, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_RCV_CR, hInst, NULL);

        hwndSNDMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            50, 75, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_SND_U, hInst, NULL);
        hwndSNDMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            205, 75, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_SND_TE, hInst, NULL);
        hwndSNDMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            360, 75, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_SND_CR, hInst, NULL);

        hwndINFMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            50, 100, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_U, hInst, NULL);
        hwndINFMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            205, 100, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_TE, hInst, NULL);
        hwndINFMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            360, 100, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_INF_CR, hInst, NULL);

        hwndCNT_U = CreateWindowW(TEXT("STATIC"), L"R:- S:-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            50, 130, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_U_CNT, hInst, NULL);
        hwndCNT_M_TE = CreateWindowW(TEXT("STATIC"), L"R:- S:-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            205, 130, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_TE_CNT, hInst, NULL);
        hwndCNT_M_CR = CreateWindowW(TEXT("STATIC"), L"R:- S:-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            360, 130, 150, 20, hwnd, (HMENU)ID_STATIC_OTE_SIM_VIEW_CR_CNT, hInst, NULL);


        if (init_sock_u(hwnd) == 0) {
            woMSG << L"SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();
        }
        else {
            woMSG << L"SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_U);wsMSG.clear();

            close_WorkWnd();
        }

        if (init_sock_m_te(hwnd) == 0) {
            woMSG << L"SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();
        }
        else {
            woMSG << L"SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_TE);wsMSG.clear();

            close_WorkWnd();
        }

        if (init_sock_m_cr(hwnd) == 0) {
            woMSG << L"SOCK OK";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
        }
        else {
            woMSG << L"SOCK ERR";
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG, ID_SOCK_CODE_CR);wsMSG.clear();
            close_WorkWnd();
        }

        //マルチキャスト送信電文初期値セット
        set_msg_m_te(ID_MULTI_MSG_SET_MODE_INIT);

        //マルチキャスト送信タイマ起動
        SetTimer(hwnd, ID_OTE_SIM_TIMER, OTE_SIM_SCAN_TIME, NULL);

    }break;
    case WM_TIMER: {

        send_msg_m_te();
        send_msg_u();

    }break;

    case ID_UDP_EVENT_U_SIM: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_u++;

            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_u, (char*)&snd_msg_u, sizeof(ST_UOTE_SND_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
            }
            else {
                ST_UOTE_SND_MSG msg = snd_msg_u;
                woMSG << L"RCVlen: " << nRtn ;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
                woMSG << L"RCVcnt :" << nRcv_u;
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

            nRtn = recvfrom(s_m_te, (char*)&rcv_msg_m_te, sizeof(ST_MOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
            }
            else {
                ST_MOTE_RCV_MSG msg = rcv_msg_m_te;
                woMSG << L"RCVlen: " << nRtn;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
                woMSG << L"RCVcnt :" << nRcv_te;
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

            nRtn = recvfrom(s_m_cr, (char*)&rcv_msg_m_cr, sizeof(ST_MOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
            }
            else {
                ST_MOTE_SND_MSG msg = rcv_msg_m;
                woMSG << L"RCVlen: " << nRtn;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
                woMSG << L"RCVcnt :" << nRcv_cr;
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
        // 選択されたメニューの解析:
        switch (wmId)
        {

        default: break;

        }
    }break;

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
void CSimOTE::tweet2rcvMSG(const std::wstring& srcw, int code) {
    switch (code) {
    case ID_SOCK_CODE_U:
        SetWindowText(hwndRCVMSG_U, srcw.c_str());
        break;
    case ID_SOCK_CODE_TE:
        SetWindowText(hwndRCVMSG_M_TE, srcw.c_str());
        break;
    case ID_SOCK_CODE_CR:
        SetWindowText(hwndRCVMSG_M_CR, srcw.c_str());
        break;
    default: break;
    }
    return;
};
void CSimOTE::tweet2sndMSG(const std::wstring& srcw, int code) {
    switch (code) {
    case ID_SOCK_CODE_U:
        SetWindowText(hwndSNDMSG_U, srcw.c_str());
        break;
    case ID_SOCK_CODE_TE:
        SetWindowText(hwndSNDMSG_M_TE, srcw.c_str());
        break;
    case ID_SOCK_CODE_CR:
        SetWindowText(hwndSNDMSG_M_CR, srcw.c_str());
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