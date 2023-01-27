#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#include "CPolicy.h"
#include "CEnvironment.h"


#define SEMI_AUTO_TG_RESET_TIME     200
#define SEMI_AUTO_TG_SELECT_TIME    20

#define CS_SEMIAUTO_LIST_CLEAR      1
#define CS_SEMIAUTO_LIST_ADD        2
#define CS_JOB_LIST_CLEAR           3
#define CS_JOB_LIST_ADD             4


class CClientService :public CTaskObj
{
public:
    CClientService(); 
    ~CClientService();
  
   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);
   void routine_work(void* param);



private:

    int update_semiauto_joblist(int command, int code);
   
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_CS_INFO pCSinf;
    LPST_AGENT_INFO pAgent_Inf;

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


