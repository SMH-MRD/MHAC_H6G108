#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"




#define PHASE_CHECK_RANGE           0.02  // ���V�s�o�́@�ʑ����B����͈́@rad :1deg = 0.017 rad
#define AGENT_FWD                   1
#define AGENT_REW                   -1
#define AGENT_STOP                  0
#define AGENT_NA                    0

#define AGENT_AUTO_TRIG_ACK_COUNT   10
#define AGENT_CHECK_LARGE_SWAY_m2   1.0     //�N�����ɏ����U���Ƃ݂Ȃ��U���m��2��


typedef struct stAgentWork {
    double T;	                                //�U�����
    double w;	                                //�U��p���g��
    double w2;	                                //�U��p���g��2��
    double pos[MOTION_ID_MAX];	                //���݈ʒu
    double v[MOTION_ID_MAX];	                //���[�^�̑��x
    double a[MOTION_ID_MAX];	                //���[�^�̉����x
    double a_hp[MOTION_ID_MAX];	                //���[�^�̉����x
    double vmax[MOTION_ID_MAX];                 //�ݓ_�̉����x
    double acc_time2Vmax[MOTION_ID_MAX];        //�ő��������
 
    double pp_th0[MOTION_ID_MAX][ACCDEC_MAX];   //�ʑ����ʂ̉�]���S
 
    int motion_dir[MOTION_ID_MAX];              //�ړ�����
    double sway_amp[MOTION_ID_MAX];             //�U��U��
    double sway_amp2[MOTION_ID_MAX];            //�U��U���Q��
    unsigned int agent_scan_ms;                 //AGENT�^�X�N�̃X�L�����^�C��
}ST_AGENT_WORK, * LPST_AGENT_WORK;


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
      
  private:
    
    LPST_POLICY_INFO pPolicyInf;
    LPST_CS_INFO pCSInf;
    LPST_AGENT_INFO pAgentInf;
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;

    ST_AGENT_INFO   AgentInf_workbuf;
    ST_AGENT_WORK   st_as_work;                     //�U��~�߃p�^�[���쐬�p

    LPST_AGENT_WORK set_as_workbuf(ST_POS_TARGETS trgets, int type);
    
    bool can_auto_trigger();
    bool can_auto_complete();
    int cleanup_command(LPST_COMMAND_BLOCK pcom);

    LPST_COMMAND_BLOCK pCom;

   
    int dbg_mont[8];//�f�o�b�O�p

    void input();                                           //�O���f�[�^��荞��
    void main_proc();                                       //�������e
    void output();                                          //�o�̓f�[�^�X�V
 
    int set_pc_control();                                   //PC�I���w�ߎ��ݒ�
    int set_ref_mh();                                       //�����x�w�ߒl�o��
    int set_ref_gt();                                       //���s���x�w�ߒl�o��
    int set_ref_slew();                                     //���񑬓x�w�ߒl�o��
    int set_ref_bh();                                       //�������x�w�ߒl�o��

    int set_receipe_as_bh(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
    int set_receipe_as_slw(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork);
  
    
    void update_pb_lamp_com();                              //�����v�\���o��

    int parse_indata();                                     //���͐M���̕���
    int update_auto_setting();                              //���������̍X�V
    void set_auto_active(int type);                         //�e����auto_active�t���O�Z�b�g
    double cal_step(LPST_COMMAND_BLOCK pCom, int motion);     //�����w�ߏo�͒l�̌v�Z

                                                        
    //�^�u�p�l����Static�e�L�X�g��ݒ�
    void set_panel_tip_txt();
    //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
    void set_panel_pb_txt();

};

