#include "CSIM.h"
#include "CWorkWindow_SIM.h"
#include "Spec.h"

extern ST_SPEC def_spec;

CSIM::CSIM() {
    // 共有メモリオブジェクトのインスタンス化
    pSimulationStatusObj = new CSharedMem;
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    pCrane = new CCrane(); //クレーンのモデル
    CLoad* pLoad;   //吊荷のモデル

    out_size = 0;
 
    memset(&sim_stat_workbuf, 0, sizeof(ST_SIMULATION_STATUS));   //共有メモリへの出力セット作業用バッファ
};
CSIM::~CSIM() {
    // 共有メモリオブジェクトの解放
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pAgentInfObj;
 
};

int CSIM::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //出力バッファセット

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSIM::init_proc() {

    // 共有メモリ取得

     // 出力用共有メモリ取得
    out_size = sizeof(ST_SIMULATION_STATUS);
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, out_size, MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SIM_IF_SIM_MEM_NG;
    }

    // 入力用共有メモリ取得
    if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, sizeof(ST_PLC_IO), MUTEX_PLC_IO_NAME)) {
        mode |= SIM_IF_PLC_IO_MEM_NG;
    }
    set_outbuf(pPLCioObj->get_pMap());
    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= SIM_IF_CRANE_MEM_NG;
    }
    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)) {
        mode |= SIM_IF_AGENT_MEM_NG;
    }
     
    set_outbuf(pSimulationStatusObj->get_pMap());

    pCraneStat = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();
    pPLC = (LPST_PLC_IO)pPLCioObj->get_pMap();
    pAgent = (LPST_AGENT_INFO)pAgentInfObj->get_pMap();

    pCrane->set_spec(&def_spec);

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CSIM::input() {
    sim_stat_workbuf.helthy_cnt++;

    //MAINプロセス(Environmentタスクのヘルシー信号取り込み）
    source_counter = pCraneStat->env_act_count;

    //PLC 入力
    pCrane->set_v_ref(
        pCraneStat->notch_spd_ref[ID_HOIST],
        pCraneStat->notch_spd_ref[ID_GANTRY],
        pCraneStat->notch_spd_ref[ID_SLEW],
        pCraneStat->notch_spd_ref[ID_BOOM_H]
    );




    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CSIM::parse() {

    pCrane->update_ref_elapsed();   //指令出力経過時間更新
    pCrane->update_break_status();  //ブレーキ状態更新


    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSIM::output() {

    sim_stat_workbuf.mode = this->mode;                      //モードセット
    sim_stat_workbuf.helthy_cnt = my_helthy_counter++;    //ヘルシーカウンタセット

    if (out_size) { //出力処理
        memcpy_s(poutput, out_size, &sim_stat_workbuf, out_size);
    }

    return 0;
}
