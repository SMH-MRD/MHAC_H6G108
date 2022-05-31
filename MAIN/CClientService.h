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

   void input();               //�O���f�[�^��荞��
   void main_proc();           //�������e
   void output();              //�o�̓f�[�^�X�V

   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();
};

