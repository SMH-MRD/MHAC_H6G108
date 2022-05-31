#pragma once
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X

#define PLC_IF_PLC_IO_MEM_NG        0x8000
#define PLC_IF_CRANE_MEM_NG         0x4000
#define PLC_IF_SIM_MEM_NG           0x2000
#define PLC_IF_EXE_MEM_NG           0x1000


//PLC IO�C���^�[�t�F�[�X�̃f�[�^�T�C�Y
#define PLC_IF_MAIN_X_BUFSIZE       14
#define PLC_IF_MAIN_Y_BUFSIZE       6
#define PLC_IF_GNT_X_BUFSIZE        8
#define PLC_IF_GNT_Y_BUFSIZE        2
#define PLC_IF_OPE_X_BUFSIZE        10
#define PLC_IF_OPE_Y_BUFSIZE        3
#define PLC_IF_CC_X_BUFSIZE         12
#define PLC_IF_CC_Y_BUFSIZE         12
#define PLC_IF_CC_W_BUFSIZE         43
#define PLC_IF_ABS_DW_BUFSIZE       4
#define PLC_IF_SENS_W_BUFSIZE       5
#define PLC_IF_PC_B_BUFSIZE         16
#define PLC_IF_PC_W_BUFSIZE         32

typedef struct st_PLClink_tag{
    WORD main_x_buf[PLC_IF_MAIN_X_BUFSIZE];
    WORD main_y_buf[PLC_IF_MAIN_Y_BUFSIZE];
    WORD gnt_x_buf[PLC_IF_GNT_X_BUFSIZE];
    WORD gnt_y_buf[PLC_IF_GNT_Y_BUFSIZE];
    WORD ope_x_buf[PLC_IF_OPE_X_BUFSIZE];
    WORD ope_y_buf[PLC_IF_OPE_Y_BUFSIZE];
    WORD cc_x_buf[PLC_IF_CC_X_BUFSIZE];
    WORD cc_y_buf[PLC_IF_CC_Y_BUFSIZE];
    WORD padding_buf[2];
    WORD cc_w_buf[PLC_IF_CC_W_BUFSIZE];
    DWORD abso_dw_buf[PLC_IF_ABS_DW_BUFSIZE];
    WORD sensor_buf[PLC_IF_SENS_W_BUFSIZE];
    WORD pc_b_buf[PLC_IF_PC_B_BUFSIZE];
    WORD pc_w_buf[PLC_IF_PC_W_BUFSIZE];
}ST_PLC_LINK, * LPST_PLC_LINK;


class CPLC_IF :
    public CBasicControl
{

private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pPLCioObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
    CSharedMem* pExecStatusObj;

    ST_PLC_LINK plc_link;       //PLC�����N�o�b�t�@�̓��e
    ST_PLC_IO plc_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@

public:
    CPLC_IF();
    ~CPLC_IF();
 
    WORD helthy_cnt=0;

    //�I�[�o�[���C�h
    int set_outbuf(LPVOID); //�o�̓o�b�t�@�Z�b�g
    int init_proc();        //����������
    int input();            //���͏���
    int parse();            //���C������
    int output();           //�o�͏���

    //�ǉ����\�b�h
    int set_debug_status(); //�f�o�b�O���[�h���Ƀf�o�b�O�p�l���E�B���h�E����̓��͂ŏo�͓��e���㏑��
    
    void set_debug_mode(int id) {
        if (id) mode |= PLC_IF_PLC_DBG_MODE;
        else    mode &= ~PLC_IF_PLC_DBG_MODE;
    }

    int is_debug_mode() { return(mode & PLC_IF_PLC_DBG_MODE); }
};
