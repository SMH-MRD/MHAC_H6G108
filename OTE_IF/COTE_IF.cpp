#include "COTE_IF.h"
#include <windowsx.h>       //# コモンコントロール

#include <winsock2.h>

#include <iostream>
#include <iomanip>
#include <sstream>

HWND COteIF::hWorkWnd;

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

int COteIF::set_outbuf(LPVOID) { return 0; }    //出力バッファセット
int COteIF::init_proc() { return 0; }           //初期化処理
int COteIF::input() { return 0; }               //入力処理
int COteIF::parse() { return 0; }               //メイン処理
int COteIF::output() { return 0; }              //出力処理

//IF用ソケット
static WSADATA wsaData;
static SOCKET s_u, s_m;
static SOCKADDR_IN from_u, fram_m;              //送信ポートアドレス
static SOCKADDR_IN addrin_u, addrin_m;          //受信ポートアドレス
static struct ip_mreq mreq;                     //マルチキャスト受信設定用構造体
static int serverlen, nEvent;
static int nRtn = 0, nRcv = 0, nSnd = 0;
static u_short port_u = OTE_IF_IP_UNICAST_PORT_C, port_m = OTE_IF_IP_MULTICAST_PORT;
static char szBuf[512];

std::wostringstream woMSG;
std::wstring wsMSG;

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
        TEXT("SWAY SENSOR IF COMM_CHK"),
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
    closesocket(s_m);
    WSACleanup();
    DestroyWindow(hWorkWnd);  //ウィンドウ破棄
    hWorkWnd = NULL;
    return 0;
}
//*********************************************************************************************
int COteIF::init_sock(HWND hwnd) {
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {    //WinSockの初期化
        perror("WSAStartup Error\n");
        return -1;
    }

    s_u = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_u < 0) {
        perror("socket失敗\n");
        return -2;
    }
    memset(&addrin_u, 0, sizeof(addrin_u));


    addrin_u.sin_port = htons(port_u);


    addrin_u.sin_family = AF_INET;


    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_u.sin_addr.s_addr);
 

    //ユニキャスト用ソケット
    nRtn = bind(s_u, (LPSOCKADDR)&addrin_u, (int)sizeof(addrin_u)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s_u);
        WSACleanup();
        return -3;
    }

    nRtn = WSAAsyncSelect(s_u, hwnd, ID_UDP_EVENT, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s_u);
        WSACleanup();
        return -4;
    }


    //マルチキャスト受信設定
    memset(&mreq, 0, sizeof(mreq));
    mreq.imr_interface.S_un.S_addr = inet_addr(CTRL_PC_IP_ADDR_OTE);
    mreq.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);

    if (setsockopt(s_u,IPPROTO_IP,IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(mreq)) != 0) {
        perror("setopt受信設定失敗\n");
        return -5;
    }



    //マルチキャスト送信設定
    s_m = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s_m < 0) {
        perror("socket失敗\n");
        return -2;
    }
    memset(&addrin_m, 0, sizeof(addrin_m));
    addrin_m.sin_port = htons(port_m);
    addrin_m.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_MULTI_IP_ADDR, &addrin_m.sin_addr.s_addr);
    DWORD ipaddr = inet_addr(CTRL_PC_IP_ADDR_OTE);

    if (setsockopt(s_m, IPPROTO_IP, IP_MULTICAST_IF, (char*)&ipaddr, sizeof(ipaddr)) != 0) {
        perror("setopt送信設定失敗\n");
        return -6;
    }

    return 0;
    ;
}

//*********************************************************************************************
LRESULT CALLBACK COteIF::WorkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

    HDC hdc;
    switch (msg) {
    case WM_DESTROY: {
        hWorkWnd = NULL;
    }return 0;
    case WM_CREATE: {

        InitCommonControls();//コモンコントロール初期化
        HINSTANCE hInst = GetModuleHandle(0);
        /*
        CreateWindowW(TEXT("STATIC"), L"STATUS", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 5, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        hwndSTATMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 5, 440, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"RCV  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 30, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        hwndRCVMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 30, 440, 40, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"SND  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 75, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_SND, hInst, NULL);
        hwndSNDMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 75, 440, 40, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"Info ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 120, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_SND, hInst, NULL);
        hwndINFMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 120, 440, 280, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);

        if (init_sock(hwnd) == 0) {
            woMSG << L"SOCK OK";
            tweet2statusMSG(woMSG.str()); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG);wsMSG.clear();
        }
        else {
            tweet2statusMSG(woMSG.str()); woMSG.str(L""); woMSG.clear();
            wsMSG = L"No RCV MSG";
            tweet2rcvMSG(wsMSG);wsMSG.clear();
            wsMSG = L"No SND MSG";
            tweet2sndMSG(wsMSG);wsMSG.clear();

            close_WorkWnd();
        }

        iDispCam = iDispBuf = 0;
        hwndDispBufMSG = CreateWindowW(TEXT("STATIC"), L"ID:  1 BUF:0 CAM:1 TG:1    NEXT->", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 408, 220, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_DISP_SELBUF, hInst, NULL);

        hwndCamChangePB = CreateWindow(L"BUTTON", L"ID", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            320, 403, 30, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_SENSOR, hInst, NULL);

        hwndBufChangePB = CreateWindow(L"BUTTON", L"BUF", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            355, 403, 40, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_BUF, hInst, NULL);

        hwndBufChangePB = CreateWindow(L"BUTTON", L"CAM", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            400, 403, 40, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_CAM, hInst, NULL);

        hwndTargetChangePB = CreateWindow(L"BUTTON", L"TG", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            445, 403, 40, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_TG, hInst, NULL);


        hwndInfComPB = CreateWindow(L"BUTTON", L"Com", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
            15, 150, 40, 30, hwnd, (HMENU)ID_PB_SWAY_IF_INFO_COMDATA, hInst, NULL);

        hwndInfMsgPB = CreateWindow(L"BUTTON", L"MSG", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
            15, 185, 40, 30, hwnd, (HMENU)ID_PB_SWAY_IF_INFO_MSG, hInst, NULL);

        SendMessage(hwndInfComPB, BM_SETCHECK, BST_CHECKED, 0L);

        CreateWindowW(TEXT("STATIC"), L" Min \n cycle", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 300, 50, 40, hwnd, (HMENU)ID_STATIC_SWAY_IF_MINCYCLE, hInst, NULL);


        hwndCycleUpPB = CreateWindow(L"BUTTON", L"10m↑", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            15, 350, 45, 30, hwnd, (HMENU)ID_PB_SWAY_IF_MIN_CYCLE_10mUP, hInst, NULL);

        hwndCycleDnPB = CreateWindow(L"BUTTON", L"10m↓", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            15, 385, 45, 30, hwnd, (HMENU)ID_PB_SWAY_IF_MIN_CYCLE_10mDN, hInst, NULL);
        */

        //振れセンサ送信タイマ起動
        SetTimer(hwnd, ID_WORK_WND_TIMER, WORK_SCAN_TIME, NULL);

    }break;
    case WM_TIMER: {
        /*
        if (be_skiped_once_const_msg == false)
            send_msg(SID_SENSOR1, SW_SND_COM_CONST_DATA);


        be_skiped_once_const_msg = false;
*/
    }break;

    case ID_UDP_EVENT: {
        /*
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv++;
            serverlen = (int)sizeof(server);

            SOCKADDR from_addr;
            sockaddr_in* psockaddr = (sockaddr_in*)&from_addr;
            int from_size = (int)sizeof(from_addr);

            nRtn = recvfrom(s, (char*)&rcv_msg[0][0], sizeof(ST_SWAY_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_size);

            if (nRtn == SOCKET_ERROR) {
                woMSG << L" recvfrom ERROR";
                if (IsDlgButtonChecked(hwnd, ID_PB_SWAY_IF_INFO_COMDATA) == BST_CHECKED)
                    tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
            }
            else {
                ST_SWAY_RCV_MSG msg = rcv_msg[iDispSensor][iDispBuf];

                //  電文からパラメータ読み込み
                if ((swx.is_read_from_msg == false) || (swy.is_read_from_msg == false))
                    get_sensor_param_from_msg(&msg);

                //ヘッダ部表示
                woMSG << L" RCV len: " << nRtn << L" Count :" << nRcv;
                woMSG << L"\n IP: " << psockaddr->sin_addr.S_un.S_un_b.s_b1 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b2 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b3 << L"." << psockaddr->sin_addr.S_un.S_un_b.s_b4;
                woMSG << L" PORT: " << psockaddr->sin_port;
                tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();

                //受信データ表示
                if (IsDlgButtonChecked(hwnd, ID_PB_SWAY_IF_INFO_COMDATA) == BST_CHECKED) {


                    woMSG << L"Header" << L"  >> ID: " << msg.head.id[0] << msg.head.id[1] << msg.head.id[2] << msg.head.id[3];
                    //日時
                    woMSG << L"  " << msg.head.time.wMonth << L"/" << msg.head.time.wDay << L" " << msg.head.time.wHour << L":" << msg.head.time.wMinute << L":" << msg.head.time.wSecond;
                    //# 仕様
                    woMSG << L"\n\n@SPEC";
                    //画素数
                    woMSG << L"\n *nPIX x:" << msg.body[iDispCam].cam_spec.pix_x << L" y:" << msg.body[iDispCam].cam_spec.pix_y;

                    //カメラ取付距離,角度
                    woMSG << L"\n *l0 x:" << msg.body[iDispCam].cam_spec.l0_x << L" y:" << msg.body[iDispCam].cam_spec.l0_y << L"  *ph0 x:" << msg.body[iDispCam].cam_spec.ph0_x << L" y:" << msg.body[iDispCam].cam_spec.ph0_y;
                    woMSG << L"\n *phc x:" << msg.body[iDispCam].cam_spec.phc_x << L" y:" << msg.body[iDispCam].cam_spec.phc_y << L"  *Pix/Rad  x:" << msg.body[iDispCam].cam_spec.pixlrad_x << L" y:" << msg.body[iDispCam].cam_spec.pixlrad_y;

                    //# 機器状態
                    woMSG << L"\n@STATUS";
                    woMSG << L"\n *Mode:" << msg.body[iDispCam].cam_stat.mode << L" *STAT:" << msg.body[iDispCam].cam_stat.status << L" *ERR:" << msg.body[iDispCam].cam_stat.error;

                    //# Data
                    woMSG << L"\n@DATA";
                    //傾斜計
                    woMSG << L"\n *Til  X :" << msg.body[iDispCam].cam_stat.tilt_x << L"(" << (double)(msg.body[iDispCam].cam_stat.tilt_x) * 180.0 / PI180 / 1000000.0 << L"deg)  Y :" << msg.body[iDispCam].cam_stat.tilt_y << L"(" << (double)(msg.body[iDispCam].cam_stat.tilt_y) * 180.0 / PI180 / 1000000.0 << L"deg)";
                    woMSG << L"  dX :" << msg.body[iDispCam].cam_stat.tilt_dx << L" dY :" << msg.body[iDispCam].cam_stat.tilt_dy;
                    woMSG << L"\n *PIX x :" << msg.body[iDispCam].tg_stat[iDispTg].th_x << L" y :" << msg.body[iDispCam].tg_stat[iDispTg].th_y << L"  *dPIX x :" << msg.body[iDispCam].tg_stat[iDispTg].dth_x << L" y :" << msg.body[iDispCam].tg_stat[iDispTg].dth_y;
                    woMSG << L"\n *CENTER X0 :" << msg.body[iDispCam].tg_stat[iDispTg].th_x0 << L" Y0 :" << msg.body[iDispCam].tg_stat[iDispTg].th_y0 << L"\n *tgSize :" << msg.body[iDispCam].tg_stat[iDispTg].tg_size;
                    woMSG << L"\n *tg_dist x :" << msg.body[iDispCam].tg_stat[iDispTg].dpx_tgs << L" y :" << msg.body[iDispCam].tg_stat[iDispTg].dpy_tgs;
                }
                //受信内容メッセージ表示
                else {

                    woMSG << L"# Info MSG:\n";
                    woMSG << L" " << msg.body[iDispCam].info;
                    woMSG << L"\n\n";

                    INT32 msgbits = msg.body[iDispCam].cam_stat.status;
                    woMSG;
                    woMSG << L"# DETECT STATUS:\n";

                    for (int i = 0;i < 16;i++) {
                        if (msgbits & 0x1)
                            woMSG << L"    >" << ws_sensor_stat_msg[i] << L"\n";
                        msgbits = msgbits >> 1;
                    }

                    woMSG << L"\n";

                    msgbits = msg.body[iDispCam].cam_stat.error;
                    woMSG << L"# ERR STATUS:\n";
                    for (int i = 0;i < 16;i++) {
                        if (msgbits & 0x1)
                            woMSG << L"    >" << ws_sensor_err_msg[i] << L"\n";
                        msgbits = msgbits >> 1;
                    }

                }

                tweet2infMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();

            }

        }break;
        case FD_WRITE: {

        }break;
        case FD_CLOSE: {
            ;
        }break;

        }
        */
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