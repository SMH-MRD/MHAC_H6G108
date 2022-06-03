#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X

#define SIM_IF_PLC_IO_MEM_NG        0x8000
#define SIM_IF_CRANE_MEM_NG         0x4000
#define SIM_IF_SIM_MEM_NG           0x2000
#define SIM_IF_AGENT_MEM_NG         0x1000
#define SIM_IF_SWAY_MEM_NG          0x0800

class CSIM :
    public CBasicControl
{

private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pSimulationStatusObj;
 
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pPLCioObj;
    CSharedMem* pAgentInfObj;
 
    ST_SIMULATION_STATUS sim_stat_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@

    LPST_CRANE_STATUS pCrane;
    LPST_PLC_IO pPLC;
    LPST_AGENT_INFO pAgent;

public:
    CSIM();
    ~CSIM();

    WORD helthy_cnt = 0;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

    //�ǉ����\�b�h
 
    void set_mode(int id) {
        if (id) mode |= SIM_ACTIVE_MODE;
        else    mode &= ~SIM_ACTIVE_MODE;
    }

    int is_act_mode() { return(mode & SIM_ACTIVE_MODE); }

};

