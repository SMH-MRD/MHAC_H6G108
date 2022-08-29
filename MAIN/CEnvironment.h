#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PLC_IO_HELTHY_NG_COUNT      8
#define SIM_HELTHY_NG_COUNT         8
#define SWAY_HELTHY_NG_COUNT        8



class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();



    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    
    void routine_work(void* param);
    int update_semiauto_target(int ID);


private:
    ST_SPEC spec;       //仕様情報 Environmentが共有メモリにセットする。
    ST_CRANE_STATUS stWorkCraneStat;

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;
    LPST_REMOTE_IO pRemoteIO;
    LPST_SIMULATION_STATUS pSimStat;
    LPST_CS_INFO pCSInf;
    LPST_POLICY_INFO pPolicyInf;
    LPST_AGENT_INFO pAgentInf;

 
    void input();                   //外部データ取り込み
    void main_proc();               //処理内容
    void output();                  //出力データ更新

    int parse_notch_com();          //ノッチ信号を速度指令に変換セット
    int mode_set();                 //モード状態セット
    int parse_sway_stat(int ID);    //振れ状態計算
    int pos_set();                  //位置情報セット
    void chk_subproc();             //サブプロセス状態チェック

    //メインパネルのTweetテキストを設定
    void tweet_update();
                                
    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();
    
};

