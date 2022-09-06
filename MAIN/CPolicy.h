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

class CPolicy :public CTaskObj
{
public:
   CPolicy();
   ~CPolicy();



   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);
      
   int update_control(DWORD code, LPVOID optlp);            //�����I���r�b�g��ݒ�@���v�������@ENV���S��

   LPST_COMMAND_SET cal_command_recipe(int type, double* ptarget_pos);
 
private:

    LPST_POLICY_INFO pPolicyInf;
    LPST_PLC_IO pPLC_IO;
    LPST_CRANE_STATUS pCraneStat;
    LPST_REMOTE_IO pRemoteIO;

    void input();                //�O���f�[�^��荞��
    void main_proc();            //�������e
    void output();               //�o�̓f�[�^�X�V

    DWORD set_pc_control(DWORD dw_axis);
 
   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();
};

