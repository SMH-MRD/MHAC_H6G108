#pragma once

#include "resource.h"

#include "CSHAREDMEM.H"
#include "COMMON_DEF.H"
#include "CVector3.h"
#include "Spec.h"

///# �x�[�X�ݒ�

//-�^�X�N�ݒ�
#define TARGET_RESOLUTION			1		//�}���`���f�B�A�^�C�}�[����\ msec
#define SYSTEM_TICK_ms				50		//���C���X���b�h���� msec

//-Main Window�̔z�u�ݒ�
#define MAIN_WND_INIT_SIZE_W		1030    //-Main Window�̏����T�C�Y�@W
#define MAIN_WND_INIT_SIZE_H		480		//-Main Window�̏����T�C�Y�@H
#define MAIN_WND_INIT_POS_X			20		//-Main Window�̏����ʒu�ݒ�@X
#define MAIN_WND_INIT_POS_Y			585		//-Main Window�̏����ʒu�ݒ�@Y

//�\���X�V�^�C�}�[ID
#define ID_UPDATE_TIMER				100

//�\���X�V����(msec)
#define TIMER_PRIOD        			200

#define ID_STATUS					WM_USER + 600

