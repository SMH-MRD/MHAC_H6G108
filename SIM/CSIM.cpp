#include "CSIM.h"
#include "CWorkWindow_SIM.h"
#include "Spec.h"
#include "CVector3.h"
#include "SIM.h"

extern ST_SPEC def_spec;

CSIM::CSIM() {
    // 共有メモリオブジェクトのインスタンス化
    pSimulationStatusObj = new CSharedMem;
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    // MOB オブジェクトのインスタンス化
    pCrane = new CCrane(); //クレーンのモデル
    pLoad = new CLoad();   //吊荷のモデル

    out_size = 0;
 
    memset(&sim_stat_workbuf, 0, sizeof(ST_SIMULATION_STATUS));   //共有メモリへの出力セット作業用バッファ
};
CSIM::~CSIM() {
    // 共有メモリオブジェクトの解放
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pAgentInfObj;

    delete pCrane;
    delete pLoad;
 
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

    //クレーン仕様のセット
    pCrane->set_spec(&def_spec);   
    
    //クレーンの初期状態セット 
     pCrane->init_crane(SYSTEM_TICK_ms/1000.0);
 

     //吊荷ｵﾌﾞｼﾞｪｸﾄにｸﾚｰﾝｵﾌﾞｼﾞｪｸﾄを紐付け
     pLoad->set_crane(pCrane);
  
     //吊荷の初期状態セット 
    Vector3 _r(SIM_INIT_R * cos(SIM_INIT_TH) + SIM_INIT_X, SIM_INIT_R * sin(SIM_INIT_TH), def_spec.boom_high- SIM_INIT_L);  //吊点位置
    Vector3 _v(0.0, 0.0, 0.0);                          //吊点位速度
    pLoad->init_mob(SYSTEM_TICK_ms / 1000.0, _r, _v);
    pLoad->set_m(SIM_INIT_M);
 

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
        pAgent->v_ref[ID_HOIST],
        pAgent->v_ref[ID_GANTRY],
        pAgent->v_ref[ID_SLEW],
        pAgent->v_ref[ID_BOOM_H]
    );

    //スキャンタイムセット dtはマルチメディアタイマ　コールバックでセット
    pCrane->set_dt(dt);
    pLoad->set_dt(dt);

    //移動極限状態
    for (int i = 0; i < MOTION_ID_MAX;i++) {
        pCrane->is_fwd_endstop[i] = pCraneStat->is_fwd_endstop[i];
        pCrane->is_rev_endstop[i] = pCraneStat->is_rev_endstop[i];
    }


    return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CSIM::parse() {

     pCrane->update_break_status(); //ブレーキ状態更新
     pCrane->timeEvolution();       //クレーンの位置,速度計算
     pLoad->timeEvolution();        //吊荷の位置,速度計算
     pLoad->r.add(pLoad->dr);       //吊荷位置更新
     pLoad->v.add(pLoad->dv);       //吊荷速度更新
     pLoad->update_relative_vec();  //吊荷吊点相対ベクトル更新(ロープベクトル　L,vL)

    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSIM::output() {

    sim_stat_workbuf.mode = this->mode;                         //モードセット
    sim_stat_workbuf.helthy_cnt = my_helthy_counter++;          //ヘルシーカウンタセット

    set_cran_motion();  //クレーンの位置、速度情報セット
    set_sway_io();      //振れセンサIO情報セット
    
    
    if (out_size) { //出力処理
        memcpy_s(poutput, out_size, &sim_stat_workbuf, out_size);
    }

    return 0;
}
//*********************************************************************************************
// set_cran_motion() クレーン位置、速度情報セット
//*********************************************************************************************
int CSIM::set_cran_motion() {
    sim_stat_workbuf.status.v_fb[ID_HOIST] = pCrane->v0[ID_HOIST];
    sim_stat_workbuf.status.v_fb[ID_GANTRY] = pCrane->v0[ID_GANTRY];
    sim_stat_workbuf.status.v_fb[ID_SLEW] = pCrane->v0[ID_SLEW];
    sim_stat_workbuf.status.v_fb[ID_BOOM_H] = pCrane->v0[ID_BOOM_H];

    sim_stat_workbuf.status.pos[ID_HOIST] = pCrane->r0[ID_HOIST];
    sim_stat_workbuf.status.pos[ID_GANTRY] = pCrane->r0[ID_GANTRY];
    sim_stat_workbuf.status.pos[ID_SLEW] = pCrane->r0[ID_SLEW];
    sim_stat_workbuf.status.pos[ID_BOOM_H] = pCrane->r0[ID_BOOM_H];

    sim_stat_workbuf.L = pLoad->L;
    sim_stat_workbuf.vL = pLoad->vL;

    return 0;
}
//*********************************************************************************************
// output() 振れセンサIO信号セット
//*********************************************************************************************

static double radsl_last, radbh_last;
int CSIM::set_sway_io() {

    //振れセンサ信号
    double th = pCrane->r0[ID_SLEW]+PI90;//旋回角度
    // クレーン座標の振れ角をカメラ座標に変換(旋回角度分回転）
    double phx = ((pLoad->L.x) * cos(th) + (pLoad->L.y) * sin(th))/pCrane->l_mh;
    double phy = (-(pLoad->L.x) * sin(th) + (pLoad->L.y) * cos(th)) / pCrane->l_mh;
    sim_stat_workbuf.rad_cam_x = phx;
    sim_stat_workbuf.rad_cam_y = phy;

    //　カメラ設置パラメータ読み込み
    double Dx = def_spec.Csw[SID_CAM1][SID_X][SID_D0];
    double Dy = def_spec.Csw[SID_CAM1][SID_Y][SID_D0];

    double Hx = def_spec.Csw[SID_CAM1][SID_X][SID_H0]+ def_spec.Csw[SID_CAM1][SID_X][SID_l0];
    double Hy = def_spec.Csw[SID_CAM1][SID_Y][SID_H0]+ def_spec.Csw[SID_CAM1][SID_Y][SID_l0];

    double ph0x = def_spec.Csw[SID_CAM1][SID_X][SID_ph0];
    double ph0y = def_spec.Csw[SID_CAM1][SID_Y][SID_ph0];

    //　カメラ検出角度
    sim_stat_workbuf.sway_io.rad[ID_SLEW] = atan((pCrane->l_mh * sin(phx) - Dx)/(pCrane->l_mh * cos(phx) - Hx)) - ph0x;
    sim_stat_workbuf.sway_io.rad[ID_BOOM_H] = atan((pCrane->l_mh * sin(phy) - Dy) / (pCrane->l_mh * cos(phy) - Hy)) - ph0y;
    //　カメラ検出角速度
    sim_stat_workbuf.sway_io.w[ID_SLEW] = (sim_stat_workbuf.sway_io.rad[ID_SLEW]-radsl_last)/pCrane->dt;
    sim_stat_workbuf.sway_io.w[ID_BOOM_H] = (sim_stat_workbuf.sway_io.rad[ID_BOOM_H]-radbh_last) / pCrane->dt;

    sim_stat_workbuf.w_cam_x = sim_stat_workbuf.sway_io.w[ID_SLEW];
    sim_stat_workbuf.w_cam_y = sim_stat_workbuf.sway_io.w[ID_BOOM_H];

    radsl_last = sim_stat_workbuf.sway_io.rad[ID_SLEW];
    radbh_last = sim_stat_workbuf.sway_io.rad[ID_BOOM_H];

    return 0;
}
