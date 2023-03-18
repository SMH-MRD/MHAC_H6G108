#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"
#include "ClientIf.h"

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

#define CS_SEMIAUTO_TG_SEL_DEFAULT      0
#define CS_SEMIAUTO_TG_SEL_ACTIVE       1
#define CS_SEMIAUTO_TG_SEL_FIXED        2

#define CS_JOBSET_EVENT_CLEAR           0
#define CS_JOBSET_EVENT_JOB_STANDBY         1
#define CS_JOBSET_EVENT_SEMI_STANDBY        2
#define CS_JOBSET_EVENT_JOB_TRIG            4
#define CS_JOBSET_EVENT_SEMI_TRIG           8
#define CS_JOBSET_EVENT_JOB_OVERFLOW       16

#define CS_N_MSG_HOLD                       10

#define CS_FB_CODE_COM_FIN_NORMAL           2 //コマンド正常完了
#define CS_FB_CODE_COM_FIN_ABNORMAL         3 //コマンド異常完了
#define CS_FB_CODE_COM_SUSPENDED            4 //コマンド保留
#define CS_FB_CODE_RECIPE_FIN_NORMAL       5 //コマンドレシピ正常完了
#define CS_FB_CODE_RECIPE_FIN_ABNORMAL     6 //コマンドレシピ異常完了


class CClientService :public CTaskObj
{
public:
    CClientService(); 
    ~CClientService();
  
    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
 
   
    //AGENTアクセス関数
    LPST_JOB_SET get_next_job();                                    //次のJob問い合わせ

    //POLICYアクセス関数
    int update_job_status(LPST_JOB_SET pjobset, int fb_code);       //Jobの実行状況アンサバック

private:

    int parce_onboard_input(int mode);
    int parce_ote_imput(int mode);
    int can_ote_activate();
   
    //クライアントからのメッセージ解析
    int perce_client_message(LPST_CLIENT_COM_RCV_MSG pmsg);
    
    //レシピセット
    LPST_JOB_SET set_job_recipe(LPST_JOB_SET pjob_set);
    LPST_JOB_SET set_semi_recipe(LPST_JOB_SET pjob_set);
 
   
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_CS_INFO pCSinf;
    LPST_AGENT_INFO pAgent_Inf;
    LPST_OTE_IO pOTE_IO;

    CPolicy* pPolicy;
    CEnvironment* pEnvironment;

    ST_CS_INFO CS_workbuf;

    LPST_JOB_SET p_active_job;
    int job_set_event;

    ST_CLIENT_COM_RCV_MSG client_rcv_msg[CS_N_MSG_HOLD];

   void input();               //外部データ取り込み
   void main_proc();           //処理内容
   void output();              //出力データ更新
                                  
   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();

};


