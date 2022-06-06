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

//  共有メモリステータス
#define	OK_SHMEM						0	// 共有メモリ 生成/破棄正常
#define	ERR_SHMEM_CREATE				-1	// 共有メモリ Create異常
#define	ERR_SHMEM_VIEW					-2	// 共有メモリ View異常
#define	ERR_SHMEM_NOT_AVAILABLE			-3	// 共有メモリ View異常
#define	ERR_SHMEM_MUTEX					-4	// 共有メモリ View異常

#define SMEM_DATA_SIZE_MAX				1000000	//共有メモリ割り当て最大サイズ　1Mbyte	

using namespace std;

/****************************************************************************/
/*   PLC IO定義構造体                                                     　*/
/* 　PLC_IF PROCがセットする共有メモリ上の情報　　　　　　　　　　　　　　  */
/****************************************************************************/
#define PLC_PB_MAX              64 //運転操作ボタン登録最大数
#define PLC_LAMP_MAX            64 //運転操作ボタン登録最大数
#define PLC_CTRL_MAX            64 //運転操作ボタン登録最大数
#define N_PLC_FAULTS			400	//PLCフォルトの割り当てサイズ

// PLC_User IF信号構造体（機上運転室IO)
// IO割付内容は、PLC_IO_DEF.hに定義
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	BOOL pb[PLC_PB_MAX];
	BOOL lamp[PLC_LAMP_MAX];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_状態信号構造体（機上センサ信号)
typedef struct StPLCStatus {
	BOOL ctrl[PLC_CTRL_MAX];		//制御用信号　LS,MC状態等
	double v_fb[MOTION_ID_MAX];
	double v_ref[MOTION_ID_MAX];
	double trq_fb_01per[MOTION_ID_MAX];
	double pos[MOTION_ID_MAX];
	double weight;
}ST_PLC_STATUS, * LPST_PLC_STATUS;

// PLC_IO構造体
#define PLC_IF_PC_DBG_MODE  0x00000001		//PCデバッグパネル、SIM出力からIO情報生成
typedef struct StPLCIO {
	DWORD mode;
	BOOL is_debug_mode;
	DWORD helthy_cnt;
	ST_PLC_UI ui;
	ST_PLC_STATUS status;
	CHAR faultPLC[N_PLC_FAULTS];
}ST_PLC_IO, * LPST_PLC_IO;

/****************************************************************************/
/*   振れセンサ信号定義構造体                                  　         　*/
/* 　SWAY_PC_IFがセットする共有メモリ上の情報　      　　　　　　           */
/****************************************************************************/

#define SENSOR_TARGET_MAX            4//検出ターゲット最大数
#define SID_TG1                      0//ターゲットID
#define SID_TG2                      1
#define SID_TG3                      2
#define SID_TG4                      3

#define DETECT_AXIS_MAX              4//検出軸最大数
#define SID_X                        0
#define SID_Y                        1
#define SID_TH                       2
#define SID_R                        3

#define TG_LAMP_NUM_MAX              3//ターゲット毎のランプ最大数
#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2

#define SWAY_FAULT_ITEM_MAX			 4//異常検出項目数
#define SID_COMMON_FLT               0
#define SID_GREEN                    1
#define SID_BLUE                     2


#define SWAY_IF_SIM_DBG_MODE  0x00000010	//振れデータをSIM出力から生成

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//ターゲットサ検出モード
	WORD status[SENSOR_TARGET_MAX];							//ターゲットサ検出状態
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//センサ異常状態
	double rad[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//振れ角
	double w[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//振れ角速度
	double ph[SENSOR_TARGET_MAX][DETECT_AXIS_MAX];			//振れ位相
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//ターゲット検出PIXEL数（面積）
	double skew_rad[SENSOR_TARGET_MAX];						//スキュー角
	double skew_w[SENSOR_TARGET_MAX];						//スキュー角速度
	double tilt_rad[DETECT_AXIS_MAX];						//傾斜角

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
#define SIM_ACTIVE_MODE  0x00000100	//シミュレーション実行モード

typedef struct StSimulationStatus {
	DWORD mode;
	DWORD helthy_cnt;
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
}ST_SIMULATION_STATUS, * LPST_SIMULATION_STATUS;

/****************************************************************************/
/*   クレーン状態定義構造体                                          　   　*/
/* 　Environmentタスクがセットする共有メモリ上の情報　　　　　　　 　    　 */
/****************************************************************************/
#define DBG_PLC_IO				0x00000001
#define DBG_SWAY_IO				0x00000100
#define DBG_ROS_IO				0x00010000
#define DBG_SIM_ACT				0X01000000

#define N_PC_FAULT_WORDS		16			//制御PC検出フォルトbitセットWORD数
#define N_PLC_FAULT_WORDS		32			//PLC検出フォルトbitセットWORD数

#define OPERATION_MODE_REMOTE	0x0000001

//デバッグモード設定共用体
union Udebug_mode {//[0]:デバッグモード内容  [1]-[3]:オプション内容
	DWORD all;
	UCHAR item[4];
};

//振れセンサ状態定義構造体
typedef struct StSwayStatus {

	int	dummy1;

}ST_SWAY_STATUS, * LPST_SWAY_STATUS;

typedef struct stEnvSubproc {
	bool is_plcio_join = false;
	bool is_sim_join = false;
	bool is_sway_join = false;
} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;


typedef struct StCraneStatus {
	DWORD env_act_count;			//ヘルシー信号
	ST_ENV_SUBPROC subproc_stat;	//サブプロセスの状態
	ST_SPEC spec;
	DWORD operation_mode;
	double notch_spd_ref[MOTION_ID_MAX];		//ノッチ速度指令
	WORD faultPC[N_PC_FAULT_WORDS];//制御PC検出異常
	WORD faultPLC[N_PLC_FAULT_WORDS];//制御PC検出異常
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
#define M_AXIS			8	//動作軸数
#define MH_AXIS			0	//主巻軸コード
#define TT_AXIS			1	//横行軸コード
#define GT_AXIS			2	//走行軸コード
#define BH_AXIS			3	//起伏軸コード
#define SLW_AXIS		4	//旋回軸コード
#define SKW_AXIS		5	//スキュー軸コード
#define LFT_AXIS		6	//吊具操作軸コード
#define NO_AXIS			7	//状態変更コード

#define REQ_STANDBY			1
#define REQ_ACTIVE			2
#define REQ_SUSPENDED		3
#define REQ_COMP_NORMAL		0
#define REQ_IMPOSSIBLE		-1
#define REQ_COMP_ABNORMAL   -2

typedef struct stMotionRecipe {					//移動パターン
	DWORD id;									//ID No. LOWORD:No. HIWORD:軸コード
	int motion_type;							//動作種別　移動、
	int n_step;									//動作構成要素数
	DWORD opt_dw;								//オプション条件
	int time_limit;								//タイムオーバー判定値
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//動作定義要素配列
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

/****************************************************************************/
/*   動作実行管理構造体                                                     */
/* 　動作（MOTION)実行状態管理用構造体									　  */
/****************************************************************************/
typedef struct stMotionStat {
	DWORD id;									//ID No. LOWORD:コマンドID No. HIWORD:軸コード
	int status;									//動作実行状況
	int iAct;									//実行中要素配列index -1で完了
	int act_count;								//実行中要素の実行カウント数
	int elapsed;								//MOTION開始後経過時間
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
	DWORD jobID;								//紐付けられているJOB　ID
	DWORD comID;								//ID No.
	bool is_motion_there[M_AXIS];				//動作対象軸
	ST_MOTION_RECIPE motions[M_AXIS];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

/****************************************************************************/
/*   運転実行管理構造体                                           　　      */
/****************************************************************************/
typedef struct stCommandStat {				//運転要素
	int type;								//コマンド種別
	DWORD comID;							//ID No.
	int status;								//コマンド実行状況
	int elapsed;							//経過時間
	int error_code;							//エラーコード　異常完了時
	ST_MOTION_STAT p_motion_stat[M_AXIS];	//各軸実行ステータス構造体のアドレス
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;

/***********************************************************************************/
/*   作業内容（JOB)定義構造体                                 　     　　　　　　  */
/* 　ClientService タスクがセットする共有メモリ上の情報							   */
/*   JOBの内容　:　																   */
/*			搬送　	指定カ所から荷を取って指定カ所へ蔵置後、待機位置への移動まで　 */
/*			移動　	指定カ所から荷を取って指定カ所へ蔵置後、定期定位置への移動まで */
/*			その他	電源投入,モード変更等の処理									   */
/***********************************************************************************/
#define COM_STEP_MAX		5//　JOBを構成するコマンド最大数

#define JOB_TYPE_HANDLING	0x00000001

//# ClientService タスクセット
typedef struct _stJobRecipe {
	DWORD jobID;								//JOB ID
	DWORD type;									//JOB種別（搬送、移動、操作）
	int	n_com_step;								//ステップ数
	double to_pos[COM_STEP_MAX][MOTION_ID_MAX];	//各軸STEP毎　目標位置
	DWORD option[COM_STEP_MAX][MOTION_ID_MAX];	//各軸STEP毎　オプション条件
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;


//# Policy タスクセット
typedef struct stJobStat {						//JOB実行状態
	DWORD jobID;								//JOB ID
	int n_job_step;								//構成コマンド数
	int job_step_now;							//実行中ステップ
	int status;									//job実行状況
	int elapsed;								//経過時間
	int error_code;								//エラーコード　異常完了時
}ST_JOB_STAT, * LPST_JOB_STAT;

#define JOB_HOLD_MAX		10					//	保持可能JOB最大数
#define NO_JOB_REQUIRED		-1					//  要求ジョブ無し


//# Policy タスクセット領域

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	情報定義構造体                                   　   　*/
/* 　Client Serviceタスクがセットする共有メモリ上の情報　　　　　　　 　    */
/****************************************************************************/
typedef struct stCSInfo {

	//for Policy
	int current_job;							//実行要求中のJOBインデックス -1:要求無し
	ST_JOB_RECIPE	jobs[JOB_HOLD_MAX];			

	//for Client
	DWORD req_stat;								

}ST_CS_INFO, * LPST_CS_INFO;


#define BITSEL_HOIST        0x00000001   //巻 　       ビット
#define BITSEL_GANTRY       0x00000002   //走行        ビット
#define BITSEL_TROLLY       0x00000004   //横行        ビット
#define BITSEL_BOOM_H       0x00000008   //引込        ビット
#define BITSEL_SLEW         0x00000010   //旋回        ビット
#define BITSEL_OP_ROOM      0x00000020   //運転室移動　ビット
#define BITSEL_H_ASSY       0x00000040   //吊具        ビット
#define BITSEL_COMMON       0x10000000   //共通        ビット

/****************************************************************************/
/*   Policy	情報定義構造体                                   　			  　*/
/* 　Policy	タスクがセットする共有メモリ上の情報　　　　　　　		 　		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	//for AGENT
	DWORD pc_ctrl_mode;							//制御モード
	DWORD antisway_mode;						//振れ止めモード
	int current_com;							//実行要求中のコマンドインデックス -1:要求無し
	ST_COMMAND_RECIPE commands[COM_STEP_MAX];	

	//for CS


	ST_JOB_STAT job_stat[JOB_HOLD_MAX];			

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	情報定義構造体                                   　   　		*/
/* 　Agent	タスクがセットする共有メモリ上の情報　　　　　　　 　			*/
/****************************************************************************/
typedef struct stAgentInfo {
	//for POLICY
	ST_COMMAND_STAT com_stat[COM_STEP_MAX];	
	
	//for CRANE
	double v_ref[MOTION_ID_MAX];				

}ST_AGENT_INFO, * LPST_AGENT_INFO;

static char smem_dummy_buf[SMEM_DATA_SIZE_MAX];

/****************************************************************************/
/*共有メモリクラス定義														*/
/****************************************************************************/
class CSharedMem
{
public:
	CSharedMem();
	~CSharedMem();

	int smem_available;			//共有メモリ有効
	int data_size;				//データサイズ

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
