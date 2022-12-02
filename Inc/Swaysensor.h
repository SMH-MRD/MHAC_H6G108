#pragma once

#include <winsock.h>
#include <time.h>

#define SWAY_SENSOR_N_CAM       2
#define SWAY_SENSOR_CAM1        0
#define SWAY_SENSOR_CAM2        1
#define SWAY_SENSOR_TIL_X       0
#define SWAY_SENSOR_TIL_Y       1
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_N_TARGET    2
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_TG3         2
#define SWAY_SENSOR_TG4         3

typedef struct TargetStatus {
    char	mode[4];		//���[�h
    char	status[4];		//�X�e�[�^�X
    char	error[4];		//�X�e�[�^�X
} ST_TARGET_STAT, *LPST_TARGET_STAT;

typedef struct SyaCamSetting {
    INT32	pix_x;			//�J������f��x��
    INT32	pix_y;			//�J������f��y��
    INT32	pixlrad_x;	    //�J��������\�@PIX/rad
    INT32	pixlrad_y;	    //�J��������\�@PIX/rad
    INT32	l0_x;			//�J������t�p�����[�^�o
    INT32	l0_y;			//�J������t�p�����[�^�o
    INT32	ph_x;			//�J������t�p�����[�^x1000000rad
    INT32	ph_y;			//�J������t�p�����[�^x1000000rad
}ST_SWAY_CAM_SETTING, * LPST_SWAY_CAM_SETTING;
typedef struct SwayComRcvHead { //�U��Z���T��M���b�Z�[�W�w�b�_��
    char	id[4];			                                            //PC ID
    ST_SWAY_CAM_SETTING cam_setting[SWAY_SENSOR_N_CAM];                 //�J�����z�u���
    SYSTEMTIME time;		                                            //�^�C���X�^���v
    ST_TARGET_STAT tg_stat[SWAY_SENSOR_N_CAM][SWAY_SENSOR_N_TARGET];    //�J�����Q���^�[�Q�b�g�Q
    INT32	tilt[SWAY_SENSOR_N_CAM][2];			                        //�J�����X�Ίpx�@x1000000rad
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SwayComMainData { //�U��Z���T��M���b�Z�[�W�f�[�^�\����
    INT32	th_x;			//�U�pxPIX
    INT32	th_y;			//�U�pyPIX
    INT32	dth_x;			//�U�p���xx�@PIX/s
    INT32	dth_y;			//�U�p���xy�@PIX/s
    INT32	th_x0;			//�U�p0�_xPIX
    INT32	th_y0;			//�U�p0�_yPIX
    INT32	dpx_tgs;		//�^�[�Q�b�g�ԋ���X����
    INT32	dpy_tgs;		//�^�[�Q�b�g�ԋ���Y����
    INT32	tg_size;		//�^�[�Q�b�g�T�C�Y
}ST_SWAY_MAIN_DATA, * LPST_SWAY_MAIN_DATA;

typedef struct SwayComRcvBody { //�U��Z���T��M���b�Z�[�W�{�f�B��
    ST_SWAY_MAIN_DATA data[SWAY_SENSOR_N_CAM][SWAY_SENSOR_N_TARGET];
    char info[SWAY_SENSOR_N_CAM][32];
}ST_SWAY_RCV_BODY, * LPST_SWAY_RCV_BODY;



typedef struct SwayComRcvMsg { //�U��Z���T��M���b�Z�[�W
    ST_SWAY_RCV_HEAD head;
    ST_SWAY_RCV_BODY body;
}ST_SWAY_RCV_MSG, * LPST_SWAY_RCV_MSG;

typedef struct SwayComSndHead { //�U��Z���T���M���b�Z�[�W�w�b�_��
    char	id[4];			//�@��̏��
    sockaddr_in sockaddr;       //���M��IP�A�h���X
}ST_SWAY_SND_HEAD, * LPST_SWAY_SND_HEAD;

typedef struct SwayComSndBody { //�U��Z���T���M���b�Z�[�W�{�f�B��
    char command[2];
    char mode[40];
    INT16 freq;    //�ŏ���M����       
    INT32 d;       //�J����-�^�[�Q�b�g�ԋ���
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //�U��Z���T��M���b�Z�[�W
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;
