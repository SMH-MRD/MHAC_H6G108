#pragma once

#include "framework.h"
#include "common_def.h"
#include "spec.h"

#define SMEM_CRANE_STATUS_NAME			L"CRANE_STATUS"
#define SMEM_SWAY_STATUS_NAME			L"SWAY_STATUS"
#define SMEM_OPERATION_STATUS_NAME		L"OPERATION_STATUS"
#define SMEM_FAULT_STATUS_NAME			L"FAULT_STATUS"
#define SMEM_SIMULATION_STATUS_NAME		L"SIMULATION_STATUS"
#define SMEM_PLC_IO_NAME				L"PLC_IO"
#define SMEM_SWAY_IO_NAME				L"SWAY_IO"
#define SMEM_REMOTE_IO_NAME				L"REMOTE_IO"
#define SMEM_JOB_STATUS_NAME			L"JOB_STATUS"
#define SMEM_COMMAND_STATUS_NAME		L"COMMAND_STATUS"
#define SMEM_EXEC_STATUS_NAME			L"EXEC_STATUS"

#define SMEM_OBJ_ID_CRANE_STATUS		0
#define SMEM_OBJ_ID_SWAY_STATUS			1
#define SMEM_OBJ_ID_OPERATION_STATUS	2
#define SMEM_OBJ_ID_FAULT_STATUS		3
#define SMEM_OBJ_ID_SIMULATION_STATUS	4
#define SMEM_OBJ_ID_PLC_IO				5
#define SMEM_OBJ_ID_SWAY_IO				6
#define SMEM_OBJ_ID_REMOTE_IO			7
#define SMEM_OBJ_ID_JOB_STATUS			8
#define SMEM_OBJ_ID_COMMAND_STATUS		9
#define SMEM_OBJ_ID_EXEC_STATUS			10

//  ���L�������X�e�[�^�X
#define	OK_SHMEM						0	// ���L������ ����/�j������
#define	ERR_SHMEM_CREATE				-1	// ���L������ Create�ُ�
#define	ERR_SHMEM_VIEW					-2	// ���L������ View�ُ�
#define	ERR_SHMEM_NOT_AVAILABLE			-3	// ���L������ View�ُ�

#define SMEM_DATA_SIZE_MAX				1000000	//���L���������蓖�čő�T�C�Y�@1Mbyte	

using namespace std;

/****************************************************************************/
/*   PLC IO��`�\����                                                     �@*/
/* �@PLC_IF PROC���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@�@�@�@�@�@�@�@  */
/****************************************************************************/
#define PLC_PB_MAX              64 //�^�]����{�^���o�^�ő吔
#define PLC_LAMP_MAX            64 //�^�]����{�^���o�^�ő吔
#define PLC_CTRL_MAX            64 //�^�]����{�^���o�^�ő吔

#ifndef PLC_PBL_ID //PLC IO�̔z��
#endif // !PLC_PBL_ID

#define N_PLC_FAULT_WORDS		32	//PLC�t�H���g�̊��蓖�ăT�C�Y

// PLC_User IF�M���\���́i�@��^�]��IO)
typedef struct StPLCUI {
	WORD notch_pos[MOTION_ID_MAX];
	BOOL pb[PLC_PB_MAX];
	BOOL lamp[PLC_LAMP_MAX];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_��ԐM���\���́i�@��Z���T�M��)
typedef struct StPLCStatus {
	BOOL ctrl[PLC_CTRL_MAX];
	double v_fb[MOTION_ID_MAX];
	double v_ref[MOTION_ID_MAX];
	double trq_fb_01per[MOTION_ID_MAX];
	double pos[MOTION_ID_MAX];
	double weight;
	BOOL is_debug_mode;
}ST_PLC_STATUS, * LPST_PLC_STATUS;

// PLC_IO�\����
typedef struct StPLCIO {
	ST_PLC_UI ui;
	ST_PLC_STATUS status;
	WORD faultPLC[N_PLC_FAULT_WORDS];
	BOOL is_debug_mode;
}ST_PLC_IO, * LPST_PLC_IO;

/****************************************************************************/
/*   �U��Z���T�M����`�\����                                  �@         �@*/
/* �@SWAY_PC_IF���Z�b�g���鋤�L��������̏��@      �@�@�@�@�@�@           */
/****************************************************************************/
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

typedef struct StSwayIO {
	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];
	WORD status[SENSOR_TARGET_MAX];
	WORD fault[SENSOR_TARGET_MAX];
	double rad[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];
	double w[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];
	double ph[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];
	double skew_rad[SENSOR_TARGET_MAX];
	double skew_w[SENSOR_TARGET_MAX];
	double tilt_rad[DETECT_AXIS_MAX];
	BOOL is_debug_mode;
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
typedef struct StSimulationStatus {
	bool is_simproc_available;
	bool is_simulation_active;
	double spd_fb[MOTION_ID_MAX];
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
}ST_SIMULATION_STATUS, * LPST_SIMULATION_STATUS;

/****************************************************************************/
/*   �N���[����Ԓ�`�\����                                          �@   �@*/
/* �@Environment�^�X�N���Z�b�g���鋤�L��������̏��@�@�@�@�@�@�@ �@    �@ */
/****************************************************************************/
//�U��Z���T��Ԓ�`�\����
typedef struct StSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * LPST_SWAY_STATUS;

#define N_PC_FAULT_WORDS		16	//PC�t�H���g�̊��蓖�ăT�C�Y
typedef struct StCraneStatus {
	
	ST_SPEC spec;
	WORD notch_pos[MOTION_ID_MAX];
	WORD faultPC[N_PC_FAULT_WORDS];
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
/* �@ClientService �^�X�N���Z�b�g���鋤�L��������̏��								            */
/********************************************************************************/
#define JOB_STEP_MAX		5//�@JOB���\������R�}���h�ő吔

typedef struct stCommandSet {	//��Ɨv�f�iPICK�AGROUND�APARK�@....�j
	int type;							//�R�}���h��ʁiPICK�AGROUND�APARK�j
	axis_check is_valid_axis;			//�Ώۓ��쎲
	double target_pos[MOTION_ID_MAX];	//�e���ڕW�ʒu
	int option[MOTION_ID_MAX];		//�e������I�v�V��������
}ST_COMMAND_SET, * LPST_COMMAND_SET;

//#   ��ƁiJOB)���s�Ǘ��\����  
//# �@�R�}���h���s��ԊǗ��p�\����
typedef struct _stJobRecipe {				//��ƍ\���v�f�i�ۊ�,���o,�ޔ��ړ����j
	int type;										//JOB���
	int job_id;										//job���ʃR�[�h
	int n_job_step;									//�\���R�}���h��
	ST_COMMAND_SET commands[JOB_STEP_MAX];			//JOB�\���R�}���h
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

typedef struct stJOB_STAT {					//�^�]�v�f
	int type;										//JOB���
	int job_id;										//job���ʃR�[�h
	int n_job_step;									//�\���R�}���h��
	int job_step_now;								//���s���X�e�b�v
	int status;										//job���s��
	int elapsed;									//�o�ߎ���
	int error_code;									//�G���[�R�[�h�@�ُ튮����
	LPST_COMMAND_STAT p_command_stat[JOB_STEP_MAX];	//�e�R�}���h�X�e�[�^�X�\���̂̃A�h���X
}ST_JOB_STAT, * LPST_JOB_STAT;

typedef struct StJobStatus {

	ST_JOB_RECIPE	job;
	ST_JOB_STAT		job_stat;

}ST_JOB_STATUS, * LPST_JOB_STATUS;
//-------------------------------------
typedef struct StCommandStatus {
	ST_COMMAND_RECIPE command[JOB_STEP_MAX];
	ST_COMMAND_STAT command_stat[JOB_STEP_MAX];

}ST_COMMAND_STATUS, * LPST_COMMAND_STATUS;
//-------------------------------------
typedef struct StExecStatus {

	ST_MOTION_STAT motion_stat[M_AXIS];
	double v_ref[MOTION_ID_MAX];

}ST_EXEC_STATUS, * LPST_EXEC_STATUS;

static char smem_dummy_buf[SMEM_DATA_SIZE_MAX];




/***********************************************************************
�N���X��`
************************************************************************/
class CSharedMem
{
public:
	CSharedMem();
	~CSharedMem();

	int smem_available;			//���L�������L��
	int data_size;				//�f�[�^�T�C�Y(1�o�b�t�@���j
	bool use_double_buff;		//�_�u���o�b�t�@���p�̑I��

	LPVOID get_writePtr() {	if(ibuf_write) return buf1Ptr; else return buf0Ptr;}
	LPVOID get_readPtr() { if(ibuf_write) return buf0Ptr; else return buf1Ptr;}

	int create_smem(LPCTSTR szName, DWORD dwSize);
	int delete_smem();

	virtual int update();
	virtual int set_data_size();
	int clear_smem();

	wstring wstr_smem_name;

protected:
	HANDLE hMapFile;
	LPVOID pMapTop;
	DWORD  dwExist;

	LPVOID buf0Ptr;
	LPVOID buf1Ptr;
	
	int ibuf_write;//�������݃o�b�t�@�̃C���f�b�N�X(0 or 1)
};

class CCraneStatus :public CSharedMem
{
public:
	CCraneStatus() {}
	~CCraneStatus() { delete_smem(); }
	
	int set_data_size() {
		return sizeof(ST_CRANE_STATUS);
	};

	LPST_CRANE_STATUS pbuf_write(){
		return (LPST_CRANE_STATUS)get_writePtr();
	}
	LPST_CRANE_STATUS pbuf_read() {
		return (LPST_CRANE_STATUS)get_readPtr();
	}
};

class CSwayStatus :public CSharedMem
{
public:
	CSwayStatus() {}
	~CSwayStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_SWAY_STATUS);
	}

	LPST_SWAY_STATUS pbuf_write() {
		return (LPST_SWAY_STATUS)get_writePtr();
	}
	LPST_SWAY_STATUS pbuf_read() {
		return (LPST_SWAY_STATUS)get_readPtr();
	}
};

class CSimulationStatus :public CSharedMem
{
public:
	CSimulationStatus() {
	}
	~CSimulationStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_SIMULATION_STATUS);
	};

	LPST_SIMULATION_STATUS pbuf_write() {
		return (LPST_SIMULATION_STATUS)get_writePtr();
	}
	LPST_SIMULATION_STATUS pbuf_read() {
		return (LPST_SIMULATION_STATUS)get_readPtr();
	}

};

class CPLCIO :public CSharedMem
{
public:
	CPLCIO() { memset(spd_fb, 0, sizeof(spd_fb[MOTION_ID_MAX])); }
	~CPLCIO() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_PLC_IO);
	};

	LPST_PLC_IO pbuf_write() {
		return (LPST_PLC_IO)get_writePtr();
	}
	LPST_PLC_IO pbuf_read() {
		return (LPST_PLC_IO)get_readPtr();
	}

	double spd_fb[MOTION_ID_MAX];
};

class CSwayIO :public CSharedMem
{
public:
	CSwayIO() {}
	~CSwayIO() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_SWAY_IO);
	};

	LPST_SWAY_IO pbuf_write() {
		return (LPST_SWAY_IO)get_writePtr();
	}
	LPST_SWAY_IO pbuf_read() {
		return (LPST_SWAY_IO)get_readPtr();
	}
};

class CRemoteIO :public CSharedMem
{
public:
	CRemoteIO() {}
	~CRemoteIO() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_REMOTE_IO);
	};

	LPST_REMOTE_IO pbuf_write() {
		return (LPST_REMOTE_IO)get_writePtr();
	}
	LPST_REMOTE_IO pbuf_read() {
		return (LPST_REMOTE_IO)get_readPtr();
	}
};


class CJobStatus :public CSharedMem
{
public:
	CJobStatus() { }
	~CJobStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_JOB_STATUS);
	};

	LPST_JOB_STATUS pbuf_write() {
		return (LPST_JOB_STATUS)get_writePtr();
	}
	LPST_JOB_STATUS pbuf_read() {
		return (LPST_JOB_STATUS)get_readPtr();
	}
};


class CCommandStatus :public CSharedMem
{
public:
	CCommandStatus() {}
	~CCommandStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_COMMAND_STATUS);
	};

	LPST_COMMAND_STATUS pbuf_write() {
		return (LPST_COMMAND_STATUS)get_writePtr();
	}
	LPST_COMMAND_STATUS pbuf_read() {
		return (LPST_COMMAND_STATUS)get_readPtr();
	}

};

class CExecStatus :public CSharedMem
{
public:
	CExecStatus() {}
	~CExecStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_EXEC_STATUS);
	};

	LPST_EXEC_STATUS pbuf_write() {
		return (LPST_EXEC_STATUS)get_writePtr();
	}
	LPST_EXEC_STATUS pbuf_read() {
		return (LPST_EXEC_STATUS)get_readPtr();
	}

};

