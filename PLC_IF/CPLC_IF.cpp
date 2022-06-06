#include "CPLC_IF.h"
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
    pSim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= PLC_IF_CRANE_MEM_NG;
    }
    pCrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();

    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)){
        mode |= PLC_IF_AGENT_MEM_NG;
    }

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CPLC_IF::input() {
    
       
    //MAIN�v���Z�X(Environment�^�X�N�̃w���V�[�M����荞�݁j
    source_counter = pCrane->env_act_count;

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
    if (is_debug_mode()) set_debug_status(&plc_io_workbuf);

#ifdef _DVELOPMENT_MODE

    if (pSim->mode & SIM_ACTIVE_MODE) {
        set_sim_status(&plc_io_workbuf);
    }

#endif

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

    return 0;
}
//*********************************************************************************************
// set_debug_status()
//*********************************************************************************************
int CPLC_IF::set_debug_status(LPST_PLC_IO pworkbuf) {
    
    CWorkWindow_PLC* pWorkWindow;

    pworkbuf->ui.notch_pos[ID_HOIST]       = pWorkWindow->stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH;
    pworkbuf->ui.notch_pos[ID_GANTRY]      = pWorkWindow->stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH;
    pworkbuf->ui.notch_pos[ID_BOOM_H]      = pWorkWindow->stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH;
    pworkbuf->ui.notch_pos[ID_SLEW]        = pWorkWindow->stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH;

    pworkbuf->ui.pb[PLC_UI_PB_ESTOP]       = pWorkWindow->stOpePaneStat.check_estop;
    pworkbuf->ui.pb[PLC_UI_PB_ANTISWAY]    = pWorkWindow->stOpePaneStat.check_antisway;
    pworkbuf->ui.pb[PLC_UI_CS_REMOTE]      = pWorkWindow->stOpePaneStat.button_remote;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_START]  = pWorkWindow->stOpePaneStat.button_auto_start;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_RESET]  = pWorkWindow->stOpePaneStat.button_auto_reset;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_FROM1]  = pWorkWindow->stOpePaneStat.button_from1;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_FROM2]  = pWorkWindow->stOpePaneStat.button_from2;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_FROM3]  = pWorkWindow->stOpePaneStat.button_from3;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_FROM4]  = pWorkWindow->stOpePaneStat.button_from4;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_TO1]    = pWorkWindow->stOpePaneStat.button_to1;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_TO2]    = pWorkWindow->stOpePaneStat.button_to2;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_TO3]    = pWorkWindow->stOpePaneStat.button_to3;
    pworkbuf->ui.pb[PLC_UI_PB_AUTO_TO4]    = pWorkWindow->stOpePaneStat.button_to4;
    
    if(pWorkWindow->stOpePaneStat.button_source1_on)   pworkbuf->status.ctrl[PLC_STAT_CONTROL_SOURCE] = L_ON;
    if(pWorkWindow->stOpePaneStat.button_source1_off)  pworkbuf->status.ctrl[PLC_STAT_CONTROL_SOURCE] = L_OFF;
    pworkbuf->status.ctrl[PLC_STAT_REMOTE] = pWorkWindow->stOpePaneStat.button_remote;
 
    return 0;
}

//*********************************************************************************************
// set_sim_status()
//*********************************************************************************************
int CPLC_IF::set_sim_status(LPST_PLC_IO pworkbuf) {

    pworkbuf->status.v_fb[ID_HOIST]     = pSim->status.v_fb[ID_HOIST];
    pworkbuf->status.v_fb[ID_GANTRY]    = pSim->status.v_fb[ID_GANTRY];
    pworkbuf->status.v_fb[ID_BOOM_H]    = pSim->status.v_fb[ID_BOOM_H];
    pworkbuf->status.v_fb[ID_SLEW]      = pSim->status.v_fb[ID_SLEW];

    pworkbuf->status.pos[ID_HOIST]      = pSim->status.pos[ID_HOIST];
    pworkbuf->status.pos[ID_GANTRY]     = pSim->status.pos[ID_GANTRY];
    pworkbuf->status.pos[ID_BOOM_H]     = pSim->status.pos[ID_BOOM_H];
    pworkbuf->status.pos[ID_SLEW]       = pSim->status.pos[ID_SLEW];

    return 0;
}



