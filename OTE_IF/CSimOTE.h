#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X
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


#define ID_MULTI_MSG_SET_MODE_INIT          1
#define ID_MULTI_MSG_SET_MODE_CONST         0



#define ID_UDP_EVENT_U_SIM  			    10704
#define ID_UDP_EVENT_M_TE_SIM			    10705
#define ID_UDP_EVENT_M_CR_SIM			    10706


#define ID_STATIC_OTE_SIM_LABEL_COM          10707
#define ID_STATIC_OTE_SIM_VIEW_STAT_U        10708
#define ID_STATIC_OTE_SIM_VIEW_INF_U         10709

#define ID_STATIC_OTE_SIM_VIEW_STAT_TE       10710
#define ID_STATIC_OTE_SIM_VIEW_INF_TE        10711

#define ID_STATIC_OTE_SIM_VIEW_STAT_CR       10712
#define ID_STATIC_OTE_SIM_VIEW_INF_CR        10713

#define ID_CHK_OTE_SIM_MSG_SND               10714
#define IDC_RADIO_DISP_MON_OTE               10715
#define IDC_RADIO_DISP_MON_SIM               10716

#define ID_STATIC_MON_OTE_U                  10717
#define ID_STATIC_MON_CR_U                   10718
#define ID_STATIC_MON_OTE_M                  10719
#define ID_STATIC_MON_CR_M                   10720

#define ID_STATIC_MON_OTE_U_LABEL            10721
#define ID_STATIC_MON_CR_U_LABEL             10722
#define ID_STATIC_MON_OTE_M_LABEL            10723
#define ID_STATIC_MON_CR_M_LABEL             10724

//�N���^�C�}�[ID
#define ID_OTE_SIM_TIMER					106
#define OTE_SIM_SCAN_TIME					1000		// �}���`�L���X�g IF���M����msec


#define SIM_WORK_WND_X						1050		//�����e�p�l���\���ʒuX
#define SIM_WORK_WND_Y						400			//�����e�p�l���\���ʒuY
#define SIM_WORK_WND_W						800		    //�����e�p�l��WINDOW��
#define SIM_WORK_WND_H						600			//�����e�p�l��WINDOW����

#define OTE_SIM_CODE_MON_OTE                0
#define OTE_SIM_CODE_MON_SIM                1



class CSimOTE : public CBasicControl
{
private:

    HINSTANCE hInst;

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    static CSharedMem* pOteIOObj;
    static LPST_OTE_IO pOTEio;
    static ST_OTE_IO ote_io_workbuf;

    //Work Window�\���p
    static HWND hwndSTAT_U;
    static HWND hwndINFMSG_U;

    static HWND hwndSTAT_M_TE;
    static HWND hwndINFMSG_M_TE;

    static HWND hwndSTAT_M_CR;
    static HWND hwndINFMSG_M_CR;

    static HWND h_chkMsgSnd;
    static HWND h_radio_disp_monOTE;
    static HWND h_radio_disp_monSIM;

    static HWND hwndMON_U_OTE;
    static HWND hwndMON_U_CR;
    static HWND hwndMON_M_OTE;
    static HWND hwndMON_M_CR;
    static HWND hwndMON_U_OTE_LABEL;
    static HWND hwndMON_U_CR_LABEL;
    static HWND hwndMON_M_OTE_LABEL;
    static HWND hwndMON_M_CR_LABEL;

   
    static void tweet2infMSG(const std::wstring& srcw, int code);
    static void tweet2statusMSG(const std::wstring& srcw, int code);
    static void tweet2static(const std::wstring& srcw, HWND hwnd);

    //IF�p�\�P�b�g
    static WSADATA wsaData;
    static SOCKET s_u;                              //���j�L���X�g��M�\�P�b�g
    static SOCKET s_m_te, s_m_cr;                   //�}���`�L���X�g��M�\�P�b�g
    static SOCKET s_m_snd, s_m_snd_dbg;             //�}���`�L���X�g���M�\�P�b�g
    static SOCKADDR_IN addrin_u;                    //���j�L���X�g��M�A�h���X
    static SOCKADDR_IN addrin_ote_u;                //���j�L���X�g���M�A�h���X
    static SOCKADDR_IN addrin_m_te, addrin_m_cr;    //�}���`�L���X�g��M�A�h���X
    static SOCKADDR_IN addrin_m_snd;//�}���`�L���X�g���M�A�h���X
    static u_short port_u;                          //���j�L���X�g��M�|�[�g
    static u_short port_m_te, port_m_cr;           //�}���`�L���X�g��M�|�[�g

    static std::wostringstream woMSG;
    static std::wstring wsMSG;

public:
    CSimOTE();
    ~CSimOTE();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;

    static int send_msg_u();                //���j�L���X�g���M
    static int send_msg_m_te();             //�}���`�L���X�g���M
    static int set_msg_m_te(int mode, INT32 code, INT32 status);        //�}���`�L���X�g���M���b�Z�[�W�Z�b�g
    static int set_msg_u(int mode, INT32 code, INT32 status);          //���j�L���X�g���M���b�Z�[�W�Z�b�g

    static int n_active_ote;
    static int connect_no_onboad;
    static int connect_no_remorte;
    static int my_connect_no;

    static int is_ote_msg_snd;
    static int panel_disp_mode;

    void set_debug_mode(int id) {
        if (id) mode |= OTE_IF_DBG_MODE;
        else    mode &= ~OTE_IF_DBG_MODE;
    }

    int is_debug_mode() { return(mode & OTE_IF_DBG_MODE); }

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���


    virtual HWND open_WorkWnd(HWND hwnd_parent);
    static LRESULT CALLBACK OteSimWndProc(HWND, UINT, WPARAM, LPARAM);
    static int close_WorkWnd();
    static int init_sock_u(HWND hwnd);
    static int init_sock_m_te(HWND hwnd);
    static int init_sock_m_cr(HWND hwnd);

};
