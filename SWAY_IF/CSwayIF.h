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

#define ID_STATIC_SWAY_IF_DISP_SELBUF       10507
#define ID_PB_SWAY_IF_CHG_DISP_SENSOR       10508
#define ID_PB_SWAY_IF_CHG_DISP_BUF          10509
#define ID_PB_SWAY_IF_CHG_DISP_CAM          10510
#define ID_PB_SWAY_IF_CHG_DISP_TG           10511

#define ID_PB_SWAY_IF_INFO_COMDATA          10512
#define ID_PB_SWAY_IF_INFO_MSG              10513
#define ID_PB_SWAY_IF_MIN_CYCLE_10mUP       10514
#define ID_PB_SWAY_IF_MIN_CYCLE_10mDN       10515
#define ID_STATIC_SWAY_IF_MINCYCLE          10516


//�U��v�Z���[�h
#define ID_SWAY_CAL_NORMAL                  0x0000
#define ID_SWAY_CAL_NO_OFFSET               0x0001
#define ID_SWAY_CAL_NO_TILT                 0x0002
//�N���^�C�}�[ID
#define ID_WORK_WND_TIMER					100
#define WORK_SCAN_TIME						2000			// SWAY IF���M�`�F�b�N����msec

#define WORK_SCAN_TIME						2000			// SWAY IF���M�`�F�b�N����msec


#define N_SWAY_SENSOR_RCV_BUF   4  //��M�f�[�^�̃o�b�t�@��
#define N_SWAY_SENSOR_SND_BUF   4  //���M�f�[�^�̃o�b�t�@��

#define WORK_WND_X							1050		//�����e�p�l���\���ʒuX
#define WORK_WND_Y							394			//�����e�p�l���\���ʒuY
#define WORK_WND_W							540		    //�����e�p�l��WINDOW��
#define WORK_WND_H							480			//�����e�p�l��WINDOW����

typedef struct SyayCamWork {
    bool    is_read_from_msg;   //�d������ǂݍ��ݍσt���O
    double	th_cam;			    //�J�������o�p�{�J���������t���p(rad)
    double	til_cam;		    //�X�Όv���o�p�{�J�����ݒu�ʒu�p�x(rad)
    double	dth_cam;		    //�J�������o�p���x(rad/s)
    double	dtil_cam;		    //�X�Όv���o�p���x(rad/s)
    double	L0;	                //�J�������ݒu�p�x
    double	PH0;	            //�J�������ݒu�p�x
    double  l0;                 //�J������t����
    double	ph0;	            //�J������t�p�x
    double	phc;	            //�J���������t���p�x
    double	C;	                //�J��������\�@rad/PIX �U��Z���T����̌W���iPIX/rad�j�̋t��
}ST_SWAY_CAM_WORK, * LPST_SWAY_CAM_WORK;

class CSwayIF :
    public CBasicControl
{
private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pSwayIOObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;

    static ST_SWAY_RCV_MSG rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
    static ST_SWAY_SND_MSG snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
    static int i_rcv_msg[N_SWAY_SENSOR];
    static int i_snd_msg[N_SWAY_SENSOR];

    static LPST_CRANE_STATUS pCraneStat;
    static LPST_SIMULATION_STATUS pSimStat;
   
    static ST_SWAY_IO sway_io_workbuf;                             //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
   
    static int get_sensor_param_from_msg(LPST_SWAY_RCV_MSG pmsg);  //�U��v�Z�p�Z���T�p�����[�^��ʐM�d�����ǂݍ���
    static int parse_sway_stat(LPST_SWAY_RCV_MSG pmsg);            //�U��v�Z

    HINSTANCE hInst;

    static void tweet2rcvMSG(const std::wstring& srcw);
    static void tweet2sndMSG(const std::wstring& srcw);
    static void tweet2infMSG(const std::wstring& srcw);
    static void tweet2statusMSG(const std::wstring& srcw);
    static void update_buf_set_disp_msg(HWND hwnd);

    void set_sensor_msg();
 
    //�U��Z���T�ʐM�\���p 

    static HWND hwndSTATMSG;
    static HWND hwndRCVMSG;
    static HWND hwndSNDMSG;
    static HWND hwndINFMSG;

    static HWND hwndDispBufMSG;
    static HWND hwndCamChangePB;
    static HWND hwndBufChangePB;
    static HWND hwndTargetChangePB;

    static HWND hwndInfComPB;
    static HWND hwndInfMsgPB;
    static HWND hwndCycleUpPB;
    static HWND hwndCycleDnPB;

    static int iDispSensor;
    static int iDispBuf;
    static int iDispCam;
    static int iDispTg;
    static INT32 cycle_min_ms;
    static INT32 sens_mode;

    static ST_SWAY_CAM_WORK swx;
    static ST_SWAY_CAM_WORK swy;

    static wstring ws_sensor_err_msg[64];
    static wstring ws_sensor_stat_msg[64];

 

public:
    CSwayIF();
    ~CSwayIF();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;
    static INT32 cal_mode;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

    static int send_msg(int sensor_id, INT32 com_id);   //���M�����̓^�C�}�[�N��

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

 
};

