#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PLC_IO_HELTHY_NG_COUNT     8
#define SIM_HELTHY_NG_COUNT     8
#define SWAY_HELTHY_NG_COUNT     8



class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();



    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    
    void routine_work(void* param);


private:
    ST_SPEC spec;       //�d�l��� Environment�����L�������ɃZ�b�g����B
    ST_CRANE_STATUS stWorkCraneStat;

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;
    LPST_REMOTE_IO pRemoteIO;
    LPST_SIMULATION_STATUS pSimStat;
 
    void input();               //�O���f�[�^��荞��
    void main_proc();           //�������e
    void output();              //�o�̓f�[�^�X�V

    int parse_notch_com();      //�m�b�`�M���𑬓x�w�߂ɕϊ��Z�b�g

    void chk_subproc();         //�T�u�v���Z�X��ԃ`�F�b�N

    //���C���p�l����Tweet�e�L�X�g��ݒ�
    void tweet_update();
                                
    //�^�u�p�l����Static�e�L�X�g��ݒ�
    void set_panel_tip_txt();
    //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
    void set_panel_pb_txt();
    
};

