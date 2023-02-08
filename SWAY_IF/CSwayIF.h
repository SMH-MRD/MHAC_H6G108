#pragma once

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <ws2tcpip.h>
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# 共有メモリクラス
#include "Spec.h"
#include "Swaysensor.h"

#include <commctrl.h>
#include <time.h>
#include <string>

#define SWAY_IF_SWAY_IO_MEM_NG              0x8000
#define SWAY_IF_CRANE_MEM_NG                0x4000
#define SWAY_IF_SIM_MEM_NG                  0x2000

#define ID_STATIC_SWAY_IF_LABEL_RCV         10502
#define ID_STATIC_SWAY_IF_LABEL_SND         10503
#define ID_STATIC_SWAY_IF_VIEW_RCV          10504
#define ID_STATIC_SWAY_IF_VIEW_SND          10505

#define ID_UDP_EVENT				        10506

#define ID_STATIC_SWAY_IF_DISP_SELBUF       10507
#define ID_PB_SWAY_IF_CHG_DISP_SENSOR       10508
#define ID_PB_SWAY_IF_CHG_DISP_BUF         10509
#define ID_PB_SWAY_IF_CHG_DISP_CAM         10510

//起動タイマーID
#define ID_WORK_WND_TIMER					100
#define WORK_SCAN_TIME						500			// SWAY IF送信チェック周期msec


#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

#define N_SWAY_SENSOR_RCV_BUF   4  //受信データのバッファ数
#define N_SWAY_SENSOR_SND_BUF   4  //送信データのバッファ数

#define WORK_WND_X							1050		//メンテパネル表示位置X
#define WORK_WND_Y							394			//メンテパネル表示位置Y
#define WORK_WND_W							400		    //メンテパネルWINDOW幅
#define WORK_WND_H							400			//メンテパネルWINDOW高さ

#define ID_SWAYIF_REQ_CONST_DATA            0x0001      //定周期通常データ
#define ID_SWAYIF_REQ_ONE_SHOT              0x0002      //ワンショットデータ
#define ID_SWAYIF_REQ_IMG                   0x0003      //画像データ


class CSwayIF :
    public CBasicControl
{
private:

    //# 出力用共有メモリオブジェクトポインタ:
    CSharedMem* pSwayIOObj;
    //# 入力用共有メモリオブジェクトポインタ:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;

    static ST_SWAY_RCV_MSG rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
    static ST_SWAY_SND_MSG snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
    static int i_rcv_msg[N_SWAY_SENSOR];
    static int i_snd_msg[N_SWAY_SENSOR];

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //共有メモリへの出力セット作業用バッファ
    double SwayCamParam[N_SWAY_SENSOR][N_SWAY_SENSOR_CAMERA][SWAY_SENSOR_N_AXIS][CAM_SET_PARAM_N_PARAM]; //振れセンサカメラ設置パラメータ[カメラNo.][軸方向XY][a,b]

    int parse_sway_stat(int SensorID, int CameraID);
    HINSTANCE hInst;

    static void tweet2statusMSG(const std::wstring& srcw);
    static void tweet2rcvMSG(const std::wstring& srcw);
    static void tweet2sndMSG(const std::wstring& srcw);
    static void tweet2infMSG(const std::wstring& srcw);
 

public:
    CSwayIF();
    ~CSwayIF();

    static HWND hWorkWnd;
    WORD helthy_cnt = 0;

    //オーバーライド
    int set_outbuf(LPVOID); //出力バッファセット
    int init_proc();        //初期化処理
    int input();            //入力処理
    int parse();            //メイン処理
    int output();           //出力処理

    static int set_send_data(int com_id);   //送信処理はタイマー起動

    void set_debug_mode(int id) {
        if (id) mode |= SWAY_IF_SIM_DBG_MODE;
        else    mode &= ~SWAY_IF_SIM_DBG_MODE;
    }

 
    int is_debug_mode() { return(mode & SWAY_IF_SIM_DBG_MODE); }

     //追加メソッド
     int set_sim_status(LPST_SWAY_IO pworkbuf);   //デバッグモード時にSimulatorからの入力で出力内容を上書き

     virtual HWND open_WorkWnd(HWND hwnd_parent);
     static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
     static int close_WorkWnd();
     static int init_sock(HWND hwnd);

private:
    //振れセンサ通信表示用 

     static HWND hwndSTATMSG;
     static HWND hwndRCVMSG;
     static HWND hwndSNDMSG;
     static HWND hwndINFMSG;
 
     static HWND hwndDispBufMSG;
     static HWND hwndCamChangePB;
     static HWND hwndBufChangePB;
     static int iDispSensor;
     static int iDispBuf;
     static int iDispCam;

};

