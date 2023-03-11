#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#include "CPolicy.h"
#include "CEnvironment.h"


#define SEMI_AUTO_TG_RESET_TIME     100
#define SEMI_AUTO_TG_SELECT_TIME    4
#define AUTO_START_CHECK_TIME       4
#define LAMP_FLICKER_BASE_COUNT     8
#define LAMP_FLICKER_CHANGE_COUNT   5


#define CS_CLEAR_SEMIAUTO      1
#define CS_ADD_SEMIAUTO        2
#define CS_CLEAR_JOB           3
#define CS_ADD_JOB             4

#define AUTO_TG_ADJUST_100mm      0.1 //目標位置補正距離0.1m
#define AUTO_TG_ADJUST_1000mm     1.0 //目標位置補正距離01m

#define CS_NORMAL_OPERATION_MODE 0

#define CS_SEMIAUTO_TG_SEL_DEFAULT  0
#define CS_SEMIAUTO_TG_SEL_ACTIVE   1
#define CS_SEMIAUTO_TG_SEL_FIXED    2

class CClientService :public CTaskObj
{
public:
    CClientService(); 
    ~CClientService();
  
    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
    int report_job_fb(LPST_JOB_SET job, int fb_code);               //Jobの実行状況アンサバック

private:

    int parce_onboard_input(int mode);
    int parce_ote_imput(int mode);
    int can_ote_activate();
    int judge_job_list_status();

    int update_semiauto_list(int command, int type, int code);      //半自動のジョブリストの更新　command:CLEAR ADD, code, type:JOB_SEMI_PARK...
    int update_job_list(int command, int code);                     //クライアントジョブリストの更新　command:CLEAR ADD, code, type:JOB_SEMI_PARK...
   
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_CS_INFO pCSinf;
    LPST_AGENT_INFO pAgent_Inf;
    LPST_OTE_IO pOTE_IO;

    CPolicy* pPolicy;
    CEnvironment* pEnvironment;

    ST_CS_INFO CS_workbuf;

   void input();               //外部データ取り込み
   void main_proc();           //処理内容
   void output();              //出力データ更新
                                  
   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();

};


