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
	for (int i = 0;i < N_PLC_LAMP;i++) AgentInf_workbuf.PLC_LAMP_com[i] = 0;
	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;

	for (int i = 0;i < NUM_OF_AS_AXIS;i++) ph_chk_range[i] = PHASE_CHECK_RANGE;
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

	parse_indata();
	return;

};

//����������菇2�@���C������
void CAgent::main_proc() {

	update_auto_setting();	//�������s���[�h�̐ݒ�Ǝ������V�s�̃Z�b�g
	set_pc_control();		//PLC��PC�w�߂œ��삷�鎲�̑I��,

	//PLC�ւ̏o�͌v�Z
	set_ref_mh();			//�������x�w��
	set_ref_gt();			//���s���x�w��
	set_ref_slew();			//���񑬓x�w��
	set_ref_bh();			//�������x�w��
	update_pb_lamp_com();	//PB LAMP�o��

	return;

}

//����������菇3�@�M���o�͏���
/****************************************************************************/
/*   �M���o��	���L�������o��								*/
/****************************************************************************/
void CAgent::output() {

	//���L�������o�͏���
	memcpy_s(pAgentInf, sizeof(ST_AGENT_INFO), &AgentInf_workbuf, sizeof(ST_AGENT_INFO));

	wostrs << L" #SL TG:" << fixed<<setprecision(3) << AgentInf_workbuf.pos_target[ID_SLEW];
	wostrs << L",GAP: " << AgentInf_workbuf.gap_from_target[ID_SLEW];;

	wostrs << L"#BH TG: " << AgentInf_workbuf.pos_target[ID_BOOM_H];
	wostrs << L",GAP: " << AgentInf_workbuf.gap_from_target[ID_BOOM_H];

	wostrs << L",ActiveSet: " << dbg_mont[0];
	
	wostrs <<  L" --Scan " << inf.period;;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   ���͐M���̕���															*/
/*   ��������,�@0������,�@�ڕW�܂ł̋���,�@������������p�f�[�^				*/
/****************************************************************************/
int CAgent::parse_indata() {

	//0���`�F�b�N,���������v�Z
	for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
		//0���`�F�b�N
		if ((pPLC_IO->status.v_fb[i] >= pCraneStat->spec.notch_spd_f[i][NOTCH_1] * SPD0_CHECK_RETIO) ||
			(pPLC_IO->status.v_fb[i] <= pCraneStat->spec.notch_spd_r[i][NOTCH_1] * SPD0_CHECK_RETIO)) {//1�m�b�`��10�����x�ȏ�
			AgentInf_workbuf.is_spdfb_0[i] = false;	//0���łȂ�
		}
		else if (pCraneStat->is_notch_0[i] == false) {//�m�b�`0�Ŗ���
			AgentInf_workbuf.is_spdfb_0[i] = false;
		}
		else {
			AgentInf_workbuf.is_spdfb_0[i] = true;
		}

		//��������
		if (pPLC_IO->status.v_fb[i] < 0.0) {
			AgentInf_workbuf.dist_for_stop[i]
				= pPLC_IO->status.v_fb[i] * (-0.5 * pPLC_IO->status.v_fb[i] / pCraneStat->spec.accdec[i][ID_REV][ID_DEC] + pCraneStat->spec.delay_time[i][ID_DELAY_CNT_DEC]);

		}
		else {
			AgentInf_workbuf.dist_for_stop[i]
				= pPLC_IO->status.v_fb[i] * (-0.5 * pPLC_IO->status.v_fb[i] / pCraneStat->spec.accdec[i][ID_FWD][ID_DEC] + pCraneStat->spec.delay_time[i][ID_DELAY_CNT_DEC]);
		}
	}

	//������������

	//�U��U����2�悍
	double k = pCraneStat->mh_l * pCraneStat->mh_l;
	AgentInf_workbuf.sway_amp2m[ID_BOOM_H] = k * pSway_IO->rad_amp2[ID_BOOM_H];
	AgentInf_workbuf.sway_amp2m[ID_SLEW] = k * pSway_IO->rad_amp2[ID_SLEW];

	for (int i = 0;i < MOTION_ID_MAX;i++) { //�ڕW�ʒu�܂ł̋���
		AgentInf_workbuf.gap_from_target[i] = AgentInf_workbuf.pos_target[i] - pPLC_IO->status.pos[i];
		if (i == ID_SLEW){
			if (AgentInf_workbuf.gap_from_target[i] > PI180) AgentInf_workbuf.gap_from_target[i] -= PI360;
			else if(AgentInf_workbuf.gap_from_target[i] < - PI180) AgentInf_workbuf.gap_from_target[i] += PI360;
		}
	}

	AgentInf_workbuf.gap2_from_target[ID_BOOM_H] = AgentInf_workbuf.gap_from_target[ID_BOOM_H] * AgentInf_workbuf.gap_from_target[ID_BOOM_H];
	AgentInf_workbuf.gap2_from_target[ID_SLEW] = AgentInf_workbuf.gap_from_target[ID_SLEW]* AgentInf_workbuf.gap_from_target[ID_SLEW];

	//�N���J�n�ʑ�����͈́i�N���x�ꎞ�ԕ␳�j
	ph_chk_range[ID_BOOM_H] = pCraneStat->w * pCraneStat->spec.delay_time[ID_BOOM_H][ID_DELAY_0START];
	ph_chk_range[ID_SLEW] = pCraneStat->w * pCraneStat->spec.delay_time[ID_SLEW][ID_DELAY_0START];
	
	return 0;
}

/****************************************************************************/
/*�@�@PC����I���Z�b�g����													*/
/****************************************************************************/
int CAgent::set_pc_control() {

	//�f�o�b�O���[�h�v��
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE) {
		AgentInf_workbuf.pc_ctrl_mode |= (BITSEL_HOIST | BITSEL_GANTRY | BITSEL_BOOM_H | BITSEL_SLEW);
	}
	else {
		AgentInf_workbuf.pc_ctrl_mode = 0;
		if(AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_HOIST;
		if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_GANTRY;
		if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_BOOM_H;
		if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_SLEW;
	}

	return 0;
}

/****************************************************************************/
/*   ���w�ߏo�͏���		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_HOIST) {
		if (AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
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

	LPST_COMMAND_BLOCK pcom;

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_SLEW) {
		if (AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];
		else if(AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_JOB){
			pcom = &(pPolicyInf->job_com[pPolicyInf->i_jobcom]);
			AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
		}
		else {
			pcom = &(pPolicyInf->com[pPolicyInf->i_com]);
			if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_END) {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_STANDBY) {
				pcom->motion_stat[ID_SLEW].status = COMMAND_STAT_ACTIVE;
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_ACTIVE) {
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
			}
			else {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}

		//	AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
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

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_BOOM_H) {
		
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_JOB) {
			pcom = &(pPolicyInf->job_com[pPolicyInf->i_jobcom]);
			AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pcom,ID_BOOM_H);
		}
		else {
			pcom = &(pPolicyInf->com[pPolicyInf->i_com]);
			if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_END) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
			else if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_STANDBY) {
				pcom->motion_stat[ID_BOOM_H].status = COMMAND_STAT_ACTIVE;
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
			else if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_ACTIVE) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom,ID_BOOM_H);
			}
			else {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
		}
	}
	else {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
	}

	return 0;
}

/****************************************************************************/
/*   �R�}���h�X�e�[�^�X������                                               */
/****************************************************************************/
int CAgent::cleanup_command(LPST_COMMAND_BLOCK pcom) {
	pcom->com_status = COMMAND_STAT_STANDBY;
	for (int i = 0; i < MOTION_ID_MAX;i++) {
		if (AgentInf_workbuf.auto_active[i] == AUTO_TYPE_MANUAL) {
			pcom->motion_stat[i].status = COMMAND_STAT_END;
		}
		else {
			pcom->motion_stat[i].status = COMMAND_STAT_STANDBY;
		}
		pcom->motion_stat[i].iAct = 0;
		pcom->motion_stat[i].step_act_count = 0;
		pcom->motion_stat[i].direction = AGENT_STOP;
		for (int j = 0; j < MOTION_STAT_FLG_N; j++) {
			pcom->motion_stat[i].flg[j] = L_OFF;
		}
	}
	pcom->com_status = COMMAND_STAT_ACTIVE;
	GetLocalTime(&(pcom->time_start));	//�J�n���ԃZ�b�g
	return 0;
}

/****************************************************************************/
/*   STEP����		                                                        */
/****************************************************************************/
double CAgent::cal_step(LPST_COMMAND_BLOCK pCom,int motion) {

	double v_out = 0.0;

	LPST_MOTION_RECIPE precipe = &pCom->recipe[motion];
	LPST_MOTION_STAT pstat = &pCom->motion_stat[motion];
	LPST_MOTION_STEP pStep = &precipe->steps[pstat->iAct];

	if (pstat->status == COMMAND_STAT_END) {
		return 0.0;
	}
	else if (pStep->status == COMMAND_STAT_STANDBY) {
		pstat->step_act_count= 1;
		pStep->status = COMMAND_STAT_ACTIVE;
	}
	else	pstat->step_act_count++;	//�X�e�b�v���s�J�E���g�C���N�������g

	pStep->status = PTN_STEP_ON_GOING;

	switch (pStep->type) {
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
		double rad_acc = pEnv->cal_arad(motion, FWD);	//�����U��p
		double rad_acc2 = rad_acc * rad_acc;											//�����U��p�Q��

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

	default:
		v_out = 0.0;
//		pStep->status = PTN_STEP_FIN;
		break;
}

	if (pStep->status == PTN_STEP_FIN) {
		pstat->iAct++;
		pstat->step_act_count = 0;
		if (pstat->iAct >= precipe->n_step)
			pstat->status = COMMAND_STAT_END;
	}
	if ((pStep->status == PTN_STEP_ERROR)||(pStep->status == PTN_STEP_TIME_OVER)) {
		pstat->iAct++;
		pstat->step_act_count = 0;
		if (pstat->iAct >= precipe->n_step)
			pstat->status = COMMAND_STAT_END;
	}

	return v_out;
}

/****************************************************************************/
/*  �����N���ۃ`�F�b�N													*/
/****************************************************************************/
bool CAgent::can_auto_trigger()
{
	if ((pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_ACTIVE) &&
		(pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_STANDBY) &&
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_ACTIVE)&&
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_STANDBY)) {

		if ((AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL)&&(pCraneStat->auto_standby == true)) {
			//����,�����0���m�F�ŋN����
			if (AgentInf_workbuf.is_spdfb_0[ID_SLEW] && AgentInf_workbuf.is_spdfb_0[ID_BOOM_H]) {
				if ((AgentInf_workbuf.sway_amp2m[ID_BOOM_H] > pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_TRIGGER])
					|| (AgentInf_workbuf.sway_amp2m[ID_SLEW] > pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_TRIGGER])
					|| (AgentInf_workbuf.gap2_from_target[ID_BOOM_H] > pCraneStat->spec.as_pos2_level[ID_BOOM_H][ID_LV_TRIGGER])
					|| (AgentInf_workbuf.gap2_from_target[ID_SLEW] > pCraneStat->spec.as_pos2_level[ID_SLEW][ID_LV_TRIGGER])){
					return true;
				}
			}
			if ((pCraneStat->semi_auto_selected != SEMI_AUTO_TG_CLR)
				&& (pCraneStat->auto_start_pb_count > AGENT_AUTO_TRIG_ACK_COUNT)) {
				return true;
			}
		}
		else return true;
	}
	else {
		return false;
	}

	return false;
}

/****************************************************************************/
/*  ������������															*/
/****************************************************************************/
bool CAgent::can_auto_complete() {
		
	//���s���܂��͎��s�҂��R�}���h�L
	if ((pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_ACTIVE) ||
		(pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_STANDBY) ||
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_ACTIVE) ||
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_STANDBY)){ 
		return false;
	}
	//�U��~�߂͐U��U�������ڕW�ʒu�����Ŋ���
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_ANTI_SWAY) {
		if ((AgentInf_workbuf.sway_amp2m[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.sway_amp2m[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE]))
			return true;
		else return false;
	}
	//�������͖ڕW�ʒu�����Ŋ���
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_SEMI_AUTO) {
		if ((AgentInf_workbuf.gap2_from_target[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE]))
			return true;
		else return false;
	}
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_JOB) {
		if (pCSInf->n_job_standby < 1) //�ҋ@�W���u����
			return true;
		else 
			return false;
	}
	else return false;
}

/****************************************************************************/
/*  �ʎ����Ɏ��s���̎����^�C�v�Z�b�g											*/
/****************************************************************************/
void CAgent::set_auto_active(int type) {
	if (type == AUTO_TYPE_ANTI_SWAY) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_ANTI_SWAY;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_ANTI_SWAY;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else if (type == AUTO_TYPE_SEMI_AUTO) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_SEMI_AUTO;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_SEMI_AUTO;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else if (type == AUTO_TYPE_JOB) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
}

/****************************************************************************/
/*  ���s�����^�C�v�i�S�́j�Z�b�g,�@�蓮���ڕW�ʒu�Z�b�g,�������V�s�Z�b�g	*/
/****************************************************************************/
int CAgent::update_auto_setting() {
	
	dbg_mont[0] = 0;//@@@ debug/

	//�����N������
	if (pCraneStat->auto_standby == false) {//�����i�U��~�߁j���[�h�łȂ�
	
		//�蓮���ڕW�ʒu
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			AgentInf_workbuf.pos_target[i] = pPLC_IO->status.pos[i];
			AgentInf_workbuf.be_hold_target[i] = false;
		}

		if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
			set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 1;//@@@ debug/
			pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_END;
			pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_END;
		}
	}
	else if (can_auto_complete()) {
		pCom->com_status = COMMAND_STAT_END;
		pCom = NULL;
		AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
		set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 2;//@@@ debug/
	}
	else if ((pCraneStat->is_notch_0[ID_SLEW] == false) || (pCraneStat->is_notch_0[ID_BOOM_H]) == false) {
		//�蓮���ڕW�ʒu
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			AgentInf_workbuf.pos_target[i] = pPLC_IO->status.pos[i];
			AgentInf_workbuf.be_hold_target[i] = false;
		}
		if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
			set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 9;//@@@ debug/
			if (pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_END) pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_END) pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ABORT;
		}
		
		if(pCraneStat->is_notch_0[ID_SLEW] == false) AgentInf_workbuf.be_hold_target[ID_SLEW] = false;
		if (pCraneStat->is_notch_0[ID_BOOM_H] == false) AgentInf_workbuf.be_hold_target[ID_BOOM_H] = false;
	}
	else if(can_auto_trigger()) {
		if ((pCraneStat->semi_auto_selected != SEMI_AUTO_TG_CLR)
			&&(pCraneStat->auto_start_pb_count > AGENT_AUTO_TRIG_ACK_COUNT)) {	//�������ݒ�L�Ŏ����J�nPB ON
	
			//�R�}���h����
			pCom = pPolicy->generate_command(AUTO_TYPE_SEMI_AUTO, AgentInf_workbuf.pos_target);

			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_SEMI_AUTO;
				set_auto_active(AUTO_TYPE_SEMI_AUTO);	dbg_mont[0] = 3;//@@@ debug/
				cleanup_command(pCom);
				//�ڕW�L�[�v�t���O
				for (int i = 0; i < NUM_OF_AS_AXIS; i++) AgentInf_workbuf.be_hold_target[i] = true;
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 10;//@@@ debug/
			}
		}
		else if ((pCSInf->n_job_standby > 0) 
			&& (pCraneStat->auto_start_pb_count > AGENT_AUTO_TRIG_ACK_COUNT)) {
	
			//�R�}���h����
			pCom = pPolicy->generate_command(AUTO_TYPE_JOB, AgentInf_workbuf.pos_target);
			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_JOB;
				set_auto_active(AUTO_TYPE_JOB);	dbg_mont[0] = 4;//@@@ debug/
				cleanup_command(pCom);
				//�ڕW�L�[�v�t���O
				for (int i = 0; i < NUM_OF_AS_AXIS; i++) AgentInf_workbuf.be_hold_target[i] = true;
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 5;//@@@ debug/
			}
		}
		else {

			//�R�}���h����
			pCom = pPolicy->generate_command(AUTO_TYPE_ANTI_SWAY, AgentInf_workbuf.pos_target);
			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_ANTI_SWAY;
				set_auto_active(AUTO_TYPE_ANTI_SWAY);	dbg_mont[0] = 6;//@@@ debug/
				cleanup_command(pCom);
				//�ڕW�L�[�v�t���O
				for (int i = 0; i < NUM_OF_AS_AXIS; i++) AgentInf_workbuf.be_hold_target[i] = true;
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);	dbg_mont[0] = 7;//@@@ debug/
			}
		}
	}
	else;

	//�R�}���h���s��������
	if (pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_ACTIVE) {
		bool is_active_motion_there = false;
		for (int i = 0; i < MOTION_ID_MAX;i++) {
			
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_END) {
				AgentInf_workbuf.auto_active[i] = AUTO_TYPE_MANUAL; //�����[�h���}�j���A����
			}

			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ERROR)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ERROR;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ABORT)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_PAUSE)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_PAUSE;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ACTIVE) {
				is_active_motion_there = true;
			}
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_STANDBY) {
				is_active_motion_there = true;
			}
		}
		if (is_active_motion_there == false)
			pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_END;

	}

	if (pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_ACTIVE) {
		bool is_active_motion_there = false;
		for (int i = 0; i < MOTION_ID_MAX;i++) {
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_END) {
				AgentInf_workbuf.auto_active[i] = AUTO_TYPE_MANUAL; //�����[�h���}�j���A����
			}
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ERROR)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ERROR;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ABORT)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_PAUSE)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_PAUSE;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ACTIVE) {
				is_active_motion_there = true;
			}
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_STANDBY) {
				is_active_motion_there = true;
			}
		}
		if (is_active_motion_there == false)
			pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_END;
	}

	return 0;
};

/****************************************************************************/
/*  PB,�����v�w�ߍX�V														*/
/****************************************************************************/
static bool ctr_soure1_on_last, ctr_soure1_off_last, ctr_soure2_on_last, ctr_soure2_off_last;

void CAgent::update_pb_lamp_com() {
	//����PB
	//PB ON��Ԃ���莞�ԃz�[���h
	//�J�E���^�l�Z�b�g
	if (pPLC_IO->ui.PB[ID_PB_ESTOP]) AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] = AGENT_PB_OFF_DELAY;
	if ((pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_ON] == true) && (ctr_soure1_on_last == false))AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] = AGENT_PB_OFF_DELAY;
	if ((pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_OFF] == true) && (ctr_soure1_off_last == false))AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] = AGENT_PB_OFF_DELAY;
	if ((pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_ON] == true) && (ctr_soure2_on_last == false))AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] = AGENT_PB_OFF_DELAY;
	if ((pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_OFF] == true) && (ctr_soure2_off_last == false))AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] = AGENT_PB_OFF_DELAY;

	ctr_soure1_on_last = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_ON];
	ctr_soure1_off_last = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_OFF];
	ctr_soure2_on_last = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_ON];
	ctr_soure2_off_last= pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_OFF];




	if (pPLC_IO->ui.PB[ID_PB_FAULT_RESET] ==true)AgentInf_workbuf.PLC_PB_com[ID_PB_FAULT_RESET] = AGENT_PB_OFF_DELAY;

	//OFF�f�B���C
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF]--;

	if (AgentInf_workbuf.PLC_PB_com[ID_PB_FAULT_RESET] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_FAULT_RESET]--;

	//�U��~�߃����v
	if (pCraneStat->auto_standby) {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_ON;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_OFF;
	}
	else {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_OFF;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_ON;
	}

	//�����J�n�����v
	if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] = AGENT_LAMP_ON;
	else AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] = AGENT_LAMP_OFF;

	//�����������v	
	//LAMP�@�J�E���g�l�@0�F�����@�J�E���g�l%PLC_IO_LAMP_FLICKER_COUNT�@���@PLC_IO_LAMP_FLICKER_CHANGE�ȉ���OFF,�ȏ��ON�iPLC_IF�ɂďo�́j
	for (int i = 0;i < SEMI_AUTO_TG_CLR;i++) {
		if (i == pCraneStat->semi_auto_selected) {
			if((pCraneStat->semi_auto_pb_count[i] > SEMI_AUTO_TG_SELECT_TIME) && (pCraneStat->semi_auto_pb_count[i] < SEMI_AUTO_TG_RESET_TIME)) {
				AgentInf_workbuf.PLC_LAMP_semiauto_com[i]++;
			}
			else {
				AgentInf_workbuf.PLC_LAMP_semiauto_com[i] = AGENT_LAMP_ON;
			}

		}
		else if (pCraneStat->semi_auto_pb_count[i]) {
			AgentInf_workbuf.PLC_LAMP_semiauto_com[i]++;

		}
		else {
			AgentInf_workbuf.PLC_LAMP_semiauto_com[i] = AGENT_LAMP_OFF;
		}
	}
	
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

