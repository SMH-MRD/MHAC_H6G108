#include "CSIM.h"
#include "CWorkWindow_SIM.h"
#include "Spec.h"
#include "CVector3.h"
#include "SIM.h"

extern ST_SPEC def_spec;

CSIM::CSIM() {
    // ���L�������I�u�W�F�N�g�̃C���X�^���X��
    pSimulationStatusObj = new CSharedMem;
    pPLCioObj = new CSharedMem;
    pCraneStatusObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;

    // MOB �I�u�W�F�N�g�̃C���X�^���X��
    pCrane = new CCrane(); //�N���[���̃��f��
    pLoad = new CLoad();   //�ׂ݉̃��f��

    out_size = 0;
 
    memset(&sim_stat_workbuf, 0, sizeof(ST_SIMULATION_STATUS));   //���L�������ւ̏o�̓Z�b�g��Ɨp�o�b�t�@
};
CSIM::~CSIM() {
    // ���L�������I�u�W�F�N�g�̉��
    delete pPLCioObj;
    delete pCraneStatusObj;
    delete pSimulationStatusObj;
    delete pAgentInfObj;

    delete pCrane;
    delete pLoad;
 
};

int CSIM::set_outbuf(LPVOID pbuf) {
    poutput = pbuf;return 0;
};      //�o�̓o�b�t�@�Z�b�g

//******************************************************************************************
// init_proc()
//******************************************************************************************
int CSIM::init_proc() {

       
    // ���L�������擾

     // �o�͗p���L�������擾
    out_size = sizeof(ST_SIMULATION_STATUS);
    if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME, out_size, MUTEX_SIMULATION_STATUS_NAME)) {
        mode |= SIM_IF_SIM_MEM_NG;
    }

    // ���͗p���L�������擾
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

   //CraneStat�����オ��҂�
    while (pCraneStat->is_tasks_standby_ok ==false) {
        Sleep(10);
    }

    //�N���[���d�l�̃Z�b�g
    pCrane->set_spec(&def_spec);

    //�N���[���̏�����ԃZ�b�g 
    pCrane->init_crane(SYSTEM_TICK_ms / 1000.0);


    //�݉׵�޼ު�Ăɸڰݵ�޼ު�Ă�R�t��
    pLoad->set_crane(pCrane);

    //�ׂ݉̏�����ԃZ�b�g 
    Vector3 _r(SIM_INIT_R * cos(SIM_INIT_TH) + SIM_INIT_X, SIM_INIT_R * sin(SIM_INIT_TH), def_spec.boom_high - SIM_INIT_L);  //�ݓ_�ʒu
    Vector3 _v(0.0, 0.0, 0.0);                          //�ݓ_�ʑ��x
    pLoad->init_mob(SYSTEM_TICK_ms / 1000.0, _r, _v);
    pLoad->set_m(SIM_INIT_M);


    //�U��p�v�Z�p�J�����p�����[�^�Z�b�g
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
            SwayCamParam[j][CAM_SET_PARAM_d] = pCraneStat->spec.SwayCamParam[SID_SIM][j][SID_PIXlRAD];//rad��pix�ϊ��W��
     }

    return int(mode & 0xff00);
}
//*********************************************************************************************
// input()
//*********************************************************************************************
int CSIM::input() {
    sim_stat_workbuf.helthy_cnt++;

    //MAIN�v���Z�X(Environment�^�X�N�̃w���V�[�M����荞�݁j
    source_counter = pCraneStat->env_act_count;

    //PLC ����
    pCrane->set_v_ref(
        pAgent->v_ref[ID_HOIST],
        pAgent->v_ref[ID_GANTRY],
        pAgent->v_ref[ID_SLEW],
        pAgent->v_ref[ID_BOOM_H]
    );

    //�X�L�����^�C���Z�b�g dt�̓}���`���f�B�A�^�C�}�@�R�[���o�b�N�ŃZ�b�g
    pCrane->set_dt(dt);
    pLoad->set_dt(dt);

    //�ړ��Ɍ����
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

     pCrane->update_break_status(); //�u���[�L��ԍX�V
     pCrane->timeEvolution();       //�N���[���̈ʒu,���x�v�Z
     pLoad->timeEvolution();        //�ׂ݉̈ʒu,���x�v�Z
     pLoad->r.add(pLoad->dr);       //�݉׈ʒu�X�V
     pLoad->v.add(pLoad->dv);       //�݉ב��x�X�V
     pLoad->update_relative_vec();  //�݉גݓ_���΃x�N�g���X�V(���[�v�x�N�g���@L,vL)

    return 0;
}
//*********************************************************************************************
// output()
//*********************************************************************************************
int CSIM::output() {

    sim_stat_workbuf.mode = this->mode;                         //���[�h�Z�b�g
    sim_stat_workbuf.helthy_cnt = my_helthy_counter++;          //�w���V�[�J�E���^�Z�b�g

    set_cran_motion();  //�N���[���̈ʒu�A���x���Z�b�g
    set_sway_io();      //�U��Z���TIO���Z�b�g
    
    
    if (out_size) { //�o�͏���
        memcpy_s(poutput, out_size, &sim_stat_workbuf, out_size);
    }

    return 0;
}
//*********************************************************************************************
// set_cran_motion() �N���[���ʒu�A���x���Z�b�g
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
// output() �U��Z���T�M���Z�b�g
//*********************************************************************************************

static double thcamx_last=0, thcamy_last=0;

int CSIM::set_sway_io() {
      
    // �X�Όv���o�p�x
    double tilt_x = 0.0;
    double tilt_y = 0.0;
    sim_stat_workbuf.rcv_msg.head.tilt_x = (UINT32)(tilt_x * 1000000.0);
    sim_stat_workbuf.rcv_msg.head.tilt_y = (UINT32)(tilt_y * 1000000.0);
    
    // �N���[��xy���W���J����xy���W�ɉ�]�ϊ��@���@�p�xrad�ɕϊ��@
    double th = pCrane->r0[ID_SLEW];//����p�x
    double thx = asin(((pLoad->L.x) * sin(th) + (pLoad->L.y) * -cos(th)) / pCrane->l_mh);
    double thy = asin(((pLoad->L.x) * cos(th) + (pLoad->L.y) * sin(th)) / pCrane->l_mh);

   
    // �J������t�I�t�Z�b�g�l�̌v�Z
    double ax = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double bx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dx = SwayCamParam[SID_AXIS_X][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double ay = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_a];//�Z���T���o�p�␳�l
    double by = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_b];//�Z���T���o�p�␳�l
    double cy = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_c];//�Z���T���o�p�␳�l
    double dy = SwayCamParam[SID_AXIS_Y][CAM_SET_PARAM_d];//rad��pix�ϊ��W��
    double L = pCraneStat->mh_l;
    double T = pCraneStat->T;
    double w = pCraneStat->w;

    double phx = tilt_x + cx;
    double phy = tilt_y + cy;
    double offset_thx = asin(ax * sin(phx + bx) / L);
    double offset_thy = asin(ay * sin(phy + by) / L);

 
    //�J�������o�p�xrad
    double th_camx = thx - offset_thx - phx;
    double th_camy = thy - offset_thy - phy;
    //�J�������o�p���xrad
    double dth_camx = (th_camx - thcamx_last) / pCrane->dt;
    double dth_camy = (th_camy - thcamy_last) / pCrane->dt;

    thcamx_last = th_camx;
    thcamy_last = th_camy;

    //�J�������o�p�xpix
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].th_x = (INT32)(th_camx * dx);
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].th_y = (INT32)(th_camy * dy);
    //�J�������o�p�xpix
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].dth_x = (INT32)(dth_camx * dx);
    sim_stat_workbuf.rcv_msg.body.data[SWAY_SENSOR_TG1].dth_y = (INT32)(dth_camy * dy);
    
    //�V�~�����[�^���W�b�N�`�F�b�N�p�o�b�t�@�Z�b�g
    sim_stat_workbuf.sway_io.th[ID_SLEW] = thx;
    sim_stat_workbuf.sway_io.th[ID_BOOM_H] = thy;
    sim_stat_workbuf.sway_io.dth[ID_SLEW] = dth_camx;
    sim_stat_workbuf.sway_io.dth[ID_BOOM_H] = dth_camy;
    
    return 0;
}
