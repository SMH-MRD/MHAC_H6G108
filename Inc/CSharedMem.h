#pragma once

#include <string>
#include "common_def.h"
#include "spec.h"
#include "CVector3.h"
#include "Swaysensor.h"


#define SMEM_CRANE_STATUS_NAME			L"CRANE_STATUS"
#define SMEM_SWAY_STATUS_NAME			L"SWAY_STATUS"
#define SMEM_OPERATION_STATUS_NAME		L"OPERATION_STATUS"
#define SMEM_FAULT_STATUS_NAME			L"FAULT_STATUS"
#define SMEM_SIMULATION_STATUS_NAME		L"SIMULATION_STATUS"
#define SMEM_PLC_IO_NAME				L"PLC_IO"
#define SMEM_SWAY_IO_NAME				L"SWAY_IO"
#define SMEM_REMOTE_IO_NAME				L"REMOTE_IO"
#define SMEM_CS_INFO_NAME				L"CS_INFO"
#define SMEM_POLICY_INFO_NAME			L"POLICY_INFO"
#define SMEM_AGENT_INFO_NAME			L"AGENT_INFO"

#define MUTEX_CRANE_STATUS_NAME			L"MU_CRANE_STATUS"
#define MUTEX_SWAY_STATUS_NAME			L"MU_SWAY_STATUS"
#define MUTEX_OPERATION_STATUS_NAME		L"MU_OPERATION_STATUS"
#define MUTEX_FAULT_STATUS_NAME			L"MU_FAULT_STATUS"
#define MUTEX_SIMULATION_STATUS_NAME	L"MU_SIMULATION_STATUS"
#define MUTEX_PLC_IO_NAME				L"MU_PLC_IO"
#define MUTEX_SWAY_IO_NAME				L"MU_SWAY_IO"
#define MUTEX_REMOTE_IO_NAME			L"MU_REMOTE_IO"
#define MUTEX_CS_INFO_NAME				L"MU_CS_INFO"
#define MUTEX_POLICY_INFO_NAME			L"MU_POLICY_INFO"
#define MUTEX_AGENT_INFO_NAME			L"MU_AGENT_INFO"

#define SMEM_OBJ_ID_CRANE_STATUS		0
#define SMEM_OBJ_ID_SWAY_STATUS			1
#define SMEM_OBJ_ID_OPERATION_STATUS	2
#define SMEM_OBJ_ID_FAULT_STATUS		3
#define SMEM_OBJ_ID_SIMULATION_STATUS	4
#define SMEM_OBJ_ID_PLC_IO				5
#define SMEM_OBJ_ID_SWAY_IO				6
#define SMEM_OBJ_ID_REMOTE_IO			7
#define SMEM_OBJ_ID_CS_INFO				8
#define SMEM_OBJ_ID_POLICY_INFO			9
#define SMEM_OBJ_ID_AGENT_INFO			10

//  ���L�������X�e�[�^�X
#define	OK_SHMEM						0	// ���L������ ����/�j������
#define	ERR_SHMEM_CREATE				-1	// ���L������ Create�ُ�
#define	ERR_SHMEM_VIEW					-2	// ���L������ View�ُ�
#define	ERR_SHMEM_NOT_AVAILABLE			-3	// ���L������ View�ُ�
#define	ERR_SHMEM_MUTEX					-4	// ���L������ View�ُ�

#define SMEM_DATA_SIZE_MAX				1000000	//���L���������蓖�čő�T�C�Y�@1Mbyte	

using namespace std;

/****************************************************************************/
/*   PLC IO��`�\����                                                     �@*/
/* �@PLC_IF PROC���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@�@�@�@�@�@�@�@  */
/****************************************************************************/
#define N_PLC_PB				64 //�^�]����PB��
#define N_PLC_LAMP				64 //BIT STATUS��
#define N_PLC_CTRL_WORDS        16 //����Z���T�M��WORD��
#define N_PLC_FAULTS			400	//PLC�t�H���g�̊��蓖�ăT�C�Y

#define ID_PB_ESTOP				0
#define ID_PB_ANTISWAY_ON		1
#define ID_PB_ANTISWAY_OFF		2
#define ID_PB_AUTO_START		3
#define ID_PB_CRANE_MODE		12
#define ID_PB_REMOTE_MODE		13
#define ID_PB_CTRL_SOURCE_ON	14
#define ID_PB_CTRL_SOURCE_OFF	15
#define ID_PB_CTRL_SOURCE2_ON	16
#define ID_PB_CTRL_SOURCE2_OFF	17
#define ID_PB_AUTO_RESET		18
#define ID_PB_FAULT_RESET		19


#define PLC_IO_LAMP_FLICKER_COUNT    40 //�����v�t���b�J�̊Ԋu�J�E���g
#define PLC_IO_LAMP_FLICKER_CHANGE   20 //�����v�t���b�J�̊Ԋu�J�E���g

#define PLC_IO_OFF_DELAY_COUNT		 4	//PB����I�t�f�B���C�J�E���g�l

// PLC_User IF�M���\���́i�@��^�]��IO)
// IO���t���e�́APLC_IO_DEF.h�ɒ�`
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	int PB[N_PLC_PB];
	int PBsemiauto[SEMI_AUTO_TARGET_MAX];
	int LAMP[N_PLC_LAMP];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_��ԐM���\���́i�@��Z���T�M��)
typedef struct StPLCStatus {
	UINT16 ctrl[N_PLC_CTRL_WORDS];		//����p�M���@LS,MC��ԓ�
	double v_fb[MOTION_ID_MAX];
	double v_ref[MOTION_ID_MAX];
	double trq_fb_01per[MOTION_ID_MAX];
	double pos[MOTION_ID_MAX];
	double weight;
}ST_PLC_STATUS, * LPST_PLC_STATUS;

// PLC_IO�\����
#define PLC_IF_PC_DBG_MODE  0x00000001		//PC�f�o�b�O�p�l���ASIM�o�͂���IO��񐶐�
typedef struct StPLCIO {
	DWORD mode;
	DWORD helthy_cnt;
	ST_PLC_UI ui;
	ST_PLC_STATUS status;
	CHAR faultPLC[N_PLC_FAULTS];
}ST_PLC_IO, * LPST_PLC_IO;

/****************************************************************************/
/*   �U��Z���T�M����`�\����                                  �@         �@*/
/* �@SWAY_PC_IF���Z�b�g���鋤�L��������̏��@      �@�@�@�@�@�@           */
/****************************************************************************/

#define SENSOR_TARGET_MAX            4//���o�^�[�Q�b�g�ő吔
#define SID_TG1                      0//�^�[�Q�b�gID
#define SID_TG2                      1
#define SID_TG3                      2
#define SID_TG4                      3

#define DETECT_AXIS_MAX              4// ���o����


#define TG_LAMP_NUM_MAX              3//�^�[�Q�b�g���̃����v�ő吔

#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2

#define SWAY_FAULT_ITEM_MAX			 4//�ُ팟�o���ڐ�
#define SID_COMMON_FLT               0

#define SWAY_IF_SIM_DBG_MODE  0x00000010	//�U��f�[�^��SIM�o�͂��琶��


#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���[�h
	WORD status[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//�Z���T�ُ���
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//�^�[�Q�b�g���oPIXEL���i�ʐρj
	double tilt_rad[MOTION_ID_MAX];							//�X�Ίp

	double th[MOTION_ID_MAX];		//�U�p			rad
	double dth[MOTION_ID_MAX];		//�U�p���x		rad/s
	double dthw[MOTION_ID_MAX];		//�U�p���x/�ց@	rad
	double ph[MOTION_ID_MAX];		//�ʑ����ʈʑ�	rad
	double rad_amp2[MOTION_ID_MAX];		//�U����2��		rad2

}ST_SWAY_IO, * LPST_SWAY_IO;

/****************************************************************************/
/*   ���u�����M����`�\����                                  �@         �@*/
/* �@ROS_IF PROC���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@          �@    */
/****************************************************************************/
typedef struct StRemoteIO {

	ST_PLC_UI PLCui;

}ST_REMOTE_IO, * LPST_REMOTE_IO;

/****************************************************************************/
/*   �V�~�����[�V�����M����`�\����                                  �@   �@*/
/* �@SIM PROC���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@          �@    �@ */
/****************************************************************************/
#define SIM_ACTIVE_MODE  0x00000100			//�V�~�����[�V�������s���[�h
#define SIM_SWAY_PACKET_MODE 0x00000010		//�U��Z���T�p�P�b�g���M���[�h
typedef struct StSimulationStatus {
	DWORD mode;
	DWORD helthy_cnt;
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
	Vector3 L, vL;//۰���޸��(�U��j
	double rad_cam_x, rad_cam_y, w_cam_x, w_cam_y;			//�J�������W�U��p

	double kbh; //�������a�Ɉˑ����鑬�x�A�����x�␳�W��

	ST_SWAY_RCV_MSG rcv_msg;
	ST_SWAY_SND_MSG snd_msg;

}ST_SIMULATION_STATUS, * LPST_SIMULATION_STATUS;

/****************************************************************************/
/*   �N���[����Ԓ�`�\����                                          �@   �@*/
/* �@Environment�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    �@ */
/****************************************************************************/
#define DBG_PLC_IO				0x00000001
#define DBG_SWAY_IO				0x00000100
#define DBG_ROS_IO				0x00010000
#define DBG_SIM_ACT				0X01000000

#define N_PC_FAULT_WORDS		16			//����PC���o�t�H���gbit�Z�b�gWORD��
#define N_PLC_FAULT_WORDS		32			//PLC���o�t�H���gbit�Z�b�gWORD��

#define OPERATION_MODE_REMOTE		0x0000001
#define OPERATION_MODE_SIMULATOR	0x0100000
#define OPERATION_MODE_PLC_DBGIO	0x0001000

#define N_SWAY_DIR				4

typedef struct stEnvSubproc {

	bool is_plcio_join = false;
	bool is_sim_join = false;
	bool is_sway_join = false;

} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;

#define MANUAL_MODE				0
#define ANTI_SWAY_MODE			1
#define SEMI_AUTO_ACTIVE		2
#define AUTO_ACTIVE				3

#define BITSEL_HOIST        0x0001		//�� �@       �r�b�g
#define BITSEL_GANTRY       0x0002		//���s        �r�b�g
#define BITSEL_TROLLY       0x0004		//���s        �r�b�g
#define BITSEL_BOOM_H       0x0008		//����        �r�b�g
#define BITSEL_SLEW         0x0010		//����        �r�b�g
#define BITSEL_OP_ROOM      0x0020		//�^�]���ړ��@�r�b�g
#define BITSEL_H_ASSY       0x0040		//�݋�        �r�b�g
#define BITSEL_COMMON       0x0080		//����        �r�b�g

#define BITSEL_SEMIAUTO     0x0001
#define BITSEL_AUTO			0x0002

#define SPD0_CHECK_RETIO	0.1

#define STAT_ACC			0;
#define STAT_DEC			1;

typedef struct StCraneStatus {
//Event Update				:�C�x���g�����ōX�V
	bool is_tasks_standby_ok;												//�^�X�N�̗����オ��m�F
	ST_SPEC spec;															//�N���[���d�l


//Periodical Update			�F������X�V
	DWORD env_act_count=0;													//�w���V�[�M��
	ST_ENV_SUBPROC subproc_stat;											//�T�u�v���Z�X�̏��
	WORD operation_mode;													//�^�]���[�h�@�@��,�����[�g
	bool is_notch_0[MOTION_ID_MAX];											//0�m�b�`����
	Vector3 rc;																//�N���[���ݓ_�̃N���[����_�Ƃ�x,y,z���΍��W
	Vector3 rl;																//�ׂ݉̃N���[���ݓ_�Ƃ�x,y,z���΍��W
	Vector3 rcam_m;															//�U��Z���T���ox,y,z���W m
	double notch_spd_ref[MOTION_ID_MAX];									//�m�b�`���x�w��
	double mh_l;															//���[�v��
	double T;																//�U����		s
	double w;																//�U�p���g��	/s
	double w2;																//�U�p���g����2��
	double R;																//���񔼌a

	WORD faultPC[N_PC_FAULT_WORDS];											//PLC���o�ُ�
	WORD faultPLC[N_PLC_FAULT_WORDS];										//����PC���o�ُ�

	bool is_fwd_endstop[MOTION_ID_MAX];										//���]�Ɍ�����
	bool is_rev_endstop[MOTION_ID_MAX];										//�t�]�Ɍ�����

}ST_CRANE_STATUS, * LPST_CRANE_STATUS;

#define SEMI_AUTO_TG_CLR	8
#define SEMI_AUTO_TG1		0
#define SEMI_AUTO_TG2		1
#define SEMI_AUTO_TG3		2
#define SEMI_AUTO_TG4		3
#define SEMI_AUTO_TG5		4
#define SEMI_AUTO_TG6		5
#define SEMI_AUTO_TG7		6
#define SEMI_AUTO_TG8		7



/****************************************************************************/
/*   �^���v�f��`�\����                                                     */
/* �@�����A�葬�A�������̈�A�̓���́A���̗v�f�̑g�ݍ��킹�ō\�����܂��B   */
/****************************************************************************/


//���V�s�@Type
#define CTR_TYPE_WAIT_TIME					0	//�ҋ@�i���Ԍo�ߑ҂��j
#define CTR_TYPE_WAIT_POS_OTHERS			1	//�������B�҂�
#define CTR_TYPE_WAIT_POS_AND_PH			2	//�������B+�ʑ��҂�
#define CTR_TYPE_WAIT_LAND					4	//�����҂�
#define CTR_TYPE_WAIT_PH					8	//�U��ʑ��҂�

#define CTR_TYPE_VOUT_TIME					100  //�X�e�b�v���x�@���Ԋ���
#define CTR_TYPE_VOUT_V						101  //�X�e�b�v���x�@���x���B����
#define CTR_TYPE_VOUT_POS					102  //�X�e�b�v���x�@�ʒu���B����
#define CTR_TYPE_VOUT_PHASE     			104  //�X�e�b�v���x�@�ʑ����B����
#define CTR_TYPE_VOUT_LAND					105  //�X�e�b�v���x�@��������

#define CTR_TYPE_AOUT_TIME					110  //�������x�@���Ԋ���
#define CTR_TYPE_AOUT_V						111  //�������x�@���x���B����
#define CTR_TYPE_AOUT_POS					112  //�������x�@�ʒu���B����
#define CTR_TYPE_AOUT_PHASE     			114  //�������x�@�ʑ����B����
#define CTR_TYPE_AOUT_LAND					115  //�������x�@��������

#define CTR_TYPE_FINE_POS					200	//�����ʒu���킹
#define CTR_TYPE_FB_SWAY					300	//FB�U��~��
#define CTR_TYPE_FB_SWAY_POS				301	//FB�U��~�߈ʒu����

#define TIME_LIMIT_CONFIRMATION				0.1		//�p�^�[���o�͒������� �b
#define TIME_LIMIT_FINE_POS					10.0	//�����ʒu���킹�������� �b
#define TIME_LIMIT_ERROR_DETECT				120		//�ُ팟�o����

typedef struct stMotionElement {	//�^���v�f
	int type;				//������
	int status;				//������
	int time_count;			//�\��p�����Ԃ̃J�E���^�ϊ��l
	double _a;				//�ڕW�������x
	double _v;				//�ڕW���x
	double _p;				//�ڕW�ʒu
	double _t;				//�p������
	double opt_d[8];		//�I�v�V����double
	int opt_i[8];			//�I�v�V����int
}ST_MOTION_STEP, * LPST_MOTION_STEP;

/****************************************************************************/
/*   ������e��`�\���́i�P���j												*/
/* �@�P���̖ڕW��ԂɈڍs���铮��p�^�[�����^���v�f�̑g�ݍ��킹�Ŏ������܂� */
/****************************************************************************/
#define M_STEP_MAX	32

#define MOTION_COMPLETE	0x0080
#define MOTION_STANDBY	0x0001
#define MOTION_ACTIVE	0x0002

#define STEP_FIN		0x0080
#define STEP_STANDBY	0x0001
#define STEP_ACTIVE		0x0002
#define STEP_FAULT		0x0010
#define STEP_TIMEOVER	0x0020


#define _ACT_STANDBY	0




//Recipe
typedef struct stMotionRecipe {					//�ړ��p�^�[��
	DWORD motion_type;							//�����ʁA
	int n_step;									//����\���v�f��
	int direction;								//�������
	DWORD opt_dw;								//�I�v�V��������
	int time_limit;								//�^�C���I�[�o�[����l
	ST_MOTION_STEP steps[M_STEP_MAX];			//�����`�v�f�z��
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

//Status

typedef struct stMotionStat {
	int status;									//������s��
	int iAct;									//���s���v�f�z��index -1�Ŋ���
	int step_act_count;							//���s���v�f�̎��s�J�E���g��
	int elapsed;								//MOTION�J�n��o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����

}ST_MOTION_STAT, * LPST_MOTION_STAT;

/********************************************************************************/
/*   ���A���^�]���e(COMMAND)��`�\����                             �@�@�@�@�@�@ */
/* �@�ړI�������������^�]���e��P������̑g�ݍ��킹�Ŏ������܂�               */
/********************************************************************************/

#define JOB_COMMAND_MAX			10			//�@JOB���\������R�}���h�ő吔
#define	CODE_TYPE_JOB			0x8000
#define AUTO_TYPE_ANTI_SWAY		0x0100
#define AUTO_TYPE_MANUAL		0x0000
#define AUTO_TYPE_SEMIAUTO		0x8200
#define AUTO_TYPE_JOB			0x8400
#define COM_TYPE_PICK			0x0001
#define COM_TYPE_GRND			0x0002
#define COM_TYPE_PARK			0x0004
#define COM_TYPE_FROM_TO		0x0008

#define COM_NO_SEMIAUTO			0


typedef struct stCommandBlock {
	//POLICY SET
	int no;											//�R�}���h No(�V�[�P���X�ԍ��j
	int type;										//�R�}���h���
	bool is_active_axis[MOTION_ID_MAX];				//����Ώێ��@����̎��𓮍삳���Ȃ����Ɏg�p
	ST_MOTION_RECIPE recipe[MOTION_ID_MAX];

	//AGENT SET
	SYSTEMTIME time_start;					//�J�n���̎��� SYSTEMTIME
	SYSTEMTIME time_end;					//�I�����̎��� SYSTEMTIME
	int com_status;							//���s��ԁ@
	int status_code;						//�G���[�R�[�h���i�ُ튮�����j�@
	ST_MOTION_STAT	motion_stat[MOTION_ID_MAX];

}ST_COMMAND_BLOCK, * LPST_COMMAND_BLOCK;

typedef struct stCommandList {
	int job_type;							//JOB��� (job,semiauto)
	int job_id;								//�R�t�����Ă���job�̃V�[�P���X�ԍ�
	int current_step;						//���s���R�}���h�X�e�b�v
	ST_COMMAND_BLOCK commands[JOB_COMMAND_MAX];
}ST_COMMAND_LIST, * LPST_COMMAND_LIST;


//# Policy �^�X�N�Z�b�g�̈�

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/************************************************************************************/
/*   ��Ɠ��e�iJOB)��`�\����                                 �@     �@�@�@�@�@�@	*/
/* �@ClientService �^�X�N���Z�b�g���鋤�L��������̏��								*/
/* �@JOB	:From-To�̔����R�}���h													*/
/*   COMMAND:1��JOB���A�����̃R�}���h�ō\��	PICK GRAND PARK						*/
/* �@JOB	:From-To�̔������													*/
/************************************************************************************/
#define JOB_REGIST_MAX			10					//�@JOB�o�^�ő吔
#define JOB_N_STEP_SEMIAUTO		1

typedef struct StPosTargets {
	double pos[MOTION_ID_MAX];
	bool is_held[MOTION_ID_MAX];		//�ڕW�ʒu�z�[���h���t���O
}ST_POS_TARGETS, * LPST_POS_TARGETS;


typedef struct stJobSet {

	//CS SET
	int no;										//JOB No(�V�[�P���X�ԍ��j
	int type;									//JOB��ʁiJOB,������,PARK,PICK,GRAND�j�j
	int n_command;								//JOB�\���R�}���h��
	int step_type[JOB_COMMAND_MAX];				//�e�X�e�b�v�̃^�C�v
	ST_POS_TARGETS target[JOB_COMMAND_MAX];		//�e�R�}���h�̖ڕW�ʒu	
		
	//POLICY SET
	int status;									//JOB���s���
	DWORD step_elapsed[JOB_COMMAND_MAX];		//step�o�ߎ���ms
	SYSTEMTIME time_start;
	SYSTEMTIME time_end;
	LPST_COMMAND_LIST lp_commands;				//�Ή�����R�}���h�̃��X�g

}ST_JOB_SET, * LPST_JOB_SET;

//JOB LIST
typedef struct _stJobList {
	int job_wait_n;									//�����҂��o�^Job��
	int semiauto_wait_n;							//�����҂��o�^Semiauto��
	int i_job_active;									//�������҂�Job(���s��or�ҋ@���j	  id
	int i_semiauto_active;							//�������҂�Semiauto(���s��or�ҋ@���j id
	ST_JOB_SET job[JOB_REGIST_MAX];					//�o�^job
	ST_JOB_SET semiauto[JOB_REGIST_MAX];			//�o�^job
}ST_JOB_LIST, * LPST_JOB_LIST;


/****************************************************************************/
/*   Client Service	����`�\����                                   �@   �@*/
/* �@Client Service�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    */
/****************************************************************************/

#define JOB_HOLD_MAX			10					//	�ێ��\JOB�ő吔

typedef struct stCSInfo {

	ST_JOB_LIST	job_list;

	//UI�֘A
	int plc_lamp[N_PLC_LAMP];											//PLC�����v�\���o�͗p�i�����J�n�j
	int plc_pb[N_PLC_PB];												//PLC����PB���͊m�F�p�i�����J�n�j
	int semiauto_lamp[SEMI_AUTO_TARGET_MAX];							//�����������v�\���o�͗p
	int semiauto_pb[SEMI_AUTO_TARGET_MAX];								//������PB���͏����p
	ST_POS_TARGETS semi_auto_setting_target[SEMI_AUTO_TARGET_MAX];		//�������ݒ�ڕW�ʒu
	int	 semi_auto_selected;											//�I�𒆂̔�����ID

	//����,���u�ݒ�i���[�h�j
	bool auto_standby;													//�������[�h

}ST_CS_INFO, * LPST_CS_INFO;

/****************************************************************************/
/*   Policy	����`�\����                                   �@			  �@*/
/* �@Policy	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@		 �@		*/
/****************************************************************************/

#define FAULT_MAP_W_SIZE	64	//�t�H���g�}�b�v�T�C�Y

typedef struct stPolicyInfo {

	WORD fault_map[FAULT_MAP_W_SIZE];
	ST_COMMAND_LIST command_list;

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	����`�\����                                   �@   �@		*/
/* �@Agent	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@			*/
/****************************************************************************/
typedef struct stAgentInfo {

	ST_COMMAND_BLOCK comset_as;						//�U��~�ߗp�R�}���h�Z�b�g
	ST_POS_TARGETS auto_pos_target;					//�����ڕW�ʒu
	int antisway_comple_status;						//�U��~�ߊ������
	double dist_for_target[MOTION_ID_MAX];			//�ڕW�܂ł̋���
	int auto_on_going;								//���s���̎������
	int auto_active[MOTION_ID_MAX];				//�������s���t���O(����)
	bool is_spdfb_0[MOTION_ID_MAX];					//�U��~�ߋN������p���xFB����

	WORD pc_ctrl_mode;								//PC����̎w�߂œ��삳���鎲�̎w��
	double v_ref[MOTION_ID_MAX];					//���x�w�ߏo�͒l
	int PLC_PB_com[N_PLC_PB];						//PLC�ւ�DO�w�߁iPB���͑����w�߁j

}ST_AGENT_INFO, * LPST_AGENT_INFO;

static char smem_dummy_buf[SMEM_DATA_SIZE_MAX];

/****************************************************************************/
/*���L�������N���X��`														*/
/****************************************************************************/
class CSharedMem
{
public:
	CSharedMem();
	~CSharedMem();

	int smem_available;			//���L�������L��
	int data_size;				//�f�[�^�T�C�Y

	int create_smem(LPCTSTR szName, DWORD dwSize, LPCTSTR szMuName);
	int delete_smem();
	int clear_smem();

	wstring wstr_smem_name;

	HANDLE get_hmutex() { return hMutex; }
	LPVOID get_pMap() { return pMapTop; }

protected:
	HANDLE hMapFile;
	LPVOID pMapTop;
	DWORD  dwExist;

	HANDLE hMutex;
};


