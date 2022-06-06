#include "CSwayIF.h"


CSwayIF::CSwayIF() {
    // 共有メモリオブジェクトのインスタンス化
    pSwayIOObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    out_size = 0;
    memset(&sway_io_workbuf, 0, sizeof(ST_SWAY_IO));   //共有メモリへの出力セット作業用バッファ
};
CSwayIF::~CSwayIF() {
    // 共有メモリオブジェクトの解放
    delete pSwayIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
};

int CSwayIF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //出力バッファセット

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSwayIF::init_proc() {

    // 共有メモリ取得

     // 出力用共有メモリ取得
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pSwayIOObj->create_smem(SMEM_SWAY_IO_NAME, out_size, MUTEX_SWAY_IO_NAME)) {
        mode |= SWAY_IF_SWAY_IO_MEM_NG;
    }
    set_outbuf(pSwayIOObj->get_pMap());

    // 入力用共有メモリ取得
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SWAY_IF_SIM_MEM_NG;
    }
    pSim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= SWAY_IF_CRANE_MEM_NG;
    }

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CSwayIF::input() {
 
    LPST_CRANE_STATUS pcrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();
    LPST_SIMULATION_STATUS psim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();

    //MAINプロセス(Environmentタスクのヘルシー信号取り込み）
    source_counter = pcrane->env_act_count;

    //PLC 入力

    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CSwayIF::parse() {

#ifdef _DVELOPMENT_MODE

    if (pSim->mode & SIM_ACTIVE_MODE) {
        set_sim_status(&sway_io_workbuf);
    }

#endif

    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSwayIF::output() {

    sway_io_workbuf.proc_mode = this->mode;              //モードセット
    sway_io_workbuf.helthy_cnt = my_helthy_counter++;    //ヘルシーカウンタセット

    if (out_size) { //出力処理
        memcpy_s(poutput, out_size, &sway_io_workbuf, out_size);
    }

    return 0;
}

//*********************************************************************************************
// set_sim_status()
//*********************************************************************************************
int CSwayIF::set_sim_status(LPST_SWAY_IO pworkbuf) {

    pworkbuf->rad[SID_TG1][SID_X] = pSim->sway_io.rad[SID_TG1][SID_X];
    pworkbuf->rad[SID_TG1][SID_Y] = pSim->sway_io.rad[SID_TG1][SID_Y];
    pworkbuf->rad[SID_TG1][SID_R] = pSim->sway_io.rad[SID_TG1][SID_R];
    pworkbuf->rad[SID_TG1][SID_TH] = pSim->sway_io.rad[SID_TG1][SID_TH];

    pworkbuf->w[SID_TG1][SID_X] = pSim->sway_io.w[SID_TG1][SID_X];
    pworkbuf->w[SID_TG1][SID_Y] = pSim->sway_io.w[SID_TG1][SID_Y];
    pworkbuf->w[SID_TG1][SID_R] = pSim->sway_io.w[SID_TG1][SID_R];
    pworkbuf->w[SID_TG1][SID_TH] = pSim->sway_io.w[SID_TG1][SID_TH];

    pworkbuf->ph[SID_TG1][SID_X] = pSim->sway_io.ph[SID_TG1][SID_X];
    pworkbuf->ph[SID_TG1][SID_Y] = pSim->sway_io.ph[SID_TG1][SID_Y];
    pworkbuf->ph[SID_TG1][SID_R] = pSim->sway_io.ph[SID_TG1][SID_R];
    pworkbuf->ph[SID_TG1][SID_TH] = pSim->sway_io.ph[SID_TG1][SID_TH];

    return 0;
}
