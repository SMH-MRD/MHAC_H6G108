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

    //�N���[���d�l�̃Z�b�g
    pCrane->set_spec(&def_spec);   
    
    //�N���[���̏�����ԃZ�b�g 
     pCrane->init_crane(SYSTEM_TICK_ms/1000.0);
 

     //�݉׵�޼ު�Ăɸڰݵ�޼ު�Ă�R�t��
     pLoad->set_crane(pCrane);
  
     //�ׂ݉̏�����ԃZ�b�g 
    Vector3 _r(SIM_INIT_R * cos(SIM_INIT_TH) + SIM_INIT_X, SIM_INIT_R * sin(SIM_INIT_TH), def_spec.boom_high- SIM_INIT_L);  //�ݓ_�ʒu
    Vector3 _v(0.0, 0.0, 0.0);                          //�ݓ_�ʑ��x
    pLoad->init_mob(SYSTEM_TICK_ms / 1000.0, _r, _v);
    pLoad->set_m(SIM_INIT_M);
 

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
// output() �U��Z���TIO�M���Z�b�g
//*********************************************************************************************

static double radsl_last, radbh_last;
int CSIM::set_sway_io() {

    //�U��Z���T�M��
    double th = pCrane->r0[ID_SLEW]+PI90;//����p�x
    // �N���[�����W�̐U��p���J�������W�ɕϊ�(����p�x����]�j
    double phx = ((pLoad->L.x) * cos(th) + (pLoad->L.y) * sin(th))/pCrane->l_mh;
    double phy = (-(pLoad->L.x) * sin(th) + (pLoad->L.y) * cos(th)) / pCrane->l_mh;
    sim_stat_workbuf.rad_cam_x = phx;
    sim_stat_workbuf.rad_cam_y = phy;

    //�@�J�����ݒu�p�����[�^�ǂݍ���
    double Dx = def_spec.Csw[SID_CAM1][SID_X][SID_D0];
    double Dy = def_spec.Csw[SID_CAM1][SID_Y][SID_D0];

    double Hx = def_spec.Csw[SID_CAM1][SID_X][SID_H0]+ def_spec.Csw[SID_CAM1][SID_X][SID_l0];
    double Hy = def_spec.Csw[SID_CAM1][SID_Y][SID_H0]+ def_spec.Csw[SID_CAM1][SID_Y][SID_l0];

    double ph0x = def_spec.Csw[SID_CAM1][SID_X][SID_ph0];
    double ph0y = def_spec.Csw[SID_CAM1][SID_Y][SID_ph0];

    //�@�J�������o�p�x
    sim_stat_workbuf.sway_io.rad[ID_SLEW] = atan((pCrane->l_mh * sin(phx) - Dx)/(pCrane->l_mh * cos(phx) - Hx)) - ph0x;
    sim_stat_workbuf.sway_io.rad[ID_BOOM_H] = atan((pCrane->l_mh * sin(phy) - Dy) / (pCrane->l_mh * cos(phy) - Hy)) - ph0y;
    //�@�J�������o�p���x
    sim_stat_workbuf.sway_io.w[ID_SLEW] = (sim_stat_workbuf.sway_io.rad[ID_SLEW]-radsl_last)/pCrane->dt;
    sim_stat_workbuf.sway_io.w[ID_BOOM_H] = (sim_stat_workbuf.sway_io.rad[ID_BOOM_H]-radbh_last) / pCrane->dt;

    sim_stat_workbuf.w_cam_x = sim_stat_workbuf.sway_io.w[ID_SLEW];
    sim_stat_workbuf.w_cam_y = sim_stat_workbuf.sway_io.w[ID_BOOM_H];

    radsl_last = sim_stat_workbuf.sway_io.rad[ID_SLEW];
    radbh_last = sim_stat_workbuf.sway_io.rad[ID_BOOM_H];

    return 0;
}
