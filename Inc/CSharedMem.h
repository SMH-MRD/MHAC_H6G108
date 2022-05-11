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

//  共有メモリステータス
#define	OK_SHMEM						0	// 共有メモリ 生成/破棄正常
#define	ERR_SHMEM_CREATE				-1	// 共有メモリ Create異常
#define	ERR_SHMEM_VIEW					-2	// 共有メモリ View異常
#define	ERR_SHMEM_NOT_AVAILABLE			-3	// 共有メモリ View異常

#define SMEM_DATA_SIZE_MAX				1000000	//共有メモリ割り当て最大サイズ　1Mbyte	

using namespace std;

/****************************************************************************/
/*   PLC IO定義構造体                                                     　*/
/* 　PLC_IF PROCがセットする共有メモリ上の情報　　　　　　　　　　　　　　  */
/****************************************************************************/
#define PLC_PB_MAX              64 //運転操作ボタン登録最大数
#define PLC_LAMP_MAX            64 //運転操作ボタン登録最大数
#define PLC_CTRL_MAX            64 //運転操作ボタン登録最大数

#ifndef PLC_PBL_ID //PLC IOの配列
#endif // !PLC_PBL_ID

#define N_PLC_FAULT_WORDS		32	//PLCフォルトの割り当てサイズ

// PLC_User IF信号構造体（機上運転室IO)
typedef struct StPLCUI {
	WORD notch_pos[MOTION_ID_MAX];
	BOOL pb[PLC_PB_MAX];
	BOOL lamp[PLC_LAMP_MAX];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_状態信号構造体（機上センサ信号)
typedef struct StPLCStatus {
	BOOL ctrl[PLC_CTRL_MAX];
	double v_fb[MOTION_ID_MAX];
	double v_ref[MOTION_ID_MAX];
	double trq_fb_01per[MOTION_ID_MAX];
	double pos[MOTION_ID_MAX];
	double weight;
	BOOL is_debug_mode;
}ST_PLC_STATUS, * LPST_PLC_STATUS;

// PLC_IO構造体
typedef struct StPLCIO {
	ST_PLC_UI ui;
	ST_PLC_STATUS status;
	WORD faultPLC[N_PLC_FAULT_WORDS];
	BOOL is_debug_mode;
}ST_PLC_IO, * LPST_PLC_IO;

/****************************************************************************/
/*   振れセンサ信号定義構造体                                  　         　*/
/* 　SWAY_PC_IFがセットする共有メモリ上の情報　      　　　　　　           */
/****************************************************************************/
//SWAY_IO
#ifndef SWAY_IO_ID

#define SENSOR_TARGET_MAX            4//検出ターゲット最大数
#define DETECT_AXIS_MAX              4//検出軸最大数
#define TG_LAMP_NUM_MAX              3//ターゲット毎のランプ最大数

#define SID_TG1                      0//ターゲットID
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
/*   遠隔操作卓信号定義構造体                                  　         　*/
/* 　ROS_IF PROCがセットする共有メモリ上の情報　　　　　　　          　    */
/****************************************************************************/
typedef struct StRemoteIO {

	ST_PLC_UI PLCui;

}ST_REMOTE_IO, * LPST_REMOTE_IO;

/****************************************************************************/
/*   シミュレーション信号定義構造体                                  　   　*/
/* 　SIM PROCがセットする共有メモリ上の情報　　　　　　　          　    　 */
/****************************************************************************/
typedef struct StSimulationStatus {
	bool is_simproc_available;
	bool is_simulation_active;
	double spd_fb[MOTION_ID_MAX];
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
}ST_SIMULATION_STATUS, * LPST_SIMULATION_STATUS;

/****************************************************************************/
/*   クレーン状態定義構造体                                          　   　*/
/* 　Environmentタスクがセットする共有メモリ上の情報　　　　　　　 　    　 */
/****************************************************************************/
//振れセンサ状態定義構造体
typedef struct StSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * LPST_SWAY_STATUS;

#define N_PC_FAULT_WORDS		16	//PCフォルトの割り当てサイズ
typedef struct StCraneStatus {
	
	ST_SPEC spec;
	WORD notch_pos[MOTION_ID_MAX];
	WORD faultPC[N_PC_FAULT_WORDS];
	ST_SWAY_STATUS sway_stat;

}ST_CRANE_STATUS, * LPST_CRANE_STATUS;

/****************************************************************************/
/*   運動要素定義構造体                                                     */
/* 　加速、定速、減速等の一連の動作は、この要素の組み合わせで構成します。   */
/****************************************************************************/
typedef struct stMotionElement {	//運動要素
	int type;				//制御種別
	int status;				//制御種状態
	int time_count;			//予定継続時間のカウンタ変換値
	double _a;				//目標加減速度
	double _v;				//目標速度
	double _p;				//目標位置
	double _t;				//継続時間
	double v_max;			//速度制限High
	double v_min;			//速度制限Low
	double phase1;			//起動位相１
	double phase2;			//起動位相 2
	double opt_d[8];		//オプションdouble
	int opt_i[8];			//オプションint
}ST_MOTION_ELEMENT, * LPST_MOTION_ELEMENT;

/****************************************************************************/
/*   動作内容定義構造体（単軸）												*/
/* 　単軸の目標状態に移行する動作パターンを運動要素の組み合わせで実現します */
/****************************************************************************/
#define M_ELEMENT_MAX	32
#define M_AXIS			8	//動作軸
#define MH_AXIS			0	//主巻動作
#define TT_AXIS			1	//横行動作
#define GT_AXIS			2	//走行動作
#define BH_AXIS			3	//起伏動作
#define SLW_AXIS		4	//旋回動作
#define SKW_AXIS		5	//スキュー動作
#define LFT_AXIS		6	//吊具操作

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

typedef struct stMotionRecipe {					//移動パターン
	int axis;									//動作軸
	int n_motion;								//ID No.
	int n_step;									//移動パターン構成要素数
	int motion_type;							//オプション指定
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//移動パターン定義運動要素配列
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

/****************************************************************************/
/*   動作実行管理構造体                                                     */
/* 　動作（MOTION)実行状態管理用構造体									　  */
/****************************************************************************/
typedef struct stMotionStat {
	int axis;									//動作軸
	int id;									//ID No.
	int status;									//動作実行状況
	int iAct;									//実行中要素配列index -1で完了
	int act_count;								//実行中要素の実行カウント数
	int elapsed;								//経過時間
	int error_code;								//エラーコード　異常完了時
}ST_MOTION_STAT, * LPST_MOTION_STAT;

/********************************************************************************/
/*   軸連動運転内容定義構造体                                      　　　　　　 */
/* 　目的動作を実現する運転内容を単軸動作の組み合わせで実現します               */
/********************************************************************************/
#define STOP_COMMAND		0//　停止運転
#define PICK_COMMAND		1//　荷掴み運転
#define GRND_COMMAND		2//　荷卸し運転
#define PARK_COMMAND		3//　移動運転

typedef struct stCommandRecipe {				//運転要素
	int type;									//コマンド種別
	int id;									//ID No.
	ST_MOTION_RECIPE motions[M_AXIS];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

/****************************************************************************/
/*   運転実行管理構造体                                                 */
/****************************************************************************/
typedef struct stCommandStat {				//運転要素
	int type;									//コマンド種別
	int id;									//ID No.
	int status;									//コマンド実行状況
	int elapsed;								//経過時間
	int error_code;								//エラーコード　異常完了時
	axis_check isnot_axis_completed;			//各軸実行中判定フラグ　0で完了
	LPST_MOTION_STAT p_motion_stat[M_AXIS];		//各軸実行ステータス構造体のアドレス
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;



/********************************************************************************/
/*   作業内容（JOB)定義構造体                                      　　　　　　	*/
/* 　ClientService タスクがセットする共有メモリ上の情報								            */
/********************************************************************************/
#define JOB_STEP_MAX		5//　JOBを構成するコマンド最大数

typedef struct stCommandSet {	//作業要素（PICK、GROUND、PARK　....）
	int type;							//コマンド種別（PICK、GROUND、PARK）
	axis_check is_valid_axis;			//対象動作軸
	double target_pos[MOTION_ID_MAX];	//各軸目標位置
	int option[MOTION_ID_MAX];		//各軸動作オプション条件
}ST_COMMAND_SET, * LPST_COMMAND_SET;

//#   作業（JOB)実行管理構造体  
//# 　コマンド実行状態管理用構造体
typedef struct _stJobRecipe {				//作業構成要素（保管,払出,退避移動等）
	int type;										//JOB種別
	int job_id;										//job識別コード
	int n_job_step;									//構成コマンド数
	ST_COMMAND_SET commands[JOB_STEP_MAX];			//JOB構成コマンド
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

typedef struct stJOB_STAT {					//運転要素
	int type;										//JOB種別
	int job_id;										//job識別コード
	int n_job_step;									//構成コマンド数
	int job_step_now;								//実行中ステップ
	int status;										//job実行状況
	int elapsed;									//経過時間
	int error_code;									//エラーコード　異常完了時
	LPST_COMMAND_STAT p_command_stat[JOB_STEP_MAX];	//各コマンドステータス構造体のアドレス
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

