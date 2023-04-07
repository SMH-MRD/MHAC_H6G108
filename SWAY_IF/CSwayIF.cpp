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
HWND CSwayIF::hwndTargetChangePB;
int CSwayIF::iDispSensor = 0;
int CSwayIF::iDispBuf = 0;
int CSwayIF::iDispCam = 0;
int CSwayIF::iDispTg = 0;

HWND CSwayIF::hwndInfComPB;
HWND CSwayIF::hwndInfMsgPB;
HWND CSwayIF::hwndCycleUpPB;
HWND CSwayIF::hwndCycleDnPB;
INT32 CSwayIF::cycle_min_ms;
INT32 CSwayIF::sens_mode;
INT32 CSwayIF::cal_mode = ID_SWAY_CAL_NORMAL;

LPST_CRANE_STATUS CSwayIF :: pCraneStat;
LPST_SIMULATION_STATUS CSwayIF :: pSimStat;

ST_SWAY_RCV_MSG CSwayIF::rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
ST_SWAY_SND_MSG CSwayIF::snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
int CSwayIF::i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
int CSwayIF::i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

ST_SWAY_IO CSwayIF::sway_io_workbuf;
ST_SWAY_CAM_WORK CSwayIF::swx;
ST_SWAY_CAM_WORK CSwayIF::swy;

wstring CSwayIF::ws_sensor_err_msg[64];
wstring CSwayIF::ws_sensor_stat_msg[64];

static bool be_skiped_once_const_msg = false;

CSwayIF::CSwayIF() {
    // 共有メモリオブジェクトのインスタンス化
    hInst = NULL;
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
    out_size = sizeof(ST_SWAY_IO);
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

      
    //振れ角計算用カメラパラメータデフォルト値セット
    swx.is_read_from_msg = false;
    swy.is_read_from_msg = false;

    swx.L0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_L0];
    swy.L0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_L0];

    swx.PH0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_PH0];
    swy.PH0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_PH0];

    swx.l0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_l0];
    swy.l0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_l0];

    swx.ph0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_ph0];
    swy.ph0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_ph0];

    swx.phc = pCraneStat->spec.SwayCamParam[0][0][0][SID_phc];
    swy.phc = pCraneStat->spec.SwayCamParam[0][0][1][SID_phc];

    swx.C = 1.0/pCraneStat->spec.SwayCamParam[0][0][0][SID_PIXlRAD];
    swy.C = 1.0/pCraneStat->spec.SwayCamParam[0][0][1][SID_PIXlRAD];

    cycle_min_ms = SW_SND_DEFAULT_SCAN;
    sens_mode = SW_SND_MODE_NORMAL;

    init_rcv_msg();     //受信バッファの初期化
    set_sensor_msg();   //振れセンサ電文コードに対応する状態表示テキストメッセージの登録
       
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
    if(is_debug_mode() && !(pSimStat->mode & SIM_SWAY_PACKET_MODE)){
        set_sim_status(&sway_io_workbuf);                                       //　振れセンサ受信バッファの値をSIMからSWAY_IFのバッファにコピー
        parse_sway_stat(&rcv_msg[SID_SIM][i_rcv_msg[SID_SIM]]);                 //　シミュレータの受信バッファを解析（カメラ座標での振れ検出値）
    }
    else {
        parse_sway_stat(&rcv_msg[SID_SENSOR1][i_rcv_msg[SID_SENSOR1]]);         //　実センサからの受信バッファを解析
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
// 振れ計算用センサパラメータを通信電文より読み込み
//*********************************************************************************************
int CSwayIF::get_sensor_param_from_msg(LPST_SWAY_RCV_MSG pmsg) {
    swx.L0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_L0];
    swy.L0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_L0];

    swx.PH0 = pCraneStat->spec.SwayCamParam[0][0][0][SID_PH0];
    swy.PH0 = pCraneStat->spec.SwayCamParam[0][0][1][SID_PH0];

    swx.l0 = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.l0_x / 1000.0;       //mm->m
    swy.l0 = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.l0_y / 1000.0;       //mm->m

    swx.ph0 = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.ph0_x / 1000000.0;  //x10^6->x1
    swy.ph0 = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.ph0_y / 1000000.0;  //x10^6->x1

    swx.phc = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.phc_x / 1000000.0;  //x10^6->x1
    swy.phc = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.phc_y / 1000000.0;  //x10^6->x1

    if (pmsg->body[SWAY_SENSOR_CAM1].cam_spec.pixlrad_x > 1) {
        swx.C = 1.0 / (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.pixlrad_x;
    }
    if (pmsg->body[SWAY_SENSOR_CAM1].cam_spec.pixlrad_x > 1) {
        swx.C = 1.0 / (double)pmsg->body[SWAY_SENSOR_CAM1].cam_spec.pixlrad_y;
    }
    swx.is_read_from_msg = true;
    swy.is_read_from_msg = true;
    return 0;
}

//*********************************************************************************************
// 振れセンサデータを解析
//*********************************************************************************************

int CSwayIF::parse_sway_stat(LPST_SWAY_RCV_MSG pmsg) {
            
    double PHx, PHy, dPHx, dPHy,thx,thy, dthx, dthy,tilx_cam, tily_cam,xrx,yrx,xry,yry;
   
    //θt+φc dθt+dφc PIX→rad変換
    swx.th_cam  = swx.C * (double)(pmsg->body[SWAY_SENSOR_CAM1].tg_stat[SWAY_SENSOR_TG1].th_x) + swx.phc;
    swy.th_cam  = swy.C * (double)(pmsg->body[SWAY_SENSOR_CAM1].tg_stat[SWAY_SENSOR_TG1].th_y) + swy.phc;
    swx.dth_cam = swx.C * (double)(pmsg->body[SWAY_SENSOR_CAM1].tg_stat[SWAY_SENSOR_TG1].dth_x);
    swy.dth_cam = swy.C * (double)(pmsg->body[SWAY_SENSOR_CAM1].tg_stat[SWAY_SENSOR_TG1].dth_y);

    //φt+φ0　x10^6→10^0
    tilx_cam = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_stat.tilt_x / 1000000.0;
    tily_cam = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_stat.tilt_y / 1000000.0;

    if (cal_mode & ID_SWAY_CAL_NO_TILT) {   //調整用機能 傾斜データ無し
        swx.til_cam     = swx.ph0;
        swy.til_cam     = swy.ph0;
        swx.dtil_cam    = 0.0;
        swy.dtil_cam    = 0.0;
    }
    else {
        swx.til_cam     = tilx_cam + swx.ph0;
        swy.til_cam     = tily_cam + swy.ph0;
        //dφt　x10^6→10^0
        swx.dtil_cam    = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_stat.tilt_dx / 1000000.0;
        swy.dtil_cam    = (double)pmsg->body[SWAY_SENSOR_CAM1].cam_stat.tilt_dy / 1000000.0;
    }

    if (cal_mode & ID_SWAY_CAL_NO_OFFSET) {  //カメラ位置オフセット無し
        xrx = yrx = xry = yry = 0.0;
    }
    else {
        xrx = swx.L0 * sin(swx.PH0) + swx.l0 * sin(swx.til_cam);
        yrx = swx.L0 * cos(swx.PH0) + swx.l0 * cos(swx.til_cam);
        xry = swy.L0 * sin(swy.PH0) + swy.l0 * sin(swy.til_cam);
        yry = swy.L0 * cos(swy.PH0) + swy.l0 * cos(swy.til_cam);
    }

    thx = swx.th_cam + swx.til_cam;
    thy = swy.th_cam + swy.til_cam;
    double x_off = (yrx * sin(thx) + xrx * cos(thx)) / pCraneStat->mh_l;
    PHx = thx - asin(x_off);
    PHy = thy - asin((yry * sin(thy) + xry * cos(thy)) / pCraneStat->mh_l);
 
    dthx = swx.dth_cam + swx.dtil_cam;
    dthy = swy.dth_cam + swy.dtil_cam;
    dPHx = dthx * (1.0 - (yrx * sin(thx) + xrx * tan(thx)) / (cos(PHx) + sin(PHx) * tan(thx)) / pCraneStat->mh_l);
    dPHy = dthy * (1.0 - (yry * sin(thy) + xry * tan(thy)) / (cos(PHy) + sin(PHy) * tan(thy)) / pCraneStat->mh_l);
      
    //傾斜計データ  
    sway_io_workbuf.tilt_rad[ID_SLEW]   = tilx_cam;
    sway_io_workbuf.tilt_rad[ID_BOOM_H] = tily_cam;

    //吊荷振れ角  
    sway_io_workbuf.th[ID_SLEW]     = PHx;//接線方向は、旋回速度＋方向が+
    sway_io_workbuf.th[ID_BOOM_H]   = PHy;

    //吊荷振れ角速度  
    sway_io_workbuf.dth[ID_SLEW]    = dPHx; //接線方向は、旋回速度＋方向が+
    sway_io_workbuf.dth[ID_BOOM_H]  = dPHy; 

    sway_io_workbuf.dthw[ID_SLEW] = sway_io_workbuf.dth[ID_SLEW] / pCraneStat->w;
    sway_io_workbuf.dthw[ID_BOOM_H] = sway_io_workbuf.dth[ID_BOOM_H] / pCraneStat->w;

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
    else{ //位相は-π～πの範囲で表現
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
    else { //位相は-π～πの範囲で表現
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
static u_short port = SWAY_IF_IP_SWAY_PORT_C;
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
        TEXT("SWAY SENSOR IF COMM_CHK"),
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
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_SWAY, &addrin.sin_addr.s_addr);
    
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
    server.sin_port = htons(SWAY_IF_IP_SWAY_PORT_C);
    server.sin_family = AF_INET;
    inet_pton(AF_INET, SWAY_SENSOR_IP_ADDR, &server.sin_addr.s_addr);

    return 0;
    ; 
}

void  CSwayIF::init_rcv_msg() {
    for (int i = 0; i < N_SWAY_SENSOR;i++)
        for (int k = 0; k < N_SWAY_SENSOR_RCV_BUF;k++) {
            rcv_msg[i][k].head.id[0] = 'N';
            rcv_msg[i][k].head.id[1] = 'A';
            rcv_msg[i][k].head.id[2] = '\0';
            GetLocalTime(&rcv_msg[i][k].head.time);
            for (int l = 0;l < SWAY_SENSOR_N_CAM;l++) {
                rcv_msg[i][k].body[l].cam_spec.l0_x = (INT32)pCraneStat->spec.SwayCamParam[0][0][0][SID_l0];
                rcv_msg[i][k].body[l].cam_spec.l0_y = (INT32)pCraneStat->spec.SwayCamParam[0][0][1][SID_l0];
                rcv_msg[i][k].body[l].cam_spec.ph0_x = (INT32)pCraneStat->spec.SwayCamParam[0][0][0][SID_ph0];
                rcv_msg[i][k].body[l].cam_spec.ph0_y = (INT32)pCraneStat->spec.SwayCamParam[0][0][1][SID_ph0];
                rcv_msg[i][k].body[l].cam_spec.phc_x = (INT32)pCraneStat->spec.SwayCamParam[0][0][0][SID_phc];
                rcv_msg[i][k].body[l].cam_spec.phc_y = (INT32)pCraneStat->spec.SwayCamParam[0][0][1][SID_phc];
                rcv_msg[i][k].body[l].cam_spec.pixlrad_x = (INT32)pCraneStat->spec.SwayCamParam[0][0][0][SID_PIXlRAD];
                rcv_msg[i][k].body[l].cam_spec.pixlrad_y = (INT32)pCraneStat->spec.SwayCamParam[0][0][1][SID_PIXlRAD];
                rcv_msg[i][k].body[l].cam_spec.pix_x = 1000;
                rcv_msg[i][k].body[l].cam_spec.pix_y = 1000;

                rcv_msg[i][k].body[l].cam_stat.error = 0;
                rcv_msg[i][k].body[l].cam_stat.mode = 0;
                rcv_msg[i][k].body[l].cam_stat.status = 0;
                rcv_msg[i][k].body[l].cam_stat.tilt_dx = 0;
                rcv_msg[i][k].body[l].cam_stat.tilt_dy = 0;
                rcv_msg[i][k].body[l].cam_stat.tilt_x = 0;
                rcv_msg[i][k].body[l].cam_stat.tilt_y = 0;
   
                for (int m = 0;m < SWAY_SENSOR_N_TARGET;m++) {
                    rcv_msg[i][k].body[l].tg_stat[m].dpx_tgs = 100;
                    rcv_msg[i][k].body[l].tg_stat[m].dpy_tgs = 100;
                    rcv_msg[i][k].body[l].tg_stat[m].dth_x = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].dth_y = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].th_x = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].th_y = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].tg_size = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].th_x0 = 0;
                    rcv_msg[i][k].body[l].tg_stat[m].th_y0 = 0;
                }
            }
        }

}

//*********************************************************************************************

int CSwayIF::send_msg(int sensor_id,INT32 com_id) {

    i_snd_msg[sensor_id] = 0;

    snd_msg[sensor_id][i_snd_msg[sensor_id]].head.id[0] = 'P';
    snd_msg[sensor_id][i_snd_msg[sensor_id]].head.id[1] = 'C';
    snd_msg[sensor_id][i_snd_msg[sensor_id]].head.id[2] = '0';
    snd_msg[sensor_id][i_snd_msg[sensor_id]].head.id[3] = '1';
    snd_msg[sensor_id][i_snd_msg[sensor_id]].head.sockaddr = addrin;

    snd_msg[sensor_id][i_snd_msg[sensor_id]].body.command = com_id;
    snd_msg[sensor_id][i_snd_msg[sensor_id]].body.mode = sens_mode;
    snd_msg[sensor_id][i_snd_msg[sensor_id]].body.freq = cycle_min_ms;
    snd_msg[sensor_id][i_snd_msg[sensor_id]].body.d[0] = (INT32)(pCraneStat->mh_l*1000);
    snd_msg[sensor_id][i_snd_msg[sensor_id]].body.d[1] = (INT32)(pCraneStat->mh_l * 1000);

    int n = sizeof(ST_SWAY_SND_MSG);

    nRtn = sendto(s, reinterpret_cast<const char*> (&snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]]), n, 0, (LPSOCKADDR)&server, sizeof(ST_SWAY_SND_MSG));

    if (nRtn == n) {
        nSnd++;
        woMSG << L" SND len: " << nRtn << L"  Count :" << nSnd << L"    COM:" << snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]].body.command
            << L"   scan:" << snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]].body.freq << L"\n "
            << L"MODE:" << snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]].body.mode
            << L"   D1:" << snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]].body.d[0] << L"   D2:" << snd_msg[SID_SENSOR1][i_snd_msg[SID_SENSOR1]].body.d[1];

        be_skiped_once_const_msg = true;
    }
    else if (nRtn == SOCKET_ERROR) {
        woMSG << L" SOCKET ERROR: CODE ->   " << WSAGetLastError();
    }
    else {
        woMSG << L" sendto size ERROR ";
    }
    tweet2sndMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();

    return nRtn;
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

void CSwayIF::update_buf_set_disp_msg(HWND hwnd) {

    woMSG.str(L"");wsMSG.clear();
    if (iDispSensor == SID_SIM)
        woMSG << L"ID SIM" << L" Buf " << iDispBuf << L" CAM " << iDispCam + 1 << L" TG " << iDispTg + 1 << L"      NEXT->";
    else
        woMSG << L"ID   " << iDispSensor + 1 << L" Buf " << iDispBuf << L" CAM " << iDispCam + 1 << L" TG " << iDispTg + 1 << L"      NEXT-> ";

    if (IsDlgButtonChecked(hwnd, ID_PB_SWAY_IF_INFO_COMDATA) == BST_CHECKED)
        SetWindowText(hwndDispBufMSG, woMSG.str().c_str());woMSG.str(L"");wsMSG.clear();
    return;
}

//*********************************************************************************************
void CSwayIF::set_sensor_msg() {

  //エラーメッセージ
    ws_sensor_err_msg[0] = SW_TXT_ERR_CODE_CAMERA;
    ws_sensor_err_msg[1] = SW_TXT_ERR_CODE_TILT;
    ws_sensor_err_msg[4] = SW_TXT_ERR_CODE_DETECT_TG1;
    ws_sensor_err_msg[5] = SW_TXT_ERR_CODE_DETECT_TG2;


  //検出状態
    ws_sensor_stat_msg[0] = SW_TXT_STAT_TG1_DETECTED;
    ws_sensor_stat_msg[1] = SW_TXT_STAT_TG2_DETECTED;
    ws_sensor_stat_msg[4] = SW_TXT_STAT_INIT_SENSOR;
    ws_sensor_stat_msg[5] = SW_TXT_STAT_INIT_TILT;
    ws_sensor_stat_msg[6] = SW_TXT_STAT_0SET_SENSOR;
    ws_sensor_stat_msg[7] = SW_TXT_STAT_0SET_TILT;
}
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

        //振れセンサ送信タイマ起動
        SetTimer(hwnd, ID_WORK_WND_TIMER, WORK_SCAN_TIME, NULL);

     }break;
    case WM_TIMER: {
       if(be_skiped_once_const_msg == false) 
            send_msg(SID_SENSOR1,SW_SND_COM_CONST_DATA);
        be_skiped_once_const_msg = false;

    }break;

    case ID_UDP_EVENT: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv++;
            serverlen = (int)sizeof(server);
 
            SOCKADDR from_addr;
            sockaddr_in* psockaddr = (sockaddr_in *)&from_addr;
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
                    woMSG << L" "<< msg.body[iDispCam].info ;
                    woMSG << L"\n\n";

                    INT32 msgbits = msg.body[iDispCam].cam_stat.status;
                    woMSG ;
                    woMSG << L"# DETECT STATUS:\n";

                    for (int i = 0;i < 16;i++) {
                        if (msgbits & 0x1)
                            woMSG << L"    >" << ws_sensor_stat_msg[i] << L"\n";
                        msgbits = msgbits >> 1;
                    }

                    woMSG << L"\n";
                    
                    msgbits = msg.body[iDispCam].cam_stat.error;
                    woMSG << L"# ERR STATUS:\n" ;
                    for (int i = 0;i < 16;i++) {
                        if (msgbits & 0x1)
                            woMSG << L"    >" << ws_sensor_err_msg[i] << L"\n";
                        msgbits = msgbits >> 1;
                    }

                }

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
         case ID_PB_SWAY_IF_CHG_DISP_SENSOR:
            iDispSensor++;
            if (iDispSensor >= N_SWAY_SENSOR)iDispSensor = 0;
             update_buf_set_disp_msg(hwnd);

            break;

        case ID_PB_SWAY_IF_CHG_DISP_BUF:
            iDispBuf++;
            if (iDispBuf >= N_SWAY_SENSOR_RCV_BUF) iDispBuf = 0;
            update_buf_set_disp_msg(hwnd);
            break;
       
        case ID_PB_SWAY_IF_CHG_DISP_CAM:
            iDispCam++;
            if (iDispCam >= N_SWAY_SENSOR_CAMERA) iDispCam = 0;
            update_buf_set_disp_msg(hwnd);
             break;

        case ID_PB_SWAY_IF_CHG_DISP_TG:
            iDispTg++;
            if (iDispTg >= N_SWAY_SENSOR_TARGET) iDispTg = 0;
            update_buf_set_disp_msg(hwnd);
            break;

        case ID_PB_SWAY_IF_INFO_COMDATA:
            break;
        case  ID_PB_SWAY_IF_INFO_MSG:
            break;
        case  ID_PB_SWAY_IF_MIN_CYCLE_10mUP:
            cycle_min_ms += 10;
            break;
        case  ID_PB_SWAY_IF_MIN_CYCLE_10mDN :
            if(cycle_min_ms >=20) cycle_min_ms -= 10;
            break;

        default: break;

        }
    }break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    
    return 0; 
}


