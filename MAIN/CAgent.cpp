#include "CAgent.h"
#include "CPolicy.h"
#include "CEnvironment.h"

//-���L�������I�u�W�F�N�g�|�C���^:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

static CPolicy* pPolicy;
static CEnvironment* pEnv;

/****************************************************************************/
/*   �R���X�g���N�^�@�f�X�g���N�^                                           */
/****************************************************************************/
CAgent::CAgent() {
	pPolicyInf = NULL;
	pPLC_IO = NULL;
	pCraneStat = NULL;
}

CAgent::~CAgent() {

}

/****************************************************************************/
/*   �^�X�N����������                                                       */
/* �@���C���X���b�h�ŃC���X�^���X��������ɌĂт܂��B                       */
/****************************************************************************/

void CAgent::init_task(void* pobj) {

	//���L�������\���̂̃|�C���^�Z�b�g
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pCSInf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnv = (CEnvironment*)VectpCTaskObj[g_itask.environment];
	
	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;

	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;

	set_panel_tip_txt();

	inf.is_init_complete = true;
	return;
};

/****************************************************************************/
/*   �^�X�N���������                                                       */
/* �@�^�X�N�X���b�h�Ŗ��������s�����֐�			�@                      */
/****************************************************************************/
void CAgent::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//����������菇1�@�O���M�����͉��H����
void CAgent::input() {
	
	//�W���u���X�g�̃`�F�b�N ���@�R�}���h�̎�荞��
	if (can_job_trigger()) {														//�W���u�۔���
		if (pCSInf->job_list.hot_job_status & STAT_STANDBY) {						//���s�҂��W���u����
			pCom = pPolicy->req_command();											//�R�}���h��荞��
			if (pCom != NULL) {														//�R�}���h�X�e�[�^�X������											
				AgentInf_workbuf.auto_on_going = pCom->type;						//JOB or SEMIAUTO
				startup_command(pCom);												//��荞�񂾃R�}���h���s�p�ϐ������������ăX�e�[�^�X�����s���ɂ���
			}
		}
	}



	return;

};

//����������菇2�@���C������
void CAgent::main_proc() {

	//���s���̎������[�h�Z�b�g
	if ((pCSInf->antisway_mode == false) && (pCSInf->auto_mode == false)) {	//����OFF�@�U��~�߃��[�hOFF
		AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	}
	//j���s���R�}���h�L
	else if (pCom != NULL) {												//���s���R�}���h�L
		if (AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) {
			if (pCSInf->antisway_mode == L_ON)
				AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;
			else
				AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
		}

		if(pCom->type == AUTO_TYPE_JOB)		AgentInf_workbuf.auto_on_going |= AUTO_TYPE_JOB;
	    else								AgentInf_workbuf.auto_on_going |= AUTO_TYPE_SEMIAUTO;
	}
	//�U��~�ߖ������
	else if (AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) {
		if (pCSInf->antisway_mode == L_ON)
			AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;
		else
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
	}
	else {
		AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	}

	update_motion_setting();				//PLC��PC�w�߂œ��삷�鎲�̑I��,
	update_auto_control();					//�������s���[�h�̐ݒ�Ǝ������V�s�̃Z�b�g

	//PLC�ւ̏o�͌v�Z
	set_ref_mh();							//�������x�w��
	set_ref_gt();							//���s���x�w��
	set_ref_slew();							//���񑬓x�w��
	set_ref_bh();							//�������x�w��
	update_pb_lamp_com();					//PB LAMP�o��

	return;

}

//����������菇3�@�M���o�͏���
/****************************************************************************/
/*   �M���o��	���L�������o��												*/
/****************************************************************************/
void CAgent::output() {

	//�����֘A�񍐏���
	

	//���L�������o�͏���
	memcpy_s(pAgentInf, sizeof(ST_AGENT_INFO), &AgentInf_workbuf, sizeof(ST_AGENT_INFO));

	//�^�X�N�p�l���ւ̕\���o��
	wostrs << L" #SL TG:" << fixed<<setprecision(3) << AgentInf_workbuf.auto_pos_target.pos[ID_SLEW];
	wostrs << L",GAP: " << pEnv->cal_dist4target(ID_SLEW,false);

	wostrs << L"#BH TG: " << AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H];
	wostrs << L",GAP: " << pEnv->cal_dist4target(ID_BOOM_H, false);

	wostrs << L",ActiveSet: " << dbg_mont[0];
	
	wostrs <<  L" --Scan " << inf.period;;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   JOB�֘A����															*/
/****************************************************************************/
//�W���u�̋N���۔���
bool CAgent::can_job_trigger() { 
	if (AgentInf_workbuf.auto_on_going | CODE_TYPE_JOB) return false;
	return true; 
}

//�R�}���h��������
bool CAgent::is_command_completed(LPST_COMMAND_BLOCK pCom) {

	bool ans = true;
	
	if (pCom == NULL) return true;
	
	for (int i = 0;i < MOTION_ID_MAX;i++) {
		if (pCom->is_active_axis[i] == true) {
			if((pCom->motion_stat[i].status & MOTION_COMPLETE)) ans = false;
			break;
		}
	}

	return ans;
}

/****************************************************************************/
/*�@�@PC����I���Z�b�g����													*/
/****************************************************************************/
int CAgent::update_motion_setting() {
	/*

		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
	*/

	if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
	}
	else if((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO)||(AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
	}
	else if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
	}
	else {
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
	}

	AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;

	//PLC�ւ�PC�I���w�߃Z�b�g
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE) {
		AgentInf_workbuf.pc_ctrl_mode |= (BITSEL_HOIST | BITSEL_GANTRY | BITSEL_BOOM_H | BITSEL_SLEW);
	}
	else {
		AgentInf_workbuf.pc_ctrl_mode = 0;
		if (AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_HOIST;
		if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_GANTRY;
		if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_BOOM_H;
		if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_SLEW;
	}

	return 0;
}

/****************************************************************************/
/*	�����֘A�ݒ�					*/
/****************************************************************************/
//�U��~�ߊ�������
int CAgent::check_as_completion() {

	int check = AS_COMPLETE_0;

	//�U��U���A�ʒu����Ƃ��Ɋ������x���ȓ�
	if ((pEnv->cal_sway_r_amp2_m() < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
		&& (pEnv->cal_dist4target(ID_BOOM_H, true) < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_COMPLE]))

		check |= AS_COMPLETE_BH;


	if ((pEnv->cal_sway_th_amp2_m() < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
		&& (pEnv->cal_dist4target(ID_SLEW, true) < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_COMPLE]))

		check |= AS_COMPLETE_SLEW;


	//�U��U���A�ʒu����Ƃ��ɋN�����背�x���ȓ��@���@�U��~�ߊ����t���OON
	if (pAgentInf->antisway_comple_status | AS_COMPLETE_BH) {
		if ((pEnv->cal_sway_r_amp2_m() < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_TRIGGER])
			&& (pEnv->cal_dist4target(ID_BOOM_H, true) < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_TRIGGER]))

			check |= AS_COMPLETE_BH;
	}

	if (pAgentInf->antisway_comple_status | AS_COMPLETE_SLEW) {
		if ((pEnv->cal_sway_th_amp2_m() < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_TRIGGER])
			&& (pEnv->cal_dist4target(ID_SLEW, true) < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_TRIGGER]))

			check |= AS_COMPLETE_SLEW;
	}


	pAgentInf->antisway_comple_status = check;

	return check;
}

int CAgent::set_receipe_as_bh(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};
int CAgent::set_receipe_as_slw(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};

void CAgent::set_as_workbuf() {
	return;;
}

//�U��~�߃R�}���h�Z�b�g
int CAgent::setup_as_command() {

	pAgentInf->comset_as.is_active_axis[ID_HOIST] = false;
	pAgentInf->comset_as.is_active_axis[ID_SLEW] = true;
	pAgentInf->comset_as.is_active_axis[ID_BOOM_H] = true;

	set_as_workbuf(); //�U��~�߃p�^�[���쐬�p�f�[�^��荞��

	set_receipe_as_bh(&(AgentInf_workbuf.comset_as.recipe[ID_BOOM_H]), true, &st_as_work);
	set_receipe_as_slw(&(AgentInf_workbuf.comset_as.recipe[ID_SLEW]), true, &st_as_work);

	return 0;
}


//�����֘A�e��ݒ�E�U��~�߃��V�s�ݒ�
int CAgent::update_auto_control() {

	dbg_mont[0] = 0;//@@@ debug/

	/*### �U��~�ߊ�������@###*/
	AgentInf_workbuf.antisway_comple_status = check_as_completion();
					
	/*### �ڕW�ʒu�ݒ�@###*/

	//�����i�U��~�߁j���[�h�łȂ�
	if ((pCSInf->auto_mode == false) && (pCSInf->antisway_mode == false)){
		//�ڕW�ʒu�����݈ʒu
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			AgentInf_workbuf.auto_pos_target.pos[i] = pPLC_IO->status.pos[i];
			AgentInf_workbuf.auto_pos_target.is_held[i] = false;
		}
	}
	//�������s�@�}�j���A�����[�h
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
		//�ڕW�ʒu�ێ��t���O���������݈ʒu�@�ڕW�ʒu�ێ��t���O���聁�ڕW�ʒu�ύX�Ȃ����݈ʒu
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			//�m�b�`����ŖڕW�ێ��t���O�N���A
			if(pCraneStat->is_notch_0[i] == false)	AgentInf_workbuf.auto_pos_target.is_held[i] = false;

			//�ڕW�ʒu�ێ��t���O�؂�Ō��݈ʒu
			if(AgentInf_workbuf.auto_pos_target.is_held[i] == false) AgentInf_workbuf.auto_pos_target.pos[i] = pPLC_IO->status.pos[i];

		}
	}
	//�������s�@�U��~�ߎ��s�A�W���u���s���͕ύX�Ȃ�
	else {
		;//�R�}���h�N�����ɖڕW�ʒu�ƖڕW�ʒu�ێ��t���O��ݒ肷��B
	}

	/*### �U��~�߃R�}���h�ݒ�@###*/
	
	if (pCSInf->antisway_mode == L_ON){	//�U��~�߃��[�h
		//�U��~�ߖ����ŐU��~�߃R�}���h�����s���łȂ���΁A�U��~�߃R�}���h�ݒ�
		if((AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) &&  (AgentInf_workbuf.comset_as.com_status == STAT_REQ_WAIT)){	//�U��~�ߗp�R�}���h�Z�b�g���v���҂�
				setup_as_command();
		}
	}

	return 0;
};


/****************************************************************************/
/*   ���w�ߏo�͏���		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_HOIST) {
		if (AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
		else if ((AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_SEMIAUTO)) {
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_HOIST] = cal_step(pCom, ID_HOIST);
		}
		else
			AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
	}
	else {
		AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
	}
	return 0; 
}

/****************************************************************************/
/*   ���s�w�ߏo�͏���		                                                */
/****************************************************************************/
int CAgent::set_ref_gt(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_GANTRY) {
		if (AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_GANTRY] = pCraneStat->notch_spd_ref[ID_GANTRY];
		else if ((AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_SEMIAUTO)) {
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_GANTRY] = cal_step(pCom, ID_GANTRY);
		}

		else
			AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
	}
	else {
		AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
	}
	return 0;
}

/****************************************************************************/
/*   ����w�ߏo�͏���		                                                */
/****************************************************************************/
int CAgent::set_ref_slew(){

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_SLEW) {										//����PC�w�ߑI��ON
		if (AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_MANUAL)							//�}�j���A�����[�h
			AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];

		else if((AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_JOB)||
				(AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_SEMIAUTO)){					//�����^�]��
			if(pCom == NULL)	AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pCom, ID_SLEW);
		}
		else if (AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_FB_ANTI_SWAY){				//�U��~�ߒ�
			if (pAgentInf->antisway_comple_status | AS_COMPLETE_SLEW)								//�U��~�ߊ���
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else																					//�U��~�ߖ���
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(&pAgentInf->comset_as, ID_SLEW);	
		}
		else {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
		} 
	}
	else {
		AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
	}

	return 0;
}
/****************************************************************************/
/*   �����w�ߏo�͏���		                                                */
/****************************************************************************/
int CAgent::set_ref_bh(){

	LPST_COMMAND_BLOCK pcom;

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_BOOM_H) {									//����PC�w�ߑI��ON
		
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL)						//�}�j���A�����[�h
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];

		else if ((AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_SEMIAUTO)) {					//�����^�]��
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom, ID_BOOM_H);
		}
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_FB_ANTI_SWAY) {			//�U��~�ߒ�
			if(pAgentInf->antisway_comple_status | AS_COMPLETE_BH )									//�U��~�ߊ���
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else 																					//�U��~�ߖ���
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(&pAgentInf->comset_as, ID_BOOM_H);
		}
		else {
			AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
		}
	}
	else {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
	}

	return 0;
}

/****************************************************************************/
/*  �R�}���h�u���b�N���s�O����������                                        */
/*  ���s�Ǘ��X�e�[�^�X�̃N���A�ƃR�}���h���s���X�e�[�^�X�Z�b�g				*/
/****************************************************************************/
int CAgent::startup_command(LPST_COMMAND_BLOCK pcom) {

	for (int i = 0; i < MOTION_ID_MAX;i++) {						//�e���̎��s�X�e�[�^�X�̏�����
		if (AgentInf_workbuf.auto_active[i] == AUTO_TYPE_MANUAL) {
			pcom->motion_stat[i].status = MOTION_COMPLETE;
		}
		else {
			if (pCom->is_active_axis[i] == true) {
				pcom->motion_stat[i].status = MOTION_STANDBY;
				pcom->motion_stat[i].iAct = 0;
				pcom->motion_stat[i].step_act_count = 0;
				pcom->motion_stat[i].elapsed = 0;
				pcom->motion_stat[i].error_code = 0;

				for (int k = 0;k < pcom->recipe[i].n_step;k++) {
					pcom->recipe[i].steps[k].status = STEP_STANDBY;
					pcom->recipe[i].steps[k].time_count = 0;
				}
			}
			else pcom->motion_stat[i].status = MOTION_COMPLETE;
		}
	}
	pcom->com_status = STAT_ACTIVE;									//�R�}���h�X�e�[�^�X���s����
	GetLocalTime(&(pcom->time_start));								//�J�n���ԃZ�b�g

	return 0;
}

/****************************************************************************/
/*   STEP����		                                                        */
/****************************************************************************/
double CAgent::cal_step(LPST_COMMAND_BLOCK pCom,int motion) {


	double v_out = 0.0;

	LPST_MOTION_RECIPE precipe = &pCom->recipe[motion];
	LPST_MOTION_STAT pmotion_stat = &pCom->motion_stat[motion];
	LPST_MOTION_STEP pStep = &precipe->steps[pmotion_stat->iAct];


	if (pmotion_stat->status & MOTION_COMPLETE) {		 //���V�s�o�͊�����
		return 0.0;
	}
	else if (pStep->status == STAT_STANDBY) {   //�X�e�b�v�N����
		pmotion_stat->step_act_count= 1;
		pStep->status = STAT_ACTIVE;			//�X�e�b�v���s���X�e�b�v�o�͒��ɍX�V
	}
	else	pmotion_stat->step_act_count++;			//�X�e�b�v���s�J�E���g�C���N�������g



	switch (pStep->type) {
#if 0
		//#	���ԑҋ@
	case CTR_TYPE_TIME_WAIT: {
		pstat->direction = AGENT_STOP;
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = pStep->_v;
	}break;
		//#	�U��ʑ��҂��Q����
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		pstat->direction = AGENT_NA;	//�����܂ňړ���������
		bool is_step_end = false;
		v_out = pStep->_v;

		//�ُ튮������
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//�w��͈͊O�@-�΁`��
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//�w��͈͊O�@-�΁`��
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
			pStep->status = PTN_STEP_FIN;is_step_end = true;
		}
		if (is_step_end) {
			pstat->direction = AGENT_STOP;
			v_out = 0.0;
			break;
		}


		//���C������
		//���]�J�n�p�ʑ�����
		double chk_ph = PI360;
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_F] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if (pStep->opt_d[MOTHION_OPT_PHASE_F] > pSway_IO->ph[motion]) {
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F];
		}

		if (chk_ph < ph_chk_range[motion]) {	//�ڕW�ʑ��ɓ��B
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_FWD;
			if (motion == ID_SLEW)
				v_out += 0.0;
			break;
		}

		//�t�]�J�n�p�ʑ�����
		chk_ph = PI360;
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_R] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];	
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if(pStep->opt_d[MOTHION_OPT_PHASE_R] > pSway_IO->ph[motion]){
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R];
		}
		if (chk_ph < ph_chk_range[motion]) {	//�ڕW�ʑ��ɓ��B
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_REW;
			if (motion == ID_SLEW)
				v_out += 0.0;
			break;
		}

	}break;
	//#	�U��ʑ��҂�1����
	case CTR_TYPE_SINGLE_PHASE_WAIT: {

		bool is_step_end = false;
		v_out = pStep->_v;

		//�ُ튮������
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//�w��͈͊O�@-�΁`��
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//�w��͈͊O�@-�΁`��
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
			pStep->status = PTN_STEP_FIN;is_step_end = true;
		}
		if (is_step_end) {
			pstat->direction = AGENT_STOP;
			v_out = 0.0;
			break;
		}


		//���C������
		//�J�n�p�ʑ�����
		int dir;
		double target_ph;

		if (motion == ID_SLEW) {
			if ((pAgentInf->pos_target[motion] - pPLC_IO->status.pos[motion]) < -PI180) {
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else if ((pAgentInf->pos_target[motion] - pPLC_IO->status.pos[motion]) > PI180) {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
			else if (pPLC_IO->status.pos[motion] < pAgentInf->pos_target[motion]) {
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
		}
		else {
			if (pPLC_IO->status.pos[motion] < pAgentInf->pos_target[motion]) {//�ڕW�ʒu����O
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
		}
		
		double chk_ph = PI360;
		if ((target_ph <= 0) && (pSway_IO->ph[motion] >= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = pSway_IO->ph[motion] - target_ph;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if ((target_ph >= 0) && (pSway_IO->ph[motion] <= 0)) {	//�ڕW�����@���ݒl����
			chk_ph = target_ph - pSway_IO->ph[motion];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//���������̊p�x��
		}
		else if (target_ph > pSway_IO->ph[motion]) {
			chk_ph = target_ph - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - target_ph;
		}

		if (chk_ph < ph_chk_range[motion]) {	//�ڕW�ʑ��ɓ��B
			pStep->status = PTN_STEP_FIN;
			pstat->direction = dir;
			break;
		}

	}break;

	//#	�U��~�߈ړ��N���^�C�~���O�����i�����U�ꂪ�傫���Ƃ������J�n�^�C�~���O�𒲐��j
	case CTR_TYPE_ADJUST_MOTION_TRIGGER: {
		v_out = 0.0;
	//	double rad_acc = pEnv->cal_arad_acc(motion, FWD);		//�����U��p
	//	double rad_acc2 =pEnv-> rad_acc * rad_acc;				//�����U��p�Q��
		double rad_acc2 =pEnv->cal_arad2(motion,FWD);			//�����U��p�Q��

		if (sqrt(pSway_IO->rad_amp2[motion]) > rad_acc2) { // �U��p�U���������U��p���傫��
			if (AgentInf_workbuf.gap_from_target[motion] > 0) {//�ڕW�ʒu��茻�݈ʒu����O���i�s�����{
				if ((pSway_IO->ph[motion] < PI180) && (pSway_IO->ph[motion] > PI90))
					pStep->status = PTN_STEP_FIN;
			}
			else {
				if ((pSway_IO->ph[motion] < 0.0) && (pSway_IO->ph[motion] > -PI90))
					pStep->status = PTN_STEP_FIN;
			}
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_ON;
		}
		else if (AgentInf_workbuf.sway_amp2m[motion] > AGENT_CHECK_LARGE_SWAY_m2) {
			if (AgentInf_workbuf.gap_from_target[motion] > 0) {//�ڕW�ʒu��茻�݈ʒu����O���i�s�����{
				if ((pSway_IO->ph[motion] < PI90) && (pSway_IO->ph[motion] > 0.0))
					pStep->status = PTN_STEP_FIN;
			}
			else {
				if ((pSway_IO->ph[motion] < -PI90) && (pSway_IO->ph[motion] > -PI180))
					pStep->status = PTN_STEP_FIN;
			}
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_ON;
		}
		else {
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_OFF;
		}

		//�^�C���I�[�o�[
		if (pstat->step_act_count >= pStep->time_count) {
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_OFF;
			pStep->status = PTN_STEP_FIN;
		}

	}break;

		//#	�U��~�߉���
	case CTR_TYPE_ACC_AS: {
		if (pstat->step_act_count >= pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = (double)pstat->direction * pStep->_v;

	}break;

	case CTR_TYPE_ACC_V: {
		if (pstat->step_act_count >= pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = pStep->_v;

	}break;
	//#	����
	case  CTR_TYPE_DEC_V: {
		if (pStep->_v < 0.0) {
			if (pPLC_IO->status.v_fb[motion] >= pStep->_v) pStep->status = PTN_STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (pPLC_IO->status.v_fb[motion] <= pStep->_v) pStep->status = PTN_STEP_FIN;
		}
		else;
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;

	}break;
	//#	�葬�i���������葬�j
	case CTR_TYPE_CONST_V_TIME: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;
	}break;
	//#	�葬�i���������葬�j
	case CTR_TYPE_CONST_V_ACC_STEP:
	{
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if(pstat->flg[MOTION_ACC_STEP_BYPASS] == L_ON )	//�����U�ꂪ�傫���Ƃ�1��̉����ɂ���t���O
			pStep->status = PTN_STEP_FIN;
		v_out = pStep->_v;
	}break;
	case CTR_TYPE_CONST_V_DEC_STEP:
	{
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pstat->flg[MOTION_DEC_STEP_BYPASS] == L_ON)	//�����U�ꂪ�傫���Ƃ�1��̌����ɂ���t���O
			pStep->status = PTN_STEP_FIN;
		v_out = pStep->_v;
	}break;

	//#	�葬�i���������葬�j
	case CTR_TYPE_CONST_V_TOP_STEP:
	{
		double chk_d = pStep->_p - pPLC_IO->status.pos[motion];
		if (chk_d < 0.0) chk_d *= -1.0; //STEP�ڕW�ʒu�܂ł̋���

		if ((pStep->_v >= 0.0) && (pPLC_IO->status.pos[motion] >= pStep->_p)) {			//�O�i�@�ڕW�ʒu���B
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if ((pStep->_v <= 0.0) && (pPLC_IO->status.pos[motion] <= pStep->_p)) {	//��i�@�ڕW�ʒu���B
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if (chk_d < AgentInf_workbuf.dist_for_stop[motion]) {						// STEP�ڕW�ʒu�܂ł̋��������������ȉ�

			double rad_acc2 = pEnv->cal_arad2(motion, FWD);	//�����U��p2��
			if (pEnv->is_sway_larger_than_accsway(motion)) {							// �U�ꂪ�����U�����
				if (AgentInf_workbuf.gap_from_target[motion] > 0) {						//�ڕW�ʒu��茻�݈ʒu����O���i�s�����{
					if ((pSway_IO->ph[motion] < PI180) && (pSway_IO->ph[motion] > PI90))//��/2-�΂̈ʑ��Ȃ犮��->�����ɓ���
						pStep->status = PTN_STEP_FIN;
				}
				else {																	//�ڕW�ʒu��茻�݈ʒu����끨�i�s����-
					if ((pSway_IO->ph[motion] < 0.0) && (pSway_IO->ph[motion] > -PI90))	//0- -��/2�̈ʑ��Ȃ犮��->�����ɓ���
						pStep->status = PTN_STEP_FIN;
				}
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_ON;
			}
			else if (AgentInf_workbuf.sway_amp2m[motion] > AGENT_CHECK_LARGE_SWAY_m2) {
				if (AgentInf_workbuf.gap_from_target[motion] > 0) {//�ڕW�ʒu��茻�݈ʒu����O���i�s�����{
					if ((pSway_IO->ph[motion] < PI90) && (pSway_IO->ph[motion] > 0.0))
						pStep->status = PTN_STEP_FIN;
				}
				else {
					if ((pSway_IO->ph[motion] < -PI90) && (pSway_IO->ph[motion] > -PI180))
						pStep->status = PTN_STEP_FIN;
				}
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_ON;
			}
			else;
		}
		else {
			if (pstat->step_act_count >= pStep->time_count) {
				pStep->status = PTN_STEP_FIN;
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
			}
		}

		v_out = pStep->_v;

	}break;

	//#	�葬�i���������葬�j
	case CTR_TYPE_FINE_POSITION: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pAgentInf->gap2_from_target[motion] < pCraneStat->spec.as_pos2_level[motion][ID_LV_COMPLE])
			pStep->status = PTN_STEP_FIN;

		double dir = 0.0;
		if (pPLC_IO->status.pos[motion] > AgentInf_workbuf.pos_target[motion]) dir = -1.0;
		else dir = 1.0;

		v_out = (double)dir * pStep->_v;
	}break;

	//#	���莲�̈ʒu�҂�
	case CTR_TYPE_OTHER_POS_WAIT: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (motion == ID_SLEW) {
			if (pAgentInf->gap2_from_target[ID_BOOM_H] < pStep->opt_d[MOTHION_OPT_WAIT_POS])
				pStep->status = PTN_STEP_FIN;
		}
		else if (motion == ID_BOOM_H) {
			if (pAgentInf->gap2_from_target[ID_SLEW] < pStep->opt_d[MOTHION_OPT_WAIT_POS])
				pStep->status = PTN_STEP_FIN;
		}
		else {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = 0.0;

	}break;
#endif
	default:
		v_out = 0.0;
		break;
}

	if (pStep->status & STEP_FIN) {
		pmotion_stat->iAct++;
		if (pmotion_stat->iAct >= precipe->n_step) pmotion_stat->status = MOTION_COMPLETE;
	}

	return v_out;
}

/****************************************************************************/
/*  PB�w�ߍX�V	(����~,�劲PB���j										*/
/****************************************************************************/

void CAgent::update_pb_lamp_com() {
	//����PB(��芸����PLC���͒l��荞�݁iPLC IO�ɂ�OFF DELAY�g�ݍ��ݍρj
	AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] = pPLC_IO->ui.PB[ID_PB_ESTOP];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_ON];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_OFF];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_ON];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_OFF];
	AgentInf_workbuf.PLC_PB_com[ID_PB_FAULT_RESET] = pPLC_IO->ui.PB[ID_PB_FAULT_RESET];
	
	return;
};

/****************************************************************************/
/*   �^�X�N�ݒ�^�u�p�l���E�B���h�E�̃R�[���o�b�N�֐�                       */
/****************************************************************************/
LRESULT CALLBACK CAgent::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_TASK_FUNC_RADIO1:
		case IDC_TASK_FUNC_RADIO2:
		case IDC_TASK_FUNC_RADIO3:
		case IDC_TASK_FUNC_RADIO4:
		case IDC_TASK_FUNC_RADIO5:
		case IDC_TASK_FUNC_RADIO6:
			inf.panel_func_id = LOWORD(wp); set_panel_tip_txt(); set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0); 
			reset_panel_item_pb(hDlg);
			break;

		case IDC_TASK_ITEM_RADIO1:
		case IDC_TASK_ITEM_RADIO2:
		case IDC_TASK_ITEM_RADIO3:
		case IDC_TASK_ITEM_RADIO4:
		case IDC_TASK_ITEM_RADIO5:
		case IDC_TASK_ITEM_RADIO6:
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO6) {
				if (inf.panel_type_id == IDC_TASK_ITEM_RADIO1) {
					;
				}
			}
			break;
		case IDSET: {
			wstring wstr, wstr_tmp;

			//�T���v���Ƃ��Ă��낢��Ȍ^�œǂݍ���ŕ\�����Ă���
			wstr += L"Param 1(d):";
			int n = GetDlgItemText(hDlg, IDC_TASK_EDIT1, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stod(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 2(i):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT2, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stoi(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 3(f):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT3, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stof(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 4(l):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT4, (LPTSTR)wstr_tmp.c_str(), 128);
			if (n) wstr_tmp = to_wstring(stol(wstr_tmp));	wstr = wstr + wstr_tmp.c_str();

			wstr += L",  Param 5(c):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT5, (LPTSTR)wstr_tmp.c_str(), 128);
			wstr += wstr_tmp.c_str();

			wstr += L",   Param 6(c):";
			n = GetDlgItemText(hDlg, IDC_TASK_EDIT6, (LPTSTR)wstr_tmp.c_str(), 128);
			wstr += wstr_tmp.c_str();

			txout2msg_listbox(wstr);


		}break;
		case IDRESET: {
			set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			reset_panel_func_pb(hDlg);
			reset_panel_item_pb(hDlg);
		}break;

		case IDC_TASK_OPTION_CHECK1:
			SendMessage(GetDlgItem(hDlg, IDC_TASK_OPTION_CHECK2), BM_SETCHECK, BST_UNCHECKED, 0L);
			if (IsDlgButtonChecked(hDlg, IDC_TASK_OPTION_CHECK1) == BST_CHECKED) inf.work_select = THREAD_WORK_OPTION1;
			else inf.work_select = THREAD_WORK_ROUTINE;
			break;

		case IDC_TASK_OPTION_CHECK2:
			SendMessage(GetDlgItem(hDlg, IDC_TASK_OPTION_CHECK1), BM_SETCHECK, BST_UNCHECKED, 0L);
			if (IsDlgButtonChecked(hDlg, IDC_TASK_OPTION_CHECK2) == BST_CHECKED) inf.work_select = THREAD_WORK_OPTION2;
			else inf.work_select = THREAD_WORK_ROUTINE;
			break;
		}
	}
	return 0;
};

/****************************************************************************/
/*   �^�X�N�ݒ�p�l���̑���{�^�������e�L�X�g�ݒ�֐�                       */
/****************************************************************************/
void CAgent::set_panel_tip_txt()
{
	wstring wstr_type; wstring wstr;
	switch (inf.panel_func_id) {
	case IDC_TASK_FUNC_RADIO1: {
		wstr = L"Type for Func1 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO2: {
		wstr = L"Type of Func2 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO3: {
		wstr = L"Type for Func3 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO4: {
		wstr = L"Type for Func4 \n\r 1:VP 2 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO5: {
		wstr = L"Type for Func5 \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	case IDC_TASK_FUNC_RADIO6: {
		wstr = L"Func6(Debug) \n\r 1:SIM 2:PLC 3:SWAY 4:RIO 5:?? 6:??";
		switch (inf.panel_type_id) {
		case IDC_TASK_ITEM_RADIO1:
			wstr_type += L"Param of type1 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO2:
			wstr_type += L"Param of type2 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO3:
			wstr_type += L"Param of type3 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO4:
			wstr_type += L"Param of type4 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO5:
			wstr_type += L"Param of type5 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		case IDC_TASK_ITEM_RADIO6:
			wstr_type += L"Param of type6 \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
			break;
		default:break;
		}
	}break;
	default: {
		wstr = L"Type for Func? \n\r 1:?? 2:?? 3:?? \n\r 4:?? 5:?? 6:??";
		wstr_type += L"(Param of type?) \n\r 1:?? 2:??  3:?? \n\r 4:?? 5:?? 6:??";
	}break;
	}

	SetWindowText(GetDlgItem(inf.hWnd_opepane, IDC_STATIC_TASKSET2), wstr.c_str());
	SetWindowText(GetDlgItem(inf.hWnd_opepane, IDC_STATIC_TASKSET3), wstr_type.c_str());
}

/****************************************************************************/
/*�@�@�^�X�N�ݒ�p�l���{�^���̃e�L�X�g�Z�b�g					            */
/****************************************************************************/
void CAgent::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};

