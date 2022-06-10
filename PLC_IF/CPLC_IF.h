#pragma once
#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X

#define PLC_IF_PLC_IO_MEM_NG        0x8000
#define PLC_IF_CRANE_MEM_NG         0x4000
#define PLC_IF_SIM_MEM_NG           0x2000
#define PLC_IF_AGENT_MEM_NG          0x1000


//PLC IO�C���^�[�t�F�[�X�̃f�[�^�T�C�Y
//PLC LINK�f�o�C�X�̊��t�}�b�v�T�C�Y
#define PLC_IF_MAIN_X_BUFSIZE       14
#define PLC_IF_MAIN_Y_BUFSIZE       6
#define PLC_IF_GNT_X_BUFSIZE        8
#define PLC_IF_GNT_Y_BUFSIZE        2
#define PLC_IF_OPE_X_BUFSIZE        10
#define PLC_IF_OPE_Y_BUFSIZE        3
#define PLC_IF_CC_X_BUFSIZE         12
#define PLC_IF_CC_Y_BUFSIZE         12
#define PLC_IF_CC_W_BUFSIZE         43
#define PLC_IF_ABS_DW_BUFSIZE       8   //WORD���@DWORDx2
#define PLC_IF_SENS_W_BUFSIZE       5

#define PLC_IF_READ_B_BUFSIZE       32


typedef struct st_PLCreadB_tag {                //���񖢎g�p
    short dummy_buf[PLC_IF_READ_B_BUFSIZE];     
}ST_PLC_READ_B, * LPST_PLC_READ_B;

typedef struct st_PLCreadW_tag{
    short main_x_buf[PLC_IF_MAIN_X_BUFSIZE];    //MAINPLC X
    short main_y_buf[PLC_IF_MAIN_Y_BUFSIZE];    //MAINPLC Y
    short gnt_x_buf[PLC_IF_GNT_X_BUFSIZE];      //���sPLC X
    short gnt_y_buf[PLC_IF_GNT_Y_BUFSIZE];      //���sPLC Y
    short ope_x_buf[PLC_IF_OPE_X_BUFSIZE];      //�^�]��PLC X
    short ope_y_buf[PLC_IF_OPE_Y_BUFSIZE];      //�^�]��PLC Y
    short cc_x_buf[PLC_IF_CC_X_BUFSIZE];        //CC�@LINK X
    short cc_y_buf[PLC_IF_CC_Y_BUFSIZE];        //CC�@LINK Y
    short padding_buf[2];                       //�\��(DI,AI���E
    short cc_w_buf[PLC_IF_CC_W_BUFSIZE];        //CC�@LINK W
    short abso_dw_buf[PLC_IF_ABS_DW_BUFSIZE];   //�A�u�\�R�[�_data
    short sensor_buf[PLC_IF_SENS_W_BUFSIZE];    //���̑��A�i���O�M��
}ST_PLC_READ_W, * LPST_PLC_READ_W;

#define PLC_IF_PC_B_WRITE_COMSIZE         16    //PC�R�}���h�o�͕��T�C�Y
#define PLC_IF_PC_B_WRITE_SIMSIZE         4     //PC�V�~�����[�V�����o�͕��T�C�Y
#define PLC_IF_PC_W_WRITE_COMSIZE         16    //PC�R�}���h�o�͕��T�C�Y
#define PLC_IF_PC_W_WRITE_SIMSIZE         PLC_IF_CC_W_BUFSIZE + PLC_IF_ABS_DW_BUFSIZE + PLC_IF_SENS_W_BUFSIZE //PC�V�~�����[�V�����o�͕��T�C�Y

typedef struct st_PLCwriteB_tag {
    short pc_com_buf[PLC_IF_PC_B_WRITE_COMSIZE];
    short pc_sim_buf[PLC_IF_PC_B_WRITE_SIMSIZE];
}ST_PLC_WRITE_B, * LPST_PLC_WRITE_B;

typedef struct st_PLCwriteW_tag {
    short pc_com_buf[PLC_IF_PC_W_WRITE_COMSIZE];
    short pc_sim_buf[PLC_IF_PC_W_WRITE_SIMSIZE];
}ST_PLC_WRITE_W, * LPST_PLC_WRITE_W;

#define MELSEC_NET_CH               51      //MELSECNET/H�{�[�h�̃`���l��No.
#define MELSEC_NET_NW_NO            2       //MELSECNET/H�l�b�g���[�N�ԍ�
#define MELSEC_NET_MY_NW_NO         0       //MELSECNET/H�{�[�h  ��NW�w�� 0�i�{�[�h�ݒ�l�Ƃ͈قȂ�j
#define MELSEC_NET_MY_STATION       255     //MELSECNET/H�{�[�h�v���ǔ� 255�i�{�[�h�ݒ�l�Ƃ͈قȂ�j
#define MELSEC_NET_SOURCE_STATION   1       //PLC�ǔ�
#define MELSEC_NET_B_WRITE_START    0x0A00  //�������݊J�n�A�h���X�iPC�{�[�h��LB�̃A�h���X�w��j
#define MELSEC_NET_W_WRITE_START    0x0A00  //�������݊J�n�A�h���X�iPC�{�[�h��LW�̃A�h���X�w��j
#define MELSEC_NET_B_READ_START     0x0000  //�ǂݍ��݊J�n�A�h���X�iPLC MAP����B�̃A�h���X�w��j
#define MELSEC_NET_W_READ_START     0x0000  //�ǂݍ��݊J�n�A�h���X�iPLC MAP����W�̃A�h���X�w��j

#define MELSEC_NET_OK               1
#define MELSEC_NET_SEND_ERR         -1
#define MELSEC_NET_RECEIVE_ERR      -2
#define MELSEC_NET_CLOSE            0

#define MELSEC_NET_RETRY_CNT        100 //�G���[��Retry�J�E���g����

#define MELSEC_NET_CODE_LW          24  //�f�o�C�X�R�[�h
#define MELSEC_NET_CODE_LB          23  //�f�o�C�X�R�[�h
#define MELSEC_NET_CODE_SM          5   //�f�o�C�X�R�[�h
#define MELSEC_NET_CODE_SB          5   //�f�o�C�X�R�[�h
#define MELSEC_NET_CODE_SD          14  //�f�o�C�X�R�[�h
#define MELSEC_NET_CODE_SW          14  //�f�o�C�X�R�[�h

typedef struct st_MelsecNet_tag {
    short chan;                 //�ʐM����̃`���l��No.
    short mode;                 //�_�~�[
    long  path;                 //�I�[�v�����ꂽ����̃p�X�@����N���[�Y���ɕK�v
    long err;                  //�G���[�R�[�h
    short status;               //����̏�ԁ@0:������m���@0����F����@0��艺�F�ُ�
    short retry_cnt;            //����I�[�v�����g���C�J�E���g �}���`���f�B�A�^�C�}�����̔{�������ԊԊu
    
    long write_size_w;          //LW�������݃T�C�Y
    long write_size_b;          //LB�������݃T�C�Y
    ST_PLC_WRITE_B plc_w_buf_B; //PLC�o�͌��o�b�t�@
    ST_PLC_WRITE_W plc_w_buf_W; //PLC�o�͌��o�b�t�@

    long read_size_w;           //LW�ǂݍ��݃T�C�Y
    long read_size_b;           //LB�ǂݍ��݃T�C�Y
    ST_PLC_READ_B  plc_r_buf_B; //PLC���̓o�b�t�@
    ST_PLC_READ_W  plc_r_buf_W; //PLC���̓o�b�t�@

}ST_MELSEC_NET, * LPST_MELSEC_NET;


class CPLC_IF :    public CBasicControl
{

private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pPLCioObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;
    CSharedMem* pAgentInfObj;

    ST_MELSEC_NET   melnet;
    ST_PLC_IO plc_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@

    LPST_SIMULATION_STATUS pSim;    //�V�~�����[�^�X�e�[�^�X
    LPST_CRANE_STATUS pCrane;

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
    int set_debug_status(LPST_PLC_IO pworkbuf); //�f�o�b�O���[�h���Ƀf�o�b�O�p�l���E�B���h�E����̓��͂ŏo�͓��e���㏑��
    int set_sim_status(LPST_PLC_IO pworkbuf);   //�f�o�b�O���[�h����Simulator����̓��͂ŏo�͓��e���㏑��
    int closeIF();
    
    void set_debug_mode(int id) {
        if (id) mode |= PLC_IF_PC_DBG_MODE;
        else    mode &= ~PLC_IF_PC_DBG_MODE;
    }

    int is_debug_mode() { return(mode & PLC_IF_PC_DBG_MODE); }
};
