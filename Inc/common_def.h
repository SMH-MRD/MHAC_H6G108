#pragma once
#include <windows.h>

#define _DVELOPMENT_MODE                //開発環境を有効にする

///#  共通フラグ
#define     L_ON                        0x01  // ON
#define     L_OFF                       0x00  // OFF

///# 共通マクロ
#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

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
#define PI1DEG          0.017453

#define RAD2DEG         57.29578
#define DEG2RAD         0.0174533

/*** マクロ ***/
#ifndef MIN
#  define MIN(a,b)  ((a) > (b) ? (b) : (a))
#endif

#ifndef MAX
#  define MAX(a,b)  ((a) < (b) ? (b) : (a))
#endif

#ifndef iABS
#  define iABS(a)  (a < 0 ? -a : a)
#endif

#ifndef fABS
#  define fABS(a)  (a < 0.0 ? -a : a)
#endif

#ifndef dABS
#  define dABS(a)  (a < 0.0 ? -a : a)
#endif

/*** 配列参照用　動作インデックス ***/
#define MOTION_ID_MAX   8  //制御軸最大数

#define ID_HOIST        0   //巻 　       ID
#define ID_GANTRY       1   //走行        ID
#define ID_TROLLY       2   //横行        ID
#define ID_BOOM_H       3   //引込        ID
#define ID_SLEW         4   //旋回        ID
#define ID_OP_ROOM      5   //運転室移動　ID
#define ID_H_ASSY       6   //吊具        ID
#define ID_COMMON       7   //共通        ID

/*** 配列参照用　方向インデックス ***/
#define ID_UP           0   //左側
#define ID_DOWN         1   //右側
#define ID_FWD          0   //前進
#define ID_REV          1   //後進
#define ID_LEFT         0   //左側
#define ID_RIGHT        1   //右側
#define ID_ACC          0   //加速
#define ID_DEC          1   //減速
#define SID_X           0   // X方向
#define SID_Y           1   // Y方向
#define SID_R			2   // 半径方向
#define SID_T           3   // 接線方向

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

class CBasicControl //基本制御クラス
{
public:
    LPVOID poutput = NULL;      //結果出力メモリ
    size_t out_size = 0;        //出力バッファのサイズ
    DWORD  mode;                //結果出力モード
    DWORD  source_counter;      //メインプロセスのヘルシーカウンタ
    DWORD  my_helthy_counter=0; //自スレッドのヘルシーカウンタ

    CBasicControl() { mode = source_counter = 0; }
    ~CBasicControl() {}
    virtual int set_outbuf(LPVOID)=0;           //出力バッファセット
    virtual int init_proc() = 0;                //初期化処理
    virtual int input() = 0;                    //入力処理
    virtual int parse() = 0;                     //メイン処理
    virtual int output() = 0;                   //出力処理
};
