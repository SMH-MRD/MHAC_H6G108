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

LPST_OTE_IO COteIF::pOTEio;
LPST_CRANE_STATUS COteIF::pCraneStat;
LPST_PLC_IO COteIF::pPLCio;
LPST_CS_INFO COteIF::pCSInf;
LPST_AGENT_INFO COteIF::pAgentInf;

ST_OTE_IO_WORK COteIF::ote_io_workbuf;

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
    pPLCioObj = new CSharedMem;
    pCSInfObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

  };
COteIF::~COteIF() {
    // 共有メモリオブジェクトの解放
    delete pOteIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pPLCioObj;
    delete pCSInfObj;
    delete pAgentInfObj;
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

    if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, sizeof(ST_PLC_IO), MUTEX_PLC_IO_NAME)) {
        mode |= OTE_IF_PLC_MEM_NG;
    }

    if (OK_SHMEM != pCSInfObj->create_smem(SMEM_CS_INFO_NAME, sizeof(ST_CS_INFO), MUTEX_CS_INFO_NAME)) {
        mode |= OTE_IF_PLC_MEM_NG;
    }

    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)) {
        mode |= OTE_IF_PLC_MEM_NG;
    }

    pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
    pPLCio = (LPST_PLC_IO)(pPLCioObj->get_pMap());
    pOTEio = (LPST_OTE_IO)pOteIOObj->get_pMap();
    pCSInf = (LPST_CS_INFO )pCSInfObj->get_pMap();
    pAgentInf = (LPST_AGENT_INFO)pAgentInfObj->get_pMap();
    
    
    pOTEio->OTEsim_status = L_OFF;          //端末シミュレーションモードOFF

    ote_io_workbuf.te_connect_time_limit = TE_CONNECT_TIMEOVER_MS / ID_WORK_WND_TIMER;
    ote_io_workbuf.te_multi_snd_cycle = MULTI_SND_SCAN_TIME_MS / ID_WORK_WND_TIMER;

    //デバッグモード　ON　製番ではOFFで初期化
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif

    return 0; 
}
int COteIF::input() {                            //入力処理
    ote_io_workbuf.ote_io.OTEsim_status = ((LPST_OTE_IO)poutput)->OTEsim_status;//OTE Simuratorの稼働状態

    return 0; 
} 
int COteIF::parse() { 
    for (int i = 0;i < SEMI_AUTO_TARGET_MAX;i++) {
        if (ote_io_workbuf.ote_io.rcv_msg_u.body.pb[ID_PB_SEMI_AUTO_S1 + i]) ote_io_workbuf.ote_io.ui.PBsemiauto[i] = PLC_IO_OFF_DELAY_COUNT;
        else if (ote_io_workbuf.ote_io.ui.PBsemiauto[i] > 0)ote_io_workbuf.ote_io.ui.PBsemiauto[i]--;
        else;
     }

    return 0;
}               //メイン処理
int COteIF::output() {                          //出力処理

   if (out_size) { //出力処理
       memcpy_s(poutput, out_size, &ote_io_workbuf.ote_io, out_size);
   }

   return 0; 
}

static struct ip_mreq mreq_te, mreq_cr;                     //マルチキャスト受信設定用構造体
static int serverlen, nEvent;
static int nRtn = 0, nRcv_u = 0, nRcv_te = 0, nRcv_cr = 0, nSnd_u = 0, nSnd_m = 0;
static int lRcv_u = 0, lRcv_te = 0, lRcv_cr = 0, lSnd_u = 0, lSnd_m = 0;


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
/************ クレーンユニキャスト受信ソケット初期化　*************/
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
  

    //送信メッセージ初期化
    set_msg_u(ID_MSG_SET_MODE_INIT,ID_OTE_EVENT_CODE_CONNECTED);
 
    return 0;
 }

/************ 端末マルチキャスト送信ソケット初期化　***************/
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

/************ クレーンマルチキャスト受信ソケット初期化　***********/
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
    mreq_cr.imr_interface.S_un.S_addr = inet_addr(CTRL_PC_IP_ADDR_OTE);     //利用ネットワーク
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

    //送信メッセージ初期化
    set_msg_m_cr(ID_MSG_SET_MODE_INIT, ID_OTE_EVENT_CODE_CONST);

    return 0;
}

/************ ユニキャスト送信　***********************************/
/*### 送信関数 　　　　　　　　###*/
int COteIF::send_msg_u() {
 
    int n = sizeof(ST_UOTE_SND_MSG);

    nRtn = sendto(s_u, reinterpret_cast<const char*> (&ote_io_workbuf.ote_io.snd_msg_u), n, 0, (LPSOCKADDR)&addrin_ote_u, sizeof(addrin_ote_u));

    if (nRtn == n) {
        nSnd_u++;
        lSnd_u = nRtn;
        woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L"  Snd n:" << nSnd_u << L" l:" << lSnd_u;
        tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

        woMSG << L"ID:" << ote_io_workbuf.ote_io.snd_msg_u.head.myid << L" CD:" << ote_io_workbuf.ote_io.snd_msg_u.head.code;
        sockaddr_in* psockaddr = (sockaddr_in*)&ote_io_workbuf.ote_io.snd_msg_u.head.addr;
        woMSG << L" IP:" << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
        woMSG << L" PORT: " << htons(psockaddr->sin_port);
        woMSG << L" ST:" << ote_io_workbuf.ote_io.snd_msg_u.head.status << L" ID:" << ote_io_workbuf.ote_io.snd_msg_u.head.tgid;
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L" SOCKET ERROR: CODE ->   " << WSAGetLastError();
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
    }
    else {
        woMSG << L" sendto size ERROR ";
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
    }
 
    return nRtn;
}

/*### 送信メッセージセット関数 ###*/
int COteIF::set_msg_u(int mode, INT32 code) {                                //ユニキャスト送信メッセージセット(初期化用）
   
   //Header部
    if (mode) {
        ote_io_workbuf.ote_io.snd_msg_u.head.addr = addrin_u;
        ote_io_workbuf.ote_io.snd_msg_u.head.myid = pCraneStat->spec.device_code.no;
    }
    ote_io_workbuf.ote_io.snd_msg_u.head.tgid = ote_io_workbuf.id_connected_te;;
    ote_io_workbuf.ote_io.snd_msg_u.head.code = code;
    ote_io_workbuf.ote_io.snd_msg_u.head.status = ote_io_workbuf.status_connected_te;

    //Body部
    //ランプ
    for (int i = 0;i < N_UI_LAMP;i++) ote_io_workbuf.ote_io.snd_msg_u.body.lamp[i] = pCSInf->ui_lamp[i];
    //ノッチ指令
    for (int i = 0;i < MOTION_ID_MAX;i++) ote_io_workbuf.ote_io.snd_msg_u.body.notch_pos[i] = pPLCio->ui.notch_pos[i];
    //各軸位置
    for (int i = 0;i < MOTION_ID_MAX;i++) ote_io_workbuf.ote_io.snd_msg_u.body.pos[i] = (INT32)(pPLCio->status.pos[i] * 1000.0);
    //各軸速度FB
    for (int i = 0;i < MOTION_ID_MAX;i++) ote_io_workbuf.ote_io.snd_msg_u.body.v_fb[i] = (INT32)(pPLCio->status.v_fb[i] * 1000.0);
    //各軸速度指令
    for (int i = 0;i < MOTION_ID_MAX;i++) ote_io_workbuf.ote_io.snd_msg_u.body.v_ref[i] = (INT32)(pPLCio->status.v_ref[i] * 1000.0);

    //吊荷位置
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_pos[0] = (INT32)(pCraneStat->rl.x * 1000.0);
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_pos[1] = (INT32)(pCraneStat->rl.y * 1000.0);
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_pos[2] = (INT32)(pCraneStat->rl.z * 1000.0);

    //吊荷速度
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_v_fb[0] = 0;
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_v_fb[1] = 0;
    ote_io_workbuf.ote_io.snd_msg_u.body.ld_v_fb[2] = 0;

    //自動目標位置
    ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos[0] = (INT32)(pAgentInf->auto_pos_target.pos[ID_BOOM_H] * 1000.0);
    ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos[1] = (INT32)(pAgentInf->auto_pos_target.pos[ID_SLEW] * 1000.0);
    ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos[2] = (INT32)(pAgentInf->auto_pos_target.pos[ID_HOIST] * 1000.0);

    //半自動目標位置
    for (int i = 0;i < 6;i++) {
        ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos_semi[i][0] = (INT32)(pCSInf->semi_auto_setting_target[i].pos[ID_BOOM_H] * 1000.0);
        ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos_semi[i][1] = (INT32)(pCSInf->semi_auto_setting_target[i].pos[ID_SLEW] * 1000.0);
        ote_io_workbuf.ote_io.snd_msg_u.body.tg_pos_semi[i][2] = (INT32)(pCSInf->semi_auto_setting_target[i].pos[ID_HOIST] * 1000.0);
    }

    //PLCデータ
    for (int i = 0;i < PLC_IO_MONT_WORD_NUM;i++) ote_io_workbuf.ote_io.snd_msg_u.body.plc_data[i] = pPLCio->plc_data[i];
    
    return 0; 
}                 

/*********** マルチキャスト送信　***********************************/
/*### 送信関数 　　　　　　　　###*/
int COteIF::send_msg_m() {
    //送信メッセージセット
    int n = sizeof(ST_MOTE_SND_MSG);
    nRtn = sendto(s_m_snd, reinterpret_cast<const char*> (&ote_io_workbuf.ote_io.snd_msg_m), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));
    nRtn = sendto(s_m_snd_dbg, reinterpret_cast<const char*> (&ote_io_workbuf.ote_io.snd_msg_m), n, 0, (LPSOCKADDR)&addrin_m_snd, sizeof(addrin_m_snd));
    woMSG.str(L"");
    if (nRtn == n) {
        nSnd_m++;
        lSnd_m = nRtn;
        woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L"  Snd n:" << nSnd_u << L" l:" << lSnd_u;
        tweet2infMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();

        woMSG << L"ID:" << ote_io_workbuf.ote_io.snd_msg_m.head.myid << L" CD:" << ote_io_workbuf.ote_io.snd_msg_m.head.code;
        sockaddr_in* psockaddr = (sockaddr_in*)&ote_io_workbuf.ote_io.snd_msg_m.head.addr;
        woMSG << L" IP:" << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
        woMSG << L" PORT: " << htons(psockaddr->sin_port);
        woMSG << L" ST:" << ote_io_workbuf.ote_io.snd_msg_m.head.status << L" ID:" << ote_io_workbuf.ote_io.snd_msg_m.head.tgid;
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();

    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L"ERR CODE ->" << WSAGetLastError();
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
    }
    else {
        woMSG << L" sendto size ERROR ";
        tweet2sndMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
    }

    return nRtn;
}

/*### 送信メッセージセット関数 ###*/
int COteIF::set_msg_m_cr(int mode, INT32 code) {                         //マルチキャスト送信メッセージセット(初期化用）
    ote_io_workbuf.ote_io.snd_msg_m.head.addr = addrin_u;
    ote_io_workbuf.ote_io.snd_msg_m.head.myid = pCraneStat->spec.device_code.no;
    ote_io_workbuf.ote_io.snd_msg_m.head.tgid = ote_io_workbuf.id_connected_te;
   
    ote_io_workbuf.ote_io.snd_msg_m.head.code = code;
    ote_io_workbuf.ote_io.snd_msg_m.head.status = ote_io_workbuf.status_connected_te;
    ote_io_workbuf.ote_io.snd_msg_m.head.status = pCraneStat->OTE_req_status;

    return 0;
}   

static int ote_req_last;

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
        SetTimer(hwnd, ID_WORK_WND_TIMER, OTE_IO_TIME_CYCLE_MS, NULL);

    }break;
    case WM_TIMER: {

        if (ote_io_workbuf.te_connect_chk_counter > 0) {
            ote_io_workbuf.te_connect_chk_counter--;
        }
        else {
            ote_io_workbuf.id_connected_te = 0;                     //接続中OTEクリア
        }
       
        if (ote_io_workbuf.te_connect_chk_counter > 0) {            //定周期送信待機タイミング
            ote_io_workbuf.te_connect_chk_counter--;
            if (ote_req_last != pCraneStat->OTE_req_status) {       //イベント送信条件発生
                set_msg_m_cr(ID_MSG_SET_MODE_CONST, ID_OTE_EVENT_CODE_STAT_REPORT);
                send_msg_m();
            }
        }
        else {                                              //定周期送信タイミング
            set_msg_m_cr(ID_MSG_SET_MODE_CONST, ID_OTE_EVENT_CODE_CONST);
            send_msg_m();
            ote_io_workbuf.te_connect_chk_counter = ote_io_workbuf.te_multi_snd_cycle;
        }

        //OTEの接続チェックカウンタデクリメント→0以下で切断判定
        if(ote_io_workbuf.ote_io.OTE_healty>0)ote_io_workbuf.ote_io.OTE_healty--;
        ote_req_last = pCraneStat->OTE_req_status;

    }break;

    case ID_UDP_EVENT_U: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv_u++;

            SOCKADDR from_addr;                             //送信元アドレス取り込みバッファ
            int from_addr_size = (int)sizeof(from_addr);    //送信元アドレスサイズバッファ

            nRtn = recvfrom(s_u, (char*)&ote_io_workbuf.ote_io.rcv_msg_u, sizeof(ST_UOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();
            }
            else {
                addrin_ote_u.sin_addr = ((sockaddr_in*)&from_addr)->sin_addr;
                addrin_ote_u.sin_port = ote_io_workbuf.ote_io.rcv_msg_u.head.addr.sin_port;

                int p = htons(addrin_ote_u.sin_port);
                set_msg_u(0,0);
                send_msg_u();

                woMSG << L"SOCK OK";
                woMSG << L"  From IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
 
                tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L""); woMSG.clear();

                woMSG << L"ID:" << ote_io_workbuf.ote_io.rcv_msg_u.head.myid << L" CD:" << ote_io_workbuf.ote_io.rcv_msg_u.head.code;
                psockaddr = (sockaddr_in*)&ote_io_workbuf.ote_io.rcv_msg_u.head.addr;
                woMSG << L" IP:" << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                woMSG << L" PORT: " << htons(psockaddr->sin_port);
                woMSG << L" ST:" << ote_io_workbuf.ote_io.rcv_msg_u.head.status << L" ID:" << ote_io_workbuf.ote_io.rcv_msg_u.head.tgid;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

                lRcv_u = nRtn;
                woMSG << L"Rcv n:" << nRcv_u << L" l:" << lRcv_u << L"  Snd n:" << nSnd_u << L" l:" << lSnd_u;
                tweet2infMSG(woMSG.str(), ID_SOCK_CODE_U); woMSG.str(L"");woMSG.clear();

                ote_io_workbuf.ote_io.OTE_healty = ote_io_workbuf.te_connect_time_limit;
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

                nRtn = recvfrom(s_m_te, (char*)&ote_io_workbuf.ote_io.rcv_msg_m_te, sizeof(ST_MOTE_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

                sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

                woMSG << L"SOCK OK";
                woMSG << L"  From IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;

                tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L""); woMSG.clear();
                
                
                if (nRtn == SOCKET_ERROR) {
                    woMSG << L"recvfrom ERROR";
                    tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();
                }
                else {

                    lRcv_te = nRtn;
                    woMSG << L"Rcv n:" << nRcv_te << L" l:" << lRcv_te ;
                    tweet2infMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();

                    woMSG << L"ID:" << ote_io_workbuf.ote_io.rcv_msg_m_te.head.myid << L" CD:" << ote_io_workbuf.ote_io.rcv_msg_m_te.head.code;
                    psockaddr = (sockaddr_in*)&ote_io_workbuf.ote_io.rcv_msg_m_te.head.addr;
                    woMSG << L" IP:" << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                    woMSG << L" PORT: " << htons(psockaddr->sin_port);
                    woMSG << L" ST:" << ote_io_workbuf.ote_io.rcv_msg_m_te.head.status << L" ID:" << ote_io_workbuf.ote_io.rcv_msg_m_te.head.tgid;
                    tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_TE); woMSG.str(L"");woMSG.clear();


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

            nRtn = recvfrom(s_m_cr, (char*)&ote_io_workbuf.ote_io.rcv_msg_m_cr, sizeof(ST_MOTE_SND_MSG), 0, (SOCKADDR*)&from_addr, &from_addr_size);

            sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;

            woMSG << L"SOCK OK";
            woMSG << L"  From IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
           // woMSG << L" PORT: " << htons(psockaddr->sin_port);
            tweet2statusMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L""); woMSG.clear();
  
            if (nRtn == SOCKET_ERROR) {
                woMSG << L"recvfrom ERROR";
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();
            }
            else {
 
                woMSG << L"ID:" << ote_io_workbuf.ote_io.rcv_msg_m_cr.head.myid << L" CD:" << ote_io_workbuf.ote_io.rcv_msg_m_cr.head.code;
                psockaddr = (sockaddr_in*)&ote_io_workbuf.ote_io.rcv_msg_m_cr.head.addr;
                woMSG << L" IP:" << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                woMSG << L" PORT: " << htons(psockaddr->sin_port);
                woMSG << L" ST:" << ote_io_workbuf.ote_io.rcv_msg_m_cr.head.status << L" ID:" << ote_io_workbuf.ote_io.rcv_msg_m_cr.head.tgid;
                tweet2rcvMSG(woMSG.str(), ID_SOCK_CODE_CR); woMSG.str(L"");woMSG.clear();

                lRcv_cr = nRtn;
                woMSG << L"Rcv n:" << nRcv_cr << L" l:" << lRcv_cr << L"  Snd n:" << nSnd_m << L"  l:" << lSnd_m;
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