#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"



#define AGENT_PB_OFF_DELAY          10                            //PB出力OFF DELAY COUNT
#define AGENT_LAMP_ON               PLC_IO_LAMP_FLICKER_CHANGE    //LAMP ON出力カウント値
#define AGENT_LAMP_OFF              0                             //LAMP OFF出力カウント値


class CAgent:public CTaskObj
{
public:
    CAgent();
    ~CAgent();

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
    DWORD auto_type;                 //ANTI_SWAY,SEMI_AUTO,JOB
    DWORD auto_ctrl_mode;            //STAND_BY,SUSPEND,ACTIVE

    int receipt_auto_com(int type, int id, int action);  //Job受付
    int receipt_ope_com(int type, int id);          //Operation Command受付

private:
    
    LPST_POLICY_INFO pPolicyInf;
    LPST_AGENT_INFO pAgentInf;
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;

    ST_AGENT_INFO   AgentInf_workbuf;

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void set_lamp();            //ランプの状態セット

    void output();              //出力データ更新
    int set_ref_mh();           //巻速度指令値出力
    int set_ref_gt();           //走行速度指令値出力
    int set_ref_slew();         //旋回速度指令値出力
    int set_ref_bh();           //引込速度指令値出力
    void update_pb_lamp_com();


    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

};

