#pragma once

#include "resource.h"

///# �x�[�X�ݒ�

//-�^�X�N�ݒ�
#define TARGET_RESOLUTION			1		//�}���`���f�B�A�^�C�}�[����\ msec
#define SYSTEM_TICK_ms				25		//���C���X���b�h���� msec

//-Main Window�̔z�u�ݒ�
#define MAIN_WND_INIT_SIZE_W		400		//-Main Window�̏����T�C�Y�@W
#define MAIN_WND_INIT_SIZE_H		200		//-Main Window�̏����T�C�Y�@H
#define MAIN_WND_INIT_POS_X			680		//-Main Window�̏����ʒu�ݒ�@X
#define MAIN_WND_INIT_POS_Y			450		//-Main Window�̏����ʒu�ݒ�@Y

//-ID��` Main�X���b�h�p�@WM_USER + 1000 + 100 +��
#define ID_STATUS					WM_USER + 1201

///# �^�X�N�N���Ǘ��p�\����
//-�^�X�N�ݒ�
#define TARGET_RESOLUTION			1		//�}���`���f�B�A�^�C�}�[����\ msec
#define SYSTEM_TICK_ms				25		//���C���X���b�h���� msec
#define MAX_APP_TASK				8		//�^�X�N�I�u�W�F�N�g�X���b�h�ő吔
#define INITIAL_TASK_STACK_SIZE		16384	//�^�X�N�I�u�W�F�N�g�X���b�h�p�X�^�b�N�T�C�Y

typedef struct stKnlManageSetTag {
	WORD mmt_resolution = TARGET_RESOLUTION;			//�}���`���f�B�A�^�C�}�[�̕���\
	unsigned int cycle_base = SYSTEM_TICK_ms;			//�}���`���f�B�A�^�C�}�[�̕���\
	WORD KnlTick_TimerID = 0;							//�}���`���f�B�A�^�C�}�[��ID
	unsigned int num_of_task = 0;						//�A�v���P�[�V�����ŗ��p����X���b�h��
	unsigned long sys_counter = 0;						//�}���`���f�B�A�N���^�C�}�J�E���^
	SYSTEMTIME Knl_Time = { 0,0,0,0,0,0,0,0 };			//�A�v���P�[�V�����J�n����̌o�ߎ���
	unsigned int stackSize = INITIAL_TASK_STACK_SIZE;	//�^�X�N�̏����X�^�b�N�T�C�Y
}ST_KNL_MANAGE_SET, * PSTT_KNL_MANAGE_SET;