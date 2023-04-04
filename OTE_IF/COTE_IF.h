#pragma once

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

#define SWAY_IF_SWAY_IO_MEM_NG              0x8000
#define SWAY_IF_CRANE_MEM_NG                0x4000
#define SWAY_IF_SIM_MEM_NG                  0x2000
#define SWAY_IF_SIM_DBG_MODE                0x00000010	//�U��f�[�^��SIM�o�͂��琶��


#define ID_STATIC_SWAY_IF_LABEL_RCV         10602
#define ID_STATIC_SWAY_IF_LABEL_SND         10603
#define ID_STATIC_SWAY_IF_VIEW_RCV          10604
#define ID_STATIC_SWAY_IF_VIEW_SND          10605

#define ID_UDP_EVENT				        10606

#define ID_STATIC_SWAY_IF_DISP_SELBUF       10607
#define ID_PB_SWAY_IF_CHG_DISP_SENSOR       10608
#define ID_PB_SWAY_IF_CHG_DISP_BUF          10609
#define ID_PB_SWAY_IF_CHG_DISP_CAM          10610
#define ID_PB_SWAY_IF_CHG_DISP_TG           10611

#define ID_PB_SWAY_IF_INFO_COMDATA          10612
#define ID_PB_SWAY_IF_INFO_MSG              10613
#define ID_PB_SWAY_IF_MIN_CYCLE_10mUP       10614
#define ID_PB_SWAY_IF_MIN_CYCLE_10mDN       10615
#define ID_STATIC_SWAY_IF_MINCYCLE          10616


//�N���^�C�}�[ID
#define ID_WORK_WND_TIMER					100
#define WORK_SCAN_TIME						2000			// �}���`�L���X�g IF���M����msec


#define WORK_WND_X							1050		//�����e�p�l���\���ʒuX
#define WORK_WND_Y							394			//�����e�p�l���\���ʒuY
#define WORK_WND_W							540		    //�����e�p�l��WINDOW��
#define WORK_WND_H							480			//�����e�p�l��WINDOW����


class COteIF :  public CBasicControl
{
private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
  CSharedMem* pOteIOObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
  CSharedMem* pCraneStatusObj;
  CSharedMem* pSimulationStatusObj;


    HINSTANCE hInst;

    void init_rcv_msg();

public:
    COteIF();
    ~COteIF();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;

    static ST_UOTE_SND_MSG snd_msg_u;
    static ST_UOTE_RCV_MSG rcv_msg_u;
    static ST_MOTE_SND_MSG snd_msg_m;
    static ST_MOTE_RCV_MSG rcv_msg_m;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

  
     virtual HWND open_WorkWnd(HWND hwnd_parent);
     static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
     static int close_WorkWnd();
     static int init_sock(HWND hwnd);

};
