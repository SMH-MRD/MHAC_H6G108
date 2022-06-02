#include "CPLC_IF_CTRL.h"
#include "PLC_IO_DEF.h"
#include "CWorkWindow_PLC.h"

CPLC_IF::CPLC_IF() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    out_size = 0;
    memset(&plc_link,0,sizeof(ST_PLC_LINK)) ;       //PLC�����N�o�b�t�@�̓��e
    memset(&plc_io_workbuf,0,sizeof(ST_PLC_IO));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
};
CPLC_IF::~CPLC_IF() {
    // ���L�������I�u�W�F�N�g�̉��
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pAgentInfObj;
};

int CPLC_IF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //�o�̓o�b�t�@�Z�b�g

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CPLC_IF::init_proc() {

    // ���L�������擾

     // �o�͗p���L�������擾
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, out_size, MUTEX_PLC_IO_NAME)) {
        mode |= PLC_IF_PLC_IO_MEM_NG;
    }
    set_outbuf(pPLCioObj->get_pMap());
 
    // ���͗p���L�������擾
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= PLC_IF_SIM_MEM_NG;
    }

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= PLC_IF_CRANE_MEM_NG;
    }

    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)){
        mode |= PLC_IF_EXE_MEM_NG;
    }

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CPLC_IF::input() {
    LPST_CRANE_STATUS pcrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();
    LPST_SIMULATION_STATUS psim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();
    
    //MAIN�v���Z�X(Environment�^�X�N�̃w���V�[�M����荞�݁j
    source_counter = pcrane->env_act_count;

    //PLC ����

    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CPLC_IF::parse() { 


    //PLC�����N���͂����
    if (B_HST_NOTCH_0)plc_io_workbuf.ui.notch_pos[ID_HOIST] = 0;
    
    //�f�o�b�O���[�h���́A����p�l���E�B���h�E�̓��e���㏑��
    if (is_debug_mode()) set_debug_status();

    return 0; 
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CPLC_IF::output() { 
 
    plc_io_workbuf.mode = this->mode;                   //���[�h�Z�b�g
    plc_io_workbuf.helthy_cnt = my_helthy_counter++;    //�w���V�[�J�E���^�Z�b�g

    if (out_size) { //�o�͏���
        memcpy_s(poutput, out_size, &plc_io_workbuf, out_size);
    }

    ST_PLC_IO* ptemp = (LPST_PLC_IO)poutput;

    return 0;
}
//*********************************************************************************************
// set_debug_status()
//*********************************************************************************************
int CPLC_IF::set_debug_status() {
    
    CWorkWindow_PLC* pWorkWindow;

    plc_io_workbuf.ui.notch_pos[ID_HOIST]       = pWorkWindow->stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_GANTRY]      = pWorkWindow->stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_BOOM_H]      = pWorkWindow->stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_SLEW]        = pWorkWindow->stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH;

    plc_io_workbuf.ui.pb[PLC_UI_PB_ESTOP]       = pWorkWindow->stOpePaneStat.check_estop;
    plc_io_workbuf.ui.pb[PLC_UI_PB_ANTISWAY]    = pWorkWindow->stOpePaneStat.check_antisway;
    plc_io_workbuf.ui.pb[PLC_UI_CS_REMOTE]      = pWorkWindow->stOpePaneStat.button_remote;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_START]  = pWorkWindow->stOpePaneStat.button_auto_start;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_RESET]  = pWorkWindow->stOpePaneStat.button_auto_reset;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_FROM1]  = pWorkWindow->stOpePaneStat.button_from1;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_FROM2]  = pWorkWindow->stOpePaneStat.button_from2;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_FROM3]  = pWorkWindow->stOpePaneStat.button_from3;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_FROM4]  = pWorkWindow->stOpePaneStat.button_from4;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_TO1]    = pWorkWindow->stOpePaneStat.button_to1;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_TO2]    = pWorkWindow->stOpePaneStat.button_to2;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_TO3]    = pWorkWindow->stOpePaneStat.button_to3;
    plc_io_workbuf.ui.pb[PLC_UI_PB_AUTO_TO4]    = pWorkWindow->stOpePaneStat.button_to4;
    
    if(pWorkWindow->stOpePaneStat.button_source1_on)   plc_io_workbuf.status.ctrl[PLC_STAT_CONTROL_SOURCE] = L_ON;
    if(pWorkWindow->stOpePaneStat.button_source1_off)  plc_io_workbuf.status.ctrl[PLC_STAT_CONTROL_SOURCE] = L_OFF;
    plc_io_workbuf.status.ctrl[PLC_STAT_REMOTE] = pWorkWindow->stOpePaneStat.button_remote;
 
    return 0;
}


