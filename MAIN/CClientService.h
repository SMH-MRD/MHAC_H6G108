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

 
   void input();               //�O���f�[�^��荞��
   void main_proc();           //�������e
   void output();              //�o�̓f�[�^�X�V


                                  
   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();

};


