#include "CSwayIF.h"
#include <windowsx.h>       //# �R�����R���g���[��

#include <iostream>
#include <iomanip>
#include <sstream>

HWND CSwayIF::hWorkWnd;
HWND CSwayIF::hwndSTATMSG;
HWND CSwayIF::hwndRCVMSG;
HWND CSwayIF::hwndSNDMSG;
HWND CSwayIF::hwndINFMSG;

ST_SWAY_RCV_MSG CSwayIF::rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
ST_SWAY_SND_MSG CSwayIF::snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
int CSwayIF::i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
int CSwayIF::i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

CSwayIF::CSwayIF() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pSwayIOObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    out_size = 0;
    memset(&sway_io_workbuf, 0, sizeof(ST_SWAY_IO));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
};
CSwayIF::~CSwayIF() {
    // ���L�������I�u�W�F�N�g�̉��
    delete pSwayIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
};

int CSwayIF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //�o�̓o�b�t�@�Z�b�g

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSwayIF::init_proc() {

     // ���L�������擾

     // �o�͗p���L�������擾
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pSwayIOObj->create_smem(SMEM_SWAY_IO_NAME, out_size, MUTEX_SWAY_IO_NAME)) {
        mode |= SWAY_IF_SWAY_IO_MEM_NG;
    }
    set_outbuf(pSwayIOObj->get_pMap());

    // ���͗p���L�������擾
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SWAY_IF_SIM_MEM_NG;
    }
 
    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= SWAY_IF_CRANE_MEM_NG;
    }

    //�f�o�b�O���[�h�@ON�@���Ԃł�OFF�ŏ�����
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif
   
    //���L�N���[���X�e�[�^�X�\���̂̃|�C���^�Z�b�g
    pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
    pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());

    //CraneStat�����オ��҂�
    while (pCraneStat->is_tasks_standby_ok == false) {
        Sleep(10);
    }
    
    //�U��p�v�Z�p�J�����p�����[�^�Z�b�g
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
            SwayCamParam[i][j][CAM_SET_PARAM_d] = pCraneStat->spec.SwayCamParam[i][j][SID_PIXlRAD];//rad��pix�ϊ��W��

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

    //MAIN�v���Z�X(Environment�^�X�N�̃w���V�[�M����荞�݁j
    source_counter = pcrane->env_act_count;

    //PLC ����

    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CSwayIF::parse() {


#ifdef _DVELOPMENT_MODE
//    if (pSimStat->mode & SIM_ACTIVE_MODE) {
    if(is_debug_mode() && !(pSimStat->mode & SIM_SWAY_PACKET_MODE)){
        set_sim_status(&sway_io_workbuf);   //�@�U��Z���T��M�o�b�t�@�̒l��SIM����SWAY_IF�̃o�b�t�@�ɃR�s�[
        parse_sway_stat(SID_SIM);           //�@�V�~�����[�^�̎�M�o�b�t�@����́i�J�������W�ł̐U�ꌟ�o�l�j
    }
    else {
        parse_sway_stat(SID_CAM1);          //�@���Z���T����̎�M�o�b�t�@�����
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

    sway_io_workbuf.proc_mode = this->mode;              //���[�h�Z�b�g
    sway_io_workbuf.helthy_cnt = my_helthy_counter++;    //�w���V�[�J�E���^�Z�b�g

    if (out_size) { //�o�͏���
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
    
    double ax = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double bx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double ay = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double by = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double L = pCraneStat->mh_l;
    double T = pCraneStat->T;
    double w = pCraneStat->w;

    double tilt_x = ((double)rcv_msg[ID]->head.tilt[SWAY_SENSOR_CAM1][SWAY_SENSOR_TIL_X]) / 1000000.0;
    double tilt_y = ((double)rcv_msg[ID]->head.tilt[SWAY_SENSOR_CAM1][SWAY_SENSOR_TIL_Y]) / 1000000.0;
 	
    double phx = tilt_x + cx;
    double phy = tilt_y + cy;

    double psix = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_CAM1][SWAY_SENSOR_TG1].th_x) / dx + phx;
    double psiy = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_CAM1][SWAY_SENSOR_TG1].th_y) / dy + phy;

    double offset_thx = asin(ax * sin(phx + bx) / L);
    double offset_thy = asin(ay * sin(phy + by) / L);

    //�U�p�@�U�p���x�@�U���@�ʑ��@
	//    �J�����ʒu����̐U��p���J�������o�p�{��t�I�t�Z�b�g  
    sway_io_workbuf.th[ID_SLEW] = (psix + offset_thx);//�ڐ������́A���񑬓x�{������+
    sway_io_workbuf.th[ID_BOOM_H] = psiy + offset_thy;

    sway_io_workbuf.dth[ID_SLEW] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_CAM1][SWAY_SENSOR_TG1].dth_x) / dx;  // rad�ɕϊ��@�ڐ������́A���񑬓x�{������+
    sway_io_workbuf.dth[ID_BOOM_H] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_CAM1][SWAY_SENSOR_TG1].dth_y) / dy;// rad�ɕϊ�

    sway_io_workbuf.dthw[ID_SLEW] = sway_io_workbuf.dth[ID_SLEW] / w;
    sway_io_workbuf.dthw[ID_BOOM_H] = sway_io_workbuf.dth[ID_BOOM_H] / w;

    sway_io_workbuf.rad_amp2[ID_SLEW] = sway_io_workbuf.th[ID_SLEW] * sway_io_workbuf.th[ID_SLEW] + sway_io_workbuf.dthw[ID_SLEW] * sway_io_workbuf.dthw[ID_SLEW];
    sway_io_workbuf.rad_amp2[ID_BOOM_H] = sway_io_workbuf.th[ID_BOOM_H] * sway_io_workbuf.th[ID_BOOM_H] + sway_io_workbuf.dthw[ID_BOOM_H] * sway_io_workbuf.dthw[ID_BOOM_H];

    //�����U��  
    sway_io_workbuf.rad_amp2[ID_COMMON] = sway_io_workbuf.rad_amp2[ID_SLEW] + sway_io_workbuf.rad_amp2[ID_BOOM_H];

    //�ʑ�(X���j
    if (sway_io_workbuf.th[ID_SLEW] > 0.00001) {
        sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]);
    }
    else if (sway_io_workbuf.th[ID_SLEW] < -0.00001) { // atan()������0�����
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) + PI180;
        else                                    sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) - PI180;
    }
    else{ //�ʑ���-�΁`�΂͈̔͂ŕ\��
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = PI90;
        else                                    sway_io_workbuf.ph[ID_SLEW] = -PI90;
    }

    //�ʑ�(Y���j
    if (sway_io_workbuf.th[ID_BOOM_H] > 0.00001) {
        sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]);
    }
    else if (sway_io_workbuf.th[ID_BOOM_H] < -0.00001) { // atan()������0�����
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) + PI180;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) - PI180;
    }
    else { //�ʑ���-�΁`�΂͈̔͂ŕ\��
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = PI90;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = -PI90;
    }
    
	return 0;
}
//*********************************************************************************************
//IF�p�\�P�b�g
static WSADATA wsaData;
static SOCKET s;
static SOCKADDR_IN addrin;//���M�|�[�g�A�h���X
static SOCKADDR_IN server;//��M�|�[�g�A�h���X
static int serverlen, nEvent;
static int nRtn = 0, nRcv = 0, nSnd = 0;
static u_short port = SWAY_IF_IP_PORT_C;
static char szBuf[512];

std::wostringstream woMSG;
std::wstring wsMSG;


HWND CSwayIF::open_WorkWnd(HWND hwnd_parent) {
    InitCommonControls();//�R�����R���g���[��������

    WNDCLASSEX wc;

    hInst = GetModuleHandle(0);

    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WorkWndProc;// !CALLBACK��return��Ԃ��Ă��Ȃ���WindowClass�̓o�^�Ɏ��s����
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
    DestroyWindow(hWorkWnd);  //�E�B���h�E�j��
    hWorkWnd = NULL;
    return 0;
}
//*********************************************************************************************
int CSwayIF::init_sock(HWND hwnd) { 
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {    //WinSock�̏�����
        perror("WSAStartup Error\n");
        return -1;
    }

    s = socket(AF_INET, SOCK_DGRAM, 0);                  //Socket�I�[�v��
    if (s < 0) {
        perror("socket���s\n");
        return -2;
    }
    memset(&addrin, 0, sizeof(addrin));
    addrin.sin_port = htons(port);
    addrin.sin_family = AF_INET;
    inet_pton(AF_INET, SWAY_SENSOR_IP_ADDR, &addrin.sin_addr.s_addr);

    nRtn = bind(s, (LPSOCKADDR)&addrin, (int)sizeof(addrin)); //�\�P�b�g�ɖ��O��t����
    if (nRtn == SOCKET_ERROR) {
        perror("bind�G���[�ł�\n");
        closesocket(s);
        WSACleanup();
        return -3;
    }
  
    nRtn = WSAAsyncSelect(s, hwnd, ID_UDP_EVENT, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"�񓯊������s";
        closesocket(s);
        WSACleanup();
        return -4;
    }

    //���M��A�h���X�����l�ݒ�
    memset(&server, 0, sizeof(server));
    server.sin_port = htons(SWAY_IF_IP_PORT_C);
    server.sin_family = AF_INET;
  //  inet_pton(AF_INET, SWAY_SENSOR_IP_ADDR, &server.sin_addr.s_addr);
    inet_pton(AF_INET, "192.168.100.100", &server.sin_addr.s_addr);
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
        snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].body.command[0] = 'C';
        snd_msg[SID_CAM1][i_snd_msg[SID_CAM1]].body.command[0] = '1';
    }


    return 0;
}


//# �E�B���h�E�ւ̃��b�Z�[�W�\���@wstring
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

        InitCommonControls();//�R�����R���g���[��������
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

        //�U��Z���T���M�^�C�}�N��
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
        else {
            woMSG << L" sendto ERROR ";
        }
        tweet2sndMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
    }break;

    case ID_UDP_EVENT: {
        nEvent = WSAGETSELECTEVENT(lp);
        switch (nEvent) {
        case FD_READ: {
            nRcv++;
            serverlen = (int)sizeof(server);
 
            SOCKADDR temp_addr;
            int tempi = (int)sizeof(temp_addr);
//           nRtn = recvfrom(s, (char*)&rcv_msg[0][0], sizeof(ST_SWAY_RCV_MSG), 0, (SOCKADDR*)&server, &serverlen);
           nRtn = recvfrom(s, (char*)&rcv_msg[0][0], sizeof(ST_SWAY_RCV_MSG), 0, (SOCKADDR*)&temp_addr, &tempi);
           
            if (nRtn == SOCKET_ERROR) {
                woMSG << L" recvfrom ERROR";
                tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
            }
            else {
                woMSG << L" RCV len: " << nRtn << L" Count :" << nRcv ;
                woMSG << L">> ID: " << rcv_msg[0][0].head.id[0] << rcv_msg[0][0].head.id[1] << rcv_msg[0][0].head.id[2] << rcv_msg[0][0].head.id[3];
                woMSG << L"\n IP: " <<server.sin_addr.S_un.S_un_b.s_b1 << L"."<< server.sin_addr.S_un.S_un_b.s_b2 << L"." << server.sin_addr.S_un.S_un_b.s_b3 << L"." << server.sin_addr.S_un.S_un_b.s_b4;
                woMSG << L" PORT: " << server.sin_port;
                tweet2rcvMSG(woMSG.str()); woMSG.str(L"");woMSG.clear();
   
                woMSG << rcv_msg[0][0].head.time.wMonth << L"/" << rcv_msg[0][0].head.time.wDay << L" " << rcv_msg[0][0].head.time.wHour << L":" << rcv_msg[0][0].head.time.wMinute << L":" << rcv_msg[0][0].head.time.wSecond;
                woMSG << L"\n nPX:" << rcv_msg[0][0].head.cam_setting[SWAY_SENSOR_CAM1].pix_x << L" nPY:" << rcv_msg[0][0].head.cam_setting[SWAY_SENSOR_CAM1].pix_y << L" P1X:" << rcv_msg[0][0].head.cam_setting[SWAY_SENSOR_CAM1].pixlrad_x << L" P1Y:" << rcv_msg[0][0].head.cam_setting[SWAY_SENSOR_CAM1].pixlrad_y;
                woMSG << L"\n TilX :" << rcv_msg[0][0].head.tilt[SWAY_SENSOR_CAM1][SWAY_SENSOR_TIL_X] << L" TilY :" << rcv_msg[0][0].head.tilt[SWAY_SENSOR_CAM1][SWAY_SENSOR_TIL_Y] ;
                woMSG << L"\n\n thX :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].th_x << L" thY :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].th_y << L" wX :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].dth_x << L" wY :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].dth_y;
                woMSG << L"\n X0 :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].th_x0 << L" Y0 :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].th_y0 << L" tg1Size :" << rcv_msg[0][0].body.data[SWAY_SENSOR_CAM1][0].tg_size ;
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
 //       switch (LOWORD(wp)) {
 //       case ID_WORK_WND_CLOSE_PB: {
 //       }break;
 //      }
    }break;

    default:
        return DefWindowProc(hwnd, msg, wp, lp);
    }
    
    return 0; 
}