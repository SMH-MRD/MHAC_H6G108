#include "CPLC_IF_CTRL.h"
#include "plcio_def.h"

CPLC_IF::CPLC_IF() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    pExecStatusObj = new CSharedMem;

    out_size = 0;
    memset(&plc_link,0,sizeof(ST_PLC_LINK)) ;       //PLC�����N�o�b�t�@�̓��e
    memset(&plc_io_workbuf,0,sizeof(ST_PLC_IO));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
};
CPLC_IF::~CPLC_IF() {
    // ���L�������I�u�W�F�N�g�̉��
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pExecStatusObj;
};

int CPLC_IF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //�o�̓o�b�t�@�Z�b�g

//*********************************************************************************************
// init_proc()
//*********************************************************************************************
int CPLC_IF::init_proc() {

    // ���L�������擾
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

    if (OK_SHMEM != pExecStatusObj->create_smem(SMEM_EXEC_STATUS_NAME, sizeof(ST_EXEC_STATUS), MUTEX_EXEC_STATUS_NAME)){
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

    
    //�f�o�b�O���[�h���́A����p�l���E�B���h�E�̓��e�ŏ㏑��
    if (is_debug_mode()) set_debug_status();

    //PLC�����N���͂����
    if(B_HST_NOTCH_0)plc_io_workbuf.ui.notch_pos[ID_HOIST]=0;

    //PLC�����N�o�͓��e�����


    return 0; 
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CPLC_IF::output() { 
    plc_io_workbuf.helthy_cnt = my_helthy_counter++;//�w���V�[�J�E���^�Z�b�g

    if (out_size) { //�o�͏���
        memcpy_s(poutput, out_size, &plc_io_workbuf, out_size);
    }

    return 0;
}
//*********************************************************************************************
// set_debug_status()
//*********************************************************************************************
int CPLC_IF::set_debug_status() {




    return 0;
}


