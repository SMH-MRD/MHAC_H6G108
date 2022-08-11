#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#include "CPolicy.h"
#include "CEnvironment.h"


class CClientService :public CTaskObj
{
public:
    CClientService(); 
    ~CClientService();
  
   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);

   int receipt_job_feedback();


private:
   
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_CS_INFO pCSinf;

    CPolicy* pPolicy;
    CEnvironment* pEnvironment;


    ST_CS_INFO CSinf_workbuf;

    int semi_auto_target;

    int semi_auto_pb_count[SEMI_AUTO_TARGET_MAX];

   void input();               //�O���f�[�^��荞��
   void main_proc();           //�������e
   void output();              //�o�̓f�[�^�X�V

   void manage_semi_auto();
   void control_request();     //Client����̗v��������
                               
   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();



};


