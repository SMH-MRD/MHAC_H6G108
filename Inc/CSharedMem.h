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
#define N_PLC_PB				64 //運転操作PB数
#define N_PLC_LAMP				64 //BIT STATUS数
#define N_PLC_CTRL_WORDS        16 //制御センサ信号WORD数
#define N_PLC_FAULTS			400	//PLCフォルトの割り当てサイズ

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


#define PLC_IO_LAMP_FLICKER_COUNT    40 //ランプフリッカの間隔カウント
#define PLC_IO_LAMP_FLICKER_CHANGE   20 //ランプフリッカの間隔カウント

// PLC_User IF信号構造体（機上運転室IO)
// IO割付内容は、PLC_IO_DEF.hに定義
typedef struct StPLCUI {
	int notch_pos[MOTION_ID_MAX];
	bool PB[N_PLC_PB];
	bool PBsemiauto[SEMI_AUTO_TARGET_MAX];
	bool LAMP[N_PLC_LAMP];
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


#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

typedef struct StSwayIO {
	DWORD proc_mode;
	DWORD helthy_cnt;

	char sensorID[4];
	WORD mode[SENSOR_TARGET_MAX];							//ターゲットサ検出モード
	WORD status[SENSOR_TARGET_MAX];							//ターゲットサ検出状態
	DWORD fault[SWAY_FAULT_ITEM_MAX];						//センサ異常状態
	double pix_size[SENSOR_TARGET_MAX][TG_LAMP_NUM_MAX];	//ターゲット検出PIXEL数（面積）
	double tilt_rad[DETECT_AXIS_MAX];						//傾斜角

	double th[MOTION_ID_MAX];		//振角			rad
	double dth[MOTION_ID_MAX];		//振角速度		rad/s
	double dthw[MOTION_ID_MAX];		//振角速度/ω　	rad
	double ph[MOTION_ID_MAX];		//位相平面位相	rad
	double amp2[MOTION_ID_MAX];		//振幅の2乗		rad2


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
#define SIM_ACTIVE_MODE  0x00000100			//シミュレーション実行モード
#define SIM_SWAY_PACKET_MODE 0x00000010		//振れセンサパケット送信モード
typedef struct StSimulationStatus {
	DWORD mode;
	DWORD helthy_cnt;
	ST_PLC_STATUS status;
	ST_SWAY_IO sway_io;
	Vector3 L, vL;//ﾛｰﾌﾟﾍﾞｸﾄﾙ(振れ）
	double rad_cam_x, rad_cam_y, w_cam_x, w_cam_y;			//カメラ座標振れ角

	ST_SWAY_RCV_MSG rcv_msg;
	ST_SWAY_SND_MSG snd_msg;

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

typedef struct stEnvSubproc {

	bool is_plcio_join = false;
	bool is_sim_join = false;
	bool is_sway_join = false;

} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;

#define MANUAL_MODE				0
#define ANTI_SWAY_MODE			1
#define SEMI_AUTO_ACTIVE		2
#define AUTO_ACTIVE				3

#define BITSEL_HOIST        0x0001		//巻 　       ビット
#define BITSEL_GANTRY       0x0002		//走行        ビット
#define BITSEL_TROLLY       0x0004		//横行        ビット
#define BITSEL_BOOM_H       0x0008		//引込        ビット
#define BITSEL_SLEW         0x0010		//旋回        ビット
#define BITSEL_OP_ROOM      0x0020		//運転室移動　ビット
#define BITSEL_H_ASSY       0x0040		//吊具        ビット
#define BITSEL_COMMON       0x0080		//共通        ビット

#define BITSEL_SEMIAUTO     0x0001
#define BITSEL_AUTO			0x0002

#define SPD0_CHECK_RETIO	0.1

#define STAT_ACC			0;
#define STAT_DEC			1;

typedef struct StCraneStatus {
	DWORD env_act_count=0;													//ヘルシー信号
	ST_ENV_SUBPROC subproc_stat;											//サブプロセスの状態
	bool is_tasks_standby_ok;												//タスクの立ち上がり確認
	ST_SPEC spec;															//クレーン仕様
	WORD operation_mode;													//運転モード　機上,リモート

	bool	auto_standby;													//自動モード
	double semi_auto_setting_target[SEMI_AUTO_TARGET_MAX][MOTION_ID_MAX];	//半自動設定目標位置
	int	 semi_auto_selected;												//半自動選択ID
	int	 semi_auto_pb_count[SEMI_AUTO_TARGET_MAX];							//半自動PB　ONカウント
	int	 auto_start_pb_count;												//自動開始PB　ONカウント
	bool is_notch_0[MOTION_ID_MAX];											//振れ止めモードノッチ条件
	double r0[MOTION_ID_MAX];	                //加速振 rad
	
	double notch_spd_ref[MOTION_ID_MAX];		//ノッチ速度指令
	WORD faultPC[N_PC_FAULT_WORDS];				//PLC検出異常
	WORD faultPLC[N_PLC_FAULT_WORDS];			//制御PC検出異常
	Vector3 rc0;								//クレーン基準点のx,y,z座標（x:走行位置 y:旋回中心 z:走行レール高さ）
	Vector3 rc;									//クレーン吊点のクレーン基準点とのx,y,z相対座標
	Vector3 rl;									//吊荷のクレーン吊点とのx,y,z相対座標
	Vector3 rcam;								//振れセンサ検出x,y,z座標
	bool is_fwd_endstop[MOTION_ID_MAX];			//正転極限判定
	bool is_rev_endstop[MOTION_ID_MAX];			//逆転極限判定
	double mh_l;								//ロープ長
	double T;									//振周期		s
	double w;									//振角周波数	/s
	double w2;									//振角周波数の2乗
	double a[MOTION_ID_MAX][2];					//加減速度現在値[0]加速　[1］減速

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
/*   作業内容（JOB)定義構造体                                 　     　　　　　　	*/
/* 　ClientService タスクがセットする共有メモリ上の情報								*/
/* 　JOB	:From-Toの搬送コマンド													*/
/*   COMMAND:1つのJOBを、複数のコマンドで構成	PICK GRAND PARK						*/
/* 　JOB	:From-Toの搬送作業													*/
/************************************************************************************/
#define COM_STEP_MAX		10					//　JOBを構成するコマンド最大数
#define JOB_TYPE_HANDLING	0x00000001

//Recipe
#define JOB_STEP_TYPE_NULL	0
#define JOB_STEP_TYPE_PICK	1
#define JOB_STEP_TYPE_GRAND	2
#define JOB_STEP_TYPE_PARK	3

typedef struct _stJobRecipe {
	SYSTEMTIME time_start_planed;				//予定開始時間
	int n_step;									//ステップ数
	int step_type[COM_STEP_MAX];				//各ステップのタイプ
	double target[COM_STEP_MAX][MOTION_ID_MAX];//各STEP毎　各軸目標
	int option[COM_STEP_MAX][MOTION_ID_MAX];	//各軸STEP毎　オプション条件
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
typedef struct stJobStat {						//JOB実行状態
	int status;									//完了コード　異常完了時エラーコード
	int current_step;							//実行中ステップ
	int step_status[COM_STEP_MAX];				//step実行状況
	DWORD step_elapsed[COM_STEP_MAX];			//step経過時間ms
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
	int type;								//JOB種別（搬送、移動、操作)
	ST_JOB_RECIPE	recipe;
	ST_JOB_STAT		status;
}ST_JOB_SET, * LPST_JOB_SET;

/****************************************************************************/
/*   運動要素定義構造体                                                     */
/* 　加速、定速、減速等の一連の動作は、この要素の組み合わせで構成します。   */
/****************************************************************************/
#define MOTHION_OPT_V_MAX	0
#define MOTHION_OPT_V_MIN		0
#define MOTHION_OPT_PHASE_F		1
#define MOTHION_OPT_PHASE_R		2
#define MOTHION_OPT_WAIT_POS	3

#define MOTHION_OPT_AS_TYPE		0



// Control Type
#define CTR_TYPE_TIME_WAIT					1  //待機（時間経過待ち）
#define CTR_TYPE_SINGLE_PHASE_WAIT			2  //位相待ち１カ所(振れ止め用）
#define CTR_TYPE_DOUBLE_PHASE_WAIT			3  //位相待ち２カ所(振れ止め用）
#define CTR_TYPE_OTHER_POS_WAIT				4	//他軸位置到達待ち
#define CTR_TYPE_ADJUST_MOTION_TRIGGER		5	//動作起動調整(振れ止め移動用）

#define CTR_TYPE_CONST_V_TIME				10  //定速固定時間出力
#define CTR_TYPE_CONST_V_ACC_STEP			11  //定速出力 加速中
#define CTR_TYPE_CONST_V_DEC_STEP			12  //定速出力 減速中
#define CTR_TYPE_CONST_V_TOP_STEP			13  //定速出力 トップ速度
#define CTR_TYPE_FINE_POSITION				14	//微小位置合わせ

#define CTR_TYPE_ACC_TIME					20  //Specified time acceleration
#define CTR_TYPE_ACC_V						21  //Toward specified speed acceleration
#define CTR_TYPE_ACC_AS						23	//振れ止め加速
#define CTR_TYPE_DEC_TIME					30  //Specified time deceleration
#define CTR_TYPE_DEC_V						31  //Toward specified speed deceleration


#define PTN_CONFIRMATION_TIME				0.1		//パターン出力調整時間
#define PTN_FINE_POS_LIMIT_TIME				5.0		//微小位置合わせ制限時間
#define PTN_ERROR_CHECK_TIME				60		//異常検出時間

#define PTN_STEP_STANDBY					0
#define PTN_STEP_FIN						1
#define PTN_STEP_ON_GOING					2
#define PTN_STEP_ERROR						-1
#define PTN_STEP_TIME_OVER					-2

typedef struct stMotionElement {	//運動要素
	int type;				//制御種別
	int status;				//制御状態
	int time_count;			//予定継続時間のカウンタ変換値
	double _a;				//目標加減速度
	double _v;				//目標速度
	double _p;				//目標位置
	double _t;				//継続時間
	double opt_d[8];		//オプションdouble
	int opt_i[8];			//オプションint
}ST_MOTION_STEP, * LPST_MOTION_STEP;

/****************************************************************************/
/*   動作内容定義構造体（単軸）												*/
/* 　単軸の目標状態に移行する動作パターンを運動要素の組み合わせで実現します */
/****************************************************************************/
#define M_STEP_MAX	32

//Recipe
typedef struct stMotionRecipe {					//移動パターン
	DWORD motion_type;							//動作種別、
	int n_step;									//動作構成要素数
	DWORD opt_dw;								//オプション条件
	int time_limit;								//タイムオーバー判定値
	ST_MOTION_STEP steps[M_STEP_MAX];			//動作定義要素配列
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
	int status;									//動作実行状況
	int iAct;									//実行中要素配列index -1で完了
	int step_act_count;							//実行中要素の実行カウント数
	int elapsed;								//MOTION開始後経過時間
	int error_code;								//エラーコード　異常完了時
	int direction;								//動作方向
	int flg[MOTION_STAT_FLG_N];				//実行ステータスオプションフラグ
}ST_MOTION_STAT, * LPST_MOTION_STAT;

/********************************************************************************/
/*   軸連動運転内容(COMMAND)定義構造体                             　　　　　　 */
/* 　目的動作を実現する運転内容を単軸動作の組み合わせで実現します               */
/********************************************************************************/

typedef struct stCommandSet {
	//POLICY SET
	int id;									//コマンドID
	int type;								//コマンド種別
	int job_id;
	int job_step;
	bool is_required_motion[MOTION_ID_MAX];		//動作対象軸
	ST_MOTION_RECIPE recipe[MOTION_ID_MAX];

	//AGENT SET
	SYSTEMTIME time_start;					//開始時の時間 SYSTEMTIME
	SYSTEMTIME time_end;					//終了時の時間 SYSTEMTIME
	int com_status;							//実行状態　
	int status_code;						//エラーコード等（異常完了時）　
	ST_MOTION_STAT	motion_stat[MOTION_ID_MAX];

}ST_COMMAND_SET, * LPST_COMMAND_SET;


//# Policy タスクセット領域

#define MODE_PC_CTRL		0x00000001
#define MODE_ANTISWAY		0x00010000
#define MODE_RMOTE_PANEL	0x00000100

/****************************************************************************/
/*   Client Service	情報定義構造体                                   　   　*/
/* 　Client Serviceタスクがセットする共有メモリ上の情報　　　　　　　 　    */
/****************************************************************************/

#define JOB_HOLD_MAX			10					//	保持可能JOB最大数

typedef struct stCSInfo {

	int n_job_standby;
	int i_current_job;							//実行中のJOBインデックス  -1:実行無し
	ST_JOB_SET	job_list[JOB_HOLD_MAX];

	//for Client
	DWORD req_status;
	DWORD n_job_hold;							//未完JOB数

}ST_CS_INFO, * LPST_CS_INFO;

/****************************************************************************/
/*   Policy	情報定義構造体                                   　			  　*/
/* 　Policy	タスクがセットする共有メモリ上の情報　　　　　　　		 　		*/
/****************************************************************************/
typedef struct stPolicyInfo {

	int i_jobcom;							//現在のコマンドINDEX
	int i_com;								//現在のコマンドINDEX
	ST_COMMAND_SET job_com[COM_STEP_MAX];
	ST_COMMAND_SET com[COM_STEP_MAX];
	int auto_ctrl_ptn[MOTION_ID_MAX];

}ST_POLICY_INFO, * LPST_POLICY_INFO;

/****************************************************************************/
/*   Agent	情報定義構造体                                   　   　		*/
/* 　Agent	タスクがセットする共有メモリ上の情報　　　　　　　 　			*/
/****************************************************************************/

#define AUTO_TYPE_ANTI_SWAY	0x01
#define AUTO_TYPE_SEMI_AUTO	0x11
#define AUTO_TYPE_JOB		0x21
#define AUTO_TYPE_MANUAL	0x00

typedef struct stAgentInfo {

	WORD pc_ctrl_mode; //PCからの指令で動作させる軸の指定

	double v_ref[MOTION_ID_MAX];
	int PLC_PB_com[N_PLC_PB];
	int PLC_LAMP_com[N_PLC_LAMP];
	int PLC_LAMP_semiauto_com[SEMI_AUTO_TARGET_MAX];

	int auto_on_going;								//実行中の自動
	UCHAR auto_active[MOTION_ID_MAX];				//自動実行中(軸毎)
	double dist_for_stop[MOTION_ID_MAX];			//減速停止距離
	double pos_target[MOTION_ID_MAX];				//位置決め目標位置
	bool is_spdfb_0[MOTION_ID_MAX];					//振れ止め速度FB条件
	bool be_hold_target[MOTION_ID_MAX];				//目標位置キープフラグ
	double gap_from_target[MOTION_ID_MAX];			//目標位置からのずれ
	double gap2_from_target[MOTION_ID_MAX];			//目標位置からのずれ2乗
	double sway_amp2m[MOTION_ID_MAX];				//振れ振幅の2乗m2

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

//コマンド要求応答コード
#define REQ_ACCEPTED			0


//オペレーションコマンド
#define OPE_COM_PB_SET				1
#define OPE_COM_LAMP_ON				2
#define OPE_COM_LAMP_OFF			3
#define OPE_COM_LAMP_FLICKER		4
#define OPE_COM_SEMI_LAMP_ON		5
#define OPE_COM_SEMI_LAMP_OFF		6
#define OPE_COM_SEMI_LAMP_FLICKER	7


