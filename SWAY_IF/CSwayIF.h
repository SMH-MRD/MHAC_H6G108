#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X


#define SWAY_IF_SWAY_IO_MEM_NG        0x8000
#define SWAY_IF_CRANE_MEM_NG         0x4000
#define SWAY_IF_SIM_MEM_NG           0x2000


class CSwayIF :
    public CBasicControl
{
private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pSwayIOObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
 
    ST_SWAY_IO sway_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@

public:
    CSwayIF();
    ~CSwayIF();

    WORD helthy_cnt = 0;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

    void set_debug_mode(int id) {
        if (id) mode |= SWAY_IF_SIM_DBG_MODE;
        else    mode &= ~SWAY_IF_SIM_DBG_MODE;
    }

    int is_debug_mode() { return(mode & SWAY_IF_SIM_DBG_MODE); }

};

