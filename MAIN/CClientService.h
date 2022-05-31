#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

class CClientService :public CTaskObj
{
public:
    CClientService(); 
    ~CClientService();
  
   LPST_CRANE_STATUS pCraneStat;
   LPST_PLC_IO pPLC_IO;

   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);

   void input();               //外部データ取り込み
   void main_proc();           //処理内容
   void output();              //出力データ更新

   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();
};

