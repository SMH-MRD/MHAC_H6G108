#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"



#define AGENT_PB_OFF_DELAY          10                            //PB�o��OFF DELAY COUNT
#define AGENT_LAMP_ON               PLC_IO_LAMP_FLICKER_CHANGE    //LAMP ON�o�̓J�E���g�l
#define AGENT_LAMP_OFF              0                             //LAMP OFF�o�̓J�E���g�l

#define PHASE_CHECK_RANGE       0.02  // ���V�s�o�́@�ʑ����B����͈́@rad :1deg = 0.017 rad
#define AGENT_FWD               1
#define AGENT_REW               -1
#define AGENT_STOP              0

class CAgent:public CTaskObj
{
public:
    CAgent();
    ~CAgent();

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
    DWORD auto_type;                                //ANTI_SWAY,SEMI_AUTO,JOB
    DWORD auto_ctrl_mode;                           //STAND_BY,SUSPEND,ACTIVE

    int receipt_ope_com(int type, int id);          //Operation Command��t

  private:
    
    LPST_POLICY_INFO pPolicyInf;
    LPST_CS_INFO pCSInf;
    LPST_AGENT_INFO pAgentInf;
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;

    ST_AGENT_INFO   AgentInf_workbuf;

    bool can_auto_trigger();
    bool can_auto_complete();
    int cleanup_command(LPST_COMMAND_SET pcom);

    LPST_COMMAND_SET pCom;

    double ph_chk_range[NUM_OF_AS_AXIS];

 

    void input();                                           //�O���f�[�^��荞��
    void main_proc();                                       //�������e
 
    void output();                                          //�o�̓f�[�^�X�V
    int set_pc_control();                                   //PC�I���w�ߎ��ݒ�
    int set_ref_mh();                                       //�����x�w�ߒl�o��
    int set_ref_gt();                                       //���s���x�w�ߒl�o��
    int set_ref_slew();                                     //���񑬓x�w�ߒl�o��
    int set_ref_bh();                                       //�������x�w�ߒl�o��
    void update_pb_lamp_com();                              //�����v�\���o��

    int parse_indata();                                     //���͐M���̕���
    int update_auto_setting();                              //���������̍X�V
    void set_auto_active(int type);                         //�e����auto_active�t���O�Z�b�g
    double cal_step(LPST_COMMAND_SET pCom, int motion);     //�����w�ߏo�͒l�̌v�Z

    //�^�u�p�l����Static�e�L�X�g��ݒ�
    void set_panel_tip_txt();
    //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
    void set_panel_pb_txt();

};

