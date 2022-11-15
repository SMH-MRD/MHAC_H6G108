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

// PLC_User IF�M���\���́i�@��^�]��IO)
// IO���t���e�́APLC_IO_DEF.h�ɒ�`
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	bool PB[N_PLC_PB];
	bool PBsemiauto[SEMI_AUTO_TARGET_MAX];
	bool LAMP[N_PLC_LAMP];
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
	double tilt_rad[DETECT_AXIS_MAX];						//�X�Ίp

	double th[MOTION_ID_MAX];		//�U�p			rad
	double dth[MOTION_ID_MAX];		//�U�p���x		rad/s
	double dthw[MOTION_ID_MAX];		//�U�p���x/�ց@	rad
	double ph[MOTION_ID_MAX];		//�ʑ����ʈʑ�	rad
	double amp2[MOTION_ID_MAX];		//�U����2��		rad2


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
	DWORD env_act_count=0;													//�w���V�[�M��
	ST_ENV_SUBPROC subproc_stat;											//�T�u�v���Z�X�̏��
	bool is_tasks_standby_ok;												//�^�X�N�̗����オ��m�F
	ST_SPEC spec;															//�N���[���d�l
	WORD operation_mode;													//�^�]���[�h�@�@��,�����[�g

	bool	auto_standby;													//�������[�h
	double semi_auto_setting_target[SEMI_AUTO_TARGET_MAX][MOTION_ID_MAX];	//�������ݒ�ڕW�ʒu
	int	 semi_auto_selected;												//�������I��ID
	int	 semi_auto_pb_count[SEMI_AUTO_TARGET_MAX];							//������PB�@ON�J�E���g
	int	 auto_start_pb_count;												//�����J�nPB�@ON�J�E���g
	bool is_notch_0[MOTION_ID_MAX];											//�U��~�߃��[�h�m�b�`����
	double r0[MOTION_ID_MAX];	                //�����U rad
	
	double notch_spd_ref[MOTION_ID_MAX];		//�m�b�`���x�w��
	WORD faultPC[N_PC_FAULT_WORDS];				//PLC���o�ُ�
	WORD faultPLC[N_PLC_FAULT_WORDS];			//����PC���o�ُ�
	Vector3 rc0;								//�N���[����_��x,y,z���W�ix:���s�ʒu y:���񒆐S z:���s���[�������j
	Vector3 rc;									//�N���[���ݓ_�̃N���[����_�Ƃ�x,y,z���΍��W
	Vector3 rl;									//�ׂ݉̃N���[���ݓ_�Ƃ�x,y,z���΍��W
	Vector3 rcam;								//�U��Z���T���ox,y,z���W
	bool is_fwd_endstop[MOTION_ID_MAX];			//���]�Ɍ�����
	bool is_rev_endstop[MOTION_ID_MAX];			//�t�]�Ɍ�����
	double mh_l;								//���[�v��
	double T;									//�U����		s
	double w;									//�U�p���g��	/s
	double w2;									//�U�p���g����2��
	double a[MOTION_ID_MAX][2];					//�������x���ݒl[0]�����@[1�n����

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

/************************************************************************************/
/*   ��Ɠ��e�iJOB)��`�\����                                 �@     �@�@�@�@�@�@	*/
/* �@ClientService �^�X�N���Z�b�g���鋤�L��������̏��								*/
/* �@JOB	:From-To�̔����R�}���h													*/
/*   COMMAND:1��JOB���A�����̃R�}���h�ō\��	PICK GRAND PARK						*/
/* �@JOB	:From-To�̔������													*/
/************************************************************************************/
#define COM_STEP_MAX		10					//�@JOB���\������R�}���h�ő吔
#define JOB_TYPE_HANDLING	0x00000001

//Recipe
#define JOB_STEP_TYPE_NULL	0
#define JOB_STEP_TYPE_PICK	1
#define JOB_STEP_TYPE_GRAND	2
#define JOB_STEP_TYPE_PARK	3

typedef struct _stJobRecipe {
	SYSTEMTIME time_start_planed;				//�\��J�n����
	int n_step;									//�X�e�b�v��
	int step_type[COM_STEP_MAX];				//�e�X�e�b�v�̃^�C�v
	double target[COM_STEP_MAX][MOTION_ID_MAX];//�eSTEP���@�e���ڕW
	int option[COM_STEP_MAX][MOTION_ID_MAX];	//�e��STEP���@�I�v�V��������
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

#define JOB_STAT_STANDBY			0x0001
#define JOB_STAT_ON_GOING			0x0002
#define JOB_STAT_COMPLETE			0x0004

#define JOB_STEP_STANDBY			0x0001
#define JOB_STEP_ON_GOING			0x0002
#define JOB_STEP_COMPLE_NORMAL		0x0004
#define JOB_STEP_COMPLE_ABNORMAL	0x0008
#define JOB_STEP_ABOTED				0x0010
#define JOB_STEP_SUSPENDED			0x0020

//Status
typedef struct stJobStat {						//JOB���s���
	int status;									//�����R�[�h�@�ُ튮�����G���[�R�[�h
	int current_step;							//���s���X�e�b�v
	int step_status[COM_STEP_MAX];				//step���s��
	DWORD step_elapsed[COM_STEP_MAX];			//step�o�ߎ���ms
	SYSTEMTIME time_start;
	SYSTEMTIME time_end;

}ST_JOB_STAT, * LPST_JOB_STAT;

//Set
#define JOB_TYPE_NULL		0
#define JOB_TYPE_OPEROOM	1
#define JOB_TYPE_CLIENT_PC	2
#define JOB_TYPE_REMOTE		3
#define JOB_TYPE_MAINTE_PC	4

typedef struct stJobSet {
	int id;									//JOB No
	int type;								//JOB��ʁi�����A�ړ��A����)
	ST_JOB_RECIPE	recipe;
	ST_JOB_STAT		status;
}ST_JOB_SET, * LPST_JOB_SET;

/****************************************************************************/
/*   �^���v�f��`�\����                                                     */
/* �@�����A�葬�A�������̈�A�̓���́A���̗v�f�̑g�ݍ��킹�ō\�����܂��B   */
/****************************************************************************/
#define MOTHION_OPT_V_MAX	0
#define MOTHION_OPT_V_MIN		0
#define MOTHION_OPT_PHASE_F		1
#define MOTHION_OPT_PHASE_R		2
#define MOTHION_OPT_WAIT_POS	3

#define MOTHION_OPT_AS_TYPE		0



// Control Type
#define CTR_TYPE_TIME_WAIT					1  //�ҋ@�i���Ԍo�ߑ҂��j
#define CTR_TYPE_SINGLE_PHASE_WAIT			2  //�ʑ��҂��P�J��(�U��~�ߗp�j
#define CTR_TYPE_DOUBLE_PHASE_WAIT			3  //�ʑ��҂��Q�J��(�U��~�ߗp�j
#define CTR_TYPE_OTHER_POS_WAIT				4	//�����ʒu���B�҂�
#define CTR_TYPE_ADJUST_MOTION_TRIGGER		5	//����N������(�U��~�߈ړ��p�j

#define CTR_TYPE_CONST_V_TIME				10  //�葬�Œ莞�ԏo��
#define CTR_TYPE_CONST_V_ACC_STEP			11  //�葬�o�� ������
#define CTR_TYPE_CONST_V_DEC_STEP			12  //�葬�o�� ������
#define CTR_TYPE_CONST_V_TOP_STEP			13  //�葬�o�� �g�b�v���x
#define CTR_TYPE_FINE_POSITION				14	//�����ʒu���킹

#define CTR_TYPE_ACC_TIME					20  //Specified time acceleration
#define CTR_TYPE_ACC_V						21  //Toward specified speed acceleration
#define CTR_TYPE_ACC_AS						23	//�U��~�߉���
#define CTR_TYPE_DEC_TIME					30  //Specified time deceleration
#define CTR_TYPE_DEC_V						31  //Toward specified speed deceleration


#define PTN_CONFIRMATION_TIME				0.1		//�p�^�[���o�͒�������
#define PTN_FINE_POS_LIMIT_TIME				5.0		//�����ʒu���킹��������
#define PTN_ERROR_CHECK_TIME				60		//�ُ팟�o����

#define PTN_STEP_STANDBY					0
#define PTN_STEP_FIN						1
#define PTN_STEP_ON_GOING					2
#define PTN_STEP_ERROR						-1
#define PTN_STEP_TIME_OVER					-2

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

//Recipe
typedef struct stMotionRecipe {					//�ړ��p�^�[��
	DWORD motion_type;							//�����ʁA
	int n_step;									//����\���v�f��
	DWORD opt_dw;								//�I�v�V��������
	int time_limit;								//�^�C���I�[�o�[����l
	ST_MOTION_STEP steps[M_STEP_MAX];			//�����`�v�f�z��
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

//Status
#define COMMAND_STAT_ERROR		-1
#define COMMAND_STAT_END		0
#define COMMAND_STAT_STANDBY	1
#define COMMAND_STAT_ACTIVE		2
#define COMMAND_STAT_PAUSE		3
#define COMMAND_STAT_ABORT		4

#define MOTION_STAT_FLG_N		4 
#define MOTION_ACC_STEP_BYPASS	0 
#define MOTION_DEC_STEP_BYPASS	1 

typedef struct stMotionStat {
	int status;									//������s��
	int iAct;									//���s���v�f�z��index -1�Ŋ���
	int step_act_count;							//���s���v�f�̎��s�J�E���g��
	int elapsed;								//MOTION�J�n��o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����
	int direction;								//�������
	int flg[MOTION_STAT_FLG_N];				//���s�X�e�[�^�X�I�v�V�����t���O
}ST_MOTION_STAT, * LPST_MOTION_STAT;

/********************************************************************************/
/*   ���A���^�]���e(COMMAND)��`�\����                             �@�@�@�@�@�@ */
/* �@�ړI�������������^�]���e��P������̑g�ݍ��킹�Ŏ������܂�               */
/********************************************************************************/

typedef struct stCommandSet {
	//POLICY SET
	int id;									//�R�}���hID
	int type;								//�R�}���h���
	int job_id;
	int job_step;
	bool is_required_motion[MOTION_ID_MAX];		//����Ώێ�
	ST_MOTION_RECIPE recipe[MOTION_ID_MAX];

	//AGENT SET
	SYSTEMTIME time_start;					//�J�n���̎��� SYSTEMTIME
	SYSTEMTIME time_end;					//�I�����̎��� SYSTEMTIME
	int com_status;							//���s��ԁ@
	int status_code;						//�G���[�R�[�h���i�ُ튮�����j�@
	ST_MOTION_STAT	motion_stat[MOTION_ID_MAX];

}ST_COMMAND_SET, * LPST_COMMAND_SET;


//# Policy �^�X�N�Z�b�g�̈�

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	����`�\����                                   �@   �@*/
/* �@Client Service�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    */
/****************************************************************************/

#define JOB_HOLD_MAX			10					//	�ێ��\JOB�ő吔

typedef struct stCSInfo {

	int n_job_standby;
	int i_current_job;							//���s����JOB�C���f�b�N�X  -1:���s����
	ST_JOB_SET	job_list[JOB_HOLD_MAX];

	//for Client
	DWORD req_status;
	DWORD n_job_hold;							//����JOB��

}ST_CS_INFO, * LPST_CS_INFO;

/****************************************************************************/
/*   Policy	����`�\����                                   �@			  �@*/
/* �@Policy	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@		 �@		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	int i_jobcom;							//���݂̃R�}���hINDEX
	int i_com;								//���݂̃R�}���hINDEX
	ST_COMMAND_SET job_com[COM_STEP_MAX];
	ST_COMMAND_SET com[COM_STEP_MAX];
	int auto_ctrl_ptn[MOTION_ID_MAX];

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	����`�\����                                   �@   �@		*/
/* �@Agent	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@			*/
/****************************************************************************/

#define AUTO_TYPE_ANTI_SWAY	0x01
#define AUTO_TYPE_SEMI_AUTO	0x11
#define AUTO_TYPE_JOB		0x21
#define AUTO_TYPE_MANUAL	0x00

typedef struct stAgentInfo {

	WORD pc_ctrl_mode; //PC����̎w�߂œ��삳���鎲�̎w��

	double v_ref[MOTION_ID_MAX];
	int PLC_PB_com[N_PLC_PB];
	int PLC_LAMP_com[N_PLC_LAMP];
	int PLC_LAMP_semiauto_com[SEMI_AUTO_TARGET_MAX];

	int auto_on_going;								//���s���̎���
	UCHAR auto_active[MOTION_ID_MAX];				//�������s��(����)
	double dist_for_stop[MOTION_ID_MAX];			//������~����
	double pos_target[MOTION_ID_MAX];				//�ʒu���ߖڕW�ʒu
	bool is_spdfb_0[MOTION_ID_MAX];					//�U��~�ߑ��xFB����
	bool be_hold_target[MOTION_ID_MAX];				//�ڕW�ʒu�L�[�v�t���O
	double gap_from_target[MOTION_ID_MAX];			//�ڕW�ʒu����̂���
	double gap2_from_target[MOTION_ID_MAX];			//�ڕW�ʒu����̂���2��
	double sway_amp2m[MOTION_ID_MAX];				//�U��U����2��m2

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

//�R�}���h�v�������R�[�h
#define REQ_ACCEPTED			0


//�I�y���[�V�����R�}���h
#define OPE_COM_PB_SET				1
#define OPE_COM_LAMP_ON				2
#define OPE_COM_LAMP_OFF			3
#define OPE_COM_LAMP_FLICKER		4
#define OPE_COM_SEMI_LAMP_ON		5
#define OPE_COM_SEMI_LAMP_OFF		6
#define OPE_COM_SEMI_LAMP_FLICKER	7


