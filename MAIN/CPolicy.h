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

   void input();                //�O���f�[�^��荞��
   void main_proc();            //�������e
   void output();               //�o�̓f�[�^�X�V

   int parse_notch_com();       //�m�b�`�w�߂𑬓x�w�߂ɕϊ�

   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();
};

