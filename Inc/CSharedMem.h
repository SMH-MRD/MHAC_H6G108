#pragma once

#include "framework.h"

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
typedef struct stCraneStatus {

	int	dummy1;

}ST_CRANE_STATUS, * PST_CRANE_STATUS;
//-------------------------------------
typedef struct stSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * PST_SWAY_STATUS;
//-------------------------------------
typedef struct stOperationStatus {

	int	dummy1;

}ST_OPERATION_STATUS, * PST_OPERATION_STATUS;
//-------------------------------------
typedef struct stFaultStatus {

	int	dummy1;

}ST_FAULT_STATUS, * PST_FAULT_STATUS;
//-------------------------------------
typedef struct stSimulationStatus {

	int	dummy1;

}ST_SIMULATION_STATUS, * PST_SIMULATION_STATUS;
//-------------------------------------
typedef struct stPLCIO {

	int	dummy1;

}ST_PLC_IO, * PST_PLC_IO;
//-------------------------------------
typedef struct stSwayIO {

	int	dummy1;

}ST_SWAY_IO, * PST_SWAY_IO;
//-------------------------------------
typedef struct stRemoteStatus {

	int	dummy1;

}ST_REMOTE_IO, * PST_REMOTE_IO;
//-------------------------------------
typedef struct stJobStatus {

	int	dummy1;

}ST_JOB_STATUS, * PST_JOB_STATUS;
//-------------------------------------
typedef struct stCommandStatus {

	int	dummy1;

}ST_COMMAND_STATUS, * PST_COMMAND_STATUS;
//-------------------------------------
typedef struct stExecStatus {

	int	dummy1;

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
	LPVOID get_writePtr() {	if(ibuf_update) return buf1Ptr; else return buf0Ptr;}
	LPVOID get_readePtr() { if(ibuf_update) return buf1Ptr; else return buf0Ptr;}

	int create_smem(LPCTSTR szName, DWORD dwSize);
	int delete_smem();

	virtual int update();
	int clear_smem();

	wstring wstr_smem_name;

protected:
	HANDLE hMapFile;
	LPVOID pMapTop;
	DWORD  dwExist;

	LPVOID buf0Ptr;
	LPVOID buf1Ptr;
	
	int ibuf_update;
};

class CCraneStatus :public CSharedMem
{
public:
	CCraneStatus() {}
	~CCraneStatus() { delete_smem(); }
	int update();
};

class CSwayStatus :public CSharedMem
{
public:
	CSwayStatus() {}
	~CSwayStatus() { delete_smem(); }
	int update();
};

class COperationStatus :public CSharedMem
{
public:
	COperationStatus() {}
	~COperationStatus() { delete_smem(); }
	int update();
};

class CFaultStatus :public CSharedMem
{
public:
	CFaultStatus() {}
	~CFaultStatus() { delete_smem(); }
	int update();
};

class CSimulationStatus :public CSharedMem
{
public:
	CSimulationStatus() {}
	~CSimulationStatus() { delete_smem(); }
	int update();
};

class CPLCIO :public CSharedMem
{
public:
	CPLCIO() {}
	~CPLCIO() { delete_smem(); }
	int update();
};

class CSwayIO :public CSharedMem
{
public:
	CSwayIO() {}
	~CSwayIO() { delete_smem(); }
	int update();
};

class CRemoteIO :public CSharedMem
{
public:
	CRemoteIO() {}
	~CRemoteIO() { delete_smem(); }
	int update();
};


class CJobStatus :public CSharedMem
{
public:
	CJobStatus() {}
	~CJobStatus() { delete_smem(); }
	int update();
};

class CCommandStatus :public CSharedMem
{
public:
	CCommandStatus() {}
	~CCommandStatus() { delete_smem(); }
	int update();
};


class CExecStatus :public CSharedMem
{
public:
	CExecStatus() {}
	~CExecStatus() { delete_smem(); }
	int update();
};

