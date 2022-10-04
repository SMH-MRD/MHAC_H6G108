#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define POLICY_REQ_REMOTE       0x00000001
#define POLICY_REQ_ANTISWAY     0x00000002
#define POLICY_REQ_JOB          0x00000004
#define POLICY_REQ_DEBUG        0x00000008

//#define POLICY_TYPE_AS          0
//#define POLICY_TYPE_SEMI        1
//#define POLICY_TYPE_JOB         2

#define N_AS_PTN                8
#define AS_PTN_0                0
#define AS_PTN_1STEP            1
#define AS_PTN_2STEP            2
#define AS_PTN_2PP              2
#define AS_PTN_2PN              3
#define AS_PTN_2ACCDEC          4
#define AS_PTN_1ACCDEC          5
#define AS_PTN_3STEP            6
#define AS_PTN_OK               1
#define AS_PTN_NG               0

#define N_AUTO_PARAM            8
#define POLICY_FWD              1
#define POLICY_REW              2
#define POLICY_STOP             0
#define POLICY_NA               0

typedef struct stPolicyWork {
    double T;                                   //����
    double w;                                   //�p���g��
    double l;                                   //���[�v��
    double r[MOTION_ID_MAX];	                //�U���]���l
    double pos[MOTION_ID_MAX];	                //�U���]���l
    double v[MOTION_ID_MAX];	                //�U���]���l
    //double r0[MOTION_ID_MAX];	                //�������U���S
    double a[MOTION_ID_MAX];	                //�����x
    double vmax[MOTION_ID_MAX];                 //�ő呬�x
    double acc_time2Vmax[MOTION_ID_MAX];        //�ő��������
    double dist_for_target[MOTION_ID_MAX];      //�ڕW�܂ł̋���
    double pp_th0[NUM_OF_AS_AXIS][ACCDEC_MAX];  //�ʑ����ʂ̉�]���S
    double pos_target[MOTION_ID_MAX];           //�ʑ����ʂ̉�]���S
    int motion_dir[NUM_OF_AS_AXIS];             //�ړ�����
    double as_gain_phase[NUM_OF_AS_AXIS];       //�U��~�߃Q�C���ʑ�(�ʑ����ʏ�̉������̈ʑ��ω��ʁj
    double as_gain_time[NUM_OF_AS_AXIS];        //�U��~�߃Q�C����������
    bool is_sway_over_r0[MOTION_ID_MAX];        //�U��U���������U��ȏ�
    unsigned int agent_scan_ms;                 //AGENT�^�X�N�̃X�L�����^�C��

}ST_POLICY_WORK, * LPST_POLICY_WORK;


class CPolicy :public CTaskObj
{
public:
   CPolicy();
   ~CPolicy();

   LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

   void init_task(void* pobj);

   void routine_work(void* param);
 
   LPST_COMMAND_SET generate_command(int type, double* ptarget_pos);
   int  update_com_status(LPST_COMMAND_SET pcom);
 
private:

    LPST_POLICY_INFO pPolicyInf;
    LPST_PLC_IO pPLC_IO;
    LPST_CRANE_STATUS pCraneStat;
    LPST_REMOTE_IO pRemoteIO;
    LPST_AGENT_INFO pAgentInf;
    LPST_SWAY_IO pSway_IO;

    void input();               //�O���f�[�^��荞��
    void main_proc();           //�������e
    void output();              //�o�̓f�[�^�X�V

    LPST_COMMAND_SET next_command(int type); //���̃R�}���h��
    int set_pp_th0(int motion);
    int set_pattern_cal_base(int auto_type, int motion);
    int judge_auto_ctrl_ptn(int auto_type, int motion); //�U��~�߃p�^�[������
    void set_as_gain(int motion, int as_type);          //�U��~�߃Q�C���v�Z

    int set_recipe(LPST_COMMAND_SET pcom, int motion, int ptn);
    int set_recipe1step(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2pn(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe2ad(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe1ad(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe3step(LPST_MOTION_RECIPE precipe, int motion);
    int set_recipe0step(LPST_MOTION_RECIPE precipe, int motion);
     
   //�^�u�p�l����Static�e�L�X�g��ݒ�
   void set_panel_tip_txt();
   //�^�u�p�l����Function�{�^����Static�e�L�X�g��ݒ�
   void set_panel_pb_txt();

   ST_POLICY_WORK   st_work;
   int command_id = 0;
    
   const double param_auto[NUM_OF_AS_AXIS][N_AUTO_PARAM] =
   { 
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
     {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}
   };

};

