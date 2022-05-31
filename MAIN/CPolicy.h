#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

class CPolicy :public CTaskObj
{
public:
    CPolicy();
    ~CPolicy();

    LPST_COMMAND_STATUS pCom;
    LPST_PLC_IO pPLC_IO;
    LPST_CRANE_STATUS pCraneStat;

   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);

   void input();                //外部データ取り込み
   void main_proc();            //処理内容
   void output();               //出力データ更新

   int parse_notch_com();       //ノッチ指令を速度指令に変換

   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();
};

