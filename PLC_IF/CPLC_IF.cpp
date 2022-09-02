#include "CPLC_IF.h"
#include "PLC_IO_DEF.h"
#include "CWorkWindow_PLC.h"
#include <windows.h>
#include "Mdfunc.h"

extern ST_SPEC def_spec;

CPLC_IF::CPLC_IF() {
    // 共有メモリオブジェクトのインスタンス化
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    out_size = 0;
    memset(&melnet,0,sizeof(ST_MELSEC_NET)) ;      //PLCリンク構造体
    memset(&plc_io_workbuf,0,sizeof(ST_PLC_IO));   //共有メモリへの出力セット作業用バッファ

    melnet.chan = MELSEC_NET_CH;
    melnet.mode = 0;
    melnet.path = NULL;
    melnet.err = 0;
    melnet.status = 0;
    melnet.retry_cnt = MELSEC_NET_RETRY_CNT;
    melnet.read_size_b = 0;                         //PLCでLWのbitでセットする為LBは未使用
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
    pSim = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_pMap();

    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= PLC_IF_CRANE_MEM_NG;
    }
    pCrane = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap();

    if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)){
        mode |= PLC_IF_AGENT_MEM_NG;
    }

    pAgentInf = (LPST_AGENT_INFO)pAgentInfObj->get_pMap();

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
            &melnet.read_size_w,                //読み込みバイトサイズ
            melnet.plc_b_out);                   //読み込みバッファ

         //W読み込み
        melnet.err = mdReceiveEx(melnet.path,    //チャネルのパス
            MELSEC_NET_NW_NO,                   //ネットワーク番号      
            MELSEC_NET_SOURCE_STATION,          //局番
            MELSEC_NET_CODE_LW,                 //デバイスタイプ
            MELSEC_NET_W_READ_START,            //先頭デバイス
            &melnet.read_size_w,                //読み込みバイトサイズ
            melnet.pc_w_out);     //読み込みバッファ

        if (melnet.err != 0)melnet.status = MELSEC_NET_RECEIVE_ERR;
    }
      
    //MAINプロセス(Environmentタスクのヘルシー信号取り込み）
    source_counter = pCrane->env_act_count;

     return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CPLC_IF::parse() { 

    //PLCからの入力をオウム返しで出力（試験対応処理）
 //   memcpy_s(melnet.plc_w_buf_B.pc_com_buf, 16, melnet.plc_r_buf_B.spare, 16);     //Bレジスタ
 //   memcpy_s(melnet.plc_w_buf_W.pc_com_buf, 16, melnet.plc_r_buf_W.main_x_buf, 16); //Wレジスタ

    //### PLCリンク入力を解析
    parse_notch_com();
    

    //運転室操作内容 
    parse_ope_com();
    //デバッグモード時は、操作パネルウィンドウの内容を上書き 
    if (is_debug_mode()) set_debug_status();

    //### センサ検出内容取込
    parse_sensor_fb();
    //シミュレーションモード時シミュレーションの結果で上書き
#ifdef _DVELOPMENT_MODE
    if (pSim->mode & SIM_ACTIVE_MODE) {
        set_sim_status();
    }
#endif

    //### PLCへの出力信号バッファセット

    //ノッチ出力信号セット
    set_notch_ref();
    set_bit_coms();

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
 
    //MELSECNETへの出力処理
    if (melnet.status == MELSEC_NET_OK) {
        //LB書き込み
        melnet.err = mdSendEx(melnet.path,  //チャネルのパス
            MELSEC_NET_MY_NW_NO,            //ネットワーク番号   
            MELSEC_NET_MY_STATION,          //局番
            MELSEC_NET_CODE_LB,             //デバイスタイプ
            MELSEC_NET_B_WRITE_START,       //先頭デバイス
            &melnet.write_size_b,           //書き込みバイトサイズ
            melnet.pc_b_out); //ソースバッファ
        //LW書き込み
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

    if (pWorkWindow->stOpePaneStat.check_antisway) {
        plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON] = true; plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF] = false;
     }
    else {
        plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON] = false; plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF] |= true;
    }
    plc_io_workbuf.ui.PB[ID_PB_REMOTE_MODE] = pWorkWindow->stOpePaneStat.button_remote;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_START] = pWorkWindow->stOpePaneStat.button_auto_start;
    plc_io_workbuf.ui.PB[ID_PB_AUTO_RESET] = pWorkWindow->stOpePaneStat.button_auto_reset;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG1] = pWorkWindow->stOpePaneStat.button_from1;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG2] = pWorkWindow->stOpePaneStat.button_from2;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG3] = pWorkWindow->stOpePaneStat.button_from3;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG4] = pWorkWindow->stOpePaneStat.button_from4;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG5] = pWorkWindow->stOpePaneStat.button_to1;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG6] = pWorkWindow->stOpePaneStat.button_to2;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG7] = pWorkWindow->stOpePaneStat.button_to3;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG8] = pWorkWindow->stOpePaneStat.button_to4;
    
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON] = pWorkWindow->stOpePaneStat.button_source1_on;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = pWorkWindow->stOpePaneStat.button_source1_off;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON] = pWorkWindow->stOpePaneStat.button_source1_on;
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = pWorkWindow->stOpePaneStat.button_source1_off;
  
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
// AGENTタスクのノッチ位置指令に応じてIO出力を設定
//*********************************************************************************************
int CPLC_IF::set_notch_ref() {

    //巻ノッチ
    //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_0[ID_WPOS]] &= NOTCH_PTN0_CLR;

    if ((pAgentInf->v_ref[ID_HOIST] < (def_spec.notch_spd_r[ID_HOIST][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_HOIST] > (def_spec.notch_spd_f[ID_HOIST][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
        if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_r1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_f[ID_HOIST][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_HOIST] > def_spec.notch_spd_f[ID_HOIST][NOTCH_2]) {
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
        else if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_f[ID_GANTRY][NOTCH_1]) { //正転1ノッチ以上

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
        if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_r1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_BOOM_H] > def_spec.notch_spd_f[ID_BOOM_H][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_f1[ID_BPOS];
            }
        }
    }
    else {//0ノッチ
        melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_0[ID_BPOS];
    }

    //旋回ノッチ
    //ノッチクリア
    melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_0[ID_WPOS]] &= NOTCH_PTN1_CLR;

    if ((pAgentInf->v_ref[ID_SLEW] < (def_spec.notch_spd_r[ID_SLEW][NOTCH_1]))             //指令が-1ノッチより小
        || (pAgentInf->v_ref[ID_SLEW] > (def_spec.notch_spd_f[ID_SLEW][NOTCH_1]))) {       //指令が+1ノッチより大
        //ノッチセット
        if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_1]) {     //逆転1ノッチ以下

            if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r5[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r4[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r3[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r2[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_r1[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_r1[ID_BPOS];
            }
        }
        else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_f[ID_SLEW][NOTCH_1]) { //正転1ノッチ以上

            if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_5]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f5[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f5[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_4]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f4[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f4[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_3]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f3[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f3[ID_BPOS];
            }
            else if (pAgentInf->v_ref[ID_SLEW] > def_spec.notch_spd_f[ID_SLEW][NOTCH_2]) {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f2[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f2[ID_BPOS];
            }
            else {
                melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_f1[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_f1[ID_BPOS];
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
// AGENTタスクのビット指令に応じてIO出力を設定
//*********************************************************************************************
int CPLC_IF::set_bit_coms() {
    
    //非常停止PB
    if (pAgentInf->PLC_PB_com[ID_PB_ESTOP]) melnet.pc_b_out[melnet.pc_b_map.com_estop[ID_WPOS]] |= melnet.pc_b_map.com_estop[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_estop[ID_WPOS]] &= ~melnet.pc_b_map.com_estop[ID_BPOS];

    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE_ON]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_on[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source_on[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_on[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source_on[ID_BPOS];

    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_off[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source_off[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source_off[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source_off[ID_BPOS];

    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE2_ON]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_on[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source2_on[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_on[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source2_on[ID_BPOS];

    if (pAgentInf->PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF]) melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_off[ID_WPOS]] |= melnet.pc_b_map.com_ctrl_source2_off[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.com_ctrl_source2_off[ID_WPOS]] &= ~melnet.pc_b_map.com_ctrl_source2_off[ID_BPOS];

    
    //ランプ類
    if ((pAgentInf->PLC_PB_com[ID_PB_ANTISWAY_OFF]%PLC_IO_LAMP_FLICKER_COUNT) >= PLC_IO_LAMP_FLICKER_CHANGE) 
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_off[ID_WPOS]] |= melnet.pc_b_map.lamp_as_off[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_as_off[ID_WPOS]] &= ~melnet.pc_b_map.lamp_as_off[ID_BPOS];

    if ((pAgentInf->PLC_PB_com[ID_PB_ANTISWAY_ON] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_as_on[ID_WPOS]] |= melnet.pc_b_map.lamp_as_on[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_as_on[ID_WPOS]] &= ~melnet.pc_b_map.lamp_as_on[ID_BPOS];

    if ((pAgentInf->PLC_PB_com[ID_PB_AUTO_START] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_start[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_start[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_auto_start[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_start[ID_BPOS];
    
    //半自動ランプ
    if ((pAgentInf->PLC_LAMP_semiauto_com [SEMI_AUTO_TG1] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg1[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg1[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg1[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg1[ID_BPOS];

    if ((pAgentInf->PLC_LAMP_semiauto_com[SEMI_AUTO_TG2] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg2[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg2[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg2[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg2[ID_BPOS];

    if ((pAgentInf->PLC_LAMP_semiauto_com[SEMI_AUTO_TG3] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg3[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg3[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg3[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg3[ID_BPOS];

    if ((pAgentInf->PLC_LAMP_semiauto_com[SEMI_AUTO_TG4] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
        melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg4[ID_WPOS]] |= melnet.pc_b_map.lamp_auto_tg4[ID_BPOS];
    else melnet.pc_b_out[melnet.pc_b_map.lamp_auto_tg4[ID_WPOS]] &= ~melnet.pc_b_map.lamp_auto_tg4[ID_BPOS];

     return 0;
}

//*********************************************************************************************
// parse_notch_com()
// UIノッチ指令読み込み
//*********************************************************************************************
int CPLC_IF::parse_notch_com() {
    
    INT16 check_i;
 
    //巻きノッチ
    check_i = melnet.plc_w_out[melnet.plc_w_map.com_hst_notch_0[ID_WPOS]] & NOTCH_PTN0_ALL;

    if(check_i & NOTCH_PTN0_0) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_0;
    else if(check_i & NOTCH_PTN0_F1) {
      if(check_i == NOTCH_PTN0_F5) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_5;
      else if (check_i == NOTCH_PTN0_F4) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_4;
      else if (check_i == NOTCH_PTN0_F3) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_3;
      else if (check_i == NOTCH_PTN0_F2) plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_2;
      else plc_io_workbuf.ui.notch_pos[ID_HOIST] = NOTCH_1;
    }
    else if (check_i & NOTCH_PTN0_R1) {
      if (check_i == NOTCH_PTN0_R5) plc_io_workbuf.ui.notch_pos[ID_HOIST] = -NOTCH_5;
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

    plc_io_workbuf.ui.PB[ID_PB_ESTOP] = melnet.plc_w_out[melnet.plc_w_map.com_estop[ID_WPOS]] & melnet.plc_w_map.com_estop[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_ON] = melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source_on[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source_on[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE_OFF] = melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source_off[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source_off[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_ON] = melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source2_on[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source2_on[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_CTRL_SOURCE2_OFF] = melnet.plc_w_out[melnet.plc_w_map.com_ctrl_source2_off[ID_WPOS]] & melnet.plc_w_map.com_ctrl_source2_off[ID_BPOS];

    plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_ON] = melnet.plc_w_out[melnet.plc_b_map.PB_as_on[ID_WPOS]] & melnet.plc_b_map.PB_as_on[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_ANTISWAY_OFF] = melnet.plc_w_out[melnet.plc_b_map.PB_as_off[ID_WPOS]] & melnet.plc_b_map.PB_as_off[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_AUTO_START] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_start[ID_WPOS]] & melnet.plc_b_map.PB_auto_start[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG1] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target1[ID_WPOS]] & melnet.plc_b_map.PB_auto_target1[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG2] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target2[ID_WPOS]] & melnet.plc_b_map.PB_auto_target2[ID_BPOS];;
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG3] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target3[ID_WPOS]] & melnet.plc_b_map.PB_auto_target3[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG4] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target4[ID_WPOS]] & melnet.plc_b_map.PB_auto_target4[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG5] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target5[ID_WPOS]] & melnet.plc_b_map.PB_auto_target1[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG6] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target6[ID_WPOS]] & melnet.plc_b_map.PB_auto_target2[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG7] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target7[ID_WPOS]] & melnet.plc_b_map.PB_auto_target3[ID_BPOS];
    plc_io_workbuf.ui.PBsemiauto[SEMI_AUTO_TG8] = melnet.plc_w_out[melnet.plc_b_map.PB_auto_target8[ID_WPOS]] & melnet.plc_b_map.PB_auto_target4[ID_BPOS];
    plc_io_workbuf.ui.PB[ID_PB_CRANE_MODE];
    plc_io_workbuf.ui.PB[ID_PB_REMOTE_MODE];

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
    plc_io_workbuf.status.v_fb[ID_BOOM_H] = def_spec.notch_spd_f[ID_BOOM_H][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_bh_fb[ID_WPOS]] / 1000.0;
    plc_io_workbuf.status.v_fb[ID_SLEW] = def_spec.notch_spd_f[ID_SLEW][5] * (double)melnet.plc_w_out[melnet.plc_w_map.spd_slw_fb[ID_WPOS]] / 1000.0;

    plc_io_workbuf.status.weight = (double)melnet.plc_w_out[melnet.plc_w_map.load_fb[ID_WPOS]] * 100.0; //Kg
    
    plc_io_workbuf.status.pos[ID_HOIST] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_hst_fb[ID_WPOS]] / 10.0;   //m
    plc_io_workbuf.status.pos[ID_GANTRY] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_gnt_fb[ID_WPOS]] / 10.0;  //m
    plc_io_workbuf.status.pos[ID_BOOM_H] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_bh_fb[ID_WPOS]] / 10.0;   //m
    plc_io_workbuf.status.pos[ID_SLEW] = (double)melnet.plc_w_out[melnet.plc_w_map.pos_slw_fb[ID_WPOS]] * PI1DEG;  //rad
    return 0;
}