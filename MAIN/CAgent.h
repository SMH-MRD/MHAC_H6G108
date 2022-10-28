#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"



#define AGENT_PB_OFF_DELAY          40                            //PB出力OFF DELAY COUNT
#define AGENT_LAMP_ON               39    //LAMP ON出力カウント値
#define AGENT_LAMP_OFF              0                             //LAMP OFF出力カウント値

#define PHASE_CHECK_RANGE           0.02  // レシピ出力　位相到達判定範囲　rad :1deg = 0.017 rad
#define AGENT_FWD                   1
#define AGENT_REW                   -1
#define AGENT_STOP                  0
#define AGENT_NA                    0

#define AGENT_AUTO_TRIG_ACK_COUNT   10
#define AGENT_CHECK_LARGE_SWAY_m2   1.0     //起動時に初期振れ大とみなす振れ量mの2乗

class CAgent:public CTaskObj
{
public:
    CAgent();
    ~CAgent();

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
    DWORD auto_type;                                //ANTI_SWAY,SEMI_AUTO,JOB
    DWORD auto_ctrl_mode;                           //STAND_BY,SUSPEND,ACTIVE
      
  private:
    
    LPST_POLICY_INFO pPolicyInf;
    LPST_CS_INFO pCSInf;
    LPST_AGENT_INFO pAgentInf;
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;

    ST_AGENT_INFO   AgentInf_workbuf;

    bool can_auto_trigger();
    bool can_auto_complete();
    int cleanup_command(LPST_COMMAND_SET pcom);

    LPST_COMMAND_SET pCom;

    double ph_chk_range[NUM_OF_AS_AXIS];

    int dbg_mont[8];//デバッグ用


     void input();                                           //外部データ取り込み
    void main_proc();                                       //処理内容
 
    void output();                                          //出力データ更新
    int set_pc_control();                                   //PC選択指令軸設定
    int set_ref_mh();                                       //巻速度指令値出力
    int set_ref_gt();                                       //走行速度指令値出力
    int set_ref_slew();                                     //旋回速度指令値出力
    int set_ref_bh();                                       //引込速度指令値出力
  
    
    void update_pb_lamp_com();                              //ランプ表示出力

    int parse_indata();                                     //入力信号の分析
    int update_auto_setting();                              //自動条件の更新
    void set_auto_active(int type);                         //各軸のauto_activeフラグセット
    double cal_step(LPST_COMMAND_SET pCom, int motion);     //自動指令出力値の計算

                                                        
    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

};

