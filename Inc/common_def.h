#pragma once
#include <windows.h>

/*** �����萔�A�W�� ***/
#define GA				9.80665     //�d�͉����x

#define PI360           6.2832      //2��
#define PI330           5.7596   
#define PI315           5.4978
#define PI300           5.2360 
#define PI270           4.7124      
#define PI180           3.1416      //��
#define PI90            1.5708
#define PI60            1.0472
#define PI45            0.7854
#define PI30            0.5236

#define RAD2DEG         57.29578
#define DEG2RAD         0.0174533

/*** �z��Q�Ɨp�@����C���f�b�N�X ***/
#define MOTION_ID_MAX   8  //���䎲�ő吔

#define ID_HOIST        0   //�� �@       ID
#define ID_GANTRY       1   //���s        ID
#define ID_TROLLY       2   //���s        ID
#define ID_BOOM_H       3   //����        ID
#define ID_SLEW         4   //����        ID
#define ID_OP_ROOM      5   //�^�]���ړ��@ID
#define ID_H_ASSY       6   //�݋�        ID
#define ID_MOTION1      7   //�\��        ID

/*** �z��Q�Ɨp�@�����C���f�b�N�X ***/
#define ID_UP         0   //����
#define ID_DOWN       1   //�E��
#define ID_FWD        0   //�O�i
#define ID_REV        1   //��i
#define ID_LEFT       0   //����
#define ID_RIGHT      1   //�E��
#define ID_ACC        0   //����
#define ID_DEC        1   //����


/*** MODE ***/
//�V�~�����[�V����
#define IO_PRODUCTION                   0x0000//���@
#define USE_CRANE_SIM                   0x1000//�N���[�������V�~�����[�^�̏o�͂�FB�l�ɓK�p����
#define USE_PLC_SIM_COMMAND				0x0100//�@�㑀����͂�PLC�V�~�����[�^�̏o�͒l���g��
#define USE_REMOTE_SIM_COMMAND          0x0010//���u������͂Ƀ����[�g�V�~�����[�^�̏o�͒l���g��
#define USE_SWAY_CRANE_SIM		        0x0001//�U��Z���T�̐M�����N���[�������V�~�����[�^�̏o�͂��琶������

/*** �������� ***/
//�U��~�߃p�^�[��
#define AS_PTN_1P           1   //1�p���X
#define AS_PTN_2P           2   //2�p���X
#define AS_PTN_TR           3   //1��`����
#define AS_PTN_3TR          4   //3��`����
#define AS_PTN_TRTR         5   //��`�{��`����(2�i�������j

