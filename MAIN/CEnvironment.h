#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PLC_IO_HELTHY_NG_COUNT     8
#define SIM_HELTHY_NG_COUNT     8

typedef struct stEnvSubproc {
    bool is_plcio_join = false;
    bool is_sim_join = false;
} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;


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

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_REMOTE_IO pRemoteIO;
 
    ST_ENV_SUBPROC st_subproc;

    void input();               //�O���f�[�^��荞��
    void main_proc();           //�������e
    void output();              //�o�̓f�[�^�X�V

    int parse_notch_com();       //�m�b�`�M���𑬓x�w�߂ɕϊ��Z�b�g

    //�^�u�p�l����Static�e�L�X�g��ݒ�
    void set_panel_tip_txt();
    //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
    void set_panel_pb_txt();
    
};

