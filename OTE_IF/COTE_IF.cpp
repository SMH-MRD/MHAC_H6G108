#include "COTE_IF.h"
#include <windowsx.h>       //# コモンコントロール

#include <winsock2.h>

#include <iostream>
#include <iomanip>
#include <sstream>

HWND COteIF::hWorkWnd = NULL;

//Work Window表示用
HWND COteIF::hwndSTAT_U;
HWND COteIF::hwndRCVMSG_U;
HWND COteIF::hwndSNDMSG_U;
HWND COteIF::hwndINFMSG_U;

HWND COteIF::hwndSTAT_M_TE;
HWND COteIF::hwndRCVMSG_M_TE;
HWND COteIF::hwndSNDMSG_M_TE;
HWND COteIF::hwndINFMSG_M_TE;

HWND COteIF::hwndSTAT_M_CR;
HWND COteIF::hwndRCVMSG_M_CR;
HWND COteIF::hwndSNDMSG_M_CR;
HWND COteIF::hwndINFMSG_M_CR;


ST_OTE_IO COteIF::ote_io_workbuf;

//IF用ソケット
WSADATA COteIF::wsaData;
SOCKET COteIF::s_u;                                         //ユニキャスト受信ソケット
SOCKET COteIF::s_m_te, COteIF::s_m_cr;                      //マルチキャスト受信ソケット
SOCKET COteIF::s_m_snd, COteIF::s_m_snd_dbg;                                         //マルチキャスト送信ソケット
SOCKADDR_IN COteIF::addrin_u;                               //ユニキャスト受信アドレス
SOCKADDR_IN COteIF::addrin_ote_u;                           //ユニキャスト送信アドレス
SOCKADDR_IN COteIF::addrin_m_te, COteIF::addrin_m_cr;       //マルチキャスト受信アドレス
SOCKADDR_IN COteIF::addrin_m_snd;                           //マルチキャスト送信アドレス

u_short COteIF::port_u = OTE_IF_IP_UNICAST_PORT_S;          //ユニキャスト受信ポート
u_short COteIF::port_m_te = OTE_IF_IP_MULTICAST_PORT_TE;
u_short COteIF::port_m_cr = OTE_IF_IP_MULTICAST_PORT_CR;    //マルチキャスト受信ポート

std::wostringstream COteIF::woMSG;
std::wstring COteIF::wsMSG;

COteIF::COteIF() {

    // 共有メモリオブジェクトのインスタンス化
    pOteIOObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
  };
COteIF::~COteIF() {
    // 共有メモリオブジェクトの解放
    delete pOteIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
};

int COteIF::set_outbuf(LPVOID pbuf) { 
    poutput = pbuf;return 0;
    return 0; 

}    //出力バッファセット

/*****************************************************************************/
/*初期化処理                                                                 */
/*****************************************************************************/
int COteIF::init_proc() {
    // 共有メモリ取得

    // 出力用共有メモリ取得
    out_size = sizeof(ST_OTE_IO);
    if (OK_SHMEM != pOteIOObj->create_smem(SMEM_OTE_IO_NAME, out_size, MUTEX_OTE_IO_NAME)) {
        mode |= OTE_IF_OTE_IO_MEM_NG;
    }
    set_outbuf(pOteIOObj->get_pMap());

    // 入力用共有メモリ取得
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= OTE_IF_SIM_MEM_NG;
    }

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= OTE_IF_CRANE_MEM_NG;
    }

    LPST_OTE_IO pOTEio = (LPST_OTE_IO)pOteIOObj->get_pMap();
    pOTEio->OTEsim_status = L_OFF;          //端末シミュレーションモードOFF

    //デバッグモード　ON　製番ではOFFで初期化
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif

    return 0; 
}
int COteIF::input() {                            //入力処理
    ote_io_workbuf.OTEsim_status = ((LPST_OTE_IO)poutput)->OTEsim_status;//OTE Simuratorの稼働状態
    return 0; 
} 
int COteIF::parse() { return 0; }               //メイン処理
int COteIF::output() {                          //出力処理

    ((LPST_OTE_IO)poutput)->rcv_msg_m_cr = ote_io_workbuf.rcv_msg_m_cr;
    ((LPST_OTE_IO)poutput)->rcv_msg_m_te = ote_io_workbuf.rcv_msg_m_te;
    ((LPST_OTE_IO)poutput)->rcv_msg_u = ote_io_workbuf.rcv_msg_u;
    ((LPST_OTE_IO)poutput)->snd_msg_m = ote_io_workbuf.snd_msg_m;
    ((LPST_OTE_IO)poutput)->snd_msg_u = ote_io_workbuf.snd_msg_u;

    return 0; 
}



static struct ip_mreq mreq_te, mreq_cr;                     //マルチキャスト受信設定用構造体
static int serverlen, nEvent;
static int nRtn = 0, nRcv_u = 0, nRcv_te = 0, nRcv_cr = 0, nSnd_u = 0, nSnd_m = 0;

static char szBuf[512];

//*********************************************************************************************
/*モニタ用ウィンドウ生成関数*/
HWND COteIF::open_WorkWnd(HWND hwnd_parent) {
    InitCommonControls();//コモンコントロール初期化

    WNDCLASSEX wc;

    hInst = GetModuleHandle(0);

    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WorkWndProc;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInst;
    wc.hIcon = NULL;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = TEXT("WorkWnd");
    wc.hIconSm = NULL;
    ATOM fb = RegisterClassExW(&wc);

    hWorkWnd = CreateWindow(TEXT("WorkWnd"),
        TEXT("OTE IF COMM_CHK"),
        WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H,
        hwnd_parent,
        0,
        hInst,
        NULL);

    ShowWindow(hWorkWnd, SW_SHOW);
    UpdateWindow(hWorkWnd);

    return hWorkWnd;
}
//*********************************************************************************************
int COteIF::close_WorkWnd() {
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
int COteIF::init_sock_u(HWND hwnd) {    //ユニキャスト
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
    addrin_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_S);
    addrin_u.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_u.sin_addr.s_addr);

    nRtn = bind(s_u, (LPSOCKADDR)&addrin_u, (int)sizeof(addrin_u)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_u);
        WSACleanup();
        return -3;
    }

    nRtn = WSAAsyncSelect(s_u, hwnd, ID_UDP_EVENT_U, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_u);
        WSACleanup();
        return -4;
    }


    //# 送信先アドレスdefault設定
    memset(&addrin_ote_u, 0, sizeof(addrin_ote_u));
    addrin_ote_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_C);
    addrin_ote_u.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_ote_u.sin_addr.s_addr);
      
    return 0;
 }
int COteIF::init_sock_m_te(HWND hwnd) {
 
    //マルチキャスト用受信ソケット（操作端末グループ）
    //ターミナル情報受信用
    s_m_te = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_te < 0) {
        perror("socket失敗\n");
        return -5;
    }
    memset(&addrin_m_te, 0, sizeof(addrin_m_te));
    addrin_m_te.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_TE);
    addrin_m_te.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_m_te.sin_addr.s_addr);

    nRtn = bind(s_m_te, (LPSOCKADDR)&addrin_m_te, (int)sizeof(addrin_m_te)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_m_te);
        WSACleanup();
        return -6;
    }

    nRtn = WSAAsyncSelect(s_m_te, hwnd, ID_UDP_EVENT_M_TE, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_m_te);
        WSACleanup();
        return -7;
    }

    //マルチキャストグループ参加登録
    memset(&mreq_te, 0, sizeof(mreq_te));
    mreq_te.imr_interface.S_un.S_addr = inet_addr(CTRL_PC_IP_ADDR_OTE);     //パケット出力元IPアドレス
    mreq_te.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);       //マルチキャストIPアドレス
    if (setsockopt(s_m_te, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_te, sizeof(mreq_te)) != 0) {
        perror("setopt受信設定失敗\n");
        return -8;
    }
    return 0;
}
int COteIF::init_sock_m_cr(HWND hwnd) {
 
    //マルチキャス受信用ソケット（クレーングループ）
    //制御PC情報受信用
    s_m_cr = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_cr < 0) {
        perror("socket失敗\n");
        return -9;
    }
    memset(&addrin_m_cr, 0, sizeof(addrin_m_cr));
    addrin_m_cr.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_CR);
    addrin_m_cr.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_m_cr.sin_addr.s_addr);

    nRtn = bind(s_m_cr, (LPSOCKADDR)&addrin_m_cr, (int)sizeof(addrin_m_cr)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_m_cr);
        WSACleanup();
        return -10;
    }

    nRtn = WSAAsyncSelect(s_m_cr, hwnd, ID_UDP_EVENT_M_CR, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_m_cr);
        WSACleanup();
        return -11;
    }

    //マルチキャストグループ参加登録
    memset(&mreq_cr, 0, sizeof(mreq_cr));
    mreq_cr.imr_interface.S_un.S_addr = inet_addr(CTRL_PC_IP_ADDR_OTE);     //パケット出力元IPアドレス
    mreq_cr.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);       //マルチキャストIPアドレス
    if (setsockopt(s_m_cr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_cr, sizeof(mreq_cr)) != 0) {
        perror("setopt受信設定失敗\n");
        return -12;
    }

     
    //# 送信先アドレス設定
    memset(&addrin_m_snd, 0, sizeof(addrin_m_snd));
    addrin_m_snd.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_CR);
    addrin_m_snd.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_MULTI_IP_ADDR, &addrin_m_snd.sin_addr.s_addr);

    //# 送信ソケット設定
    s_m_snd = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_snd < 0) {
        perror("socket失敗\n");
        return -33;
    }
   // DWORD ipaddr = inet_addr(CTRL_PC_IP_ADDR_OTE);
    DWORD ipaddr = inet_addr(CTRL_PC_IP_ADDR_OTE);
    //出力インターフェイス指定
    if (setsockopt(s_m_snd,IPPROTO_IP,IP_MULTICAST_IF,(char*)&ipaddr, sizeof(ipaddr)) != 0) {
        printf("setsockopt : %d\n", WSAGetLastError());
        return -13;
    }


    //デバッグ用
    s_m_snd_dbg = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m_snd_dbg < 0) {
        perror("socket失敗\n");
        return -34;
    }
    // DWORD ipaddr = inet_addr(CTRL_PC_IP_ADDR_OTE);
    ipaddr = inet_addr(OTE_DEFAULT_IP_ADDR);
    //出力インターフェイス指定
    if (setsockopt(s_m_snd_dbg, IPPROTO_IP, IP_MULTICAST_IF, (char*)&ipaddr, sizeof(ipaddr)) != 0) {
        printf("setsockopt : %d\n", WSAGetLastError());
        return -13;
    }


    return 0;
}

//*********************************************************************************************
int COteIF::send_msg_u() {
 
    int n = sizeof(ST_UOTE_SND_MSG);

    nRtn = sendto(s_u, reinterpret_cast<const char*> (&ote_io_workbuf.snd_msg_u), n, 0, (LPSOCKADDR)&addrin_ote_u, sizeof(addrin_ote_u));

    if (nRtn == n) {
        nSnd_u++;
        woMSG << L" SND len: " << nRtn << L"  Count :" << nSnd_m << L"    OTE:" << ote_io_workbuf.snd_msg_u.head.myid;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L" SOCKET ERROR: CODE ->   " << WSAGetLastError();
    }
    else {
        woMSG << L" sendto size ERROR ";
    }
    tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

    return nRtn;
}

int COteIF::send_msg_m() {
    int n = sizeof(ST_MOTE_SND_MSG);

    nRtn = sendto(s_m_snd, reinterpret_cast<const char*> (&ote_io_workbuf.snd_msg_m), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));
    nRtn = sendto(s_m_snd_dbg, reinterpret_cast<const char*> (&ote_io_workbuf.snd_msg_m), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));
    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_m++;
        woMSG << L"SNDlen: " << nRtn;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L"ERR CODE ->" << WSAGetLastError();
    }
    else {
        woMSG << L" sendto size ERROR ";
    }
    tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();

    woMSG.str(L"");
    woMSG << L"SND" << nSnd_m;
    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();

    return nRtn;
}

//*********************************************************************************************
LRESULT CALLBACK COteIF::WorkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    HDC hdc;
    switch (msg) {
    case WM_DESTROY: {
        hWorkWnd = NULL;
        closesocket(s_u);
        closesocket(s_m_te);
        closesocket(s_m_cr);
        closesocket(s_m_snd);
        closesocket(s_m_snd_dbg);

    }return 0;
    case WM_CREATE: {

        InitCommonControls();//コモンコントロール初期化
        HINSTANCE hInst = GetModuleHandle(0);
 
        CreateWindowW(TEXT("STATIC"), L"UNI", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 5, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSTAT_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 5, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_STAT_U, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"RCV  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 30, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndRCVMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 30, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_RCV_U, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"SND  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 55, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSNDMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 55, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_SND_U, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"Info ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 80, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndINFMSG_U = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 80, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_INF_U, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"M-TE", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 110, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSTAT_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 110, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_STAT_TE, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"RCV  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 135, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndRCVMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 135, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_RCV_TE, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"SND  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 160, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSNDMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 160, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_SND_TE, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"Info ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 185, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndINFMSG_M_TE = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 185, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_INF_TE, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"M-CR", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 215, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSTAT_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 215, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_STAT_CR, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"RCV  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 240, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndRCVMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 240, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_RCV_CR, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"SND  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 265, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndSNDMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 265, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_SND_CR, hInst, NULL);

        CreateWindowW(TEXT("STATIC"), L"Info ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 290, 55, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_LABEL_COM, hInst, NULL);
        hwndINFMSG_M_CR = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 290, 440, 20, hwnd, (HMENU)ID_STATIC_OTE_IF_VIEW_INF_CR, hInst, NULL);

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


        //振れセンサ送信タイマ起動
        SetTimer(hwnd, ID_WORK_WND_TIMER, MULTI_SND_SCAN_TIME, NULL);

    }break;
    case WM_TIMER: {
        send_msg_m();
    }break;

    case ID_UDP_EVENT_U: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_u++;

            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_u, (char*)&ote_io_workbuf.rcv_msg_u, sizeof(ST_UOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
            }
            else {
                woMSG << L"RCVlen: " << nRtn;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
                woMSG << L"RCVcnt :" << nRcv_u;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

                //woMSG << L"\n IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                //woMSG << L" PORT: " << psockaddr->sin_port;

                addrin_ote_u.sin_addr = ((sockaddr_in*)&from_addr)->sin_addr;
                send_msg_u();
            }
        }break;
        case FD_WRITE: {

        }break;
        case FD_CLOSE: {
            ;
        }break;
        }
    }break;
    case ID_UDP_EVENT_M_TE: {
            nEvent = WSAGETSELECTEVENT(lp);
            switch (nEvent) {
            case FD_READ: {
                nRcv_te++;
                SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
                int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

                nRtn = recvfrom(s_m_te, (char*)&ote_io_workbuf.rcv_msg_m_te, sizeof(ST_MOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

                sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

                if (nRtn == SOCKET_ERROR) {
                    woMSG << L"recvfrom ERROR";
                    tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
                }
                else {
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
    case ID_UDP_EVENT_M_CR: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_cr++;
            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_m_cr, (char*)&ote_io_workbuf.snd_msg_m, sizeof(ST_MOTE_SND_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
            }
            else {
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
void COteIF::tweet2statusMSG(const std::wstring& srcw, int code) {
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
void COteIF::tweet2rcvMSG(const std::wstring& srcw, int code) {
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
void COteIF::tweet2sndMSG(const std::wstring& srcw, int code) {
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
void COteIF::tweet2infMSG(const std::wstring& srcw, int code) {
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