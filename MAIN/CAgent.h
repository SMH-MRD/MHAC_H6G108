#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"




#define PHASE_CHECK_RANGE           0.02  // レシピ出力　位相到達判定範囲　rad :1deg = 0.017 rad
#define AGENT_FWD                   1
#define AGENT_REW                   -1
#define AGENT_STOP                  0
#define AGENT_NA                    0

#define AGENT_AUTO_TRIG_ACK_COUNT   10
#define AGENT_CHECK_LARGE_SWAY_m2   1.0     //起動時に初期振れ大とみなす振れ量mの2乗


typedef struct stAgentWork {
    double T;	                                //振れ周期
    double w;	                                //振れ角周波数
    double w2;	                                //振れ角周波数2乗
    double pos[MOTION_ID_MAX];	                //現在位置
    double v[MOTION_ID_MAX];	                //モータの速度
    double a[MOTION_ID_MAX];	                //モータの加速度
    double a_hp[MOTION_ID_MAX];	                //モータの加速度
    double vmax[MOTION_ID_MAX];                 //吊点の加速度
    double acc_time2Vmax[MOTION_ID_MAX];        //最大加速時間
 
    double pp_th0[MOTION_ID_MAX][ACCDEC_MAX];   //位相平面の回転中心
 
    int motion_dir[MOTION_ID_MAX];              //移動方向
    double sway_amp[MOTION_ID_MAX];             //振れ振幅
    double sway_amp2[MOTION_ID_MAX];            //振れ振幅２乗
    unsigned int agent_scan_ms;                 //AGENTタスクのスキャンタイム
}ST_AGENT_WORK, * LPST_AGENT_WORK;


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
    ST_AGENT_WORK   st_as_work;                     //振れ止めパターン作成用

    LPST_AGENT_WORK set_as_workbuf(ST_POS_TARGETS trgets, int type);
    
    bool can_auto_trigger();
    bool can_auto_complete();
    int cleanup_command(LPST_COMMAND_BLOCK pcom);

    LPST_COMMAND_BLOCK pCom;

   
    int dbg_mont[8];//デバッグ用

    void input();                                           //外部データ取り込み
    void main_proc();                                       //処理内容
    void output();                                          //出力データ更新
 
    int set_pc_control();                                   //PC選択指令軸設定
    int set_ref_mh();                                       //巻速度指令値出力
    int set_ref_gt();                                       //走行速度指令値出力
    int set_ref_slew();                                     //旋回速度指令値出力
    int set_ref_bh();                                       //引込速度指令値出力

    int set_receipe_as_bh(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
    int set_receipe_as_slw(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
  
    
    void update_pb_lamp_com();                              //ランプ表示出力

    int parse_indata();                                     //入力信号の分析
    int update_auto_setting();                              //自動条件の更新
    void set_auto_active(int type);                         //各軸のauto_activeフラグセット
    double cal_step(LPST_COMMAND_BLOCK pCom, int motion);     //自動指令出力値の計算

                                                        
    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

};

