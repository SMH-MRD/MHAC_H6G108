#pragma once
#include <windows.h>

/*** ��� ***/
#define ID_UP         0   //����
#define ID_DOWN       1   //�E��
#define ID_FWD        0   //�O�i
#define ID_REV        1   //��i
#define ID_LEFT       0   //����
#define ID_RIGHT      1   //�E��
#define ID_ACC        0   //����
#define ID_DEC        1   //����

//�z��Q�Ɨp����ID
#define MOTION_ID_MAX   8  //���䎲�ő吔

#define ID_HOIST        0   //�� �@       ID
#define ID_GANTRY       1   //���s        ID
#define ID_TROLLY       2   //���s        ID
#define ID_BOOM_H       3   //����        ID
#define ID_SLEW         4   //����        ID
#define ID_OP_ROOM      5   //�^�]���ړ��@ID
#define ID_H_ASSY       6   //�݋�        ID
#define ID_MOTION1      7   //�\��        ID

/*** �����萔�A�W�� ***/
#ifndef PHSICS_CONST

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
#endif // !PHSICS_CONST

/*** ���L������ ***/
//PLC IO
#define PLC_PB_MAX              64 //�^�]����{�^���o�^�ő吔
#define PLC_LAMP_MAX            64 //�^�]����{�^���o�^�ő吔
#define PLC_CTRL_MAX            64 //�^�]����{�^���o�^�ő吔

#ifndef PLC_PBL_ID

#define PID_E_STOP                   0
#define PID_CONTROL_SOURCE1_ON       1
#define PID_CONTROL_SOURCE1_OFF      2
#define PID_CONTROL_SOURCE2_ON       3
#define PID_CONTROL_SOURCE2_OFF      4
#define PID_FAULT_RESET              5
#define PID_IL_BYPASS                6
#define PID_FOOK_L                   7
#define PID_FOOK_R                   8
#define PID_TARGET1_MEM              9
#define PID_TARGET2_MEM              10
#define PID_TARGET3_MEM              11
#define PID_TARGET4_MEM              12
#define PID_MODE_OP_ROOM             13
#define PID_MODE_TELECON             14
#define PID_ANTSWAY_ON               15
#define PID_ANTSWAY_OFF              16
#define PID_AUTO_START               17
#define PID_CONTROL_SOURCE           18
#define PID_RAIL_CRAMP_ACTIVE        19
#define PID_RAIL_CRAMP_DEACTIVE      20
#define PID_TTB_ACTIVE               21
#define PID_SLEW_GRACE_ACTIVE        22
#define PID_GANTRY_GRACE_ACTIVE      23
#define PID_SLEW_PUMP_ACTIVE         24
#define PID_COIL_LIFTER              25
#define PID_HOIST_ASSY_RELEASE       26
#define PID_FAULT                    27
#define PID_ALARM                    28
#define PID_OP_TRY_SOURCE_ON         29
#define PID_OP_TRY_SOURCE_OFF        30
#define PID_LIFT_MAGNET_SELECT       31
#define PID_TEMP_HOLD_ON             32
#define PID_SAFE_SW_SAFE             33
#define PID_SAFE_SW_DANJER           34
#define PID_BORDING_CAUTION          35
#define PID_COIL_LIFT_MC3            36
#define PID_MODE_OP_REMOTE           37

#define ID_HOIST_BRK				 1   //�� �@       
#define ID_GANTRY_BRK			     2   //���s        
#define ID_TROLLY_BRK				 3   //���s        
#define ID_BOOM_H_BRK				 4   //����        
#define ID_SLEW_BRK					 5   //����        
#define ID_OP_ROOM_BRK				 6   //�^�]���ړ��@
#endif // !PLC_PBL_ID

//EXEC_STATUS
#ifndef AGENT_DO_ID

#define AGENT_DO_MAX            64 
#define AID_CONTROL_SOURCE1          1
#define AID_CONTROL_SOURCE2          2
#define AID_E_STOP                   3
#define AID_FAULT_RESET              4
#define AID_IL_BYPASS                5
#define AID_FOOK_L                   6
#define AID_FOOK_R                   7
#define AID_TARGET1_MEM              8
#define AID_TARGET2_MEM              9
#define AID_TARGET3_MEM3             10
#define AID_TARGET4_MEM4             11
#define AID_TARGET1_SELECT           12 
#define AID_TARGET2_SELECT           13
#define AID_TARGET3_SELECT           14
#define AID_TARGET4_SELECT           15
#define AID_ANTISWAY_ON              16
#define AID_AUTO_ACTIVE              17
#define AID_AUTO_SUSPEND             18
#define AID_AUTO_PROHIBIT            19
#endif // !AGENT_DO_ID

//SWAY_IO
#ifndef SWAY_IO_ID

#define SENSOR_TARGET_MAX            4//���o�^�[�Q�b�g�ő吔
#define DETECT_AXIS_MAX              4//���o���ő吔
#define TG_LAMP_NUM_MAX              3//�^�[�Q�b�g���̃����v�ő吔

#define SID_TG1                      0//�^�[�Q�b�gID
#define SID_TG2                      1
#define SID_TG3                      2
#define SID_TG4                      3

#define SID_X                        0
#define SID_Y                        1
#define SID_TH                       2
#define SID_R                        3

#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2
#endif // !SWAY_IO_ID


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


/****************************************************************************/
/*   �^���v�f��`�\����                                                     */
/* �@�����A�葬�A�������̈�A�̓���́A���̗v�f�̑g�ݍ��킹�ō\�����܂��B   */
/****************************************************************************/
typedef struct stMotionElement {	//�^���v�f
	int type;				//������
	int status;				//�������
	int time_count;			//�\��p�����Ԃ̃J�E���^�ϊ��l
	double _a;				//�ڕW�������x
	double _v;				//�ڕW���x
	double _p;				//�ڕW�ʒu
	double _t;				//�p������
	double v_max;			//���x����High
	double v_min;			//���x����Low
	double phase1;			//�N���ʑ��P
	double phase2;			//�N���ʑ� 2
	double opt_d[8];		//�I�v�V����double
	int opt_i[8];			//�I�v�V����int
}ST_MOTION_ELEMENT, * LPST_MOTION_ELEMENT;

/****************************************************************************/
/*   ������e��`�\���́i�P���j												*/
/* �@�P���̖ڕW��ԂɈڍs���铮��p�^�[�����^���v�f�̑g�ݍ��킹�Ŏ������܂� */
/****************************************************************************/
#define M_ELEMENT_MAX	32
#define M_AXIS			8	//���쎲
#define MH_AXIS			0	//�努����
#define TT_AXIS			1	//���s����
#define GT_AXIS			2	//���s����
#define BH_AXIS			3	//�N������
#define SLW_AXIS		4	//���񓮍�
#define SKW_AXIS		5	//�X�L���[����
#define LFT_AXIS		6	//�݋��

#define REQ_IMPOSSIBLE		0
#define REQ_STANDBY			1
#define REQ_ACTIVE			2
#define REQ_SUSPENDED		3
#define REQ_COMP_NORMAL		-1
#define REQ_COMP_ABNORMAL   -2

union axis_check {
char axis[M_AXIS];
LONG64 all;
};

typedef struct stMotionRecipe {					//�ړ��p�^�[��
	int axis;									//���쎲
	int n_motion;								//ID No.
	int n_step;									//�ړ��p�^�[���\���v�f��
	int motion_type;							//�I�v�V�����w��
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//�ړ��p�^�[����`�^���v�f�z��
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

/****************************************************************************/
/*   ������s�Ǘ��\����                                                     */
/* �@����iMOTION)���s��ԊǗ��p�\����									�@  */
/****************************************************************************/
typedef struct stMotionStat {
	int axis;									//���쎲
	int id;									//ID No.
	int status;									//������s��
	int iAct;									//���s���v�f�z��index -1�Ŋ���
	int act_count;								//���s���v�f�̎��s�J�E���g��
	int elapsed;								//�o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����
}ST_MOTION_STAT, * LPST_MOTION_STAT;

/********************************************************************************/
/*   ���A���^�]���e��`�\����                                      �@�@�@�@�@�@ */
/* �@�ړI�������������^�]���e��P������̑g�ݍ��킹�Ŏ������܂�               */
/********************************************************************************/
#define STOP_COMMAND		0//�@��~�^�]
#define PICK_COMMAND		1//�@�ג͂݉^�]
#define GRND_COMMAND		2//�@�׉����^�]
#define PARK_COMMAND		3//�@�ړ��^�]

typedef struct stCommandRecipe {				//�^�]�v�f
	int type;									//�R�}���h���
	int id;									//ID No.
	ST_MOTION_RECIPE motions[M_AXIS];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

/****************************************************************************/
/*   �^�]���s�Ǘ��\����                                                 */
/****************************************************************************/
typedef struct stCommandStat {				//�^�]�v�f
	int type;									//�R�}���h���
	int id;									//ID No.
	int status;									//�R�}���h���s��
	int elapsed;								//�o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����
	axis_check isnot_axis_completed;			//�e�����s������t���O�@0�Ŋ���
	LPST_MOTION_STAT p_motion_stat[M_AXIS];		//�e�����s�X�e�[�^�X�\���̂̃A�h���X
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;

/********************************************************************************/
/*   ��Ɠ��e�iJOB)��`�\����                                      �@�@�@�@�@�@	*/
/* �@�ړI�̉^�]���e���`���܂�										            */
/********************************************************************************/
#define JOB_STEP_MAX		5//�@JOB���\������R�}���h�ő吔

typedef struct stCommandSet {	//��Ɨv�f�iPICK�AGROUND�APARK�@....�j
	int type;							//�R�}���h��ʁiPICK�AGROUND�APARK�j
	axis_check is_valid_axis;			//�Ώۓ��쎲
	double target_pos[MOTION_ID_MAX];	//�e���ڕW�ʒu
	int option[MOTION_ID_MAX];		//�e������I�v�V��������
}ST_COMMAND_SET, * LPST_COMMAND_SET;

typedef struct _stJobRecipe {	//��ƍ\���v�f�i�ۊ�,���o,�ޔ��ړ����j
	int type;								//JOB���
	int job_id;								//job���ʃR�[�h
	int n_job_step;							//�\���R�}���h��
	ST_COMMAND_SET commands[JOB_STEP_MAX];	//JOB�\���R�}���h
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

/****************************************************************************/
/*   ��ƁiJOB)���s�Ǘ��\����                                                 */
/* �@�R�}���h���s��ԊǗ��p�\����									�@		*/
/****************************************************************************/
typedef struct stJOB_STAT {				//�^�]�v�f
	int type;								//JOB���
	int job_id;								//job���ʃR�[�h
	int n_job_step;							//�\���R�}���h��
	int job_step_now;						//���s���X�e�b�v
	int status;								//job���s��
	int elapsed;							//�o�ߎ���
	int error_code;							//�G���[�R�[�h�@�ُ튮����
	LPST_COMMAND_STAT p_command_stat[JOB_STEP_MAX];		//�e�R�}���h�X�e�[�^�X�\���̂̃A�h���X
}ST_JOB_STAT, * LPST_JOB_STAT;