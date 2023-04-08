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
#define ID_STATIC_OTE_SIM_VIEW_STAT_U        10709
#define ID_STATIC_OTE_SIM_VIEW_RCV_U         10709
#define ID_STATIC_OTE_SIM_VIEW_SND_U         10710
#define ID_STATIC_OTE_SIM_VIEW_INF_U         10711

#define ID_STATIC_OTE_SIM_VIEW_STAT_TE       10711
#define ID_STATIC_OTE_SIM_VIEW_RCV_TE        10712
#define ID_STATIC_OTE_SIM_VIEW_SND_TE        10713
#define ID_STATIC_OTE_SIM_VIEW_INF_TE        10714

#define ID_STATIC_OTE_SIM_VIEW_STAT_CR       10715
#define ID_STATIC_OTE_SIM_VIEW_RCV_CR        10716
#define ID_STATIC_OTE_SIM_VIEW_SND_CR        10717
#define ID_STATIC_OTE_SIM_VIEW_INF_CR        10718

#define ID_STATIC_OTE_SIM_VIEW_U_CNT         10719
#define ID_STATIC_OTE_SIM_VIEW_TE_CNT        10720
#define ID_STATIC_OTE_SIM_VIEW_CR_CNT        10721


//�N���^�C�}�[ID
#define ID_OTE_SIM_TIMER					106
#define OTE_SIM_SCAN_TIME					1000		// �}���`�L���X�g IF���M����msec


#define SIM_WORK_WND_X						1050		//�����e�p�l���\���ʒuX
#define SIM_WORK_WND_Y						400			//�����e�p�l���\���ʒuY
#define SIM_WORK_WND_W						540		    //�����e�p�l��WINDOW��
#define SIM_WORK_WND_H						370			//�����e�p�l��WINDOW����


class CSimOTE : public CBasicControl
{
private:

    HINSTANCE hInst;

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    static CSharedMem* pOteIOObj;
    static ST_OTE_IO ote_io_workbuf;

    void init_rcv_msg();

    static void tweet2rcvMSG(const std::wstring& srcw, int code);
    static void tweet2sndMSG(const std::wstring& srcw, int code);
    static void tweet2infMSG(const std::wstring& srcw, int code);
    static void tweet2statusMSG(const std::wstring& srcw, int code);

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
    static int set_msg_m_te(int mode, INT32 code, INT32 status);      //�}���`�L���X�g���M���b�Z�[�W�Z�b�g
    static int set_msg_m_te(int mode);      //�}���`�L���X�g���M���b�Z�[�W�Z�b�g

    //Work Window�\���p
    static HWND hwndSTAT_U;
    static HWND hwndRCVMSG_U;
    static HWND hwndSNDMSG_U;
    static HWND hwndINFMSG_U;

    static HWND hwndSTAT_M_TE;
    static HWND hwndRCVMSG_M_TE;
    static HWND hwndSNDMSG_M_TE;
    static HWND hwndINFMSG_M_TE;

    static HWND hwndSTAT_M_CR;
    static HWND hwndRCVMSG_M_CR;
    static HWND hwndSNDMSG_M_CR;
    static HWND hwndINFMSG_M_CR;

    static HWND hwndCNT_U;
    static HWND hwndCNT_M_TE;
    static HWND hwndCNT_M_CR;

    static int n_active_ote;
    static int connect_no_onboad;
    static int connect_no_remorte;
    static int my_connect_no;


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

