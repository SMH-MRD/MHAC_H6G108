#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define AUTO_DEACTIVATE         0x0000
#define AUTO_ANTI_SWAY          0x0001
#define AUTO_SEMI_AUTO          0x0010
#define AUTO_JOB_COMMAND        0x0100

#define AUTO_NOT_APPLICABLE     0x0000
#define AUTO_STANDBY            0x0001
#define AUTO_SUSPEND            0x0010
#define AUTO_ACTIVE             0x0100

#define AUTO_TO_DO_START        0x00000001
#define AUTO_TO_DO_INTERRUPT    0x00000100
#define AUTO_TO_DO_ABORT        0x00100000


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

private:
    
    LPST_POLICY_INFO pPolicyInf;
    LPST_AGENT_INFO pAgentInf;
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void output();              //出力データ更新

    int set_ref_mh();           //巻速度指令値出力
    int set_ref_gt();           //走行速度指令値出力
    int set_ref_slew();         //旋回速度指令値出力
    int set_ref_bh();           //引込速度指令値出力
    int set_ref_pbs();          //操作PB指令値出力

    int receipt_job(LPST_JOB_SET pjob, int to_do);  //Job受付
    int receipt_com(int to_do);                     //Command受付


    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

};

