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

   //CraneStat立ち上がり待ち
    while (pCraneStat->is_tasks_standby_ok ==false) {
        Sleep(10);
    }

    //クレーン仕様のセット
    pCrane->set_spec(&def_spec);

    //クレーンの初期状態セット 
    pCrane->init_crane(SYSTEM_TICK_ms / 1000.0);


    //吊荷ｵﾌﾞｼﾞｪｸﾄにｸﾚｰﾝｵﾌﾞｼﾞｪｸﾄを紐付け
    pLoad->set_crane(pCrane);

    //吊荷の初期状態セット 
    Vector3 _r(SIM_INIT_R * cos(SIM_INIT_TH) + SIM_INIT_X, SIM_INIT_R * sin(SIM_INIT_TH), def_spec.boom_high - SIM_INIT_L);  //吊点位置
    Vector3 _v(0.0, 0.0, 0.0);                          //吊点位速度
    pLoad->init_mob(SYSTEM_TICK_ms / 1000.0, _r, _v);
    pLoad->set_m(SIM_INIT_M);


    //振れ角計算用カメラパラメータセット
    for (int j = 0;j < SWAY_SENSOR_N_AXIS;j++)
    {
            double D0 = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_D0];
            double H0 = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_H0];
            double l0 = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_l0];

            SwayCamParam[j][CAM_SET_PARAM_a] = sqrt(D0 * D0 + (H0 - l0) * (H0 - l0));
            double tempd = H0 - l0; if (tempd < 0.0) tempd *= -1.0;
            if (tempd < 0.000001) {
                SwayCamParam[j][CAM_SET_PARAM_b] = 0.0;
            }
            else {
                SwayCamParam[j][CAM_SET_PARAM_b] = atan(D0 / (H0 - l0));
            }
            SwayCamParam[j][CAM_SET_PARAM_c] = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_ph0];
            SwayCamParam[j][CAM_SET_PARAM_d] = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_PIXlRAD];//rad→pix変換係数
     }

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
// output() 振れセンサ信号セット
//*********************************************************************************************

static double thcamx_last=0, thcamy_last=0;

int CSIM::set_sway_io() {
      
    // 傾斜計検出角度
    double tilt_x = 0.0;
    double tilt_y = 0.0;
    sim_stat_workbuf.rcv_msg.head.tilt_x = (UINT32)(tilt_x * 1000000.0);
    sim_stat_workbuf.rcv_msg.head.tilt_y = (UINT32)(tilt_y * 1000000.0);
    
    // クレーンxy座標をカメラxy座標に回転変換　→　角度radに変換　
    double th = pCrane->r0[ID_SLEW];//旋回角度
    double thx = asin(((pLoad->L.x) * sin(th) + (pLoad->L.y) * -cos(th)) / pCrane->l_mh);
    double thy = asin(((pLoad->L.x) * cos(th) + (pLoad->L.y) * sin(th)) / pCrane->l_mh);

   
    // カメラ取付オフセット値の計算
    double ax = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_a];//センサ検出角補正値
    double bx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_b];//センサ検出角補正値
    double cx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_c];//センサ検出角補正値
    double dx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_d];//rad→pix変換係数
    double ay = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_a];//センサ検出角補正値
    double by = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_b];//センサ検出角補正値
    double cy = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_c];//センサ検出角補正値
    double dy = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_d];//rad→pix変換係数
    double L = pCraneStat->mh_l;
    double T = pCraneStat->T;
    double w = pCraneStat->w;

    double phx = tilt_x + cx;
    double phy = tilt_y + cy;
    double offset_thx = asin(ax * sin(phx + bx) / L);
    double offset_thy = asin(ay * sin(phy + by) / L);

 
    //カメラ検出角度rad
    double th_camx = thx - offset_thx - phx;
    double th_camy = thy - offset_thy - phy;
    //カメラ検出角速度rad
    double dth_camx = (th_camx - thcamx_last) / pCrane->dt;
    double dth_camy = (th_camy - thcamy_last) / pCrane->dt;

    thcamx_last = th_camx;
    thcamy_last = th_camy;

    //カメラ検出角度pix
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].th_x = (INT32)(th_camx * dx);
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].th_y = (INT32)(th_camy * dy);
    //カメラ検出角度pix
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].dth_x = (INT32)(dth_camx * dx);
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].dth_y = (INT32)(dth_camy * dy);
    
    //シミュレータロジックチェック用バッファセット
    sim_stat_workbuf.sway_io.th[ID_SLEW] = thx;
    sim_stat_workbuf.sway_io.th[ID_BOOM_H] = thy;
    sim_stat_workbuf.sway_io.dth[ID_SLEW] = dth_camx;
    sim_stat_workbuf.sway_io.dth[ID_BOOM_H] = dth_camy;
    
    return 0;
}
