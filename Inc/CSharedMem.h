#pragma once

#include <string>
#include "common_def.h"
#include "spec.h"
#include "PLC_IO_DEF.h"

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
#define PLC_PB_MAX              64 //�^�]����{�^���o�^�ő吔
#define PLC_LAMP_MAX            64 //�^�]����{�^���o�^�ő吔
#define PLC_CTRL_MAX            64 //�^�]����{�^���o�^�ő吔
#define N_PLC_FAULTS			400	//PLC�t�H���g�̊��蓖�ăT�C�Y

// PLC_User IF�M���\���́i�@��^�]��IO)
// IO���t���e�́APLC_IO_DEF.h�ɒ�`
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	BOOL pb[PLC_PB_MAX];
	BOOL lamp[PLC_LAMP_MAX];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_��ԐM���\���́i�@��Z���T�M��)
typedef struct StPLCStatus {
	BOOL ctrl[PLC_CTRL_MAX];		//����p�M���@LS,MC��ԓ�
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
	BOOL is_debug_mode;
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

#define DETECT_AXIS_MAX              4//���o���ő吔
#define SID_X                        0
#define SID_Y                        1
#define SID_TH                       2
#define SID_R                        3

#define TG_LAMP_NUM_MAX              3//�^�[�Q�b�g���̃����v�ő吔
#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2

#define SWAY_FAULT_ITEM_MAX			 4//�ُ팟�o���ڐ�
#define SID_COMMON_FLT               0
#define SID_GREEN                    1
#define SID_BLUE                     2


#define SWAY_IF_SIM_DBG_MODE  0x00000010	//�U��f�[�^��SIM�o�͂��琶��

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���[�h
	WORD status[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//�Z���T�ُ���
	double rad[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//�U��p
	double w[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//�U��p���x
	double ph[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//�U��ʑ�
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//�^�[�Q�b�g���oPIXEL���i�ʐρj
	double skew_rad[SENSOR_TARGET_MAX];						//�X�L���[�p
	double skew_w[SENSOR_TARGET_MAX];						//�X�L���[�p���x
	double tilt_rad[DETECT_AXIS_MAX];						//�X�Ίp

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
#define SIM_ACTIVE_MODE  0x00000100	//�V�~�����[�V�������s���[�h

typedef struct StSimulationStatus {
	DWORD mode;
	DWORD helthy_cnt;
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
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

#define OPERATION_MODE_REMOTE	0x0000001

//�f�o�b�O���[�h�ݒ苤�p��
union Udebug_mode {//[0]:�f�o�b�O���[�h���e  [1]-[3]:�I�v�V�������e
	DWORD all;
	UCHAR item[4];
};

//�U��Z���T��Ԓ�`�\����
typedef struct StSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * LPST_SWAY_STATUS;

typedef struct stEnvSubproc {
	bool is_plcio_join = false;
	bool is_sim_join = false;
	bool is_sway_join = false;
} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;


typedef struct StCraneStatus {
	DWORD env_act_count;			//�w���V�[�M��
	ST_ENV_SUBPROC subproc_stat;	//�T�u�v���Z�X�̏��
	ST_SPEC spec;
	DWORD operation_mode;
	double notch_spd_ref[MOTION_ID_MAX];		//�m�b�`���x�w��
	WORD faultPC[N_PC_FAULT_WORDS];//����PC���o�ُ�
	WORD faultPLC[N_PLC_FAULT_WORDS];//����PC���o�ُ�
	ST_SWAY_STATUS sway_stat;
}ST_CRANE_STATUS, * LPST_CRANE_STATUS;

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
#define M_AXIS			8	//���쎲��
#define MH_AXIS			0	//�努���R�[�h
#define TT_AXIS			1	//���s���R�[�h
#define GT_AXIS			2	//���s���R�[�h
#define BH_AXIS			3	//�N�����R�[�h
#define SLW_AXIS		4	//���񎲃R�[�h
#define SKW_AXIS		5	//�X�L���[���R�[�h
#define LFT_AXIS		6	//�݋�쎲�R�[�h
#define NO_AXIS			7	//��ԕύX�R�[�h

#define REQ_STANDBY			1
#define REQ_ACTIVE			2
#define REQ_SUSPENDED		3
#define REQ_COMP_NORMAL		0
#define REQ_IMPOSSIBLE		-1
#define REQ_COMP_ABNORMAL   -2

typedef struct stMotionRecipe {					//�ړ��p�^�[��
	DWORD id;									//ID No. LOWORD:No. HIWORD:���R�[�h
	int motion_type;							//�����ʁ@�ړ��A
	int n_step;									//����\���v�f��
	DWORD opt_dw;								//�I�v�V��������
	int time_limit;								//�^�C���I�[�o�[����l
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//�����`�v�f�z��
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

/****************************************************************************/
/*   ������s�Ǘ��\����                                                     */
/* �@����iMOTION)���s��ԊǗ��p�\����									�@  */
/****************************************************************************/
typedef struct stMotionStat {
	DWORD id;									//ID No. LOWORD:�R�}���hID No. HIWORD:���R�[�h
	int status;									//������s��
	int iAct;									//���s���v�f�z��index -1�Ŋ���
	int act_count;								//���s���v�f�̎��s�J�E���g��
	int elapsed;								//MOTION�J�n��o�ߎ���
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
	DWORD jobID;								//�R�t�����Ă���JOB�@ID
	DWORD comID;								//ID No.
	bool is_motion_there[M_AXIS];				//����Ώێ�
	ST_MOTION_RECIPE motions[M_AXIS];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

/****************************************************************************/
/*   �^�]���s�Ǘ��\����                                           �@�@      */
/****************************************************************************/
typedef struct stCommandStat {				//�^�]�v�f
	int type;								//�R�}���h���
	DWORD comID;							//ID No.
	int status;								//�R�}���h���s��
	int elapsed;							//�o�ߎ���
	int error_code;							//�G���[�R�[�h�@�ُ튮����
	ST_MOTION_STAT p_motion_stat[M_AXIS];	//�e�����s�X�e�[�^�X�\���̂̃A�h���X
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;

/***********************************************************************************/
/*   ��Ɠ��e�iJOB)��`�\����                                 �@     �@�@�@�@�@�@  */
/* �@ClientService �^�X�N���Z�b�g���鋤�L��������̏��							   */
/*   JOB�̓��e�@:�@																   */
/*			�����@	�w��J������ׂ�����Ďw��J���֑��u��A�ҋ@�ʒu�ւ̈ړ��܂Ł@ */
/*			�ړ��@	�w��J������ׂ�����Ďw��J���֑��u��A�����ʒu�ւ̈ړ��܂� */
/*			���̑�	�d������,���[�h�ύX���̏���									   */
/***********************************************************************************/
#define COM_STEP_MAX		5//�@JOB���\������R�}���h�ő吔

#define JOB_TYPE_HANDLING	0x00000001

//# ClientService �^�X�N�Z�b�g
typedef struct _stJobRecipe {
	DWORD jobID;								//JOB ID
	DWORD type;									//JOB��ʁi�����A�ړ��A����j
	int	n_com_step;								//�X�e�b�v��
	double to_pos[COM_STEP_MAX][MOTION_ID_MAX];	//�e��STEP���@�ڕW�ʒu
	DWORD option[COM_STEP_MAX][MOTION_ID_MAX];	//�e��STEP���@�I�v�V��������
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;


//# Policy �^�X�N�Z�b�g
typedef struct stJobStat {						//JOB���s���
	DWORD jobID;								//JOB ID
	int n_job_step;								//�\���R�}���h��
	int job_step_now;							//���s���X�e�b�v
	int status;									//job���s��
	int elapsed;								//�o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����
}ST_JOB_STAT, * LPST_JOB_STAT;

#define JOB_HOLD_MAX		10					//	�ێ��\JOB�ő吔
#define NO_JOB_REQUIRED		-1					//  �v���W���u����


//# Policy �^�X�N�Z�b�g�̈�

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	����`�\����                                   �@   �@*/
/* �@Client Service�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    */
/****************************************************************************/
typedef struct stCSInfo {

	//for Policy
	int current_job;							//���s�v������JOB�C���f�b�N�X -1:�v������
	ST_JOB_RECIPE	jobs[JOB_HOLD_MAX];			

	//for Client
	DWORD req_stat;								

}ST_CS_INFO, * LPST_CS_INFO;


#define BITSEL_HOIST        0x00000001   //�� �@       �r�b�g
#define BITSEL_GANTRY       0x00000002   //���s        �r�b�g
#define BITSEL_TROLLY       0x00000004   //���s        �r�b�g
#define BITSEL_BOOM_H       0x00000008   //����        �r�b�g
#define BITSEL_SLEW         0x00000010   //����        �r�b�g
#define BITSEL_OP_ROOM      0x00000020   //�^�]���ړ��@�r�b�g
#define BITSEL_H_ASSY       0x00000040   //�݋�        �r�b�g
#define BITSEL_COMMON       0x10000000   //����        �r�b�g

/****************************************************************************/
/*   Policy	����`�\����                                   �@			  �@*/
/* �@Policy	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@		 �@		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	//for AGENT
	DWORD pc_ctrl_mode;							//���䃂�[�h
	DWORD antisway_mode;						//�U��~�߃��[�h
	int current_com;							//���s�v�����̃R�}���h�C���f�b�N�X -1:�v������
	ST_COMMAND_RECIPE commands[COM_STEP_MAX];	

	//for CS


	ST_JOB_STAT job_stat[JOB_HOLD_MAX];			

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	����`�\����                                   �@   �@		*/
/* �@Agent	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@			*/
/****************************************************************************/
typedef struct stAgentInfo {
	//for POLICY
	ST_COMMAND_STAT com_stat[COM_STEP_MAX];	
	
	//for CRANE
	double v_ref[MOTION_ID_MAX];				

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
