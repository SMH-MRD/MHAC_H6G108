#pragma once

#include "framework.h"
#include "common_def.h"

#define SMEM_NAME_CRANE_STATUS      L"SMEM_CRANE_STATU"
#define SMEM_NAME_SWAY_STATUS       L"SWAY_STATUS"
#define SMEM_NAME_OPERATION_STATUS  L"OPERATION_STATUS"
#define SMEM_NAME_FAULT_STATUS      L"FAULT_STATUS"
#define SMEM_NAME_SIMULATION_STATUS L"SMEM_SIMULATION_STATUS"
#define SMEM_NAME_PLC_IO            L"PLC_IO"
#define SMEM_NAME_SWAY_IO           L"SWAY_IO"
#define SMEM_NAME_REMOTE_IO         L"REMOTE_IO"
#define SMEM_NAME_JOB_STATUS        L"JOB_STATUS"
#define SMEM_NAME_COMMAND_STATUS    L"COMMAND_STATUS"
#define SMEM_NAME_EXEC_STATUS       L"EXEC_STATUS"

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

//  共有メモリステータス
#define	OK_SHMEM						0	// 共有メモリ 生成/破棄正常
#define	ERR_SHMEM_CREATE				-1	// 共有メモリ Create異常
#define	ERR_SHMEM_VIEW					-2	// 共有メモリ View異常
#define	ERR_SHMEM_NOT_AVAILABLE			-3	// 共有メモリ View異常

#define SMEM_DATA_SIZE_MAX				1000000	//共有メモリ割り当て最大サイズ　1Mbyte	

using namespace std;

/***********************************************************************
共有メモリ用構造体
************************************************************************/
//-------------------------------------
typedef struct StPLCUI {
	WORD notch_pos[MOTION_ID_MAX];
	BOOL pb[PLC_PB_MAX];
	BOOL lamp[PLC_LAMP_MAX];
}ST_PLC_UI, * PST_PLC_UI;

typedef struct StPLCStatus {
	BOOL ctrl[PLC_CTRL_MAX];
	double v_fb[MOTION_ID_MAX];
	double v_ref[MOTION_ID_MAX];
	double trq_fb_01per[MOTION_ID_MAX];
	double pos[MOTION_ID_MAX];
	double weight;
	BOOL is_debug_mode;
}ST_PLC_STATUS, * PST_PLC_STATUS;

#define N_PLC_FAULT_WORDS		32	//PLCフォルトの割り当てサイズ

typedef struct StPLCIO {
	ST_PLC_UI ui;
	ST_PLC_STATUS status;
	WORD faultPLC[N_PLC_FAULT_WORDS];
	BOOL is_debug_mode;
}ST_PLC_IO, * PST_PLC_IO;

//-------------------------------------
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
}ST_SWAY_IO, * PST_SWAY_IO;
//-------------------------------------
typedef struct StRemoteIO {

	ST_PLC_UI PLCui;

}ST_REMOTE_IO, * PST_REMOTE_IO;
//-------------------------------------
typedef struct StSimulationStatus {

	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;

}ST_SIMULATION_STATUS, * PST_SIMULATION_STATUS;

//-------------------------------------

#define N_PC_FAULT_WORDS			16	//遠隔操作フォルトの割り当てサイズ
typedef struct StCraneStatus {
	
	WORD notch_pos[MOTION_ID_MAX];
	WORD faultPC[N_PC_FAULT_WORDS];

}ST_CRANE_STATUS, * PST_CRANE_STATUS;
//-------------------------------------
typedef struct StSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * PST_SWAY_STATUS;
//-------------------------------------

#define N_REMOTE_FAULT_WORDS		8	//遠隔操作フォルトの割り当てサイズ

typedef struct StJobStatus {

	int	dummy1;

}ST_JOB_STATUS, * PST_JOB_STATUS;
//-------------------------------------
typedef struct StCommandStatus {

	int	dummy1;

}ST_COMMAND_STATUS, * PST_COMMAND_STATUS;
//-------------------------------------
typedef struct StExecStatus {
	double v_ref[MOTION_ID_MAX];
	BOOL dout[AGENT_DO_MAX];
}ST_EXEC_STATUS, * PST_EXEC_STATUS;

static char smem_dummy_buf[SMEM_DATA_SIZE_MAX];

/***********************************************************************
クラス定義
************************************************************************/
class CSharedMem
{
public:
	CSharedMem();
	~CSharedMem();

	int smem_available;			//共有メモリ有効
	int data_size;				//データサイズ(1バッファ分）
	bool use_double_buff;		//ダブルバッファ利用の選択

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
	
	int ibuf_write;//書き込みバッファのインデックス(0 or 1)
};

class CCraneStatus :public CSharedMem
{
public:
	CCraneStatus() {}
	~CCraneStatus() { delete_smem(); }
	
	int set_data_size() {
		return sizeof(ST_CRANE_STATUS);
	};

	PST_CRANE_STATUS pbuf_write(){
		return (PST_CRANE_STATUS)get_writePtr();
	}
	PST_CRANE_STATUS pbuf_read() {
		return (PST_CRANE_STATUS)get_readPtr();
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

	PST_SWAY_STATUS pbuf_write() {
		return (PST_SWAY_STATUS)get_writePtr();
	}
	PST_SWAY_STATUS pbuf_read() {
		return (PST_SWAY_STATUS)get_readPtr();
	}
};

class CSimulationStatus :public CSharedMem
{
public:
	CSimulationStatus() { memset(spd_fb, 0, sizeof(spd_fb[MOTION_ID_MAX])); }
	~CSimulationStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_SIMULATION_STATUS);
	};

	PST_SIMULATION_STATUS pbuf_write() {
		return (PST_SIMULATION_STATUS)get_writePtr();
	}
	PST_SIMULATION_STATUS pbuf_read() {
		return (PST_SIMULATION_STATUS)get_readPtr();
	}
	double spd_fb[MOTION_ID_MAX];
};

class CPLCIO :public CSharedMem
{
public:
	CPLCIO() { memset(spd_fb, 0, sizeof(spd_fb[MOTION_ID_MAX])); }
	~CPLCIO() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_PLC_IO);
	};

	PST_PLC_IO pbuf_write() {
		return (PST_PLC_IO)get_writePtr();
	}
	PST_PLC_IO pbuf_read() {
		return (PST_PLC_IO)get_readPtr();
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

	PST_SWAY_IO pbuf_write() {
		return (PST_SWAY_IO)get_writePtr();
	}
	PST_SWAY_IO pbuf_read() {
		return (PST_SWAY_IO)get_readPtr();
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

	PST_REMOTE_IO pbuf_write() {
		return (PST_REMOTE_IO)get_writePtr();
	}
	PST_REMOTE_IO pbuf_read() {
		return (PST_REMOTE_IO)get_readPtr();
	}
};


class CJobStatus :public CSharedMem
{
public:
	CJobStatus() { memset(&job, 0, sizeof(job)); }
	~CJobStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_JOB_STATUS);
	};

	PST_JOB_STATUS pbuf_write() {
		return (PST_JOB_STATUS)get_writePtr();
	}
	PST_JOB_STATUS pbuf_read() {
		return (PST_JOB_STATUS)get_readPtr();
	}

	ST_JOB_RECIPE	job;
	ST_JOB_STAT		job_stat;
};


class CCommandStatus :public CSharedMem
{
public:
	CCommandStatus() { memset(&command, 0, sizeof(command)); }
	~CCommandStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_COMMAND_STATUS);
	};

	PST_COMMAND_STATUS pbuf_write() {
		return (PST_COMMAND_STATUS)get_writePtr();
	}
	PST_COMMAND_STATUS pbuf_read() {
		return (PST_COMMAND_STATUS)get_readPtr();
	}

	ST_COMMAND_RECIPE command[JOB_STEP_MAX];
	ST_COMMAND_STAT command_stat[JOB_STEP_MAX];
};

class CExecStatus :public CSharedMem
{
public:
	CExecStatus() { memset(motion_stat, 0, sizeof(motion_stat[M_AXIS])); }
	~CExecStatus() { delete_smem(); }

	int set_data_size() {
		return sizeof(ST_EXEC_STATUS);
	};

	PST_EXEC_STATUS pbuf_write() {
		return (PST_EXEC_STATUS)get_writePtr();
	}
	PST_EXEC_STATUS pbuf_read() {
		return (PST_EXEC_STATUS)get_readPtr();
	}

	ST_MOTION_STAT motion_stat[M_AXIS];
	double spd_ref[MOTION_ID_MAX];
};

