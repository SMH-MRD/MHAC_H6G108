#include "CSimOTE.h"
#include <windowsx.h>       //# �R�����R���g���[��

#include <winsock2.h>

#include <iostream>
#include <iomanip>
#include <sstream>

HWND CSimOTE::hWorkWnd = NULL;

//Work Window�\���p
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


//IF�p�\�P�b�g
WSADATA CSimOTE::wsaData;
SOCKET CSimOTE::s_u;                                         //���j�L���X�g��M�\�P�b�g
SOCKET CSimOTE::s_m_te, CSimOTE::s_m_cr;                      //�}���`�L���X�g��M�\�P�b�g
SOCKADDR_IN CSimOTE::addrin_u;                               //���j�L���X�g��M�A�h���X
SOCKADDR_IN CSimOTE::addrin_m_te, CSimOTE::addrin_m_cr;       //�}���`�L���X�g��M�A�h���X
SOCKADDR_IN CSimOTE::addrin_ote_u;                           //���j�L���X�g���M�A�h���X

u_short CSimOTE::port_u = OTE_IF_IP_UNICAST_PORT_C;          //���j�L���X�g��M�|�[�g
u_short CSimOTE::port_m_te = OTE_IF_IP_MULTICAST_PORT_TE;
u_short CSimOTE::port_m_cr = OTE_IF_IP_MULTICAST_PORT_CR;    //�}���`�L���X�g��M�|�[�g

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

int CSimOTE::set_outbuf(LPVOID) { return 0; }    //�o�̓o�b�t�@�Z�b�g

/*****************************************************************************/
/*����������                                                                 */
/*****************************************************************************/
int CSimOTE::init_proc() {
 
    //�f�o�b�O���[�h�@ON�@���Ԃł�OFF�ŏ�����
#ifdef _DVELOPMENT_MODE
    set_debug_mode(L_ON);
#else
    set_debug_mode(L_OFF);
#endif
    return 0;
}
int CSimOTE::input() { return 0; }               //���͏���
int CSimOTE::parse() { return 0; }               //���C������
int CSimOTE::output() { return 0; }              //�o�͏���

std::wostringstream CSimOTE::woMSG;
std::wstring CSimOTE::wsMSG;

static struct ip_mreq mreq_te, mreq_cr;                     //�}���`�L���X�g��M�ݒ�p�\����
static int addrlen, nEvent;
static int nRtn = 0, nRcv_u = 0, nSnd_u = 0, nRcv_te = 0, nSnd_te = 0, nRcv_cr = 0;

static char szBuf[512];



//*********************************************************************************************
/*���j�^�p�E�B���h�E�����֐�*/
HWND CSimOTE::open_WorkWnd(HWND hwnd_parent) {
    InitCommonControls();//�R�����R���g���[��������

    WNDCLASSEX wc;

    hInst = GetModuleHandle(0);

    ZeroMemory(&wc, sizeof(wc));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = OteSimWndProc;// !CALLBACK��return��Ԃ��Ă��Ȃ���WindowClass�̓o�^�Ɏ��s����
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
    DestroyWindow(hWorkWnd);  //�E�B���h�E�j��
    hWorkWnd = NULL;
    return 0;
}
/*********************************************************************************************/
/*   �\�P�b�g,���M�A�h���X�̏������@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@�@*/
/*********************************************************************************************/
int CSimOTE::init_sock_u(HWND hwnd) {    //���j�L���X�g
    if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) {    //WinSock�̏�����
        perror("WSAStartup Error\n");
        return -1;
    }

    //# ��M�\�P�b�g����
    s_u = socket(AF_INET, SOCK_DGRAM, 0);                  //Socket�I�[�v��
    if (s_u < 0) {
        perror("socket���s\n");
        return -2;
    }
    memset(&addrin_u, 0, sizeof(addrin_u));
    addrin_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_C);        //�[������M�|�[�g
    addrin_u.sin_family = AF_INET;
    inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_u.sin_addr.s_addr);

    //# ���M��A�h���Xdefault�ݒ�
    memset(&addrin_ote_u, 0, sizeof(addrin_ote_u));
    addrin_ote_u.sin_port = htons(OTE_IF_IP_UNICAST_PORT_S);    //�N���[��������M�|�[�g
    addrin_ote_u.sin_family = AF_INET;
    inet_pton(AF_INET, CTRL_PC_IP_ADDR_OTE, &addrin_ote_u.sin_addr.s_addr);


    nRtn = bind(s_u, (LPSOCKADDR)&addrin_u, (int)sizeof(addrin_u)); //�\�P�b�g�ɖ��O��t����
    if (nRtn == SOCKET_ERROR) {
        perror("bind�G���[�ł�\n");
        closesocket(s_u);
        WSACleanup();
        return -3;
    }

    nRtn = WSAAsyncSelect(s_u, hwnd, ID_UDP_EVENT_U_SIM, FD_READ | FD_WRITE | FD_CLOSE);

    if (nRtn == SOCKET_ERROR) {
        woMSG << L"�񓯊������s";
        closesocket(s_u);
        WSACleanup();
        return -4;
    }

    return 0;
}
int CSimOTE::init_sock_m_te(HWND hwnd) {

    //�}���`�L���X�g�p�\�P�b�g
    {
        //�^�[�~�i������M�p
        s_m_te = socket(AF_INET, SOCK_DGRAM, 0);                                //Socket�I�[�v��
        if (s_m_te < 0) {
            perror("socket���s\n");
            return -5;
        }
        memset(&addrin_m_te, 0, sizeof(addrin_m_te));                           //�\�P�b�g�ɖ��O��t����
        addrin_m_te.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_TE);              //�[�����p�|�[�g
        addrin_m_te.sin_family = AF_INET;
        inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_m_te.sin_addr.s_addr);

        nRtn = bind(s_m_te, (LPSOCKADDR)&addrin_m_te, (int)sizeof(addrin_m_te)); //�\�P�b�g�ɖ��O��t����
        if (nRtn == SOCKET_ERROR) {
            perror("bind�G���[�ł�\n");
            closesocket(s_m_te);
            WSACleanup();
            return -6;
        }

        nRtn = WSAAsyncSelect(s_m_te, hwnd, ID_UDP_EVENT_M_TE_SIM, FD_READ | FD_WRITE | FD_CLOSE);

        if (nRtn == SOCKET_ERROR) {
            woMSG << L"�񓯊������s";
            closesocket(s_m_te);
            WSACleanup();
            return -7;
        }

        //�}���`�L���X�g�O���[�v�Q���o�^
        memset(&mreq_te, 0, sizeof(mreq_te));
        mreq_te.imr_interface.S_un.S_addr = inet_addr(OTE_DEFAULT_IP_ADDR);     //�p�P�b�g�o�͌�IP�A�h���X
        mreq_te.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);       //�}���`�L���X�gIP�A�h���X
        if (setsockopt(s_m_te, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_te, sizeof(mreq_te)) != 0) {
            perror("setopt��M�ݒ莸�s\n");
            return -8;
        }
    }
    return 0;
}
int CSimOTE::init_sock_m_cr(HWND hwnd) {

    //�}���`�L���X�g�p�\�P�b�g
    {
        //����PC����M�p
        s_m_cr = socket(AF_INET, SOCK_DGRAM, 0);                  //Socket�I�[�v��
        if (s_m_cr < 0) {
            perror("socket���s\n");
            return -9;
        }
        memset(&addrin_m_cr, 0, sizeof(addrin_m_cr));
        addrin_m_cr.sin_port = htons(OTE_IF_IP_MULTICAST_PORT_CR);
        addrin_m_cr.sin_family = AF_INET;
        inet_pton(AF_INET, OTE_DEFAULT_IP_ADDR, &addrin_m_cr.sin_addr.s_addr);

        nRtn = bind(s_m_cr, (LPSOCKADDR)&addrin_m_cr, (int)sizeof(addrin_m_cr)); //�\�P�b�g�ɖ��O��t����
        if (nRtn == SOCKET_ERROR) {
            perror("bind�G���[�ł�\n");
            closesocket(s_m_cr);
            WSACleanup();
            return -10;
        }

        nRtn = WSAAsyncSelect(s_m_cr, hwnd, ID_UDP_EVENT_M_CR_SIM, FD_READ | FD_WRITE | FD_CLOSE);

        if (nRtn == SOCKET_ERROR) {
            woMSG << L"�񓯊������s";
            closesocket(s_m_cr);
            WSACleanup();
            return -11;
        }

        //�}���`�L���X�g�O���[�v�Q���o�^
        memset(&mreq_cr, 0, sizeof(mreq_cr));
        mreq_cr.imr_interface.S_un.S_addr = inet_addr(OTE_DEFAULT_IP_ADDR);     //�p�P�b�g�o�͌�IP�A�h���X
        mreq_cr.imr_multiaddr.S_un.S_addr = inet_addr(OTE_MULTI_IP_ADDR);      //�}���`�L���X�gIP�A�h���X
        if (setsockopt(s_m_cr, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq_cr, sizeof(mreq_cr)) != 0) {
            perror("setopt��M�ݒ莸�s\n");
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

        InitCommonControls();//�R�����R���g���[��������
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

        //�}���`�L���X�g���M�d�������l�Z�b�g
        set_msg_m_te(ID_MULTI_MSG_SET_MODE_INIT);

        //�}���`�L���X�g���M�^�C�}�N��
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

            SOCKADDR from_addr;                             //���M���A�h���X��荞�݃o�b�t�@
            int from_addr_size = (int)sizeof(from_addr);    //���M���A�h���X�T�C�Y�o�b�t�@

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
            SOCKADDR from_addr;                             //���M���A�h���X��荞�݃o�b�t�@
            int from_addr_size = (int)sizeof(from_addr);    //���M���A�h���X�T�C�Y�o�b�t�@

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
            SOCKADDR from_addr;                             //���M���A�h���X��荞�݃o�b�t�@
            int from_addr_size = (int)sizeof(from_addr);    //���M���A�h���X�T�C�Y�o�b�t�@

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
        // �I�����ꂽ���j���[�̉��:
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

//# �E�B���h�E�ւ̃��b�Z�[�W�\���@wstring
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