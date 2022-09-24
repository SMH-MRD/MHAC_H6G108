#include "CSwayIF.h"


CSwayIF::CSwayIF() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pSwayIOObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pSimulationStatusObj = new CSharedMem;
    out_size = 0;
    memset(&sway_io_workbuf, 0, sizeof(ST_SWAY_IO));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
};
CSwayIF::~CSwayIF() {
    // ���L�������I�u�W�F�N�g�̉��
    delete pSwayIOObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
};

int CSwayIF::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //�o�̓o�b�t�@�Z�b�g

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSwayIF::init_proc() {

     
    // ���L�������擾

     // �o�͗p���L�������擾
    out_size = sizeof(ST_PLC_IO);
    if (OK_SHMEM != pSwayIOObj->create_smem(SMEM_SWAY_IO_NAME, out_size, MUTEX_SWAY_IO_NAME)) {
        mode |= SWAY_IF_SWAY_IO_MEM_NG;
    }
    set_outbuf(pSwayIOObj->get_pMap());

    // ���͗p���L�������擾
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SWAY_IF_SIM_MEM_NG;
    }
 
    if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) {
        mode |= SWAY_IF_CRANE_MEM_NG;
    }

    //���L�N���[���X�e�[�^�X�\���̂̃|�C���^�Z�b�g
    pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
    pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());

    //CraneStat�����オ��҂�
    while (pCraneStat->is_tasks_standby_ok == false) {
        Sleep(10);
    }
    
    //�U��p�v�Z�p�J�����p�����[�^�Z�b�g
    for(int i=0;i< N_SWAY_SENSOR;i++)
        for (int j = 0;j < SWAY_SENSOR_N_AXIS;j++)
        {
            double D0 = pCraneStat->spec.SwayCamParam[i][j][SID_D0];
            double H0 = pCraneStat->spec.SwayCamParam[i][j][SID_H0];
            double l0 = pCraneStat->spec.SwayCamParam[i][j][SID_l0];
            
            SwayCamParam[i][j][CAM_SET_PARAM_a] = sqrt(D0*D0 + (H0-l0)* (H0 - l0));
            double tempd = H0 - l0; if (tempd < 0.0) tempd *= -1.0;
            if (tempd < 0.000001) {
                SwayCamParam[i][j][CAM_SET_PARAM_b] = 0.0;
            }
            else {
                SwayCamParam[i][j][CAM_SET_PARAM_b] = atan(D0 / (H0 - l0));
            }
            SwayCamParam[i][j][CAM_SET_PARAM_c] = pCraneStat->spec.SwayCamParam[i][j][SID_ph0];
            SwayCamParam[i][j][CAM_SET_PARAM_d] = pCraneStat->spec.SwayCamParam[i][j][SID_PIXlRAD];//rad��pix�ϊ��W��
 
        }
    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CSwayIF::input() {
 
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
int CSwayIF::parse() {


#ifdef _DVELOPMENT_MODE
    if (pSimStat->mode & SIM_ACTIVE_MODE) {
        set_sim_status(&sway_io_workbuf);   //�@�U��Z���T��M�o�b�t�@�̒l��SIM����SWAY_IF�̃o�b�t�@�ɃR�s�[
        parse_sway_stat(SID_SIM);           //�@�V�~�����[�^�̎�M�o�b�t�@����́i�J�������W�ł̐U�ꌟ�o�l�j
    }
    else {
        parse_sway_stat(SID_CAM1);          //�@���Z���T����̎�M�o�b�t�@�����
    }
#else
    parse_sway_stat(SWAY_SENSOR1);
#endif

    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSwayIF::output() {

    sway_io_workbuf.proc_mode = this->mode;              //���[�h�Z�b�g
    sway_io_workbuf.helthy_cnt = my_helthy_counter++;    //�w���V�[�J�E���^�Z�b�g

    if (out_size) { //�o�͏���
        memcpy_s(poutput, out_size, &sway_io_workbuf, out_size);
    }

    return 0;
}

//*********************************************************************************************
// set_sim_status()
//*********************************************************************************************
int CSwayIF::set_sim_status(LPST_SWAY_IO pworkbuf) {

    memcpy_s(&rcv_msg[SID_SIM][0], sizeof(ST_SWAY_RCV_MSG), &pSimStat->rcv_msg, sizeof(ST_SWAY_RCV_MSG));
 
    return 0;
}

int CSwayIF::parse_sway_stat(int ID) {
    
    double ax = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double bx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dx = SwayCamParam[ID][SID_AXIS_X][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double ay = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double by = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dy = SwayCamParam[ID][SID_AXIS_Y][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double L = pCraneStat->mh_l;
    double T = pCraneStat->T;
    double w = pCraneStat->w;

    double tilt_x = ((double)rcv_msg[ID]->head.tilt_x) / 1000000.0;
    double tilt_y = ((double)rcv_msg[ID]->head.tilt_y) / 1000000.0;
 	
    double phx = tilt_x + cx;
    double phy = tilt_y + cy;

    double psix = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].th_x) / dx + phx;
    double psiy = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].th_y) / dy + phy;

    double offset_thx = asin(ax * sin(phx + bx) / L);
    double offset_thy = asin(ay * sin(phy + by) / L);

    //�U�p�@�U�p���x�@�U���@�ʑ��@
	//    �J�����ʒu����̐U��p���J�������o�p�{��t�I�t�Z�b�g  
    sway_io_workbuf.th[ID_SLEW] = psix + offset_thx;
    sway_io_workbuf.th[ID_BOOM_H] = psiy + offset_thy;

    sway_io_workbuf.dth[ID_SLEW] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].dth_x) / dx;  // rad�ɕϊ�
    sway_io_workbuf.dth[ID_BOOM_H] = (double)(rcv_msg[ID][i_rcv_msg[ID]].body.data[SWAY_SENSOR_TG1].dth_y) / dy;// rad�ɕϊ�

    sway_io_workbuf.dthw[ID_SLEW] = sway_io_workbuf.dth[ID_SLEW] / w;
    sway_io_workbuf.dthw[ID_BOOM_H] = sway_io_workbuf.dth[ID_BOOM_H] / w;

    sway_io_workbuf.amp2[ID_SLEW] = sway_io_workbuf.th[ID_SLEW] * sway_io_workbuf.th[ID_SLEW] + sway_io_workbuf.dthw[ID_SLEW] * sway_io_workbuf.dthw[ID_SLEW];
    sway_io_workbuf.amp2[ID_BOOM_H] = sway_io_workbuf.th[ID_BOOM_H] * sway_io_workbuf.th[ID_BOOM_H] + sway_io_workbuf.dthw[ID_BOOM_H] * sway_io_workbuf.dthw[ID_BOOM_H];

    //�����U��  
    sway_io_workbuf.amp2[ID_COMMON] = sway_io_workbuf.amp2[ID_SLEW] + sway_io_workbuf.amp2[ID_BOOM_H];

    //�ʑ�(X���j
    if (sway_io_workbuf.th[ID_SLEW] > 0.00001) {
        sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]);
    }
    else if (sway_io_workbuf.th[ID_SLEW] < -0.00001) { // atan()������0�����
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) + PI180;
        else                                    sway_io_workbuf.ph[ID_SLEW] = atan(sway_io_workbuf.dthw[ID_SLEW] / sway_io_workbuf.th[ID_SLEW]) - PI180;
    }
    else{ //�ʑ���-�΁`�΂͈̔͂ŕ\��
        if (sway_io_workbuf.dth[ID_SLEW] >= 0.0) sway_io_workbuf.ph[ID_SLEW] = PI90;
        else                                    sway_io_workbuf.ph[ID_SLEW] = -PI90;
    }

    //�ʑ�(Y���j
    if (sway_io_workbuf.th[ID_BOOM_H] > 0.00001) {
        sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]);
    }
    else if (sway_io_workbuf.th[ID_BOOM_H] < -0.00001) { // atan()������0�����
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) + PI180;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = atan(sway_io_workbuf.dthw[ID_BOOM_H] / sway_io_workbuf.th[ID_BOOM_H]) - PI180;
    }
    else { //�ʑ���-�΁`�΂͈̔͂ŕ\��
        if (sway_io_workbuf.dth[ID_BOOM_H] >= 0.0) sway_io_workbuf.ph[ID_BOOM_H] = PI90;
        else                                    sway_io_workbuf.ph[ID_BOOM_H] = -PI90;
    }
    
	return 0;
}
