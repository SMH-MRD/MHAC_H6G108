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
    double dist_for_target[MOTION_ID_MAX];      //目標までの距離
    int motion_dir[MOTION_ID_MAX];              //移動方向

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

      
  private:
    
    LPST_POLICY_INFO    pPolicyInf;
    LPST_CS_INFO        pCSInf;
    LPST_AGENT_INFO     pAgentInf;
    LPST_CRANE_STATUS   pCraneStat;
    LPST_PLC_IO         pPLC_IO;
    LPST_SWAY_IO        pSway_IO;

    ST_AGENT_INFO       AgentInf_workbuf;
    ST_AGENT_WORK       st_as_work;                         //振れ止めパターン作成用

 


    void input();                                           //外部データ取り込み
    void main_proc();                                       //処理内容
    void output();                                          //出力データ更新

    bool can_job_trigger();                                 //ジョブの起動可否判定
 
    int clear_comset(LPST_COMMAND_SET pcom);               //コマンド初期化

    void set_as_workbuf();                                  //振れ止めパターン作成用データ取り込み
    int cal_as_recipe(int motion);                          //振れ止めパターン計算セット
  
    double cal_step(LPST_COMMAND_SET pCom, int motion);     //自動指令出力値の計算

    bool is_command_completed(LPST_COMMAND_SET pCom);
        
    int set_ref_mh();                                       //巻速度指令値出力
    int set_ref_gt();                                       //走行速度指令値出力
    int set_ref_slew();                                     //旋回速度指令値出力
    int set_ref_bh();                                       //引込速度指令値出力
    void update_pb_lamp_com();                              //ランプ表示出力
     
    int dbg_mont[8];//デバッグ用
                                                         
    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

};

