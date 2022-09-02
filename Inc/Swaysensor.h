#pragma once

#include <winsock.h>

#define SWAY_SENSOR_N_TARGET    4
#define SWAY_SENSOR_TG1         0
#define SWAY_SENSOR_TG2         1
#define SWAY_SENSOR_TG3         2
#define SWAY_SENSOR_TG4         3

typedef struct SwayComRcvHead { //�U��Z���T��M���b�Z�[�W�w�b�_��
    char	id[4];			//�@��̏��
    INT16	pix_x;			//�J������f��x��
    INT16	pix_y;			//�J������f��y��
    INT16	pixlrad_x;	    //�J��������\�@PIX/rad
    INT16	pixlrad_y;	    //�J��������\�@PIX/rad
    INT16	l0_x;			//�J������t�p�����[�^�o
    INT16	l0_y;			//�J������t�p�����[�^�o
    INT16	ph_x;			//�J������t�p�����[�^x1000000rad
    INT16	ph_y;			//�J������t�p�����[�^x1000000rad
    TIMEVAL time;			//�^�C���X�^���v
    char	mode[16];		//���[�h
    char	status[16];		//�X�e�[�^�X
    char	error[16];		//�X�e�[�^�X
    INT32	tilt_x;			//�J�����X�Ίpx�@x1000000rad
    INT32	tilt_y;			//�J�����X�Ίpy�@x1000000rad
}ST_SWAY_RCV_HEAD, * LPST_SWAY_RCV_HEAD;

typedef struct SwayComRcvDATA { //�U��Z���T��M���b�Z�[�W�f�[�^�\����
    INT32	th_x;			//�U�pxPIX
    INT32	th_y;			//�U�pyPIX
    INT32	dth_x;			//�U�p���xx�@PIX/s
    INT32	dth_y;			//�U�p���xy�@PIX/s
    INT32	th_x0;			//�U�p0�_xPIX
    INT32	th_y0;			//�U�p0�_yPIX
    INT16	tg1_size;		//�^�[�Q�b�g�P�T�C�Y
    INT16	tr2_size;		//�^�[�Q�b�g�Q�T�C�Y
    INT32	skew;			//�X�L���[�p�@PIX
}ST_SWAY_RCV_DATA, * LPST_SWAY_RCV_DATA;

typedef struct SwayComRcvBody { //�U��Z���T��M���b�Z�[�W�{�f�B��
    ST_SWAY_RCV_DATA data[4];
    INT16 info[4];
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
    INT16 freq;    //�ŏ���M����       
    INT32 d;       //�J����-�^�[�Q�b�g�ԋ���
}ST_SWAY_SND_BODY, * LPST_SWAY_SND_BODY;

typedef struct SwayComSndMsg { //�U��Z���T��M���b�Z�[�W
    ST_SWAY_SND_HEAD head;
    ST_SWAY_SND_BODY body;
}ST_SWAY_SND_MSG, * LPST_SWAY_SND_MSG;
