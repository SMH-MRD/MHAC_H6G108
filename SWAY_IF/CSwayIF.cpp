#include "CSwayIF.h"
#include <windowsx.h>       //# コモンコントロール

#include <iostream>
#include <iomanip>
#include <sstream>

HWND CSwayIF::hWorkWnd;
HWND CSwayIF::hwndSTATMSG;
HWND CSwayIF::hwndRCVMSG;
HWND CSwayIF::hwndSNDMSG;
HWND CSwayIF::hwndINFMSG;

HWND CSwayIF::hwndDispBufMSG;
HWND CSwayIF::hwndCamChangePB;
HWND CSwayIF::hwndBufChangePB;
int CSwayIF::iDispCam;
int CSwayIF::iDispBuf;


ST_SWAY_RCV_MSG CSwayIF::rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
ST_SWAY_SND_MSG CSwayIF::snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
int CSwayIF::i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
int CSwayIF::i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

CSwayIF::CSwayIF() {
    // 共有メモリオブジェクトのインスタンス化
    pSwayIOObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    out_size = 0;
    memset(&sway_io_workbuf, 0, sizeof(ST_SWAY_IO));   //共有メモリへの出力セット作業用バッファ
};
CSwayIF::~CSwayIF() {
    // 共有メモリオブジェクトの解放
    delete pSwayIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
};

int CSwayIF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //出力バッファセット

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSwayIF::init_proc() {

     // 共有メモリ取得

     // 出力用共有メモリ取得
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pSwayIOObj->create_smem(SMEM_SWAY_IO_NAME, out_size, MUTEX_SWAY_IO_NAME)) {
        mode |= SWAY_IF_SWAY_IO_MEM_NG;
    }
    set_outbuf(pSwayIOObj->get_pMap());

    // 入力用共有メモリ取得
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SWAY_IF_SIM_MEM_NG;
    }
 
    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= SWAY_IF_CRANE_MEM_NG;
    }

    //デバッグモード　ON　製番ではOFFで初期化
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif
   
    //共有クレーンステータス構造体のポインタセット
    pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
    pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());

    //CraneStat立ち上がり待ち
    while (pCraneStat->is_tasks_standby_ok == false) {
        Sleep(10);
    }
    
    //振れ角計算用カメラパラメータセット
    for (int i = 0;i < N_SWAY_SENSOR;i++) {
        for (int j = 0;j < SWAY_SENSOR_N_AXIS;j++)
        {
            double D0 = pCraneStat->spec.SwayCamParam[i][j][SID_D0];
            double H0 = pCraneStat->spec.SwayCamParam[i][j][SID_H0];
            double l0 = pCraneStat->spec.SwayCamParam[i][j][SID_l0];

            SwayCamParam[i][j][CAM_SET_PARAM_a] = sqrt(D0 * D0 + (H0 - l0) * (H0 - l0));
            double tempd = H0 - l0; if (tempd < 0.0) tempd *= -1.0;
            if (tempd < 0.000001) {
                SwayCamParam[i][j][CAM_SET_PARAM_b] = 0.0;
            }
            else {
                SwayCamParam[i][j][CAM_SET_PARAM_b] = atan(D0 / (H0 - l0));
            }
            SwayCamParam[i][j][CAM_SET_PARAM_c] = pCraneStat->spec.SwayCamParam[i][j][SID_ph0];
            SwayCamParam[i][j][CAM_SET_PARAM_d] = pCraneStat->spec.SwayCamParam[i][j][SID_PIXlRAD];//rad→pix変換係数

        }
    }




    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CSwayIF::input() {
 
    LPST_CRANE_STATUS pcrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();
    LPST_SIMULATION_STATUS psim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();

    //MAINプロセス(Environmentタスクのヘルシー信号取り込み）
    source_counter = pcrane->env_act_count;

    //PLC 入力

    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CSwayIF::parse() {


#ifdef _DVELOPMENT_MODE
//    if (pSimStat->mode & SIM_ACTIVE_MODE) {
    if(is_debug_mode() && !(pSimStat->mode & SIM_SWAY_PACKET_MODE)){
        set_sim_status(&sway_io_workbuf);   //　振れセンサ受信バッファの値をSIMからSWAY_IFのバッファにコピー
        parse_sway_stat(SID_SIM);           //　シミュレータの受信バッファを解析（カメラ座標での振れ検出値）
    }
    else {
        parse_sway_stat(SID_CAM1);          //　実センサからの受信バッファを解析
    }
#else
    parse_sway_stat(SWAY_SENSOR1);
#endif

    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSwayIF::output() {

    sway_io_workbuf.proc_mode = this->mode;              //モードセット
    sway_io_workbuf.helthy_cnt = my_helthy_counter++;    //ヘルシーカウンタセット

    if (out_size) { //出力処理
        memcpy_s(poutput, out_size, &sway_io_workbuf, out_size);
    }

    return 0;
}
//*********************************************************************************************
// set_sim_status()
//*********************************************************************************************
int CSwayIF::set_sim_status(LPST_SWAY_IO pworkbuf) {

    memcpy_s(&rcv_msg[SID_SIM][0], sizeof(ST_SWAY_RCV_MSG), &pSimStat->rcv_msg, sizeof(ST_SWAY_RCV_MSG));
 
    return 0;
}
//*********************************************************************************************
int CSwayIF::parse_sway_stat(int ID) {
    
    double ax = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_a];//センサ検出角補正値
    double bx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_b];//センサ検出角補正値
    double cx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_c];//センサ検出角補正値
    double dx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_d];//rad→pix変換係数
    double ay = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_a];//センサ検出角補正値
    double by = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_b];//センサ検出角補正値
    double cy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_c];//センサ検出角補正値
    double dy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_d];//rad→pix変換係数
    double L = pCraneStat->mh_l;
    double T = pCraneStat->T;
    double w = pCraneStat->w;

    double tilt_x = ((double)rcv_msg[ID]->head.tilt[SWAY_SENSOR_TIL_X]) / 1000000.0;
    double tilt_y = ((double)rcv_msg[ID]->head.tilt[SWAY_SENSOR_TIL_Y]) / 1000000.0;
 	
    double phx = tilt_x + cx;
    double phy = tilt_y + cy;

    double psix = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].th_x) / dx + phx;
    double psiy = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].th_y) / dy + phy;

    double offset_thx = asin(ax * sin(phx + bx) / L);
    double offset_thy = asin(ay * sin(phy + by) / L);

    //振角　振角速度　振幅　位相　
	//    カメラ位置からの振れ角＝カメラ検出角＋取付オフセット  
    sway_io_workbuf.th[ID_SLEW] = (psix + offset_thx);//接線方向は、旋回速度＋方向が+
    sway_io_workbuf.th[ID_BOOM_H] = psiy + offset_thy;

    sway_io_workbuf.dth[ID_SLEW] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].dth_x) / dx;  // radに変換　接線方向は、旋回速度＋方向が+
    sway_io_workbuf.dth[ID_BOOM_H] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].dth_y) / dy;// radに変換

    sway_io_workbuf.dthw[ID_SLEW] = sway_io_workbuf.dth[ID_SLEW] / w;
    sway_io_workbuf.dthw[ID_BOOM_H] = sway_io_workbuf.dth[ID_BOOM_H] / w;

    sway_io_workbuf.rad_amp2[ID_SLEW] = sway_io_workbuf.th[ID_SLEW] * sway_io_workbuf.th[ID_SLEW] + sway_io_workbuf.dthw[ID_SLEW] * sway_io_workbuf.dthw[ID_SLEW];
    sway_io_workbuf.rad_amp2[ID_BOOM_H] = sway_io_workbuf.th[ID_BOOM_H] * sway_io_workbuf.th[ID_BOOM_H] + sway_io_workbuf.dthw[ID_BOOM_H] * sway_io_workbuf.dthw[ID_BOOM_H];

    //合成振幅  
    sway_io_workbuf.rad_amp2[ID_COMMON] = sway_io_workbuf.rad_amp2[ID_SLEW] + sway_io_workbuf.rad_amp2[ID_BOOM_H];

    //位相(X軸）
    if (sway_io_workbuf.th[ID_SLEW] > 0.00001) {
        sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]);
    }
    else if (sway_io_workbuf.th[ID_SLEW] < -0.00001) { // atan()引数の0割回避
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) + PI180;
        else                                    sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) - PI180;
    }
    else{ //位相は-π〜πの範囲で表現
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = PI90;
        else                                    sway_io_workbuf.ph[ID_SLEW] = -PI90;
    }

    //位相(Y軸）
    if (sway_io_workbuf.th[ID_BOOM_H] > 0.00001) {
        sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]);
    }
    else if (sway_io_workbuf.th[ID_BOOM_H] < -0.00001) { // atan()引数の0割回避
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) + PI180;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) - PI180;
    }
    else { //位相は-π〜πの範囲で表現
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = PI90;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = -PI90;
    }
    
	return 0;
}
//*********************************************************************************************
//IF用ソケット
static WSADATA wsaData;
static SOCKET s;
static SOCKADDR_IN addrin;//送信ポートアドレス
static SOCKADDR_IN server;//受信ポートアドレス
static int serverlen, nEvent;
static int nRtn = 0, nRcv = 0, nSnd = 0;
static u_short port = SWAY_IF_IP_PORT_C;
static char szBuf[512];

std::wostringstream woMSG;
std::wstring wsMSG;


HWND CSwayIF::open_WorkWnd(HWND hwnd_parent) {
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
        TEXT("COMM_CHK"),
        WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H,
        hwnd_parent,
        0,
        hInst,
        NULL);

    ShowWindow(hWorkWnd, SW_SHOW);
    UpdateWindow(hWorkWnd);

    return hWorkWnd;
    
    
    
    return 0; 
}
//*********************************************************************************************
int CSwayIF::close_WorkWnd() { 
    closesocket(s);
    WSACleanup();
    DestroyWindow(hWorkWnd);  //ウィンドウ破棄
    hWorkWnd = NULL;
    return 0;
}
//*********************************************************************************************
int CSwayIF::init_sock(HWND hwnd) { 
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {    //WinSockの初期化
        perror("WSAStartup Error\n");
        return -1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);                  //Socketオープン
    if (s < 0) {
        perror("socket失敗\n");
        return -2;
    }
    memset(&addrin, 0, sizeof(addrin));
    addrin.sin_port = htons(port);
    addrin.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR, &addrin.sin_addr.s_addr);
    
    nRtn = bind(s, (LPSOCKADDR)&addrin, (int)sizeof(addrin)); //ソケットに名前を付ける
    if (nRtn == SOCKET_ERROR) {
        perror("bindエラーです\n");
        closesocket(s);
        WSACleanup();
        return -3;
    }
  
    nRtn = WSAAsyncSelect(s, hwnd, ID_UDP_EVENT, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"非同期化失敗";
        closesocket(s);
        WSACleanup();
        return -4;
    }

    //送信先アドレス初期値設定
    memset(&server, 0, sizeof(server));
    server.sin_port = htons(SWAY_IF_IP_PORT_C);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, SWAY_SENSOR_IP_ADDR, &server.sin_addr.s_addr);

    return 0;
    ; 
}

//*********************************************************************************************
int CSwayIF::set_send_data(int com_id) {

    i_snd_msg[SID_CAM1] = 0;

    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].head.id[0] = 'P';
    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].head.id[1] = 'C';
    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].head.id[2] = '0';
    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].head.id[3] = '1';
    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].head.sockaddr = addrin;

    if (com_id == ID_SWAYIF_REQ_CONST_DATA) {
        snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].body.command[0] = '0';
        snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].body.command[1] = '0';
    }

    snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].body.freq = 1000;

    return 0;
}


//# ウィンドウへのメッセージ表示　wstring
void CSwayIF::tweet2statusMSG(const std::wstring& srcw) {
    SetWindowText(hwndSTATMSG, srcw.c_str()); return;
};
void CSwayIF::tweet2rcvMSG(const std::wstring& srcw) {
    static HWND hwndSNDMSG;
    SetWindowText(hwndRCVMSG, srcw.c_str()); return;
};
void CSwayIF::tweet2sndMSG(const std::wstring& srcw) {
    SetWindowText(hwndSNDMSG, srcw.c_str()); return;
};
void CSwayIF::tweet2infMSG(const std::wstring& srcw) {
    SetWindowText(hwndINFMSG, srcw.c_str()); return;
};

//*********************************************************************************************

LRESULT CALLBACK CSwayIF::WorkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    
    HDC hdc;
    switch (msg) {
    case WM_DESTROY: {
        hWorkWnd = NULL;
    }return 0;
    case WM_CREATE: {

        InitCommonControls();//コモンコントロール初期化
        HINSTANCE hInst = GetModuleHandle(0);

        CreateWindowW(TEXT("STATIC"), L"STATUS", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 5, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        hwndSTATMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 5, 300, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"RCV  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 30, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        hwndRCVMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 30, 300, 40, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"SND  ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 75, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_SND, hInst, NULL);
        hwndSNDMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 75, 300, 40, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);
        CreateWindowW(TEXT("STATIC"), L"Info ", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 120, 55, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_SND, hInst, NULL);
        hwndINFMSG = CreateWindowW(TEXT("STATIC"), L"-", WS_CHILD | WS_VISIBLE | SS_LEFT,
            70, 120, 300, 195, hwnd, (HMENU)ID_STATIC_SWAY_IF_LABEL_RCV, hInst, NULL);

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
        hwndDispBufMSG = CreateWindowW(TEXT("STATIC"), L"DISP: CAM 1 BUF 0", WS_CHILD | WS_VISIBLE | SS_LEFT,
            10, 328, 200, 20, hwnd, (HMENU)ID_STATIC_SWAY_IF_DISP_SELBUF, hInst, NULL);

        hwndCamChangePB = CreateWindow(L"BUTTON", L"CAM NEXT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            215, 323, 80, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_CAM, hInst, NULL);

        hwndBufChangePB = CreateWindow(L"BUTTON", L"BUF NEXT", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            300, 323, 80, 30, hwnd, (HMENU)ID_PB_SWAY_IF_CHG_DISP_BUF, hInst, NULL);


        //振れセンサ送信タイマ起動
        SetTimer(hwnd, ID_WORK_WND_TIMER, WORK_SCAN_TIME, NULL);

     }break;
    case WM_TIMER: {

      //  int n = sprintf_s(szBuf, sizeof(szBuf), "%08d", nSnd);
      //  nRtn = sendto(s, szBuf, n, 0, (LPSOCKADDR)&server, sizeof(server));
       
        set_send_data(ID_SWAYIF_REQ_CONST_DATA);
        int n= sizeof(ST_SWAY_SND_MSG);

         nRtn = sendto(s, reinterpret_cast<const char*> (&snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]]), n, 0, (LPSOCKADDR)&server, sizeof(ST_SWAY_SND_MSG));

        if (nRtn == n) {
            nSnd++;
            woMSG << L" SND len: " << nRtn << L"  Count :" << nSnd << L"\n ";
        }
        else if (nRtn == SOCKET_ERROR) {
            woMSG << L" SOCKET ERROR: CODE ->   " << WSAGetLastError();
        }
        else {
            woMSG << L" sendto size ERROR ";
        }
        tweet2sndMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
    }break;

    case ID_UDP_EVENT: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv++;
            serverlen = (int)sizeof(server);
 
            SOCKADDR from_addr;
            int from_size = (int)sizeof(from_addr);
//           nRtn = recvfrom(s, (char*)&rcv_msg[0][0], sizeof(ST_SWAY_RCV_MSG), 0, (SOCKADDR*)&server, &serverlen);
           nRtn = recvfrom(s, (char*)&rcv_msg[0][0], sizeof(ST_SWAY_RCV_MSG), 0, (SOCKADDR*)&from_addr, &from_size);
           
            if (nRtn == SOCKET_ERROR) {
                woMSG << L" recvfrom ERROR";
                tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
            }
            else {
                ST_SWAY_RCV_MSG msg = rcv_msg[iDispCam][iDispBuf];

                woMSG << L" RCV len: " << nRtn << L" Count :" << nRcv ;
                woMSG << L">> ID: " << msg.head.id[0] << msg.head.id[1] << msg.head.id[2] << msg.head.id[3];
                woMSG << L"\n IP: " <<server.sin_addr.S_un.S_un_b.s_b1 << L"."<< server.sin_addr.S_un.S_un_b.s_b2 << L"." << server.sin_addr.S_un.S_un_b.s_b3 << L"." << server.sin_addr.S_un.S_un_b.s_b4;
                woMSG << L" PORT: " << server.sin_port;
                tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
   
                woMSG << L"Header";
                //日時
                woMSG << msg.head.time.wMonth << L"/" << msg.head.time.wDay << L" " << msg.head.time.wHour << L":" << msg.head.time.wMinute << L":" << msg.head.time.wSecond;
                //画素数
                woMSG << L"\n nPIX x:" << msg.head.cam_setting.pix_x << L" y:" << msg.head.cam_setting.pix_y;
                //画角
                woMSG << L" PIX/rad x:" << msg.head.cam_setting.pixlrad_x << L" y:" << msg.head.cam_setting.pixlrad_y;
                //カメラ取付距離,角度
                woMSG << L"\n L0 x:" << msg.head.cam_setting.l0_x << L" y:" << msg.head.cam_setting.l0_y;
                woMSG << L" th0 x:" << msg.head.cam_setting.ph_x << L" y:" << msg.head.cam_setting.ph_y;
                //機器状態
                woMSG << L"\n STAT Mode:" << msg.head.tg_stat[SWAY_SENSOR_TG1].mode[0] << L" STAT:" << msg.head.tg_stat[SWAY_SENSOR_TG1].status[0] << L" ERR:" << msg.head.tg_stat[SWAY_SENSOR_TG1].error[0];
                //傾斜計
                woMSG << L"\n TilX :" << msg.head.tilt[SWAY_SENSOR_TIL_X]<<L"(" << msg.head.tilt[SWAY_SENSOR_TIL_X]*180/PI180 / 100000 << L"deg) TilY :" << msg.head.tilt[SWAY_SENSOR_TIL_Y] << L"(" << msg.head.tilt[SWAY_SENSOR_TIL_Y] * 180 / PI180 / 100000 << L"deg)";

                woMSG << L"\nBody";
                woMSG << L"\n PIX x :" << msg.body.data[SWAY_SENSOR_TG1].th_x << L" y :" << msg.body.data[SWAY_SENSOR_TG1].th_y << L" dPIX x :" << msg.body.data[SWAY_SENSOR_TG1].dth_x << L" y :" << msg.body.data[SWAY_SENSOR_TG1].dth_y;
                woMSG << L"\n X0 :" << msg.body.data[SWAY_SENSOR_TG1].th_x0 << L" Y0 :" << msg.body.data[SWAY_SENSOR_TG1].th_y0 << L" tg1Size :" << msg.body.data[SWAY_SENSOR_TG1].tg_size ;
                woMSG << L"\n tg_dist x :" << msg.body.data[SWAY_SENSOR_TG1].dpx_tgs << L" y :" << msg.body.data[SWAY_SENSOR_TG1].dpy_tgs;
                tweet2infMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();

            }
  

        }break;
        case FD_WRITE:{

        }break;
        case FD_CLOSE: {
            ;
        }break;

        }
    }break;
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
         case ID_PB_SWAY_IF_CHG_DISP_CAM:
            iDispCam++;
            if (iDispCam >= N_SWAY_SENSOR)iDispCam = 0;
 
            woMSG.str(L"");wsMSG.clear();
            if(iDispCam == SID_SIM)
             woMSG << L"DISP: CAM SIM" << L" Buf " << iDispBuf << L"  ";
            else
             woMSG << L"DISP: CAM " << iDispCam +1  << L" Buf " << iDispBuf << L"  ";

            SetWindowText(hwndDispBufMSG, woMSG.str().c_str());woMSG.str(L"");wsMSG.clear();

            break;

        case ID_PB_SWAY_IF_CHG_DISP_BUF:
            iDispBuf++;
            if (iDispBuf >= N_SWAY_SENSOR_RCV_BUF) iDispBuf = 0;

            woMSG.str(L"");wsMSG.clear();
            if (iDispCam == SID_SIM)
                woMSG << L"DISP: CAM SIM" << L" Buf " << iDispBuf << L"  ";
            else
                woMSG << L"DISP: CAM " << iDispCam + 1 << L" Buf " << iDispBuf << L"  ";

            SetWindowText(hwndDispBufMSG, woMSG.str().c_str());woMSG.str(L"");wsMSG.clear();
            break;

        default: break;

        }
    }break;


    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    
    return 0; 
}