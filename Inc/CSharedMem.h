#pragma once

#include <string>
#include "common_def.h"
#include "spec.h"
#include "CVector3.h"

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
#define N_PLC_PBS				64 //�^�]����PB��
#define N_PLC_BITS				64 //BIT STATUS��
#define N_PLC_CTRL_WORDS        16 //����Z���T�M��WORD��
#define N_PLC_FAULTS			400	//PLC�t�H���g�̊��蓖�ăT�C�Y

#define ID_PB_ESTOP				0
#define ID_PB_ANTISWAY_ON		1
#define ID_PB_ANTISWAY_OFF		2
#define ID_PB_AUTO_START		3
#define ID_PB_AUTO_TG_FROM1		4
#define ID_PB_AUTO_TG_FROM2		5
#define ID_PB_AUTO_TG_FROM3		6
#define ID_PB_AUTO_TG_FROM4		7
#define ID_PB_AUTO_TG_TO1		8
#define ID_PB_AUTO_TG_TO2		9
#define ID_PB_AUTO_TG_TO3		10
#define ID_PB_AUTO_TG_TO4		11
#define ID_PB_CRANE_MODE		12
#define ID_PB_REMOTE_MODE		13
#define ID_PB_CTRT_SOURCE_ON	14
#define ID_PB_CTRT_SOURCE_OFF	15
#define ID_PB_CTRT_SOURCE2_ON	16
#define ID_PB_CTRT_SOURCE2_OFF	17
#define ID_PB_AUTO_RESET		18




#define ID_BIT_CTRL_SOURCE_ON	0

#define ID_WORD_CTRL_SOURCE_ON	0
#define ID_WORD_CTRL_REMOTE		1

// PLC_User IF�M���\���́i�@��^�]��IO)
// IO���t���e�́APLC_IO_DEF.h�ɒ�`
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	bool PBs[N_PLC_PBS];
	bool BITs[N_PLC_BITS];
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

#define DETECT_AXIS_MAX              4// ���o����


#define TG_LAMP_NUM_MAX              3//�^�[�Q�b�g���̃����v�ő吔

#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2

#define SWAY_FAULT_ITEM_MAX			 4//�ُ팟�o���ڐ�
#define SID_COMMON_FLT               0



#define SWAY_IF_SIM_DBG_MODE  0x00000010	//�U��f�[�^��SIM�o�͂��琶��

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���[�h
	WORD status[SENSOR_TARGET_MAX];							//�^�[�Q�b�g�T���o���
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//�Z���T�ُ���
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//�^�[�Q�b�g���oPIXEL���i�ʐρj
	double tilt_rad[DETECT_AXIS_MAX];						//�X�Ίp

	double rad[MOTION_ID_MAX];								//�U��p
	double w[MOTION_ID_MAX];								//�U��p���x
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
	Vector3 L, vL;//۰���޸��(�U��j
	double rad_cam_x, rad_cam_y, w_cam_x, w_cam_y;			//�J�������W�U��p
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


//�U��Z���T��Ԓ�`�\����
typedef struct StSwaySet {
	double T;		//�U����		/s
	double w;		//�U�p���g��	/s
	double th;		//�U�p			rad
	double dth;		//�U�p���x		rad/s
	double dthw;	//�U�p���x/�ց@	rad
	double ph;		//�ʑ����ʈʑ�	rad
	double amp2;	//�U����2��		rad2
}ST_SWAY_SET, * LPST_SWAY__SET;
typedef struct StSwCamSet {
	double D0;		//�ݓ_-�J�����n�E�W���O���ʒu�Ԑ�������	m
	double H0;		//�ݓ_-�J�����n�E�W���O���ʒu�ԍ�������	m
	double l0;		//�J�����n�E�W���O��-�J�����ʒu�ԋ���	m
	double ph0;		//�J�����n�E�W���O��-�J�����ʒu�Ԋp�x	rad
}ST_SWCAM_SET, * LPST_SWCAM__SET;

typedef struct StSwayStatus {
	ST_SWAY_SET sw[MOTION_ID_MAX];
	ST_SWCAM_SET cam[MOTION_ID_MAX];
}ST_SWAY_STATUS, * LPST_SWAY_STATUS;

typedef struct stEnvSubproc {

	bool is_plcio_join = false;
	bool is_sim_join = false;
	bool is_sway_join = false;

} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;


typedef struct StCraneStatus {
	DWORD env_act_count=0;						//�w���V�[�M��
	ST_ENV_SUBPROC subproc_stat;				//�T�u�v���Z�X�̏��
	ST_SPEC spec;								//�N���[���d�l
	DWORD operation_mode;						//�^�]���[�h�@�@��,�����[�g
	double notch_spd_ref[MOTION_ID_MAX];		//�m�b�`���x�w��
	WORD faultPC[N_PC_FAULT_WORDS];				//PLC���o�ُ�
	WORD faultPLC[N_PLC_FAULT_WORDS];			//����PC���o�ُ�
	ST_SWAY_STATUS sw_stat;						//�U���ԉ�͌���
	Vector3 rc0;								//�N���[����_��x,y,z���W�ix:���s�ʒu y:���񒆐S z:���s���[�������j
	Vector3 rc;									//�N���[���ݓ_�̃N���[����_�Ƃ�x,y,z���΍��W
	Vector3 rl;									//�ׂ݉̃N���[���ݓ_�Ƃ�x,y,z���΍��W
	bool is_fwd_endstop[MOTION_ID_MAX];			//���]�Ɍ�����
	bool is_rev_endstop[MOTION_ID_MAX];			//�t�]�Ɍ�����
	double mh_l;								//���[�v��

}ST_CRANE_STATUS, * LPST_CRANE_STATUS;

/************************************************************************************/
/*   ��Ɠ��e�iJOB)��`�\����                                 �@     �@�@�@�@�@�@	*/
/* �@ClientService �^�X�N���Z�b�g���鋤�L��������̏��								*/
/* �@JOB	:From-To�̔����R�}���h													*/
/*   COMMAND:1��JOB���A�����̃R�}���h�ō\��	PICK GRAND PARK						*/
/* �@JOB	:From-To�̔������													*/
/************************************************************************************/
#define COM_STEP_MAX		10					//�@JOB���\������R�}���h�ő吔
#define COM_TARGET_MAX		MOTION_ID_MAX		//�@�R�}���h���̖ڕW�ő吔
#define JOB_TYPE_HANDLING	0x00000001

//Recipe
typedef struct _stJobRecipe {
	WORD job_id;								//JOB No
	WORD client_type;							//�˗���
	WORD job_type;								//JOB��ʁi�����A�ړ��A����)
	WORD step_n;								//�X�e�b�v��
	double target[COM_STEP_MAX][COM_TARGET_MAX];//�eSTEP���@�e���ڕW�ʒu,�ڕW������
	DWORD option[COM_STEP_MAX][COM_TARGET_MAX];	//�e��STEP���@�I�v�V��������
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

//Status
typedef struct stJobStat {						//JOB���s���
	LPST_JOB_RECIPE pjob;						//�Ώ�JOB
	int job_step_now;							//���s���X�e�b�v
	int step_status[COM_STEP_MAX];				//step���s��
	double step_elapsed[COM_STEP_MAX];			//step�o�ߎ���
	int job_status;								//�����R�[�h�@�ُ튮�����G���[�R�[�h
}ST_JOB_STAT, * LPST_JOB_STAT;

//Set
typedef struct stJobSet {
	DWORD type;
	ST_JOB_RECIPE	recipe;
	ST_JOB_STAT		status;
}ST_JOB_SET, * LPST_JOB_SET;

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

#define REQ_STANDBY			1
#define REQ_ACTIVE			2
#define REQ_SUSPENDED		3
#define REQ_COMP_NORMAL		0
#define REQ_IMPOSSIBLE		-1
#define REQ_COMP_ABNORMAL   -2

//Recipe
typedef struct stMotionRecipe {					//�ړ��p�^�[��
	DWORD motion_type;							//�����ʁA
	int n_step;									//����\���v�f��
	DWORD opt_dw;								//�I�v�V��������
	int time_limit;								//�^�C���I�[�o�[����l
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//�����`�v�f�z��
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

//Status
typedef struct stMotionStat {
	DWORD id;									//ID No. HIWORD:���R�[�h LOWORD:�V���A��No 
	int status;									//������s��
	int iAct;									//���s���v�f�z��index -1�Ŋ���
	int act_count;								//���s���v�f�̎��s�J�E���g��
	int elapsed;								//MOTION�J�n��o�ߎ���
	int error_code;								//�G���[�R�[�h�@�ُ튮����
}ST_MOTION_STAT, * LPST_MOTION_STAT;

//Set
typedef struct stMotionSet {
	ST_MOTION_RECIPE recipe;					//������e��`
	ST_MOTION_STAT status;						//������s��� 
}ST_MOTION_SET, * LPST_MOTION_SET;


/********************************************************************************/
/*   ���A���^�]���e(COMMAND)��`�\����                             �@�@�@�@�@�@ */
/* �@�ړI�������������^�]���e��P������̑g�ݍ��킹�Ŏ������܂�               */
/********************************************************************************/
#define STOP_COMMAND		0//�@��~�^�]
#define PICK_COMMAND		1//�@�ג͂݉^�]
#define GRND_COMMAND		2//�@�׉����^�]
#define PARK_COMMAND		3//�@�ړ��^�]
//Recipe
typedef struct stCommandRecipe {				//�^�]�v�f
	LPST_JOB_RECIPE pjob;						//�R�t�����Ă���JOB
	WORD job_step;								//�R�t�����Ă���JOB�̒S���X�e�b�v
	bool is_required_motion[MOTION_ID_MAX];		//����Ώێ�
	ST_MOTION_RECIPE motions[MOTION_ID_MAX];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

//Status
typedef struct stCommandStat {				//�^�]�v�f
	int type;								//�R�}���h���
	DWORD comID;							//ID No.
	int status;								//�R�}���h���s��
	int elapsed;							//�o�ߎ���
	int error_code;							//�G���[�R�[�h�@�ُ튮����
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;

//Set
typedef struct stCommandSet {
	ST_COMMAND_RECIPE	recipe;				//�R�}���h���
	ST_COMMAND_STAT		status;				//ID No.
}ST_COMMAND_SET, * LPST_COMMAND_SET;




//# Policy �^�X�N�Z�b�g�̈�

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	����`�\����                                   �@   �@*/
/* �@Client Service�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    */
/****************************************************************************/

#define JOB_HOLD_MAX		10					//	�ێ��\JOB�ő吔

typedef struct stCSInfo {

	//for Agent
	int id_job_active;							//���s����JOB�C���f�b�N�X  -1:���s����
	ST_JOB_SET	job_list[JOB_HOLD_MAX];

	//for Client
	DWORD req_status;
	DWORD n_job_hold;							//����JOB��

}ST_CS_INFO, * LPST_CS_INFO;


#define BITSEL_HOIST        0x00000001		//�� �@       �r�b�g
#define BITSEL_GANTRY       0x00000002		//���s        �r�b�g
#define BITSEL_TROLLY       0x00000004		//���s        �r�b�g
#define BITSEL_BOOM_H       0x00000008		//����        �r�b�g
#define BITSEL_SLEW         0x00000010		//����        �r�b�g
#define BITSEL_OP_ROOM      0x00000020		//�^�]���ړ��@�r�b�g
#define BITSEL_H_ASSY       0x00000040		//�݋�        �r�b�g
#define BITSEL_COMMON       0x10000000		//����        �r�b�g

#define POLICY_AUTO_OFF		0x00000000		//����OFF
#define POLICY_SEMI_AUTO_ON	0x00000001		//������MODE

#define POLICY_ANTISWAY_OFF	0x00000000		//�U��~��OFF
#define POLICY_ANTISWAY_ON	0x00000001		//�U��~��ON

/****************************************************************************/
/*   Policy	����`�\����                                   �@			  �@*/
/* �@Policy	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@		 �@		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	//for AGENT
	DWORD pc_ctrl_mode;							//���䃂�[�h
	DWORD antisway_mode;						//�U��~�߃��[�h

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	����`�\����                                   �@   �@		*/
/* �@Agent	�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@			*/
/****************************************************************************/
#define N_AGENT_PLC_PB_COM		10
#define N_AGENT_PLC_LAMP_COM	10


#define PB_COM_ESTOP			0
#define LAMP_AS_OFF				0
#define LAMP_AS_ON				1



typedef struct stAgentInfo {
	//for POLICY
	ST_COMMAND_SET com[COM_STEP_MAX];	
	
	//for CRANE
	double v_ref[MOTION_ID_MAX];
	bool pb_coms[N_AGENT_PLC_PB_COM];
	bool lamp_coms[N_AGENT_PLC_LAMP_COM];

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
