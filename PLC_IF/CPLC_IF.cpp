#include "CPLC_IF.h"
#include "PLC_IO_DEF.h"
#include "CWorkWindow_PLC.h"
#include <windows.h>
#include "Mdfunc.h"

extern ST_SPEC def_spec;

CPLC_IF::CPLC_IF() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    out_size = 0;
    memset(&melnet,0,sizeof(ST_MELSEC_NET)) ;      //PLC�����N�\����
    memset(&plc_io_workbuf,0,sizeof(ST_PLC_IO));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@

    melnet.chan = MELSEC_NET_CH;
    melnet.mode = 0;
    melnet.path = NULL;
    melnet.err = 0;
    melnet.status = 0;
    melnet.retry_cnt = MELSEC_NET_RETRY_CNT;
    melnet.read_size_b = 0;                         //PLC��LW��bit�ŃZ�b�g�����LB�͖��g�p
    melnet.read_size_w = sizeof(ST_PLC_READ_W);
    melnet.write_size_b = sizeof(ST_PLC_WRITE_B);
    melnet.write_size_w = sizeof(ST_PLC_WRITE_W);
    
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
    if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, (DWORD)out_size, MUTEX_PLC_IO_NAME)) {
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

    pAgentInf = (LPST_AGENT_INFO)pAgentInfObj->get_pMap();

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CPLC_IF::input() {
    
    plc_io_workbuf.helthy_cnt++;

        //PLC ���͏���
        //MELSECNET����m�F
    if ((!melnet.status) && (!(plc_io_workbuf.helthy_cnt % melnet.retry_cnt))) {
        if (!(melnet.err = mdOpen(melnet.chan, melnet.chan, &melnet.path)))
            melnet.status = MELSEC_NET_OK;
    }
        //PLC Read
    if (melnet.status == MELSEC_NET_OK) {
        //LB�ǂݍ��݁@����

        //B�ǂݍ���
        melnet.err = mdReceiveEx(melnet.path,   //�`���l���̃p�X
            MELSEC_NET_NW_NO,                   //�l�b�g���[�N�ԍ�           
            MELSEC_NET_SOURCE_STATION,          //�ǔ�
            MELSEC_NET_CODE_LB,                 //�f�o�C�X�^�C�v
            MELSEC_NET_B_READ_START,            //�擪�f�o�C�X
            &melnet.read_size_w,                //�ǂݍ��݃o�C�g�T�C�Y
            melnet.plc_b_out);                   //�ǂݍ��݃o�b�t�@

         //W�ǂݍ���
        melnet.err = mdReceiveEx(melnet.path,    //�`���l���̃p�X
            MELSEC_NET_NW_NO,                   //�l�b�g���[�N�ԍ�      
            MELSEC_NET_SOURCE_STATION,          //�ǔ�
            MELSEC_NET_CODE_LW,                 //�f�o�C�X�^�C�v
            MELSEC_NET_W_READ_START,            //�擪�f�o�C�X
            &melnet.read_size_w,                //�ǂݍ��݃o�C�g�T�C�Y
            melnet.pc_w_out);     //�ǂݍ��݃o�b�t�@

        if (melnet.err != 0)melnet.status = MELSEC_NET_RECEIVE_ERR;
    }
      
    //MAIN�v���Z�X(Environment�^�X�N�̃w���V�[�M����荞�݁j
    source_counter = pCrane->env_act_count;

     return 0;
}
//*********************************************************************************************
// parse()
//*********************************************************************************************
int CPLC_IF::parse() { 

    //PLC����̓��͂��I�E���Ԃ��ŏo�́i�����Ή������j
 //   memcpy_s(melnet.plc_w_buf_B.pc_com_buf, 16, melnet.plc_r_buf_B.spare, 16);     //B���W�X�^
 //   memcpy_s(melnet.plc_w_buf_W.pc_com_buf, 16, melnet.plc_r_buf_W.main_x_buf, 16); //W���W�X�^

    //PLC�����N���͂����
    parse_notch_com();
    
    //�f�o�b�O���[�h���́A����p�l���E�B���h�E�̓��e���㏑�� 
    if (is_debug_mode()) set_debug_status(&plc_io_workbuf);

 
    //### �V�~�����[�V�����̏�Ԃ���͐M���o�b�t�@�ɃZ�b�g
#ifdef _DVELOPMENT_MODE

    if (pSim->mode & SIM_ACTIVE_MODE) {
        set_sim_status(&plc_io_workbuf);
    }

#endif

    //### PLC�ւ̏o�͐M���o�b�t�@�Z�b�g

    //�m�b�`�o�͐M���Z�b�g
    set_notch_ref();

    return 0; 
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CPLC_IF::output() { 
 
    plc_io_workbuf.mode = this->mode;                   //���[�h�Z�b�g
    plc_io_workbuf.helthy_cnt = my_helthy_counter++;    //�w���V�[�J�E���^�Z�b�g
    
    //���L�������o�͏���
    if(out_size) { 
        memcpy_s(poutput, out_size, &plc_io_workbuf, out_size);
    }
 
    //MELSECNET�ւ̏o�͏���
    if (melnet.status == MELSEC_NET_OK) {
        //LB��������
        melnet.err = mdSendEx(melnet.path,  //�`���l���̃p�X
            MELSEC_NET_MY_NW_NO,            //�l�b�g���[�N�ԍ�   
            MELSEC_NET_MY_STATION,          //�ǔ�
            MELSEC_NET_CODE_LB,             //�f�o�C�X�^�C�v
            MELSEC_NET_B_WRITE_START,       //�擪�f�o�C�X
            &melnet.write_size_b,           //�������݃o�C�g�T�C�Y
            melnet.pc_b_out); //�\�[�X�o�b�t�@
        //LW��������
        melnet.err = mdSendEx(melnet.path,  //�`���l���̃p�X
            MELSEC_NET_MY_NW_NO,            //�l�b�g���[�N�ԍ�  
            MELSEC_NET_MY_STATION,          //�ǔ�
            MELSEC_NET_CODE_LW,             //�f�o�C�X�^�C�v
            MELSEC_NET_W_WRITE_START,       //�擪�f�o�C�X
            &melnet.write_size_w,           //�������݃o�C�g�T�C�Y
            melnet.pc_w_out); //�\�[�X�o�b�t�@

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

    if(pWorkWindow->stOpePaneStat.check_estop) pworkbuf->ui.PBs[ID_PB_ESTOP] |=true; else pworkbuf->ui.PBs[ID_PB_ESTOP] = false;
    if (pWorkWindow->stOpePaneStat.check_antisway) {
        pworkbuf->ui.PBs[ID_PB_ANTISWAY_ON] |= true;pworkbuf->ui.PBs[ID_PB_ANTISWAY_OFF] = false;
        
    }
    else {
        pworkbuf->ui.PBs[ID_PB_ANTISWAY_ON] = false;pworkbuf->ui.PBs[ID_PB_ANTISWAY_OFF] |= true;
    }

    if (pWorkWindow->stOpePaneStat.button_remote)pworkbuf->ui.PBs[3] |=true; else pworkbuf->ui.PBs[3] = false;
    if (pWorkWindow->stOpePaneStat.button_auto_start)pworkbuf->ui.PBs[ID_PB_AUTO_START] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_START] = false;
    if (pWorkWindow->stOpePaneStat.button_auto_reset)pworkbuf->ui.PBs[3] |=true; else pworkbuf->ui.PBs[3] = false;
    if (pWorkWindow->stOpePaneStat.button_from1)pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM1] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM1] = false;
    if (pWorkWindow->stOpePaneStat.button_from2) pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM2] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM2] = false;
    if (pWorkWindow->stOpePaneStat.button_from3)pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM3] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM3] = false;
    if (pWorkWindow->stOpePaneStat.button_from4)pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM4] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_FROM4] = false;
    if (pWorkWindow->stOpePaneStat.button_to1)pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO1] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO1] = false;
    if (pWorkWindow->stOpePaneStat.button_to2) pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO2] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO2] = false;
    if (pWorkWindow->stOpePaneStat.button_to3)pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO3] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO3] = false;
    if (pWorkWindow->stOpePaneStat.button_to4)pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO4] |=true; else pworkbuf->ui.PBs[ID_PB_AUTO_TG_TO4] = false;
    
    if(pWorkWindow->stOpePaneStat.button_source1_on)   pworkbuf->ui.PBs[3] |= true; else pworkbuf->ui.PBs[3] = false;
    if(pWorkWindow->stOpePaneStat.button_source1_off)  pworkbuf->ui.PBs[3] |= true; else pworkbuf->ui.PBs[3] = false;
  
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

   //MELSECNET����N���[�Y
        melnet.err = mdClose(melnet.path);
        melnet.status = MELSEC_NET_CLOSE;
   return 0;
}


//*********************************************************************************************
// set_notch_ref()
// AGENT�^�X�N�̃m�b�`�ʒu�w�߂ɉ�����IO�o�͂�ݒ�
//*********************************************************************************************
int CPLC_IF::set_notch_ref() {

    //���m�b�`
    //�m�b�`�N���A
    melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_0[ID_WPOS]] &= NOTCH_PTN0_CLR;

    if ((pAgentInf->v_ref[ID_HOIST] < (def_spec.notch_spd_r[ID_HOIST][NOTCH_1]))             //�w�߂�-1�m�b�`��菬
        || (pAgentInf->v_ref[ID_HOIST] > (def_spec.notch_spd_f[ID_HOIST][NOTCH_1]))) {       //�w�߂�+1�m�b�`����
        //�m�b�`�Z�b�g
        if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_r[ID_HOIST][NOTCH_1]) {     //�t�]1�m�b�`�ȉ�

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
        else if (pAgentInf->v_ref[ID_HOIST] < def_spec.notch_spd_f[ID_HOIST][NOTCH_1]) { //���]1�m�b�`�ȏ�

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
    else {//0�m�b�`
        melnet.pc_b_out[melnet.pc_b_map.com_hst_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_hst_notch_0[ID_BPOS];
    }
 
    //���s�m�b�`
   //�m�b�`�N���A
    melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_0[ID_WPOS]] &= NOTCH_PTN1_CLR;

    if ((pAgentInf->v_ref[ID_GANTRY] < (def_spec.notch_spd_r[ID_GANTRY][NOTCH_1]))             //�w�߂�-1�m�b�`��菬
        || (pAgentInf->v_ref[ID_GANTRY] > (def_spec.notch_spd_f[ID_GANTRY][NOTCH_1]))) {       //�w�߂�+1�m�b�`����
        //�m�b�`�Z�b�g
        if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_r[ID_GANTRY][NOTCH_1]) {     //�t�]1�m�b�`�ȉ�

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
        else if (pAgentInf->v_ref[ID_GANTRY] < def_spec.notch_spd_f[ID_GANTRY][NOTCH_1]) { //���]1�m�b�`�ȏ�

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
    else {//0�m�b�`
        melnet.pc_b_out[melnet.pc_b_map.com_gnt_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_gnt_notch_0[ID_BPOS];
    }
 
    //�����m�b�`
    //�m�b�`�N���A
    melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_0[ID_WPOS]] &= NOTCH_PTN0_CLR;

    if ((pAgentInf->v_ref[ID_BOOM_H] < (def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]))             //�w�߂�-1�m�b�`��菬
        || (pAgentInf->v_ref[ID_BOOM_H] > (def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]))) {       //�w�߂�+1�m�b�`����
        //�m�b�`�Z�b�g
        if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_r[ID_BOOM_H][NOTCH_1]) {     //�t�]1�m�b�`�ȉ�

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
        else if (pAgentInf->v_ref[ID_BOOM_H] < def_spec.notch_spd_f[ID_BOOM_H][NOTCH_1]) { //���]1�m�b�`�ȏ�

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
    else {//0�m�b�`
        melnet.pc_b_out[melnet.pc_b_map.com_bh_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_bh_notch_0[ID_BPOS];
    }

    //����m�b�`
    //�m�b�`�N���A
    melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_0[ID_WPOS]] &= NOTCH_PTN1_CLR;

    if ((pAgentInf->v_ref[ID_SLEW] < (def_spec.notch_spd_r[ID_SLEW][NOTCH_1]))             //�w�߂�-1�m�b�`��菬
        || (pAgentInf->v_ref[ID_SLEW] > (def_spec.notch_spd_f[ID_SLEW][NOTCH_1]))) {       //�w�߂�+1�m�b�`����
        //�m�b�`�Z�b�g
        if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_r[ID_SLEW][NOTCH_1]) {     //�t�]1�m�b�`�ȉ�

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
        else if (pAgentInf->v_ref[ID_SLEW] < def_spec.notch_spd_f[ID_SLEW][NOTCH_1]) { //���]1�m�b�`�ȏ�

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
    else {//0�m�b�`
        melnet.pc_b_out[melnet.pc_b_map.com_slw_notch_0[ID_WPOS]] |= melnet.pc_b_map.com_slw_notch_0[ID_BPOS];
    }
     
    return 0;
}

//*********************************************************************************************
// parse_notch_com()
// UI�m�b�`�w�߃Z�b�g
//*********************************************************************************************
int CPLC_IF::parse_notch_com() {
    
    INT16 check_i;
 
    //�����m�b�`
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
        
    //���s�m�b�`
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
    
    //�����m�b�`
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

    //����m�b�`
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
