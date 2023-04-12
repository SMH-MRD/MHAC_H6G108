#include "CPLC_IF.h"
#include "PLC_IF.h"
#include "PLC_IO_DEF.h"
#include "CWorkWindow_PLC.h"
#include <windows.h>
#include "Mdfunc.h"

extern ST_SPEC def_spec;
extern ST_KNL_MANAGE_SET    knl_manage_set;

CPLC_IF::CPLC_IF() {
    // 共有メモリオブジェクトのインスタンス化
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;
    pCSInfObj = new CSharedMem;

    out_size = 0;
    memset(&melnet,0,sizeof(ST_MELSEC_NET)) ;      //PLCリンク構造体
    memset(&plc_io_workbuf,0,sizeof(ST_PLC_IO));   //共有メモリへの出力セット作業用バッファ

    melnet.chan = MELSEC_NET_CH;
    melnet.mode = 0;
    melnet.path = NULL;
    melnet.err = 0;
    melnet.status = 0;
    melnet.retry_cnt = MELSEC_NET_RETRY_CNT;
    melnet.read_size_b = sizeof(ST_PLC_READ_B);                         //PLCでLWのbitでセットする為LBは未使用
    melnet.read_size_w = sizeof(ST_PLC_READ_W);
    melnet.write_size_b = sizeof(ST_PLC_WRITE_B);
    melnet.write_size_w = sizeof(ST_PLC_WRITE_W);

};
CPLC_IF::~CPLC_IF() {
    // 共有メモリオブジェクトの解放
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pAgentInfObj;
    delete pCSInfObj;
};

int CPLC_IF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //出力バッファセット

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CPLC_IF::init_proc() {

    // 共有メモリ取得

     // 出力用共有メモリ取得
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, (DWORD)out_size, MUTEX_PLC_IO_NAME)) {
        mode |= PLC_IF_PLC_IO_MEM_NG;
    }
    set_outbuf(pPLCioObj->get_pMap());
 
    // 入力用共有メモリ取得
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= PLC_IF_SIM_MEM_NG;
    }
    pSim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();                        //シミュレータ共有メモリ

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= PLC_IF_CRANE_MEM_NG;
    }
    pCrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();                                //クレーンステータス共有メモリ

    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)){
        mode |= PLC_IF_AGENT_MEM_NG;
    }
    pAgentInf = (LPST_AGENT_INFO)pAgentInfObj->get_pMap();                                  //Agentコントロール共有メモリ

    if (OK_SHMEM != pCSInfObj->create_smem(SMEM_CS_INFO_NAME, sizeof(ST_CS_INFO), MUTEX_CS_INFO_NAME)) {
        mode |= PLC_IF_CS_MEM_NG;
    }

    pCSInf = (LPST_CS_INFO)pCSInfObj->get_pMap();                                           //CS共有メモリ

    for (int i = 0;i < 4;i++) {
        this->melnet.is_force_set_active[i] = false;
        this->melnet.forced_dat[i] = 0;
        this->melnet.forced_index[i] = 0;
    }

    //PLCアドレスマップセット
    ST_PLC_OUT_BMAP plc_out_b_map;
    melnet.plc_b_map = plc_out_b_map;
    ST_PLC_OUT_WMAP plc_out_w_map;
    melnet.plc_w_map = plc_out_w_map;
    ST_PC_OUT_BMAP pc_out_b_map;
    melnet.pc_b_map = pc_out_b_map;
    ST_PC_OUT_WMAP pc_out_w_map;
    melnet.pc_w_map = pc_out_w_map;

    //CraneStat立ち上がり待ち
    while (pCrane->is_tasks_standby_ok == false) {
        Sleep(10);
    }

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CPLC_IF::input() {
    
    plc_io_workbuf.helthy_cnt++;

        //PLC 入力処理
        //MELSECNET回線確認
    if ((!melnet.status) && (!(plc_io_workbuf.helthy_cnt % melnet.retry_cnt))) {
        if (!(melnet.err = mdOpen(melnet.chan, melnet.chan, &melnet.path)))
            melnet.status = MELSEC_NET_OK;
    }
        //PLC Read
    if (melnet.status == MELSEC_NET_OK) {
        //LB読み込み　無し

        //B読み込み
        melnet.err = mdReceiveEx(melnet.path,   //チャネルのパス
            MELSEC_NET_NW_NO,                   //ネットワーク番号           
            MELSEC_NET_SOURCE_STATION,          //局番
            MELSEC_NET_CODE_LB,                 //デバイスタイプ
            MELSEC_NET_B_READ_START,            //先頭デバイス
            &melnet.read_size_b,                //読み込みバイトサイズ
            melnet.plc_b_out);                   //読み込みバッファ

         //W読み込み
        melnet.err = mdReceiveEx(melnet.path,    //チャネルのパス
            MELSEC_NET_NW_NO,                   //ネットワーク番号      
            MELSEC_NET_SOURCE_STATION,          //局番
            MELSEC_NET_CODE_LW,                 //デバイスタイプ
            MELSEC_NET_W_READ_START,            //先頭デバイス
            &melnet.read_size_w,                //読み込みバイトサイズ
            melnet.plc_w_out);     //読み込みバッファ
        if (melnet.err != 0)melnet.status = MELSEC_NET_RECEIVE_ERR;
    }
    //強制セット
    if (melnet.is_force_set_active[MEL_FORCE_PLC_B])melnet.plc_b_out[melnet.forced_index[MEL_FORCE_PLC_B]] = melnet.forced_dat[MEL_FORCE_PLC_B];
    if (melnet.is_force_set_active[MEL_FORCE_PLC_W])melnet.plc_w_out[melnet.forced_index[MEL_FORCE_PLC_W]] = melnet.forced_dat[MEL_FORCE_PLC_W];
      
    //MAINプロセス(Environmentタスクのヘルシー信号取り込み）
    source_counter = pCrane->env_act_count;

     return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CPLC_IF::parse() { 

    CWorkWindow_PLC* pWorkWindow;

    //### ヘルシー信号
    helthy_cnt++;

    //### PLCリンク入力を解析
    parse_notch_com();
    

    //運転室操作内容 
    //PLC IFからの入力信号取り込み（plc_io_workbuf)
    parse_ope_com();
    //デバッグモードの時、デバッグ用操作ウィンドからの入力値で上書き
    if( is_debug_mode() && (pWorkWindow->stOpePaneStat.chk_input_disable == FALSE)) set_debug_status();
   
    //### センサ検出内容取込
    //PLC IFからの入力信号取り込み（plc_io_workbuf)
    parse_sensor_fb();
    //シミュレーションモード時シミュレーションの結果で上書き
#ifdef _DVELOPMENT_MODE
    if (pSim->mode & SIM_ACTIVE_MODE) {
        set_sim_status();
    }
#endif

    //### PLCへの出力信号バッファセット
    
    set_notch_ref();  //ノッチ出力信号セット
    set_bit_coms();   //ビット出力信号セット
    set_ao_coms();    //アナログ出力信号セット

    return 0; 
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CPLC_IF::output() { 
 
    plc_io_workbuf.mode = this->mode;                   //モードセット
    plc_io_workbuf.helthy_cnt = my_helthy_counter++;    //ヘルシーカウンタセット
    
    //共有メモリ出力処理
    if(out_size) { 
        memcpy_s(poutput, out_size, &plc_io_workbuf, out_size);
    }
 
    //強制セット
    if (melnet.is_force_set_active[MEL_FORCE_PC_B])melnet.pc_b_out[melnet.forced_index[MEL_FORCE_PC_B]] = melnet.forced_dat[MEL_FORCE_PC_B];
    if (melnet.is_force_set_active[MEL_FORCE_PC_W])melnet.pc_w_out[melnet.forced_index[MEL_FORCE_PC_W]] = melnet.forced_dat[MEL_FORCE_PC_W];
  
    if (melnet.is_forced_pc_ctrl) melnet.pc_b_out[melnet.pc_b_map.com_pc_ctr_act[ID_WPOS]] |= melnet.pc_b_map.com_pc_ctr_act[ID_BPOS];
    if (melnet.is_forced_emulate) melnet.pc_b_out[melnet.pc_b_map.com_plc_emulate_act[ID_WPOS]] |= melnet.pc_b_map.com_plc_emulate_act[ID_BPOS];
   


    //MELSECNETへの出力処理
    if (melnet.status == MELSEC_NET_OK) {
        //LB書き込み
 //       melnet.pc_b_out[15] = 0xaaaa;
        melnet.err = mdSendEx(melnet.path,  //チャネルのパス
            MELSEC_NET_MY_NW_NO,            //ネットワーク番号   
            MELSEC_NET_MY_STATION,          //局番
            MELSEC_NET_CODE_LB,             //デバイスタイプ
            MELSEC_NET_B_WRITE_START,       //先頭デバイス
            &melnet.write_size_b,           //書き込みバイトサイズ
            melnet.pc_b_out); //ソースバッファ
        //LW書き込み
 //       melnet.pc_w_out[79] = 0xbbbb;
        melnet.err = mdSendEx(melnet.path,  //チャネルのパス
            MELSEC_NET_MY_NW_NO,            //ネットワーク番号  
            MELSEC_NET_MY_STATION,          //局番
            MELSEC_NET_CODE_LW,             //デバイスタイプ
            MELSEC_NET_W_WRITE_START,       //先頭デバイス
            &melnet.write_size_w,           //書き込みバイトサイズ
            melnet.pc_w_out); //ソースバッファ

        if (melnet.err < 0)melnet.status = MELSEC_NET_SEND_ERR;
    }

    return 0;
}
//*********************************************************************************************
// set_debug_status()
// デバッグモード（デバッグ用操作ウィンド）入力内容セット
//*********************************************************************************************
int CPLC_IF::set_debug_status() {
    
    CWorkWindow_PLC* pWorkWindow;

    plc_io_workbuf.ui.notch_pos[ID_HOIST]       = pWorkWindow->stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_GANTRY]      = pWorkWindow->stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_BOOM_H]      = pWorkWindow->stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH;
    plc_io_workbuf.ui.notch_pos[ID_SLEW]        = -(pWorkWindow->stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);//実機とノッチの＋方向が逆

    plc_io_workbuf.ui.PB[ID_PB_ESTOP] = pWorkWindow->stOpePaneStat.check_estop;

    plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON]     = pWorkWindow->stOpePaneStat.button_antisway;
    plc_io_workbuf.ui.PB[ID_PB_REMOTE_MODE]     = pWorkWindow->stOpePaneStat.check_remote_mode;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_START]      = pWorkWindow->stOpePaneStat.check_auto_start;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_RESET]      = pWorkWindow->stOpePaneStat.button_auto_reset;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_MODE]       = pWorkWindow->stOpePaneStat.button_auto_mode;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_Z]      = pWorkWindow->stOpePaneStat.button_set_z;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_XY]     = pWorkWindow->stOpePaneStat.button_set_xy;


    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S1]  = pWorkWindow->stOpePaneStat.check_s1;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S2]  = pWorkWindow->stOpePaneStat.check_s2;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S3]  = pWorkWindow->stOpePaneStat.check_s3;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L1]  = pWorkWindow->stOpePaneStat.check_l1;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L2]  = pWorkWindow->stOpePaneStat.check_l2;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L3]  = pWorkWindow->stOpePaneStat.check_l3;

    
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON]  = pWorkWindow->stOpePaneStat.check_source1_on;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = pWorkWindow->stOpePaneStat.check_source1_off;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_ON] = pWorkWindow->stOpePaneStat.check_source2_on;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_OFF]= pWorkWindow->stOpePaneStat.check_source2_off;

    plc_io_workbuf.ui.PB[ID_PB_FAULT_RESET]     = pWorkWindow->stOpePaneStat.button_fault_reset;

    plc_io_workbuf.ui.PB[ID_PB_MH_P1]           = pWorkWindow->stOpePaneStat.button_mh_p1;
    plc_io_workbuf.ui.PB[ID_PB_MH_P2]           = pWorkWindow->stOpePaneStat.button_mh_p2;
    plc_io_workbuf.ui.PB[ID_PB_MH_M1]           = pWorkWindow->stOpePaneStat.button_mh_m1;
    plc_io_workbuf.ui.PB[ID_PB_MH_M2]           = pWorkWindow->stOpePaneStat.button_mh_m2;
    plc_io_workbuf.ui.PB[ID_PB_SL_P1]           = pWorkWindow->stOpePaneStat.button_sl_p1;
    plc_io_workbuf.ui.PB[ID_PB_SL_P2]           = pWorkWindow->stOpePaneStat.button_sl_p2;
    plc_io_workbuf.ui.PB[ID_PB_SL_M1]           = pWorkWindow->stOpePaneStat.button_sl_m1;
    plc_io_workbuf.ui.PB[ID_PB_SL_M2]           = pWorkWindow->stOpePaneStat.button_sl_m2;
    plc_io_workbuf.ui.PB[ID_PB_BH_P1]           = pWorkWindow->stOpePaneStat.button_bh_p1;
    plc_io_workbuf.ui.PB[ID_PB_BH_P2]           = pWorkWindow->stOpePaneStat.button_bh_p2;
    plc_io_workbuf.ui.PB[ID_PB_BH_M1]           = pWorkWindow->stOpePaneStat.button_bh_m1;
    plc_io_workbuf.ui.PB[ID_PB_BH_M2]           = pWorkWindow->stOpePaneStat.button_bh_m2;

    plc_io_workbuf.ui.PB[ID_PB_PARK]            = pWorkWindow->stOpePaneStat.button_park;
    plc_io_workbuf.ui.PB[ID_PB_GRND]            = pWorkWindow->stOpePaneStat.button_grnd;
    plc_io_workbuf.ui.PB[ID_PB_PICK]            = pWorkWindow->stOpePaneStat.button_pick;

    return 0;
}


//*********************************************************************************************
// closeIF()
//*********************************************************************************************
int CPLC_IF::closeIF() {

   //MELSECNET回線クローズ
        melnet.err = mdClose(melnet.path);
        melnet.status = MELSEC_NET_CLOSE;
   return 0;
}

//*********************************************************************************************
// set_notch_ref()
// AGENTタスクの速度指令をノッチ位置指令に変換してIO出力を設定
//*********************************************************************************************
int CPLC_IF::set_notch_ref() {

    //巻ノッチ
    //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_0[ID_WPOS]] &= NOTCH_PTN0_CLR;

    if ((pAgentInf->v_ref[ID_HOIST] < (def_spec.notch_spd_r[ID_HOIST][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_HOIST] > (def_spec.notch_spd_f[ID_HOIST][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
        if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_4]) {         //-5ノッチ未満
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f1[ID_BPOS];
            }
        }
    }
    else {//0ノッチ
        melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_0[ID_BPOS];
    }
 
    //走行ノッチ
   //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_0[ID_WPOS]] &= NOTCH_PTN1_CLR;

    if ((pAgentInf->v_ref[ID_GANTRY] < (def_spec.notch_spd_r[ID_GANTRY][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_GANTRY] > (def_spec.notch_spd_f[ID_GANTRY][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
        if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_r1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_GANTRY] > def_spec.notch_spd_f[ID_GANTRY][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_GANTRY] > def_spec.notch_spd_f[ID_GANTRY][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] > def_spec.notch_spd_f[ID_GANTRY][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] > def_spec.notch_spd_f[ID_GANTRY][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_GANTRY] > def_spec.notch_spd_f[ID_GANTRY][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_f1[ID_BPOS];
            }
        }
    }
    else {//0ノッチ
        melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_0[ID_BPOS];
    }
 
    //引込ノッチ
    //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_0[ID_WPOS]] &= NOTCH_PTN0_CLR;

    if ((pAgentInf->v_ref[ID_BOOM_H] < (def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_BOOM_H] > (def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
       
        //!!引込は制御PC（出＋）速度が逆転になるのでノッチのセットパターンが逆になる

        if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r1[ID_BPOS];
            }
        }
    }
    else {//0ノッチ
        melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_0[ID_BPOS];
    }

    //旋回ノッチ
    //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_0[ID_WPOS]] &= NOTCH_PTN1_CLR;

    //!!旋回は制御PC（右回＋）速度が逆転になるのでノッチのセットパターンが逆になる
    if ((pAgentInf->v_ref[ID_SLEW] < (def_spec.notch_spd_r[ID_SLEW][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_SLEW] > (def_spec.notch_spd_f[ID_SLEW][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
 
        if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_1]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r1[ID_BPOS];
            }
        }
    }
    else {//0ノッチ
        melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_0[ID_BPOS];
    }
     
    return 0;
}

//*********************************************************************************************
//set_bit_coms()
//CSランプ表示, AGENTタスク, 操作パネルのビット指令に応じてIO出力を設定
//*********************************************************************************************
int CPLC_IF::set_bit_coms() {
    CWorkWindow_PLC* pWorkWindow;
    
    //正常クロック システムカウンタを利用 25msec counter 64*0.025=1.6
    if (knl_manage_set.sys_counter& 0x40) melnet.pc_b_out[melnet.pc_b_map.healty[ID_WPOS]] |= melnet.pc_b_map.healty[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.healty[ID_WPOS]] &= ~melnet.pc_b_map.healty[ID_BPOS];


    //制御PC制御指令動作ビット
    if (pAgentInf->auto_active[ID_SLEW] || pAgentInf->auto_active[ID_BOOM_H])
        melnet.pc_b_out[melnet.pc_b_map.com_pc_ctr_act[ID_WPOS]] |= melnet.pc_b_map.com_pc_ctr_act[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_pc_ctr_act[ID_WPOS]] &= ~melnet.pc_b_map.com_pc_ctr_act[ID_BPOS];

    //制御PCからのエミュレータ指令ビット
    if (pSim->mode & SIM_ACTIVE_MODE)
        melnet.pc_b_out[melnet.pc_b_map.com_plc_emulate_act[ID_WPOS]] |= melnet.pc_b_map.com_plc_emulate_act[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_plc_emulate_act[ID_WPOS]] &= ~melnet.pc_b_map.com_plc_emulate_act[ID_BPOS];

    //非常停止PB
    if (pAgentInf->PLC_PB_com[ID_PB_ESTOP]) melnet.pc_b_out[melnet.pc_b_map.com_estop[ID_WPOS]] |= melnet.pc_b_map.com_estop[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_estop[ID_WPOS]] &= ~melnet.pc_b_map.com_estop[ID_BPOS];
    //主幹1入
    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE_ON]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_on[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source_on[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_on[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source_on[ID_BPOS];
    //主幹1切
    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_off[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source_off[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_off[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source_off[ID_BPOS];
    //主幹2入
    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE2_ON]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_on[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source2_on[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_on[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source2_on[ID_BPOS];
    //主幹2切
    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_off[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source2_off[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_off[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source2_off[ID_BPOS];

    //故障リセット
    if (pAgentInf->PLC_PB_com[ID_PB_FAULT_RESET]) melnet.pc_b_out[melnet.pc_b_map.com_fault_reset[ID_WPOS]] |= melnet.pc_b_map.com_fault_reset[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_fault_reset[ID_WPOS]] &= ~melnet.pc_b_map.com_fault_reset[ID_BPOS];

    //PC speed ref mode
    if (pWorkWindow->stOpePaneStat.chk_PC_ref_spd) melnet.pc_b_out[melnet.pc_b_map.com_pc_analog_ref_mode[ID_WPOS]] |= melnet.pc_b_map.com_pc_analog_ref_mode[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_pc_analog_ref_mode[ID_WPOS]] &= ~melnet.pc_b_map.com_pc_analog_ref_mode[ID_BPOS];

    //Sim fb
    if (pWorkWindow->stOpePaneStat.chk_sim_fb) melnet.pc_b_out[melnet.pc_b_map.com_pc_fb[ID_WPOS]] |= melnet.pc_b_map.com_pc_fb[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_pc_fb[ID_WPOS]] &= ~melnet.pc_b_map.com_pc_fb[ID_BPOS];
   
    //ランプ類

    //振れ止めOFFランプ
    if (pCSInf->plc_lamp[ID_PB_ANTISWAY_OFF])
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_off[ID_WPOS]] |= melnet.pc_b_map.lamp_as_off[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_off[ID_WPOS]] &= ~melnet.pc_b_map.lamp_as_off[ID_BPOS];
    //振れ止めONランプ
    if (pCSInf->plc_lamp[ID_PB_ANTISWAY_ON])
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_on[ID_WPOS]] |= melnet.pc_b_map.lamp_as_on[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_on[ID_WPOS]] &= ~melnet.pc_b_map.lamp_as_on[ID_BPOS];

    //自動モードランプ
    if (pCSInf->plc_lamp[ID_PB_AUTO_MODE])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_mode[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_mode[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_mode[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_mode[ID_BPOS];

    //自動起動ランプ
    if (pCSInf->plc_lamp[ID_PB_AUTO_START])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_start[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_start[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_start[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_start[ID_BPOS];

    //自動コマンドランプ
    if (pCSInf->plc_lamp[ID_PB_PARK])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_park[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_park[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_park[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_park[ID_BPOS];
    if (pCSInf->plc_lamp[ID_PB_PICK])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_pick[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_pick[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_pick[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_pick[ID_BPOS];
    if (pCSInf->plc_lamp[ID_PB_GRND])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_grnd[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_grnd[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_grnd[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_grnd[ID_BPOS];

 
    //目標位置セットランプ
    if (pCSInf->plc_lamp[ID_PB_AUTO_SET_Z])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_set_z[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_set_z[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_set_z[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_set_z[ID_BPOS];

    if (pCSInf->plc_lamp[ID_PB_AUTO_SET_XY])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_set_xy[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_set_xy[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_set_xy[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_set_xy[ID_BPOS];

    
    //半自動ランプ
    if (pCSInf->semiauto_lamp[SEMI_AUTO_S1])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s1[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_s1[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s1[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_s1[ID_BPOS];

    if (pCSInf->semiauto_lamp[SEMI_AUTO_S2])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s2[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_s2[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s2[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_s2[ID_BPOS];

    if (pCSInf->semiauto_lamp[SEMI_AUTO_S3])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s3[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_s3[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_s3[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_s3[ID_BPOS];

    if (pCSInf->semiauto_lamp[SEMI_AUTO_L1])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l1[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_l1[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l1[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_l1[ID_BPOS];

    if (pCSInf->semiauto_lamp[SEMI_AUTO_L2])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l2[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_l2[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l2[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_l2[ID_BPOS];

    if (pCSInf->semiauto_lamp[SEMI_AUTO_L3])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l3[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg_l3[ID_BPOS];
    else 
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg_l3[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg_l3[ID_BPOS];

    //自動コマンドランプ
    if (pCSInf->plc_lamp[ID_PB_PICK])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_pick[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_pick[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_pick[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_pick[ID_BPOS];

    if (pCSInf->plc_lamp[ID_PB_GRND])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_grnd[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_grnd[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_grnd[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_grnd[ID_BPOS];

    if (pCSInf->plc_lamp[ID_PB_PARK])
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_park[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_park[ID_BPOS];
    else
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_park[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_park[ID_BPOS];
   
    return 0;
}

//*********************************************************************************************
//set_ao_coms()
// AGENTタスクのアナログ指令、ヘルシー信号等の出力セット
//*********************************************************************************************
int CPLC_IF::set_ao_coms() {
 
    melnet.pc_w_out[melnet.pc_w_map.helthy[ID_WPOS]] = helthy_cnt;

    //速度指令アナログ　0.1%単位）
    //引込は入が＋出が-
    melnet.pc_w_out[melnet.pc_w_map.spd_ref_bh[ID_WPOS]] = (INT16)(-1000 * pAgentInf->v_ref[ID_BOOM_H]/pCrane->spec.notch_spd_f[ID_BOOM_H][NOTCH_5]);
    //旋回は左が-右が+
    melnet.pc_w_out[melnet.pc_w_map.spd_ref_slw[ID_WPOS]] = (INT16)(-1000 * pAgentInf->v_ref[ID_SLEW] / pCrane->spec.notch_spd_f[ID_SLEW][NOTCH_5]);
    melnet.pc_w_out[melnet.pc_w_map.spd_ref_hst[ID_WPOS]] = (INT16)(1000 * pAgentInf->v_ref[ID_HOIST] / pCrane->spec.notch_spd_f[ID_HOIST][NOTCH_5]);
    melnet.pc_w_out[melnet.pc_w_map.spd_ref_gnt[ID_WPOS]] = (INT16)(1000 * pAgentInf->v_ref[ID_GANTRY] / pCrane->spec.notch_spd_f[ID_GANTRY][NOTCH_5]);

    return 0;
}

//*********************************************************************************************
// parse_notch_com()
// UIノッチ指令読み込み
//*********************************************************************************************
int CPLC_IF::parse_notch_com() {
    
    INT16 check_i;
 
    //巻きノッチ
    //巻に対応するbitを抽出
    check_i = melnet.plc_w_out[melnet.plc_w_map.com_hst_notch_0[ID_WPOS]] & NOTCH_PTN0_ALL;

    if(check_i & NOTCH_PTN0_0) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_0;             //0ノッチ
    else if(check_i & NOTCH_PTN0_F1) {                                                      //正転ビット（1ノッチ）ON
      if(check_i == NOTCH_PTN0_F5) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_5;         //以下ビットパターンを照合
      else if (check_i == NOTCH_PTN0_F4) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_4;
      else if (check_i == NOTCH_PTN0_F3) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_3;
      else if (check_i == NOTCH_PTN0_F2) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_2;
      else plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_1;
    }
    else if (check_i & NOTCH_PTN0_R1) {                                                     //逆転ビット（1ノッチ）ON
      if (check_i == NOTCH_PTN0_R5) plc_io_workbuf.ui.notch_pos[ID_HOIST] = -NOTCH_5;       //以下ビットパターンを照合
      else if (check_i == NOTCH_PTN0_R4) plc_io_workbuf.ui.notch_pos[ID_HOIST] = -NOTCH_4;
      else if (check_i == NOTCH_PTN0_R3) plc_io_workbuf.ui.notch_pos[ID_HOIST] = -NOTCH_3;
      else if (check_i == NOTCH_PTN0_R2) plc_io_workbuf.ui.notch_pos[ID_HOIST] =- NOTCH_2;
      else plc_io_workbuf.ui.notch_pos[ID_HOIST] = -NOTCH_1;
    }
    else plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_0;
        
    //走行ノッチ
    check_i = melnet.plc_w_out[melnet.plc_w_map.com_gnt_notch_0[ID_WPOS]] & NOTCH_PTN1_ALL;

    if (check_i & NOTCH_PTN1_0) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_0;
    else if (check_i & NOTCH_PTN1_F1) {
        if (check_i == NOTCH_PTN1_F5) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_5;
        else if (check_i == NOTCH_PTN1_F4) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_4;
        else if (check_i == NOTCH_PTN1_F3) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_3;
        else if (check_i == NOTCH_PTN1_F2) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_1;
    }
    else if (check_i & NOTCH_PTN1_R1) {
        if (check_i == NOTCH_PTN1_R5) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = -NOTCH_5;
        else if (check_i == NOTCH_PTN1_R4) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = -NOTCH_4;
        else if (check_i == NOTCH_PTN1_R3) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = -NOTCH_3;
        else if (check_i == NOTCH_PTN1_R2) plc_io_workbuf.ui.notch_pos[ID_GANTRY] = -NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_GANTRY] = -NOTCH_1;
    }
    else plc_io_workbuf.ui.notch_pos[ID_GANTRY] = NOTCH_0;
    
    //引込ノッチ
    check_i = melnet.plc_w_out[melnet.plc_w_map.com_bh_notch_0[ID_WPOS]] & NOTCH_PTN0_ALL;

    if (check_i & NOTCH_PTN0_0) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_0;
    else if (check_i & NOTCH_PTN0_F1) {
        if (check_i == NOTCH_PTN0_F5) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_5;
        else if (check_i == NOTCH_PTN0_F4) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_4;
        else if (check_i == NOTCH_PTN0_F3) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_3;
        else if (check_i == NOTCH_PTN0_F2) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_1;
    }
    else if (check_i & NOTCH_PTN0_R1) {
        if (check_i == NOTCH_PTN0_R5) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = -NOTCH_5;
        else if (check_i == NOTCH_PTN0_R4) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = -NOTCH_4;
        else if (check_i == NOTCH_PTN0_R3) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = -NOTCH_3;
        else if (check_i == NOTCH_PTN0_R2) plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = -NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = -NOTCH_1;
    }
    else plc_io_workbuf.ui.notch_pos[ID_BOOM_H] = NOTCH_0;

    //旋回ノッチ
    check_i = melnet.plc_w_out[melnet.plc_w_map.com_gnt_notch_0[ID_WPOS]] & NOTCH_PTN1_ALL;

    if (check_i & NOTCH_PTN1_0) plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_0;
    else if (check_i & NOTCH_PTN1_F1) {
        if (check_i == NOTCH_PTN1_F5) plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_5;
        else if (check_i == NOTCH_PTN1_F4) plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_4;
        else if (check_i == NOTCH_PTN1_F3) plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_3;
        else if (check_i == NOTCH_PTN1_F2) plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_1;
    }
    else if (check_i & NOTCH_PTN1_R1) {
        if (check_i == NOTCH_PTN1_R5) plc_io_workbuf.ui.notch_pos[ID_SLEW] = -NOTCH_5;
        else if (check_i == NOTCH_PTN1_R4) plc_io_workbuf.ui.notch_pos[ID_SLEW] = -NOTCH_4;
        else if (check_i == NOTCH_PTN1_R3) plc_io_workbuf.ui.notch_pos[ID_SLEW] = -NOTCH_3;
        else if (check_i == NOTCH_PTN1_R2) plc_io_workbuf.ui.notch_pos[ID_SLEW] = -NOTCH_2;
        else plc_io_workbuf.ui.notch_pos[ID_SLEW] = -NOTCH_1;
    }
    else plc_io_workbuf.ui.notch_pos[ID_SLEW] = NOTCH_0;
    
    return 0;

}

//*********************************************************************************************
// parse_ope_com()
// 運転室操作信号取り込み
//*********************************************************************************************
int CPLC_IF::parse_ope_com() {

    //PB取り込みはOFF Delay ON検出でカウント値セット→0まで減算
    
    //既設PB類はWレジスタより
    if (melnet.plc_w_out[melnet.plc_w_map.com_estop[ID_WPOS]] & melnet.plc_w_map.com_estop[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_ESTOP] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_ESTOP] > 0) plc_io_workbuf.ui.PB[ID_PB_ESTOP]--;
    else;

    if (melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source_on[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source_on[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON] > 0) plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON]--;
    else;

    if (melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source_off[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source_off[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] > 0)plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF]--;
    else;

    if (melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source_off[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source_off[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] > 0)plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF]--;
    else;

    if (melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source2_on[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source2_on[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_ON] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_ON] > 0)plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_ON]--;
    else;

    if (melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source2_off[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source2_off[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_OFF] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_OFF] > 0)plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_OFF]--;
    else;

 
      
    //追加PB類はBレジスタより
    if (melnet.plc_b_out[melnet.plc_b_map.PB_as_on[ID_WPOS]] & melnet.plc_b_map.PB_as_on[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON] > 0)plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_as_off[ID_WPOS]] & melnet.plc_b_map.PB_as_off[ID_BPOS])  plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF] > 0)plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_start[ID_WPOS]] & melnet.plc_b_map.PB_auto_start[ID_BPOS]) plc_io_workbuf.ui.PB[ID_PB_AUTO_START] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_AUTO_START] > 0)plc_io_workbuf.ui.PB[ID_PB_AUTO_START]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_s1[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_s1[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S1] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S1] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S1]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_s2[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_s2[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S2] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S2] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S2]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_s3[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_s3[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S3] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S3] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_S3]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_set_z[ID_WPOS]] & melnet.plc_b_map.PB_auto_set_z[ID_BPOS]) plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_Z] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_Z] > 0)plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_Z]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_l1[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_l1[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L1] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L1] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L1]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_l2[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_l2[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L2] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L2] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L2]--;
    else;

    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_target_l3[ID_WPOS]] & melnet.plc_b_map.PB_auto_target_l3[ID_BPOS]) plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L3] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L3] > 0)plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_L3]--;
    else;
 
    if (melnet.plc_b_out[melnet.plc_b_map.PB_auto_set_xy[ID_WPOS]] & melnet.plc_b_map.PB_auto_set_xy[ID_BPOS]) plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_XY] = PLC_IO_OFF_DELAY_COUNT;
    else if (plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_XY] > 0)plc_io_workbuf.ui.PB[ID_PB_AUTO_SET_XY]--;
    else;

    //機上モード
    if (melnet.plc_b_out[melnet.plc_b_map.PB_mode_crane[ID_WPOS]] & melnet.plc_b_map.PB_mode_crane[ID_BPOS]) plc_io_workbuf.ui.PB[ID_PB_CRANE_MODE] = L_ON;
    else  plc_io_workbuf.ui.PB[ID_PB_CRANE_MODE] = L_OFF;

    //遠隔モード
    if (melnet.plc_b_out[melnet.plc_b_map.PB_mode_remote[ID_WPOS]] & melnet.plc_b_map.PB_mode_remote[ID_BPOS]) plc_io_workbuf.ui.PB[ID_PB_REMOTE_MODE] = L_ON;
    else  plc_io_workbuf.ui.PB[ID_PB_REMOTE_MODE] = L_OFF;

    return 0;
}

//*********************************************************************************************
// set_sim_status()
//*********************************************************************************************
int CPLC_IF::set_sim_status() {

    plc_io_workbuf.status.v_fb[ID_HOIST] = pSim->status.v_fb[ID_HOIST];
    plc_io_workbuf.status.v_fb[ID_GANTRY] = pSim->status.v_fb[ID_GANTRY];
    plc_io_workbuf.status.v_fb[ID_BOOM_H] = pSim->status.v_fb[ID_BOOM_H];
    plc_io_workbuf.status.v_fb[ID_SLEW] = pSim->status.v_fb[ID_SLEW];

    plc_io_workbuf.status.pos[ID_HOIST] = pSim->status.pos[ID_HOIST];
    plc_io_workbuf.status.pos[ID_GANTRY] = pSim->status.pos[ID_GANTRY];
    plc_io_workbuf.status.pos[ID_BOOM_H] = pSim->status.pos[ID_BOOM_H];
    plc_io_workbuf.status.pos[ID_SLEW] = pSim->status.pos[ID_SLEW];

    return 0;
}

//*********************************************************************************************
// parse_sensor_fb()
// センサ信号取り込み
//*********************************************************************************************
int CPLC_IF::parse_sensor_fb() {

    plc_io_workbuf.status.v_fb[ID_HOIST] = def_spec.notch_spd_f[ID_HOIST][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_hst_fb[ID_WPOS]] / 1000.0;
    plc_io_workbuf.status.v_fb[ID_GANTRY] = def_spec.notch_spd_f[ID_GANTRY][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_gnt_fb[ID_WPOS]] / 1000.0;
    //引込は入が＋出が-で入ってくる
    plc_io_workbuf.status.v_fb[ID_BOOM_H] = -1.0 * def_spec.notch_spd_f[ID_BOOM_H][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_bh_fb[ID_WPOS]] / 1000.0;
    //旋回は左が-右が+で入ってくる
    plc_io_workbuf.status.v_fb[ID_SLEW] = -1.0 * def_spec.notch_spd_f[ID_SLEW][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_slw_fb[ID_WPOS]] / 1000.0;

    plc_io_workbuf.status.weight = (double)melnet.plc_w_out[melnet.plc_w_map.load_fb[ID_WPOS]] * 100.0; //Kg
    
    plc_io_workbuf.status.pos[ID_HOIST] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_hst_fb[ID_WPOS]] / 10.0;        //m　PLCからは0.1m単位）
    plc_io_workbuf.status.pos[ID_GANTRY] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_gnt_fb[ID_WPOS]] / 10.0;       //m　PLCからは0.1m単位）
  
    plc_io_workbuf.status.pos[ID_BOOM_H] =(double)melnet.plc_w_out[melnet.plc_w_map.pos_bh_fb[ID_WPOS]] / 10.0;   //m　PLCからは0.1m単位）
    plc_io_workbuf.status.pos[ID_SLEW] = (3600.0-(double)melnet.plc_w_out[melnet.plc_w_map.pos_slw_fb[ID_WPOS]]) * PI1DEG/10.0;  //rad PLCからは0.1deg単位）
    return 0;
}

int CPLC_IF::mel_set_force(int id, bool bset, int index, WORD value) {

    if (id < 0 || id >4) return 0;

    if (id == MEL_FORCE_RESET) {
        for (int i = 0;i < 4;i++) {
            melnet.is_force_set_active[i] = false;
            melnet.forced_index[i] = 0;
            melnet.forced_dat[i] = 0;
        }
    }
    else if (bset == true) {
        melnet.is_force_set_active[id] =true;
        melnet.forced_index[id] = index;
        melnet.forced_dat[id] = value;
    }
    else {
        melnet.is_force_set_active[id] = false;
        melnet.forced_index[id] = 0;
        melnet.forced_dat[id] = 0;
    }
    return 1;
}