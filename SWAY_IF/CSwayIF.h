#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X
#include "Spec.h"
#include "Swaysensor.h"

#include <commctrl.h>
#include <time.h>
#include <string>

#define SWAY_IF_SWAY_IO_MEM_NG              0x8000
#define SWAY_IF_CRANE_MEM_NG                0x4000
#define SWAY_IF_SIM_MEM_NG                  0x2000

#define ID_STATIC_SWAY_IF_LABEL_RCV         10502
#define ID_STATIC_SWAY_IF_LABEL_SND         10503
#define ID_STATIC_SWAY_IF_VIEW_RCV          10504
#define ID_STATIC_SWAY_IF_VIEW_SND          10505

#define ID_UDP_EVENT				        10506

//�N���^�C�}�[ID
#define ID_WORK_WND_TIMER					100
#define WORK_SCAN_TIME						500			// SWAY IF���M�`�F�b�N����msec


#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

#define N_SWAY_SENSOR_RCV_BUF   10  //��M�f�[�^�̃o�b�t�@��
#define N_SWAY_SENSOR_SND_BUF   10  //���M�f�[�^�̃o�b�t�@��

#define WORK_WND_X							1050		//MAP�\���ʒuX
#define WORK_WND_Y							394			//MAP�\���ʒuY
#define WORK_WND_W							400		    //MAP WINDOW��
#define WORK_WND_H							180			//MAP WINDOW����

class CSwayIF :
    public CBasicControl
{
private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pSwayIOObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;

    ST_SWAY_RCV_MSG rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
    ST_SWAY_SND_MSG snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
    int i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
    int i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
    double SwayCamParam[N_SWAY_SENSOR][SWAY_SENSOR_N_AXIS][CAM_SET_PARAM_N_PARAM]; //�U��Z���T�J�����ݒu�p�����[�^[�J����No.][������XY][a,b]

    int parse_sway_stat(int ID);
    HINSTANCE hInst;

    static void tweet2statusMSG(const std::wstring& srcw);
    static void tweet2rcvMSG(const std::wstring& srcw);
    static void tweet2sndMSG(const std::wstring& srcw);
 

public:
    CSwayIF();
    ~CSwayIF();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

    void set_debug_mode(int id) {
        if (id) mode |= SWAY_IF_SIM_DBG_MODE;
        else    mode &= ~SWAY_IF_SIM_DBG_MODE;
    }

    int is_debug_mode() { return(mode & SWAY_IF_SIM_DBG_MODE); }

     //�ǉ����\�b�h
     int set_sim_status(LPST_SWAY_IO pworkbuf);   //�f�o�b�O���[�h����Simulator����̓��͂ŏo�͓��e���㏑��

     virtual HWND open_WorkWnd(HWND hwnd_parent);
     static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
     static int close_WorkWnd();
     static int init_sock(HWND hwnd);
     static HWND hwndSTATMSG;
     static HWND hwndRCVMSG;
     static HWND hwndSNDMSG;


};

