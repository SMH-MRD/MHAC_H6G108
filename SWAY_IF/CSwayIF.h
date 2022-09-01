#pragma once

#include "COMMON_DEF.H"
#include "CSharedMem.h"	    //# ���L�������N���X
#include "Spec.h"
#include "Swaysensor.h"

#define SWAY_IF_SWAY_IO_MEM_NG      0x8000
#define SWAY_IF_CRANE_MEM_NG        0x4000
#define SWAY_IF_SIM_MEM_NG          0x2000

#define CAM_SET_PARAM_N_PARAM       4
#define CAM_SET_PARAM_a             0
#define CAM_SET_PARAM_b             1
#define CAM_SET_PARAM_c             2
#define CAM_SET_PARAM_d             3

#define N_SWAY_SENSOR_RCV_BUF   10  //��M�f�[�^�̃o�b�t�@��
#define N_SWAY_SENSOR_SND_BUF   10  //���M�f�[�^�̃o�b�t�@��

class CSwayIF :
    public CBasicControl
{
private:

    //# �o�͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pSwayIOObj;
    //# ���͗p���L�������I�u�W�F�N�g�|�C���^:
    CSharedMem* pCraneStatusObj;
    CSharedMem* pSimulationStatusObj;

    ST_SWAY_RCV_MSG rcv_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_RCV_BUF];
    ST_SWAY_SND_MSG snd_msg[N_SWAY_SENSOR][N_SWAY_SENSOR_SND_BUF];
    int i_rcv_msg[N_SWAY_SENSOR] = { 0,0,0 };
    int i_snd_msg[N_SWAY_SENSOR] = { 0,0,0 };

    LPST_CRANE_STATUS pCraneStat;
    LPST_SIMULATION_STATUS pSimStat;
   
    ST_SWAY_IO sway_io_workbuf;   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
    double SwayCamParam[N_SWAY_SENSOR][SWAY_SENSOR_N_AXIS][CAM_SET_PARAM_N_PARAM]; //�U��Z���T�J�����ݒu�p�����[�^[�J����No.][������XY][a,b]

    int parse_sway_stat(int ID);

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



    //�ǉ����\�b�h
     int set_sim_status(LPST_SWAY_IO pworkbuf);   //�f�o�b�O���[�h����Simulator����̓��͂ŏo�͓��e���㏑��
};

