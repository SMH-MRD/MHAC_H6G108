#pragma once

#include "common_def.h"

#ifndef MOTION_ID_MAX

#define MOTION_ID_MAX   8  //制御軸最大数

#endif//

/*** 仕様定義構造体 ***/

#define DIRECTION_MAX 2
#define ACCDEC_MAX 2
#define FWD 0
#define REV 1
#define ACC 0
#define DEC 1

#define DEMENSION_MAX 64
#define ID_BOOM_HIGHT  0

#define NUM_OF_AS_AXIS			5	//自動の制御軸数　走行、横行、旋回、スキュー
#define NUM_OF_SWAY_LEVEL		3	//完了,トリガ,制限
#define NUM_OF_POSITION_LEVEL	3	//完了,トリガ,制限

typedef struct StSpec {
	//[ID_HOIST],[ID_GANTRY],[ID_TROLLY],[ID_BOOM_H],[ID_SLEW],[ID_OP_ROOM],[ID_H_ASSY],[ID_MOTION1]


#define NOTCH_MAX 6
#define NOTCH_0	  0
#define NOTCH_1	  1
#define NOTCH_2	  2
#define NOTCH_3	  3
#define NOTCH_4	  4
#define NOTCH_5	  5

	double notch_spd_f[MOTION_ID_MAX][NOTCH_MAX] = {			//# ノッチ指令速度（機上）
	{ 0.0,	0.083,	0.25,	0.417,	0.583,	1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	0.04,	0.125,	0.25,	0.416,	0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	0.1,	0.3,	0.5,	0.7,	1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	0.0157,	0.05,	0.0785,	0.11,	0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	0.25,	0.25,	0.25,	0.25,	0.25 },			//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};
	double notch_spd_r[MOTION_ID_MAX][NOTCH_MAX] = {			//# ノッチ指令速度（機上）
	{ 0.0,	-0.083,	-0.25,	-0.417,	-0.583,	-1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	-0.04,	-0.125,	-0.25,	-0.416,	-0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	-0.1,	-0.3,	-0.5,	-0.7,	-1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	-0.0157,	-0.05,	-0.0785,-0.11,-0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	-0.25,	-0.25,	-0.25,	-0.25,	-0.25 },		//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};

	//[ID_HOIST],[ID_GANTRY],[ID_TROLLY],[ID_BOOM_H],[ID_SLEW],[ID_OP_ROOM],[ID_H_ASSY],[ID_MOTION1]

	double notch_spd_remote_f[MOTION_ID_MAX][NOTCH_MAX] = {	//# ノッチ指令速度（遠隔）
	{ 0.0,	0.083,	0.25,	0.417,	0.583,	1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	0.04,	0.125,	0.25,	0.416,	0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	0.1,	0.3,	0.5,	0.7,	1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	0.0157,	0.05,	0.0785,	0.11,	0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	0.25,	0.25,	0.25,	0.25,	0.25 },			//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};
	double notch_spd_remote_r[MOTION_ID_MAX][NOTCH_MAX] = {	//# ノッチ指令速度（遠隔）
	{ 0.0,	-0.083,	-0.25,	-0.417,	-0.583,	-1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	-0.04,	-0.125,	-0.25,	-0.416,	-0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	-0.1,	-0.3,	-0.5,	-0.7,	-1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	-0.0157,-0.05,	-0.0785,-0.11,	-0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	-0.25,	-0.25, -0.25, -0.25, -0.25 },			//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};
	double accdec[MOTION_ID_MAX][DIRECTION_MAX][ACCDEC_MAX] = {	//# 各動作加減速度
	{{ 0.387,	-0.387},	{-0.387,		0.387 }},				//[ID_HOIST]	m/s2
	{{ 0.0812,	-0.0812},	{-0.0812,	0.0812 }},				//[ID_GANTRY]	m/s2
	{{ 1.0,		-1.0},		{-1.0,		1.0 }},					//[ID_TROLLY]	m/s2
	{{ 0.333,	-0.333},	{	-0.333,	0.333 }},				//[ID_BOOM_H]	m/s2
	{{ 0.01301,	-0.01301},	{-0.01301,	0.01301 }},			//[ID_SLEW]		rad/s2;
	{{ 0.0125,	-0.0125},	{-0.0125,	0.0125 }},				//[ID_OP_ROOM]	m/s2
	{{ 0.0,	0.0},			{0.0,	0.0 }},						//[ID_H_ASSY];
	{{ 0.0,	0.0},			{0.0,	0.0 }},						//[ID_MOTION1];
	};

#define POS_LIMIT_TYPE_MAX	4
#define END_LIMIT			0
#define SPD_LIMIT1			1
#define SPD_LIMIT2			2
#define SPD_LIMIT3			3
	
	double pos_limit[MOTION_ID_MAX][DIRECTION_MAX][POS_LIMIT_TYPE_MAX] = {//# 極限寸法								//# 各種寸法
	{{ 14.5,0.0,0.0,0.0},	{ -11.0,0.0,0.0,0.0}},				//[ID_HOIST]
	{{ 80.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_GANTRY]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_TROLLY]
	{{ 27.0,0.0,0.0,0.0},	{ 8.0,0.0,0.0,0.0}},				//[ID_BOOM_H]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_SLEW]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_OP_ROOM]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_H_ASSY]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_MOTION1]
	};

#define ID_LV_COMPLE			0	//完了
#define ID_LV_TRIGGER			1	//トリガ
#define ID_LV_LIMIT				2	//制限
	double as_rad_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ角レベル(rad)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.005, 0.01, 0.020 },									//[ID_GANTRY]
	{ 0.005, 0.01, 0.020 },									//[ID_TROLLY]
	{ 0.005, 0.01, 0.020 },									//[ID_BOOM_H]
	{ 0.005, 0.01, 0.020 },									//[ID_SLEW]	
	};
	double as_rad2_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ振幅レベル(rad^2)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.000025, 0.0001, 0.0004 },								//[ID_GANTRY]
	{ 0.000025, 0.0001, 0.0004 },								//[ID_TROLLY]
	{ 0.000025, 0.0001, 0.0004 },								//[ID_BOOM_H]
	{ 0.000025, 0.0001, 0.0004 },								//[ID_SLEW]
	};
	double as_m_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ角レベル(m)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.1, 0.2, 2.00 },										//[ID_GANTRY]
	{ 0.1, 0.2, 2.00 },										//[ID_TROLLY]
	{ 0.1, 0.2, 2.00 },										//[ID_BOOM_H]
	{ 0.1, 0.2, 2.00 },										//[ID_SLEW]	
	};
	double as_m2_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ振幅レベル(m^2)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.01, 0.04, 4.0 },										//[ID_GANTRY]
	{ 0.01, 0.04, 4.0 },										//[ID_TROLLY]
	{ 0.01, 0.04, 4.0 },										//[ID_BOOM_H]
	{ 0.01, 0.04, 4.0 },										//[ID_SLEW]
	};
	double as_pos_level[NUM_OF_AS_AXIS][NUM_OF_POSITION_LEVEL] = {	//# 位置決め判定　位置ずれレベル(m) 
	{ 0.03, 0.06, 0.020 },										//m[ID_HOIST]
	{ 0.03, 0.06, 0.020 },										//m[ID_GANTRY]
	{ 0.03, 0.06, 0.020 },										//m[ID_TROLLY]
	{ 0.03, 0.06, 0.020 },										//m[ID_BOOM_H]
	{ 0.005, 0.01, 0.1 }										//rad[ID_SLEW]
	};
	double as_pos2_level[NUM_OF_AS_AXIS][NUM_OF_POSITION_LEVEL] = {	//# 位置決め判定　位置ずれレベル(m) 
	{ 0.009, 0.036, 0.020 },										//m[ID_HOIST]
	{ 0.009, 0.036, 0.020 },										//m[ID_GANTRY]
	{ 0.009, 0.036, 0.020 },										//m[ID_TROLLY]
	{ 0.009, 0.036, 0.020 },										//m[ID_BOOM_H]
	{ 0.000025, 0.0001, 0.01 }										//rad[ID_SLEW]
	};

#define NUM_OF_DELAY_PTN		5	//加減速時FB時間遅れ評価パターン数
#define ID_DELAY_0START			0	//停止からの加速
#define ID_DELAY_ACC_DEC		1	//加速中の減速切替時
#define ID_DELAY_DEC_ACC		2	//減速中の加速切替時
#define ID_DELAY_CNT_ACC		3	//定速からの加速時
#define ID_DELAY_CNT_DEC		4	//定速からの減速時	

	double delay_time[NUM_OF_AS_AXIS][NUM_OF_DELAY_PTN] = {		// 加減速時のFB一時遅れ時定数
	{ 0.3,0.3,0.3,0.3,0.3},											//[ID_HOIST]
	{ 0.3,0.3,0.3,0.3,0.3 },										//[ID_GANTRY]
	{ 0.3,0.3,0.3,0.3,0.3 },										//[ID_TROLLY]
	{ 0.3,0.3,0.3,0.3,0.3 },										//[ID_BOOM_H]
	{ 0.3,0.3,0.3,0.3,0.3 },										//[ID_SLEW]
	};
	
	//構造寸法
	double boom_high = 25.0, wheel_span = 20.0, leg_span = 15.0;// ブーム高さ,ホイルスパン,脚間スパン
	//極限
	double hoist_pos_min = -10.0, hoist_pos_max = 20.0, boom_pos_min = 5.0, boom_pos_max = 28.0, gantry_pos_min = 0.0, gantry_pos_max = 300.0;
	//吊具荷重
	double m_loard0 = 1000;	//吊具質量kg
	//振れセンサ



#define N_SWAY_SENSOR			3		//振れセンサの数
#define SID_SENSOR1				0		//No.1振れセンサ
#define SID_SENSOR2				1		//No.2振れセンサ
#define SID_SIM					2		//シミュレータ

#define SWAY_SENSOR_N_AXIS		2
#define SID_AXIS_X				0		//X軸
#define SID_AXIS_Y				1		//y軸

#define N_SWAY_SENSOR_CAMERA    2		//振れセンサ１セットのカメラの数
#define SID_CAMERA1				0		//No.1振れセンサ
#define SID_CAMERA2				1		//No.2振れセンサ

#define N_SWAY_SENSOR_TARGET    2		//振れセンサカメラ1台あたりのターゲット数
#define SID_TARGET1				0		//No.1振れセンサ
#define SID_TARGET2				1		//No.2振れセンサ

#define SWAY_CAMERA_N_PARAM		6
#define SID_L0					0		//カメラ回転軸距離
#define SID_PH0					1		//カメラ回転軸角度
#define SID_l0					2		//カメラ中心とハウジング回転軸間距離
#define	SID_ph0					3		//カメラのハウジングへの取り付け角度
#define	SID_phc					4		//カメラのハウジングへの取り付け角度
#define	SID_PIXlRAD				5		//カメラのハウジングへの取り付け角度


#define CTRL_PC_IP_ADDR			"192.168.1.31"
#define SWAY_SENSOR_IP_ADDR		"192.168.1.102"
#define CTRL_PC_IP_ADDR_REMOTE	"192.168.1.31"


#define SWAY_IF_IP_SWAY_PORT_C				10080
#define SWAY_IF_IP_SWAY_PORT_S				10081
#define SWAY_IF_IP_REMOTE_PANEL_PORT_C		10050
#define SWAY_IF_IP_REMOTE_PANEL_PORT_S		10051


	double SwayCamParam[N_SWAY_SENSOR][N_SWAY_SENSOR_CAMERA][SWAY_SENSOR_N_AXIS][SWAY_CAMERA_N_PARAM] = {	//振れセンサ　パラメータ
		{//No.1 センサ
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0,0.0,0.00,0.00,2800.0}},	//カメラ1 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0174,0.2,0.01,0.01,2800.0}},	//カメラ2 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		},
		{//No.2 センサ
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0174,0.2,0.01,0.01,2800.0}},	//カメラ1 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0174,0.2,0.01,0.01,2800.0}},	//カメラ2 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		},
		{//No.3 センサ
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0174,0.2,0.01,0.01,2800.0}},	//カメラ1 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		{{1.0,0.0174,0.2,0.01,0.01,2800.0},{1.0,0.0174,0.2,0.01,0.01,2800.0}},	//カメラ2 x,y方向 L0 m,PH rad,l0 m,ph0 rad, phc rad,pix/rad
		}
	};

#define SEMI_AUTO_TARGET_MAX	8	// 半自動目標ホールド数

	double semi_target[SEMI_AUTO_TARGET_MAX][MOTION_ID_MAX]{//半自動デフォルト目標位置
	//	HST,	GNT,	TRY,	BH,		SLW,	OPR,	ASSY,	COM
		{10.0,	0.0,	0.0,	8.0,	0.0,	0.0,	0.0,	0.0},
		{5.0,	0.0,	0.0,	25.0,	0.0,	0.0,	0.0,	0.0},
		{18.0,	0.0,	0.0,	25.0,	PI180,	0.0,	0.0,	0.0},
		{20.0,	0.0,	0.0,	25.0,	-PI90,	0.0,	0.0,	0.0},
		{3.0,	0.0,	0.0,	5.0,	-PI90,	0.0,	0.0,	0.0},
		{15.0,	0.0,	0.0,	18.0,	-PI60,	0.0,	0.0,	0.0},
		{-3.0,	0.0,	0.0,	10.0,	-PI90,	0.0,	0.0,	0.0},
		{-5.0,	0.0,	0.0,	15.0,	-PI150,	0.0,	0.0,	0.0}
	};
}ST_SPEC, * LPST_SPEC;



