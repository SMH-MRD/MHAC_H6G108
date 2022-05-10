#pragma once
#include <windows.h>

/*** 一般 ***/
#define ID_UP         0   //左側
#define ID_DOWN       1   //右側
#define ID_FWD        0   //前進
#define ID_REV        1   //後進
#define ID_LEFT       0   //左側
#define ID_RIGHT      1   //右側
#define ID_ACC        0   //加速
#define ID_DEC        1   //減速

//配列参照用動作ID
#define MOTION_ID_MAX   8  //制御軸最大数

#define ID_HOIST        0   //巻 　       ID
#define ID_GANTRY       1   //走行        ID
#define ID_TROLLY       2   //横行        ID
#define ID_BOOM_H       3   //引込        ID
#define ID_SLEW         4   //旋回        ID
#define ID_OP_ROOM      5   //運転室移動　ID
#define ID_H_ASSY       6   //吊具        ID
#define ID_MOTION1      7   //予備        ID

/*** 物理定数、係数 ***/
#ifndef PHSICS_CONST

#define GA				9.80665     //重力加速度

#define PI360           6.2832      //2π
#define PI330           5.7596   
#define PI315           5.4978
#define PI300           5.2360 
#define PI270           4.7124      
#define PI180           3.1416      //π
#define PI90            1.5708
#define PI60            1.0472
#define PI45            0.7854
#define PI30            0.5236

#define RAD2DEG         57.29578
#define DEG2RAD         0.0174533
#endif // !PHSICS_CONST

/*** 共有メモリ ***/
//PLC IO
#define PLC_PB_MAX              64 //運転操作ボタン登録最大数
#define PLC_LAMP_MAX            64 //運転操作ボタン登録最大数
#define PLC_CTRL_MAX            64 //運転操作ボタン登録最大数

#ifndef PLC_PBL_ID

#define PID_E_STOP                   0
#define PID_CONTROL_SOURCE1_ON       1
#define PID_CONTROL_SOURCE1_OFF      2
#define PID_CONTROL_SOURCE2_ON       3
#define PID_CONTROL_SOURCE2_OFF      4
#define PID_FAULT_RESET              5
#define PID_IL_BYPASS                6
#define PID_FOOK_L                   7
#define PID_FOOK_R                   8
#define PID_TARGET1_MEM              9
#define PID_TARGET2_MEM              10
#define PID_TARGET3_MEM              11
#define PID_TARGET4_MEM              12
#define PID_MODE_OP_ROOM             13
#define PID_MODE_TELECON             14
#define PID_ANTSWAY_ON               15
#define PID_ANTSWAY_OFF              16
#define PID_AUTO_START               17
#define PID_CONTROL_SOURCE           18
#define PID_RAIL_CRAMP_ACTIVE        19
#define PID_RAIL_CRAMP_DEACTIVE      20
#define PID_TTB_ACTIVE               21
#define PID_SLEW_GRACE_ACTIVE        22
#define PID_GANTRY_GRACE_ACTIVE      23
#define PID_SLEW_PUMP_ACTIVE         24
#define PID_COIL_LIFTER              25
#define PID_HOIST_ASSY_RELEASE       26
#define PID_FAULT                    27
#define PID_ALARM                    28
#define PID_OP_TRY_SOURCE_ON         29
#define PID_OP_TRY_SOURCE_OFF        30
#define PID_LIFT_MAGNET_SELECT       31
#define PID_TEMP_HOLD_ON             32
#define PID_SAFE_SW_SAFE             33
#define PID_SAFE_SW_DANJER           34
#define PID_BORDING_CAUTION          35
#define PID_COIL_LIFT_MC3            36
#define PID_MODE_OP_REMOTE           37

#define ID_HOIST_BRK				 1   //巻 　       
#define ID_GANTRY_BRK			     2   //走行        
#define ID_TROLLY_BRK				 3   //横行        
#define ID_BOOM_H_BRK				 4   //引込        
#define ID_SLEW_BRK					 5   //旋回        
#define ID_OP_ROOM_BRK				 6   //運転室移動　
#endif // !PLC_PBL_ID

//EXEC_STATUS
#ifndef AGENT_DO_ID

#define AGENT_DO_MAX            64 
#define AID_CONTROL_SOURCE1          1
#define AID_CONTROL_SOURCE2          2
#define AID_E_STOP                   3
#define AID_FAULT_RESET              4
#define AID_IL_BYPASS                5
#define AID_FOOK_L                   6
#define AID_FOOK_R                   7
#define AID_TARGET1_MEM              8
#define AID_TARGET2_MEM              9
#define AID_TARGET3_MEM3             10
#define AID_TARGET4_MEM4             11
#define AID_TARGET1_SELECT           12 
#define AID_TARGET2_SELECT           13
#define AID_TARGET3_SELECT           14
#define AID_TARGET4_SELECT           15
#define AID_ANTISWAY_ON              16
#define AID_AUTO_ACTIVE              17
#define AID_AUTO_SUSPEND             18
#define AID_AUTO_PROHIBIT            19
#endif // !AGENT_DO_ID

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


/*** MODE ***/
//シミュレーション
#define IO_PRODUCTION                   0x0000//実機
#define USE_CRANE_SIM                   0x1000//クレーン物理シミュレータの出力をFB値に適用する
#define USE_PLC_SIM_COMMAND				0x0100//機上操作入力をPLCシミュレータの出力値を使う
#define USE_REMOTE_SIM_COMMAND          0x0010//遠隔操作入力にリモートシミュレータの出力値を使う
#define USE_SWAY_CRANE_SIM		        0x0001//振れセンサの信号をクレーン物理シミュレータの出力から生成する

/*** 自動制御 ***/
//振れ止めパターン
#define AS_PTN_1P           1   //1パルス
#define AS_PTN_2P           2   //2パルス
#define AS_PTN_TR           3   //1台形動作
#define AS_PTN_3TR          4   //3台形動作
#define AS_PTN_TRTR         5   //台形＋台形動作(2段加減速）


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
/* 　目的の運転内容を定義します										            */
/********************************************************************************/
#define JOB_STEP_MAX		5//　JOBを構成するコマンド最大数

typedef struct stCommandSet {	//作業要素（PICK、GROUND、PARK　....）
	int type;							//コマンド種別（PICK、GROUND、PARK）
	axis_check is_valid_axis;			//対象動作軸
	double target_pos[MOTION_ID_MAX];	//各軸目標位置
	int option[MOTION_ID_MAX];		//各軸動作オプション条件
}ST_COMMAND_SET, * LPST_COMMAND_SET;

typedef struct _stJobRecipe {	//作業構成要素（保管,払出,退避移動等）
	int type;								//JOB種別
	int job_id;								//job識別コード
	int n_job_step;							//構成コマンド数
	ST_COMMAND_SET commands[JOB_STEP_MAX];	//JOB構成コマンド
}ST_JOB_RECIPE, * LPST_JOB_RECIPE;

/****************************************************************************/
/*   作業（JOB)実行管理構造体                                                 */
/* 　コマンド実行状態管理用構造体									　		*/
/****************************************************************************/
typedef struct stJOB_STAT {				//運転要素
	int type;								//JOB種別
	int job_id;								//job識別コード
	int n_job_step;							//構成コマンド数
	int job_step_now;						//実行中ステップ
	int status;								//job実行状況
	int elapsed;							//経過時間
	int error_code;							//エラーコード　異常完了時
	LPST_COMMAND_STAT p_command_stat[JOB_STEP_MAX];		//各コマンドステータス構造体のアドレス
}ST_JOB_STAT, * LPST_JOB_STAT;