#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define POLICY_REQ_REMOTE       0x00000001
#define POLICY_REQ_ANTISWAY     0x00000002
#define POLICY_REQ_JOB          0x00000004
#define POLICY_REQ_DEBUG        0x00000008

class CPolicy :public CTaskObj
{
public:
   CPolicy();
   ~CPolicy();



   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);
   
   
   int update_control(DWORD code, LPVOID optlp);

private:

    LPST_POLICY_INFO pPolicyInf;
    LPST_PLC_IO pPLC_IO;
    LPST_CRANE_STATUS pCraneStat;
    LPST_REMOTE_IO pRemoteIO;

    void input();                //外部データ取り込み
    void main_proc();            //処理内容
    void output();               //出力データ更新

    DWORD set_pc_control(DWORD dw_axis);

   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();
};

