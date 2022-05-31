#pragma once

#include "common_def.h"

#ifndef MOTION_ID_MAX

#define MOTION_ID_MAX   8  //制御軸最大数

#endif//

/*** 仕様定義構造体 ***/

#define NOTCH_MAX 6
#define NOTCH_0	  0
#define NOTCH_1	  1
#define NOTCH_2	  2
#define NOTCH_3	  3
#define NOTCH_4	  4
#define NOTCH_5	  5

#define DIRECTION_MAX 2
#define ACCDEC_MAX 2
#define FWD 0
#define REV 1
#define ACC 0
#define DEC 1

#define DEMENSION_MAX 64
#define ID_BOOM_HIGHT  0

#define POS_LIMIT_TYPE_MAX	4
#define END_LIMIT			0
#define SPD_LIMIT1			1
#define SPD_LIMIT2			2
#define SPD_LIMIT3			3

#define NUM_OF_POS_LIM_TYPE		5	//速度制限位置タイプ数
#define	END_STOP				0	//停止からの加速
#define REV_END_STOP			1	//加速中の減速切替時
#define FWD_ENDID_DELAY_DEC_ACC	2	//減速中の加速切替時
#define ID_DELAY_CNT_ACC		3	//定速からの加速時
#define ID_DELAY_CNT_DEC		4	//定速からの減速時


#define NUM_OF_AS_AXIS			5	//自動の制御軸数　走行、横行、旋回、スキュー
#define NUM_OF_SWAY_LEVEL		3	//完了,トリガ,制限
#define NUM_OF_POSITION_LEVEL	3	//完了,トリガ,制限

#define ID_AS_GNT				0	//走行振れ
#define ID_AS_TRY				1	//横行振れ
#define ID_AS_TH				2	//周振れ
#define ID_AS_R					3	//動径振れ
#define ID_AS_SKW				4	//スキュー振れ

#define ID_LV_COMPLE			0	//完了
#define ID_LV_TRIGGER			1	//トリガ
#define ID_LV_LIMIT				2	//制限

#define NUM_OF_DELAY_PTN		5	//加減速時FB時間遅れ評価パターン数
#define ID_DELAY_0START			0	//停止からの加速
#define ID_DELAY_ACC_DEC		1	//加速中の減速切替時
#define ID_DELAY_DEC_ACC		2	//減速中の加速切替時
#define ID_DELAY_CNT_ACC		3	//定速からの加速時
#define ID_DELAY_CNT_DEC		4	//定速からの減速時



typedef struct StSpec {
	//[ID_HOIST],[ID_GANTRY],[ID_TROLLY],[ID_BOOM_H],[ID_SLEW],[ID_OP_ROOM],[ID_H_ASSY],[ID_MOTION1]
	
	double notch_spd[MOTION_ID_MAX][NOTCH_MAX] = {			//# ノッチ指令速度（機上）
	{ 0.0,	0.083,	0.25,	0.417,	0.583,	1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	0.04,	0.125,	0.25,	0.416,	0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	0.1,	0.3,	0.5,	0.7,	1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	0.0157,	0.05,	0.0785,	0.11,	0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	0.25,	0.25,	0.25,	0.25,	0.25 },			//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};

	//[ID_HOIST],[ID_GANTRY],[ID_TROLLY],[ID_BOOM_H],[ID_SLEW],[ID_OP_ROOM],[ID_H_ASSY],[ID_MOTION1]

	double notch_spd_remote[MOTION_ID_MAX][NOTCH_MAX] = {	//# ノッチ指令速度（遠隔）
	{ 0.0,	0.083,	0.25,	0.417,	0.583,	1.666 },		//[ID_HOIST]	m/s
	{ 0.0,	0.04,	0.125,	0.25,	0.416,	0.416 },		//[ID_GANTRY]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_TROLLY]	m/s
	{ 0.0,	0.1,	0.3,	0.5,	0.7,	1.0 },			//[ID_BOOM_H]	m/s
	{ 0.0,	0.0157,	0.05,	0.0785,	0.11,	0.157 },		//[ID_SLEW]		rad/s;
	{ 0.0,	0.25,	0.25,	0.25,	0.25,	0.25 },			//[ID_OP_ROOM]	m/s
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_H_ASSY];
	{ 0.0,	0.0,	0.0,	0.0,	0.0,	0.0 },			//[ID_MOTION1];
	};

	double accdec[MOTION_ID_MAX][DIRECTION_MAX][ACCDEC_MAX] = {	//# 各動作加減速度
	{{ 0.387,	-0.387},	{0.387,		-0.387 }},				//[ID_HOIST]	m/s2
	{{ 0.0812,	-0.0812},	{0.0812,	-0.0812 }},				//[ID_GANTRY]	m/s2
	{{ 0.0,		0.0},		{0.0,		0.0 }},					//[ID_TROLLY]	m/s2
	{{ 0.333,	-0.333},	{	0.333,	-0.333 }},				//[ID_BOOM_H]	m/s2
	{{ 0.00875,	-0.00875},	{0.00875,	-0.00875 }},			//[ID_SLEW]		rad/s2;
	{{ 0.0125,	-0.0125},	{0.0125,	-0.0125 }},				//[ID_OP_ROOM]	m/s2
	{{ 0.0,	0.0},			{0.0,	0.0 }},						//[ID_H_ASSY];
	{{ 0.0,	0.0},			{0.0,	0.0 }},						//[ID_MOTION1];
	};

	double d[DEMENSION_MAX] = {								//# 各種寸法
		25.0												// ID_BOOM_HIGHT m
	};

	double pos_limit[MOTION_ID_MAX][DIRECTION_MAX][POS_LIMIT_TYPE_MAX] = {//# 極限寸法								//# 各種寸法
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_HOIST]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_GANTRY]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_TROLLY]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_BOOM_H]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_SLEW]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_OP_ROOM]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_H_ASSY]
	{{ 0.0,0.0,0.0,0.0},	{ 0.0,0.0,0.0,0.0}},				//[ID_MOTION1]
	};

	double as_rad_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ角レベル(rad)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.003, 0.006, 0.020 },									//[ID_GANTRY]
	{ 0.003, 0.006, 0.020 },									//[ID_TROLLY]
	{ 0.003, 0.006, 0.020 },									//[ID_BOOM_H]
	{ 0.003, 0.006, 0.020 },									//[ID_SLEW]	
	};
	double as_rad2_level[NUM_OF_AS_AXIS][NUM_OF_SWAY_LEVEL] = {	//# 振れ止め判定　振れ振幅レベル(rad^2)
	{ 0.0,	0.0, 0.0},											//[ID_HOIST]
	{ 0.000009, 0.000036, 0.0004 },								//[ID_GANTRY]
	{ 0.000009, 0.000036, 0.0004 },								//[ID_TROLLY]
	{ 0.000009, 0.000036, 0.0004 },								//[ID_BOOM_H]
	{ 0.000009, 0.000036, 0.0004 },								//[ID_SLEW]
	};
	double as_pos_level[NUM_OF_AS_AXIS][NUM_OF_POSITION_LEVEL] = {	//# 位置決め判定　位置ずれレベル(m) 
	{ 0.0,	0.0, 0.0},												//[ID_HOIST]
	{ 0.003, 0.006, 0.020 },										//[ID_GANTRY]
	{ 0.003, 0.006, 0.020 },										//[ID_TROLLY]
	{ 0.003, 0.006, 0.020 },										//[ID_BOOM_H]
	{ 0.003, 0.006, 0.020 },										//[ID_SLEW]
	};

	double delay_time[NUM_OF_AS_AXIS][NUM_OF_DELAY_PTN] = {		// 加減速時のFB一時遅れ時定数
	{ 0.1,0.1,0.1,0.1,0.1},											//[ID_HOIST]
	{ 0.1,0.1,0.1,0.1,0.1 },										//[ID_GANTRY]
	{ 0.1,0.1,0.1,0.1,0.1 },										//[ID_TROLLY]
	{ 0.1,0.1,0.1,0.1,0.1 },										//[ID_BOOM_H]
	{ 0.1,0.1,0.1,0.1,0.1 },										//[ID_SLEW]
	};
}ST_SPEC, * LPST_SPEC;



