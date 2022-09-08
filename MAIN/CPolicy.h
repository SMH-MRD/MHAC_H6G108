#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define POLICY_REQ_REMOTE       0x00000001
#define POLICY_REQ_ANTISWAY     0x00000002
#define POLICY_REQ_JOB          0x00000004
#define POLICY_REQ_DEBUG        0x00000008

#define POLICY_TYPE_AS          0
#define POLICY_TYPE_SEMI        1
#define POLICY_TYPE_JOB         2

#define AS_PTN_0                0
#define AS_PTN_1STEP            1
#define AS_PTN_2PP              2
#define AS_PTN_2PN              3
#define AS_PTN_2ACCDEC          4
#define AS_PTN_3STEP            5


class CPolicy :public CTaskObj
{
public:
   CPolicy();
   ~CPolicy();

   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);
 
   LPST_COMMAND_SET generate_command(int type, double* ptarget_pos);
   int  update_com_status(LPST_COMMAND_SET pcom, LPST_COMMAND_STAT presult);
 
private:

    LPST_POLICY_INFO pPolicyInf;
    LPST_PLC_IO pPLC_IO;
    LPST_CRANE_STATUS pCraneStat;
    LPST_REMOTE_IO pRemoteIO;
    LPST_AGENT_INFO pAgentInf;
    LPST_SWAY_IO pSway_IO;

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void output();              //出力データ更新

    int next_command(int type); //次のコマンドへ

    int select_as_ptn(int motion);
    int set_recipe0(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe1step(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2pn(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2ad(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe3step(LPST_MOTION_RECIPE precipe, int motion);

    int set_pp_th0();

   //タブパネルのStaticテキストを設定
   void set_panel_tip_txt();
   //タブパネルのFunctionボタンのStaticテキストを設定
   void set_panel_pb_txt();

   double pp_th0[NUM_OF_AS_AXIS][ACCDEC_MAX];   //位相平面の回転中心 

};

