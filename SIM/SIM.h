#pragma once

#include "resource.h"
///# �x�[�X�ݒ�

//-�^�X�N�ݒ�
#define TARGET_RESOLUTION			1		//�}���`���f�B�A�^�C�}�[����\ msec
#define SYSTEM_TICK_ms				25		//���C���X���b�h���� msec

//-Main Window�̔z�u�ݒ�
#define MAIN_WND_INIT_SIZE_W		620		//-Main Window�̏����T�C�Y�@W
#define MAIN_WND_INIT_SIZE_H		420		//-Main Window�̏����T�C�Y�@H
#define MAIN_WND_INIT_POS_X			680		//-Main Window�̏����ʒu�ݒ�@X
#define MAIN_WND_INIT_POS_Y			20		//-Main Window�̏����ʒu�ݒ�@Y

//-ID��` Main�X���b�h�p�@WM_USER + 1000 + 100 +��
#define ID_STATUS					WM_USER + 1101