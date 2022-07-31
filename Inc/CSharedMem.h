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
#define N_PLC_PBS				64 //運転操作PB数
#define N_PLC_BITS				64 //BIT STATUS数
#define N_PLC_CTRL_WORDS        16 //制御センサ信号WORD数
#define N_PLC_FAULTS			400	//PLCフォルトの割り当てサイズ

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

// PLC_User IF信号構造体（機上運転室IO)
// IO割付内容は、PLC_IO_DEF.hに定義
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	bool PBs[N_PLC_PBS];
	bool BITs[N_PLC_BITS];
}ST_PLC_UI, * LPST_PLC_UI;

// PLC_状態信号構造体（機上センサ信号)
typedef struct StPLCStatus {
	UINT16 ctrl[N_PLC_CTRL_WORDS];		//制御用信号　LS,MC状態等
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

#define DETECT_AXIS_MAX              4// 検出軸数


#define TG_LAMP_NUM_MAX              3//ターゲット毎のランプ最大数

#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2

#define SWAY_FAULT_ITEM_MAX			 4//異常検出項目数
#define SID_COMMON_FLT               0



#define SWAY_IF_SIM_DBG_MODE  0x00000010	//振れデータをSIM出力から生成

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//ターゲットサ検出モード
	WORD status[SENSOR_TARGET_MAX];							//ターゲットサ検出状態
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//センサ異常状態
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//ターゲット検出PIXEL数（面積）
	double tilt_rad[DETECT_AXIS_MAX];						//傾斜角

	double rad[MOTION_ID_MAX];								//振れ角
	double w[MOTION_ID_MAX];								//振れ角速度
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
	Vector3 L, vL;//ﾛｰﾌﾟﾍﾞｸﾄﾙ(振れ）
	double rad_cam_x, rad_cam_y, w_cam_x, w_cam_y;			//カメラ座標振れ角
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

#define OPERATION_MODE_REMOTE		0x0000001
#define OPERATION_MODE_SIMULATOR	0x0100000
#define OPERATION_MODE_PLC_DBGIO	0x0001000

#define N_SWAY_DIR				4


//振れセンサ状態定義構造体
typedef struct StSwaySet {
	double T;		//振周期		/s
	double w;		//振角周波数	/s
	double th;		//振角			rad
	double dth;		//振角速度		rad/s
	double dthw;	//振角速度/ω　	rad
	double ph;		//位相平面位相	rad
	double amp2;	//振幅の2乗		rad2
}ST_SWAY_SET, * LPST_SWAY__SET;
typedef struct StSwCamSet {
	double D0;		//吊点-カメラハウジング軸位置間水平距離	m
	double H0;		//吊点-カメラハウジング軸位置間高さ距離	m
	double l0;		//カメラハウジング軸-カメラ位置間距離	m
	double ph0;		//カメラハウジング軸-カメラ位置間角度	rad
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
	DWORD env_act_count=0;						//ヘルシー信号
	ST_ENV_SUBPROC subproc_stat;				//サブプロセスの状態
	ST_SPEC spec;								//クレーン仕様
	DWORD operation_mode;						//運転モード　機上,リモート
	double notch_spd_ref[MOTION_ID_MAX];		//ノッチ速度指令
	WORD faultPC[N_PC_FAULT_WORDS];				//PLC検出異常
	WORD faultPLC[N_PLC_FAULT_WORDS];			//制御PC検出異常
	ST_SWAY_STATUS sw_stat;						//振れ状態解析結果
	Vector3 rc0;								//クレーン基準点のx,y,z座標（x:走行位置 y:旋回中心 z:走行レール高さ）
	Vector3 rc;									//クレーン吊点のクレーン基準点とのx,y,z相対座標
	Vector3 rl;									//吊荷のクレーン吊点とのx,y,z相対座標
	bool is_fwd_endstop[MOTION_ID_MAX];			//正転極限判定
	bool is_rev_endstop[MOTION_ID_MAX];			//逆転極限判定
	double mh_l;								//ロープ長

}ST_CRANE_STATUS, * LPST_CRANE_STATUS;

/************************************************************************************/
/*   作業内容（JOB)定義構造体                                 　     　　　　　　	*/
/* 　ClientService タスクがセットする共有メモリ上の情報								*/
/* 　JOB	:From-Toの搬送コマンド													*/
/*   COMMAND:1つのJOBを、複数のコマンドで構成	PICK GRAND PARK						*/
/* 　JOB	:From-Toの搬送作業													*/
/************************************************************************************/
#define COM_STEP_MAX		10					//　JOBを構成するコマンド最大数
#define COM_TARGET_MAX		MOTION_ID_MAX		//　コマンド毎の目標最大数
#define JOB_TYPE_HANDLING	0x00000001

//Recipe
typedef struct _stJobRecipe {
	WORD job_id;								//JOB No
	WORD client_type;							//依頼主
	WORD job_type;								//JOB種別（搬送、移動、操作)
	WORD step_n;								//ステップ数
	double target[COM_STEP_MAX][COM_TARGET_MAX];//各STEP毎　各軸目標位置,目標搬送量
	DWORD option[COM_STEP_MAX][COM_TARGET_MAX];	//各軸STEP毎　オプション条件
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

//Status
typedef struct stJobStat {						//JOB実行状態
	LPST_JOB_RECIPE pjob;						//対象JOB
	int job_step_now;							//実行中ステップ
	int step_status[COM_STEP_MAX];				//step実行状況
	double step_elapsed[COM_STEP_MAX];			//step経過時間
	int job_status;								//完了コード　異常完了時エラーコード
}ST_JOB_STAT, * LPST_JOB_STAT;

//Set
typedef struct stJobSet {
	DWORD type;
	ST_JOB_RECIPE	recipe;
	ST_JOB_STAT		status;
}ST_JOB_SET, * LPST_JOB_SET;

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

#define REQ_STANDBY			1
#define REQ_ACTIVE			2
#define REQ_SUSPENDED		3
#define REQ_COMP_NORMAL		0
#define REQ_IMPOSSIBLE		-1
#define REQ_COMP_ABNORMAL   -2

//Recipe
typedef struct stMotionRecipe {					//移動パターン
	DWORD motion_type;							//動作種別、
	int n_step;									//動作構成要素数
	DWORD opt_dw;								//オプション条件
	int time_limit;								//タイムオーバー判定値
	ST_MOTION_ELEMENT motion[M_ELEMENT_MAX];	//動作定義要素配列
}ST_MOTION_RECIPE, * LPST_MOTION_RECIPE;

//Status
typedef struct stMotionStat {
	DWORD id;									//ID No. HIWORD:軸コード LOWORD:シリアルNo 
	int status;									//動作実行状況
	int iAct;									//実行中要素配列index -1で完了
	int act_count;								//実行中要素の実行カウント数
	int elapsed;								//MOTION開始後経過時間
	int error_code;								//エラーコード　異常完了時
}ST_MOTION_STAT, * LPST_MOTION_STAT;

//Set
typedef struct stMotionSet {
	ST_MOTION_RECIPE recipe;					//動作内容定義
	ST_MOTION_STAT status;						//動作実行状態 
}ST_MOTION_SET, * LPST_MOTION_SET;


/********************************************************************************/
/*   軸連動運転内容(COMMAND)定義構造体                             　　　　　　 */
/* 　目的動作を実現する運転内容を単軸動作の組み合わせで実現します               */
/********************************************************************************/
#define STOP_COMMAND		0//　停止運転
#define PICK_COMMAND		1//　荷掴み運転
#define GRND_COMMAND		2//　荷卸し運転
#define PARK_COMMAND		3//　移動運転
//Recipe
typedef struct stCommandRecipe {				//運転要素
	LPST_JOB_RECIPE pjob;						//紐付けられているJOB
	WORD job_step;								//紐付けられているJOBの担当ステップ
	bool is_required_motion[MOTION_ID_MAX];		//動作対象軸
	ST_MOTION_RECIPE motions[MOTION_ID_MAX];
}ST_COMMAND_RECIPE, * LPST_COMMAND_RECIPE;

//Status
typedef struct stCommandStat {				//運転要素
	int type;								//コマンド種別
	DWORD comID;							//ID No.
	int status;								//コマンド実行状況
	int elapsed;							//経過時間
	int error_code;							//エラーコード　異常完了時
}ST_COMMAND_STAT, * LPST_COMMAND_STAT;

//Set
typedef struct stCommandSet {
	ST_COMMAND_RECIPE	recipe;				//コマンド種別
	ST_COMMAND_STAT		status;				//ID No.
}ST_COMMAND_SET, * LPST_COMMAND_SET;




//# Policy タスクセット領域

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	情報定義構造体                                   　   　*/
/* 　Client Serviceタスクがセットする共有メモリ上の情報　　　　　　　 　    */
/****************************************************************************/

#define JOB_HOLD_MAX		10					//	保持可能JOB最大数

typedef struct stCSInfo {

	//for Agent
	int id_job_active;							//実行中のJOBインデックス  -1:実行無し
	ST_JOB_SET	job_list[JOB_HOLD_MAX];

	//for Client
	DWORD req_status;
	DWORD n_job_hold;							//未完JOB数

}ST_CS_INFO, * LPST_CS_INFO;


#define BITSEL_HOIST        0x00000001		//巻 　       ビット
#define BITSEL_GANTRY       0x00000002		//走行        ビット
#define BITSEL_TROLLY       0x00000004		//横行        ビット
#define BITSEL_BOOM_H       0x00000008		//引込        ビット
#define BITSEL_SLEW         0x00000010		//旋回        ビット
#define BITSEL_OP_ROOM      0x00000020		//運転室移動　ビット
#define BITSEL_H_ASSY       0x00000040		//吊具        ビット
#define BITSEL_COMMON       0x10000000		//共通        ビット

#define POLICY_AUTO_OFF		0x00000000		//自動OFF
#define POLICY_SEMI_AUTO_ON	0x00000001		//半自動MODE

#define POLICY_ANTISWAY_OFF	0x00000000		//振れ止めOFF
#define POLICY_ANTISWAY_ON	0x00000001		//振れ止めON

/****************************************************************************/
/*   Policy	情報定義構造体                                   　			  　*/
/* 　Policy	タスクがセットする共有メモリ上の情報　　　　　　　		 　		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	//for AGENT
	DWORD pc_ctrl_mode;							//制御モード
	DWORD antisway_mode;						//振れ止めモード

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	情報定義構造体                                   　   　		*/
/* 　Agent	タスクがセットする共有メモリ上の情報　　　　　　　 　			*/
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
