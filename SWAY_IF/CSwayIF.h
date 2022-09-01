#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X
#include "Spec.h"
#include <winsock.h>

#define SWAY_IF_SWAY_IO_MEM_NG      0x8000
#define SWAY_IF_CRANE_MEM_NG        0x4000
#define SWAY_IF_SIM_MEM_NG          0x2000

#define CAM_SET_PARAM_N_CAM         2
#define CAM_SET_PARAM_N_AXIS        2
#define CAM_SET_PARAM_X_AXIS        0
#define CAM_SET_PARAM_Y_AXIS        1
#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

typedef struct SwayComRcvHead { //�U��Z���T��M���b�Z�[�W�w�b�_��
    char	id[4];			//�@��̏��
    UINT16	pix_x;			//�J������f��x��
    UINT16	pix_y;			//�J������f��y��
    UINT16	pixlrad_x;	    //�J��������\�@PIX/rad
    UINT16	pixlrad_y;	    //�J��������\�@PIX/rad
    UINT16	l0_x;			//�J������t�p�����[�^�o
    UINT16	l0_y;			//�J������t�p�����[�^�o
    UINT16	ph_x;			//�J������t�p�����[�^x1000000rad
    UINT16	ph_y;			//�J������t�p�����[�^x1000000rad
    TIMEVAL time;			//�^�C���X�^���v
    char	mode[16];		//���[�h
    char	status[16];		//�X�e�[�^�X
    char	error[16];		//�X�e�[�^�X
    UINT32	tilt_x;			//�J�����X�Ίpx�@x1000000rad
    UINT32	tilt_y;			//�J�����X�Ίpy�@x1000000rad
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SwayComRcvDATA { //�U��Z���T��M���b�Z�[�W�f�[�^�\����
    UINT32	th_x;			//�U�pxPIX
    UINT32	th_y;			//�U�pyPIX
    UINT32	dth_x;			//�U�p���xx�@PIX/s
    UINT32	dth_y;			//�U�p���xy�@PIX/s
    UINT32	th_x0;			//�U�p0�_xPIX
    UINT32	th_y0;			//�U�p0�_yPIX
    UINT16	tg1_size;		//�^�[�Q�b�g�P�T�C�Y
    UINT16	tr2_size;		//�^�[�Q�b�g�Q�T�C�Y
    UINT32	skew;			//�X�L���[�p�@PIX
}ST_SWAY_RCV_DATA, * LPST_SWAY_RCV_DATA;

typedef struct SwayComRcvBody { //�U��Z���T��M���b�Z�[�W�{�f�B��
    ST_SWAY_RCV_DATA data[4];
    UINT16 info[4];
}ST_SWAY_RCV_BODY, * LPST_SWAY_RCV_BODY;

typedef struct SwayComRcvMsg { //�U��Z���T��M���b�Z�[�W
    ST_SWAY_RCV_HEAD head;
    ST_SWAY_RCV_BODY body;
}ST_SWAY_RCV_MSG, * LPST_SWAY_RCV_MSG;

typedef struct SwayComSndHead { //�U��Z���T���M���b�Z�[�W�w�b�_��
    char	id[4];			//�@��̏��
    sockaddr_in addr;       //���M��IP�A�h���X
}ST_SWAY_SND_HEAD, * LPST_SWAY_SND_HEAD;

typedef struct SwayComSndBody { //�U��Z���T���M���b�Z�[�W�{�f�B��
    char command[2];
    char mode[40];
    UINT16 freq;    //�ŏ���M����       
    UINT32 d;       //�J����-�^�[�Q�b�g�ԋ���
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //�U��Z���T��M���b�Z�[�W
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;

#define N_SWAY_SENSOR           2   //�U��Z���T�̐�
#define SWAY_SENSOR1            0   //�U��Z���TID1
#define SWAY_SENSOR2            1   //�U��Z���TID2
#define N_SWAY_SENSOR_RCV_BUF   10  //��M�f�[�^�̃o�b�t�@��
#define N_SWAY_SENSOR_SND_BUF   10  //���M�f�[�^�̃o�b�t�@��

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
    int i_rcv_msg[N_SWAY_SENSOR] = { 0,0 };
    int i_snd_msg[N_SWAY_SENSOR] = { 0,0 };

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
    double SwayCamParam[CAM_SET_PARAM_N_CAM][CAM_SET_PARAM_N_AXIS][CAM_SET_PARAM_N_PARAM]; //�U��Z���T�J�����ݒu�p�����[�^[�J����No.][������XY][a,b]

    LPST_SIMULATION_STATUS pSim;    //�V�~�����[�^�X�e�[�^�X

    int parse_sway_stat(int ID);

public:
    CSwayIF();
    ~CSwayIF();

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
};

