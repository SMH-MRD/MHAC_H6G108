#pragma once
#include <windows.h>

/*** 物理定数、係数 ***/
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

/*** 配列参照用　動作インデックス ***/
#define MOTION_ID_MAX   8  //制御軸最大数

#define ID_HOIST        0   //巻 　       ID
#define ID_GANTRY       1   //走行        ID
#define ID_TROLLY       2   //横行        ID
#define ID_BOOM_H       3   //引込        ID
#define ID_SLEW         4   //旋回        ID
#define ID_OP_ROOM      5   //運転室移動　ID
#define ID_H_ASSY       6   //吊具        ID
#define ID_MOTION1      7   //予備        ID

/*** 配列参照用　方向インデックス ***/
#define ID_UP         0   //左側
#define ID_DOWN       1   //右側
#define ID_FWD        0   //前進
#define ID_REV        1   //後進
#define ID_LEFT       0   //左側
#define ID_RIGHT      1   //右側
#define ID_ACC        0   //加速
#define ID_DEC        1   //減速


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

