#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();

    ST_SPEC spec;       //�d�l��� Environment�����L�������ɃZ�b�g����B

    LPST_CRANE_STATUS pCraneStat;

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    
    void routine_work(void* param);

    void get_external_data();   //�O���f�[�^��荞��
    void main_proc();           //�������e
    void update_shared_data();   //�o�̓f�[�^�X�V

    //�^�u�p�l����Static�e�L�X�g��ݒ�
    void set_panel_tip_txt();
    //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
    void set_panel_pb_txt();
    
};

