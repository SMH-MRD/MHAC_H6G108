#include "CPLC_IF.h"
#include "PLC_IO_DEF.h"
#include "CWorkWindow_PLC.h"
#include <windows.h>
#include "Mdfunc.h"

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

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CPLC_IF::input() {
    
    plc_io_workbuf.helthy_cnt++;

    //PLC 入力処理
    //MELSECNET回線確認
    if ((!melnet.status)&&(!(plc_io_workbuf.helthy_cnt% melnet.retry_cnt))) {
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
            melnet.plc_r_buf_B.dummy_buf);     //読み込みバッファ

         //W読み込み
        melnet.err = mdReceiveEx(melnet.path,    //チャネルのパス
            MELSEC_NET_NW_NO,                   //ネットワーク番号      
            MELSEC_NET_SOURCE_STATION,          //局番
            MELSEC_NET_CODE_LW,                 //デバイスタイプ
            MELSEC_NET_W_READ_START,            //先頭デバイス
            &melnet.read_size_w,                //読み込みバイトサイズ
            melnet.plc_r_buf_W.main_x_buf);     //読み込みバッファ

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
    memcpy_s(melnet.plc_w_buf_B.pc_com_buf, 16, melnet.plc_r_buf_B.dummy_buf, 16);     //Bレジスタ
    memcpy_s(melnet.plc_w_buf_W.pc_com_buf, 16, melnet.plc_r_buf_W.main_x_buf, 16); //Wレジスタ

    //PLCリンク入力を解析
    if (B_HST_NOTCH_0)plc_io_workbuf.ui.notch_pos[ID_HOIST] = 0;
    
    //デバッグモード時は、操作パネルウィンドウの内容を上書き 
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
 
    plc_io_workbuf.mode = this->mode;                   //モードセット
    plc_io_workbuf.helthy_cnt = my_helthy_counter++;    //ヘルシーカウンタセット
    
    //共有メモリ出力処理
    if(out_size) { 
        memcpy_s(poutput, out_size, &plc_io_workbuf, out_size);
    }
    //PLC出力処理
    if (melnet.status == MELSEC_NET_OK) {
        //LB書き込み
        melnet.err = mdSendEx(melnet.path,  //チャネルのパス
            MELSEC_NET_MY_NW_NO,            //ネットワーク番号   
            MELSEC_NET_MY_STATION,          //局番
            MELSEC_NET_CODE_LB,             //デバイスタイプ
            MELSEC_NET_B_WRITE_START,       //先頭デバイス
            &melnet.write_size_b,           //書き込みバイトサイズ
            melnet.plc_w_buf_B.pc_com_buf); //ソースバッファ
        //LW書き込み
        melnet.err = mdSendEx(melnet.path,  //チャネルのパス
            MELSEC_NET_MY_NW_NO,            //ネットワーク番号  
            MELSEC_NET_MY_STATION,          //局番
            MELSEC_NET_CODE_LW,             //デバイスタイプ
            MELSEC_NET_W_WRITE_START,       //先頭デバイス
            &melnet.write_size_w,           //書き込みバイトサイズ
            melnet.plc_w_buf_W.pc_com_buf); //ソースバッファ
       
        if (melnet.err < 0)melnet.status = MELSEC_NET_SEND_ERR;
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
//*********************************************************************************************
// closeIF()
//*********************************************************************************************
int CPLC_IF::closeIF() {

   //MELSECNET回線クローズ
        melnet.err = mdClose(melnet.path);
        melnet.status = MELSEC_NET_CLOSE;
   return 0;
}



