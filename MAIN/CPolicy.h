#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PTN_ORDINARY                0x00000000  //通常
#define PTN_NON_FBSWAY_FULL         0x00000001
#define PTN_FBSWAY_FULL             0x00000002
#define PTN_NON_FBSWAY_2INCH        0x00000004
#define PTN_FBSWAY_AS               0x00000008

#define PTN_FBSWAY_AS               0x00000008

#define POLICY_PTN_OK               1
#define POLICY_PTN_NG               0

#define N_AUTO_PARAM                8

#define SPD_FB_DELAY_TIME           0.3             //速度指令-FB遅れ時間
#define FINE_POS_TIMELIMIT          5.0             //ファインポジショニング制限時間


typedef struct stPolicyWork {
    double T;	                                //振れ周期
    double w;	                                //振れ角周波数
    double w2;	                                //振れ角周波数2乗
    double pos[MOTION_ID_MAX];	                //現在位置
    double v[MOTION_ID_MAX];	                //モータの速度
    double a_abs[MOTION_ID_MAX];	            //モータの加速度　絶対値
    double a_hp_abs[MOTION_ID_MAX];	            //吊点の加速度　絶対値
    double vmax_abs[MOTION_ID_MAX];             //モータの最大速度
    double acc_time2Vmax[MOTION_ID_MAX];        //最大加速時間
    double dist_for_target[MOTION_ID_MAX];      //目標までの距離符号あり
    double dist_for_target_abs[MOTION_ID_MAX];      //目標までの距離符号あり
    double pp_th0[MOTION_ID_MAX][ACCDEC_MAX];   //位相平面の回転中心
    ST_POS_TARGETS target;                      //目標位置
    int motion_dir[MOTION_ID_MAX];              //移動方向
     unsigned int agent_scan_ms;                 //AGENTタスクのスキャンタイム
     double agent_scan;                         //AGENTタスクのスキャンタイム 秒
}ST_POLICY_WORK, * LPST_POLICY_WORK;


class CPolicy :public CTaskObj
{
public:
   CPolicy();
   ~CPolicy();

   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);
   void routine_work(void* param);
 
  //AGENT
   LPST_COMMAND_SET req_command(LPST_JOB_SET pjob_set);                      //Agentからの要求に応じて実行コマンドをセットして返す
   int update_command_status(LPST_COMMAND_SET pcom, int code);  //Agentからのコマンド実行状況報告を受付,次のコマンドあるときはそれを返す
 
private:

    LPST_POLICY_INFO    pPolicyInf;
    LPST_PLC_IO         pPLC_IO;
    LPST_CRANE_STATUS   pCraneStat;
    LPST_OTE_IO         pOTE_IO;
    LPST_AGENT_INFO     pAgentInf;
    LPST_SWAY_IO        pSway_IO;
    LPST_CS_INFO        pCSInf;
    LPST_JOB_IO         pJob_IO;

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void output();              //出力データ更新

    LPST_COMMAND_SET setup_job_command(LPST_COM_RECIPE pcom_recipe, int type);  //実行する半自動のコマンドをセットする
  
 
    int set_recipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
    int set_recipe_semiauto_slw(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
    int set_recipe_semiauto_mh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);

    LPST_POLICY_WORK set_com_workbuf(ST_POS_TARGETS trget);
    ST_POLICY_INFO   PolicyInf_workbuf;
    ST_POLICY_WORK   st_com_work;
    int command_id = 0;

                                                         
   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();


    
   const double param_auto[NUM_OF_AS_AXIS][N_AUTO_PARAM] =
   { 
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
   };

};

