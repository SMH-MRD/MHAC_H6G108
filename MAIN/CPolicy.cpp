#include "CPolicy.h"
#include "CAgent.h"

//-���L�������I�u�W�F�N�g�|�C���^:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem*  pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

/****************************************************************************/
/*   �R���X�g���N�^�@�f�X�g���N�^                                           */
/****************************************************************************/
CPolicy::CPolicy() {
	pPolicyInf = NULL;
	pPLC_IO = NULL;
	pCraneStat = NULL;
	pRemoteIO = NULL;
	pSway_IO = NULL;
}

CPolicy::~CPolicy() {

}

/****************************************************************************/
/*   �^�X�N����������                                                       */
/* �@���C���X���b�h�ŃC���X�^���X��������ɌĂт܂��B                       */
/****************************************************************************/

int (CPolicy::*pfunc_set_recipe[N_AS_PTN])(LPST_MOTION_RECIPE, int);

static CAgent* pAgent;

void CPolicy::init_task(void* pobj) {

	//���L�������\���̂̃|�C���^�Z�b�g
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pRemoteIO = (LPST_REMOTE_IO)(pRemoteIO_Obj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());

	pAgent = (CAgent*)VectpCTaskObj[g_itask.agent];

	set_panel_tip_txt();

	pPolicyInf->i_com = 0;

	inf.is_init_complete = true;
	return;
};

/****************************************************************************/
/*   �^�X�N���������                                                       */
/* �@�^�X�N�X���b�h�Ŗ��������s�����֐�			�@                      */
/****************************************************************************/
void CPolicy::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//����������菇1�@�O���M������
void CPolicy::input() {

	
	
	
	
	return;

};

//����������菇2�@���C������
void CPolicy::main_proc() {
	
	return;

}

//����������菇3�@�M���o�͏���
void CPolicy::output() {
	
	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

int CPolicy::set_pp_th0(int motion) {

	return 0;
}


/****************************************************************************/
/*�@�@COMMAND ����															*/
/****************************************************************************/
// Command �o�b�t�@�̃C���f�b�N�X�X�V
LPST_COMMAND_SET CPolicy::next_command(int type) {
	if (type == POLICY_TYPE_JOB) {
		pPolicyInf->i_jobcom++;
		if (pPolicyInf->i_jobcom >= COM_STEP_MAX) pPolicyInf->i_jobcom = 0;
		return (pPolicyInf->job_com + pPolicyInf->i_jobcom);
	}
	else {
		pPolicyInf->i_com++;
		if (pPolicyInf->i_com >= COM_STEP_MAX) pPolicyInf->i_com = 0;
		return (pPolicyInf->com + pPolicyInf->i_com);
	}
	return	0;
};

// Command�����X�V
LPST_COMMAND_SET CPolicy::generate_command(int type, double* ptarget_pos) {

	LPST_COMMAND_SET lp_com = next_command(type); 

	//�p�^�[���v�Z�x�[�X�f�[�^�̐ݒ�
	set_pattern_cal_base(type, ID_BOOM_H);		
	set_pattern_cal_base(type, ID_SLEW);

	//�U��~�߃p�^�[���̔���
	pPolicyInf->auto_ctrl_ptn[ID_BOOM_H] = judge_auto_ctrl_ptn(type, ID_BOOM_H);
	pPolicyInf->auto_ctrl_ptn[ID_SLEW] = judge_auto_ctrl_ptn(type, ID_SLEW);

	//�p�^�[���̌v�Z
	if(AS_PTN_NG == set_recipe(lp_com, ID_BOOM_H, pPolicyInf->auto_ctrl_ptn[ID_BOOM_H]))
		return NULL;
	if(AS_PTN_NG == set_recipe(lp_com, ID_SLEW  , pPolicyInf->auto_ctrl_ptn[ID_SLEW]  ))
		return NULL;

	if (lp_com != NULL) {		//Command Set OK�Ł@AGENT�̖ڕW�ʒu�X�V
		*(ptarget_pos + ID_BOOM_H) = st_work.pos_target[ID_BOOM_H];
		*(ptarget_pos + ID_SLEW) = st_work.pos_target[ID_SLEW];
	}
	return lp_com;
}

int  CPolicy::update_com_status(LPST_COMMAND_SET pcom) {

	if (pcom->type == POLICY_TYPE_AS) {
		;
	}
	else if (pcom->type == POLICY_TYPE_SEMI) {
		;
	}
	else if (pcom->type == POLICY_TYPE_JOB) {
		;
	}
	else;

	return 0;
}

/****************************************************************************/
/*�@�@�p�^�[���v�Z�p�̑f�ރf�[�^�v�Z,�Z�b�g									*/
/****************************************************************************/
int CPolicy::set_pattern_cal_base(int auto_type, int motion) {

	//�ڕW�ʒu�ݒ�
	if (auto_type == POLICY_TYPE_SEMI) {
		st_work.pos_target[motion] = pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][motion];
	}
	else if (auto_type == POLICY_TYPE_JOB) {
		st_work.pos_target[motion] = pPLC_IO->status.pos[motion];
	}
	else {
		st_work.pos_target[motion] = pPLC_IO->status.pos[motion] + pAgentInf->dist_for_stop[motion];
	}
	
	//�ڕW�ʒu�܂ł̋����ݒ�
	st_work.dist_for_target[motion] = pPLC_IO->status.pos[motion] - st_work.pos_target[motion];
	if (st_work.dist_for_target[motion] < 0.0) st_work.dist_for_target[motion] *= -1.0;

	//�ő呬�x
	st_work.vmax[motion] = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];

	//�����x
	st_work.a[motion] = pCraneStat->spec.accdec[motion][FWD][ACC];

	//��������
	st_work.acc_time2Vmax[motion] = st_work.vmax[motion] / st_work.a[motion];

	//�������U�ꒆ�S
	st_work.pp_th0[motion][ACC] = pCraneStat->spec.accdec[motion][FWD][ACC] / pCraneStat->w2;
	st_work.pp_th0[motion][DEC] = pCraneStat->spec.accdec[motion][FWD][DEC] / pCraneStat->w2;
	if (motion == ID_SLEW) { //����̉����x��R�ƂŌv�Z
		double R = pPLC_IO->status.pos[ID_BOOM_H];
		st_work.pp_th0[motion][ACC] *= R;
		st_work.pp_th0[motion][DEC] *= R;
	}

	//�U��U���]���l
	st_work.r[motion] = sqrt(pSway_IO->amp2[motion]);

	if (st_work.r[motion] > st_work.pp_th0[motion][ACC]) st_work.is_sway_over_r0[motion] = true;
	else st_work.is_sway_over_r0[motion] = false;

	//���݈ʒu�A���x
	st_work.pos[motion] = pPLC_IO->status.pos[motion];
	st_work.v[motion] = pPLC_IO->status.v_fb[motion];

	//AGENT�X�L�����^�C��
	st_work.agent_scan_ms = pAgent->inf.cycle_ms;

	return 0;
}


/****************************************************************************/
/*�@�@1STEP,2STEP�U��~�߃p�^�[���̃Q�C���i��������(�p�x�j�v�Z				*/
/****************************************************************************/
void CPolicy::set_as_gain(int motion, int as_type) {

	double a,r,w,l,r0, vmax, max_th_of_as, acc_time2Vmax;

	//�ő呬�x�ɂ��������Ԑ���
	r = st_work.r[motion];	//�U���]���l
	r0 = st_work.pp_th0[motion][ACC];	//�������U���S
	w = pCraneStat->w;
	a = st_work.a[motion];
	vmax = st_work.vmax[motion];
	acc_time2Vmax = st_work.acc_time2Vmax[motion];
	l = pCraneStat->mh_l;

	if (as_type == AS_PTN_1STEP) {	// 1STEP
		max_th_of_as = r0 * 4.0; //1 STEP�̃��W�b�N�Ő���\�ȐU��U�����E�l�@4r0
		//�Q�C���v�Z�pR0��ݒ�i������~�b�g�j
		if (r > max_th_of_as) r = max_th_of_as;
		if (r > acc_time2Vmax * w) r = acc_time2Vmax * w; //��tmax

		st_work.as_gain_phase[motion] = acos(1 - 0.5 * r / r0);
		st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		//�ő呬�x�ɂ��������Ԑ���
		if (st_work.as_gain_time[motion] > acc_time2Vmax) {
			st_work.as_gain_time[motion] = acc_time2Vmax;
			st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
		}
	}
	else if (as_type == AS_PTN_2PN) { //2STEP round type
		
		if (r < r0) {//�U��U���������U�����
			//�U��U���ɉ����ăQ�C����ݒ�i�Q��ڂŌ���������B1��ڂ͐U��ێ��B�J�n�ʑ��Œ���
			st_work.as_gain_phase[motion] = acos(0.6*r/(r+r0));
			st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		}
		else {//�U��U���������U��O��
			//2��ڂ̃C���`���O�ŐU��~�߁i�҂����Ԓ����^�C�v�j�p
			st_work.as_gain_phase[motion] = acos(r / (r + r0));
			st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		}
		//�ő呬�x�ɂ��������Ԑ���
		if (st_work.as_gain_time[motion] > acc_time2Vmax) {
			st_work.as_gain_time[motion] = acc_time2Vmax;
			st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
		}
	}
	else if (as_type == AS_PTN_2PP) { //2STEP one way

		double dist_for_target = st_work.pos_target[motion] - pPLC_IO->status.pos[motion];
		if (dist_for_target < 0.0) dist_for_target *= -1.0;
	
		//1��̃C���`���O�ړ������@d = a*t^2�@�ړ�����S=2d���t=��(S/2a)
		st_work.as_gain_time[motion] = sqrt(0.5 * dist_for_target /a);

		//�ő呬�x�ɂ��������Ԑ���
		if (st_work.as_gain_time[motion] > acc_time2Vmax) {
			st_work.as_gain_time[motion] = acc_time2Vmax;
			st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
		}
	}
	else {
		st_work.as_gain_phase[motion] = 0.0;
		st_work.as_gain_time[motion] = 0.0;
	}
	return;
}

/****************************************************************************/
/*�@�@�����i�U��~�߁j�̈ړ��p�^�[���^�C�v�̔���							*/
/****************************************************************************/
int CPolicy::judge_auto_ctrl_ptn(int auto_type, int motion) {
	
	int ptn = AS_PTN_0;

	set_pp_th0(motion);																//�ʑ����ʂ̉�]���S�v�Z

	double T = pCraneStat->T;
	double R = pPLC_IO->status.pos[ID_BOOM_H];
	double vmax = st_work.vmax[motion];
	double a = pCraneStat->spec.accdec[motion][FWD][ACC];
	double d_from_target = st_work.dist_for_target[motion];

	double d_min_ptn2ad = vmax * (T + vmax / a);

	if (sqrt(pSway_IO->amp2[motion]) > st_work.pp_th0[motion][ACC])					//�U��U�� >�@�������̐U�ꒆ�S
		ptn = AS_PTN_1STEP;
	else if (d_from_target > d_min_ptn2ad)											//�ڕW�ʒu�܂ł̋��� > 2Step�p�^�[���̍ŏ�����
		ptn = AS_PTN_2ACCDEC; 
	else if(d_from_target < pCraneStat->spec.as_pos_level[motion][ID_LV_TRIGGER])	//�ڕW�ʒu�܂ł̋��� < �ʒu���߃g���K����
		ptn = AS_PTN_2PN;
	else if (d_from_target < 2.0*vmax*vmax/a)										//�ڕW�ʒu�܂ł̋��� > 2��̍ő呬�x�C���`���O����		
		ptn = AS_PTN_2PP;
	else 
		ptn = AS_PTN_1ACCDEC;														//������ł��Ȃ����͑�`�p�^�[��

	return ptn; 
}

/****************************************************************************/
/*�@�@�ړ��p�^�[���^�C�v�̌v�Z,�Z�b�g										*/
/****************************************************************************/
int CPolicy::set_recipe(LPST_COMMAND_SET pcom, int motion, int ptn) {
	//���쏉����
	pcom->motion_stat[motion].status = MOTION_STAT_STANDBY;
	pcom->motion_stat[motion].iAct = 0;
	
	LPST_MOTION_RECIPE precipe = &(pcom->recipe[motion]);
	if (ptn == AS_PTN_2ACCDEC)		return set_recipe2ad(precipe, motion);
	else if (ptn == AS_PTN_1STEP)	return set_recipe1step(precipe, motion);
	else if (ptn == AS_PTN_2PN)		return set_recipe2pn(precipe, motion);
	else if (ptn == AS_PTN_2PP)		return set_recipe2pp(precipe, motion);
	else if (ptn == AS_PTN_3STEP)	return set_recipe3step(precipe, motion);
	else return AS_PTN_0; 
};

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@�Pstep�^�C�v�̌v�Z,�Z�b�g							*/
/****************************************************************************/
int CPolicy::set_recipe1step(LPST_MOTION_RECIPE precipe, int motion) { 
	
	LPST_MOTION_ELEMENT pelement;

	st_work.motion_dir[motion] = POLICY_UNKNOWN;										//�ړ������s��(�ړ����������s���Ɍ��܂�j
	set_as_gain(motion, AS_PTN_1STEP);													//�U��~�߃Q�C���v�Z
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_1STEP;
		
	//Step 1�@�ʑ��҂�
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT �������B�������҂�
		pelement->_p = st_work.pos[motion];												// �ڕW�ʒu(�J�n���ʒu�j
		pelement->_t = 2.0*st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;
		pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion];			// �v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion] - PI180;	// �}�C�i�X�����p�ʑ�

	}
	//Step 2�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.a[motion] * pelement->_t;								// �ڕW�ω����x
	}
	//Step 3�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�			
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.v[motion];												// �J�n�����x
	}
	//Step 4�@�����~�ҋ@
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = st_work.v[motion];												// �J�n�����x 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->motion[i].time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->motion[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_1STEP;					//�p�^�[���o�͎�����p
	}

	return AS_PTN_OK; 
};

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@2step���̏�U��~�߃Z�b�g							*/
/****************************************************************************/
int CPolicy::set_recipe2pn(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_ELEMENT pelement;

	st_work.motion_dir[motion] = POLICY_UNKNOWN;										//�ړ������s��(�ړ����������s���Ɍ��܂�j
	set_as_gain(motion, AS_PTN_2PN);													//�U��~�߃Q�C���v�Z
	precipe->n_step = 0;																//�X�e�b�v��
	precipe->motion_type = AS_PTN_2PN;

	//Step 1�@�ʑ��҂�(�U��U���ێ��j
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT �������B�������҂�
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j
		pelement->_t = 2.0 * st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;

		//�N���ʑ�
		double temp_d;
		if (st_work.r[motion] < st_work.r0[motion])										//�����U�ꂪ�����U���菬�i�\��j
			temp_d = st_work.r[motion] / st_work.r0[motion] * (cos(st_work.as_gain_phase[motion]) - 1);
		else																			//�����U�ꂪ�����U���菬�i�\��O�j
			temp_d = cos(st_work.as_gain_phase[motion]) - 1;

		double start_phase = acos(temp_d) + st_work.as_gain_phase[motion];

		pelement->opt_d[MOTHION_OPT_PHASE_F] = start_phase;								//�v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = -PI180 + start_phase;					//�}�C�i�X�����p�ʑ�
	}
	//Step 2�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.a[motion] * pelement->_t;								// �ڕW�ω����x
	}
	//Step 3�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�			
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.v[motion];												// �J�n�����x
	}
	//Step 4�@�ʑ��҂�(�U��U�������j
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// �J�n���Ɣ��Ε����̈ʑ���҂�
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�
		pelement->_t = 2.0 * st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;

		pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;										// �v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;									// �}�C�i�X�����p�ʑ�
	}
	//Step 5�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.a[motion] * precipe->motion[1]._t;						// �ڕW�ω����x
	}
	//Step 6�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_p = st_work.pos_target[motion];										// �J�n���ʒu			
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.v[motion];												// �J�n�����x
	}
	//Step 7�@�����~�ҋ@
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �J�n���ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->motion[i].time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->motion[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PN;						//�p�^�[���o�͎�����p
	}

	return AS_PTN_OK;

};

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@2step���ړ�										*/
/****************************************************************************/
int CPolicy::set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_ELEMENT pelement;
	double dir;

	//�ړ������Z�b�g
	if (st_work.pos[motion] < st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_FWD;
		dir = 1.0;
	}
	else if (st_work.pos[motion] > st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_REW;
		dir = -1.0;
	}
	else {
		st_work.motion_dir[motion] = POLICY_STOP;
		dir = 0.0;
	}

	set_as_gain(motion, AS_PTN_2PP);													//�U��~�߃Q�C���v�Z
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_2PP;

	//Step 1�@�ʑ��҂�(�U��U���ێ��j
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;									// �ʑ��҂�
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 2.0 * st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;

		//�N���ʑ�
		double temp_d;
		if (st_work.r[motion] < st_work.r0[motion])										// �����U�ꂪ�����U���菬�i�\��j
			temp_d = st_work.r[motion] / st_work.r0[motion] * (cos(st_work.as_gain_phase[motion]) - 1);
		else																			// �����U�ꂪ�����U���菬�i�\��O�j
			temp_d = cos(st_work.as_gain_phase[motion]) - 1;

		double start_phase = acos(temp_d) + st_work.as_gain_phase[motion];

		pelement->opt_d[MOTHION_OPT_PHASE_F] = start_phase;								// �v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = -PI180 + start_phase;					// �}�C�i�X�����p�ʑ�
	}
	//Step 2�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// �ڕW�ω����x
		pelement->_p = (pelement-1)->_p + 0.5 * pelement->_v * pelement->_t;			// �ڕW�ʒu
	}
	//Step 3�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement-1)->_v * pelement->_t;		// �ڕW�ʒu
	}
	//Step 4�@�ʑ��҂�
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT �������B�������҂�
		pelement->_p = (pelement - 1)->_p;												// �ڕW�ʒu(���݈ʒu���Z�b�g�����j
		pelement->_t = 2.0 * st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;

		pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;										//phase1 �v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;									//phase2 �}�C�i�X�����p�ʑ�
	}
	//Step 5�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// �ڕW�ω����x
		pelement->_p = (pelement - 1)->_p + 0.5 * pelement->_v * pelement->_t;			// �ڕW�ʒu
	}
	//Step 6�@����
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement - 1)->_v * pelement->_t;	// �ڕW�ʒu
	}
	//Step 7�@�����~�ҋ@
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PP;			//�p�^�[���o�͎�����p
	}

	return AS_PTN_OK;
};

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@2�i������											*/
/****************************************************************************/
int CPolicy::set_recipe2ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_ELEMENT pelement;
	double T = pCraneStat->T;
	double a = st_work.a[motion];
	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];
	double v_half = pCraneStat->spec.notch_spd_f[motion][NOTCH_3];
	double ta = v_top / a;
	double ta_half = ta/2.0;
	double tc_half = (T - ta) / 2.0; 
	if (ta > T) tc_half += T;										//�������Ԃ��U��������������Ƃ��́{T
	double dmin = v_top * ta + 2.0 * tc_half * v_half;
	double d = st_work.pos[motion] - st_work.pos_target[motion];	//�ړ�����
	if (d < 0.0) d *= -1.0;
	double initial_sway = st_work.r[motion];

	double t_expected = 2.0 * (ta + tc_half);	//�\��ړ�����
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//�ړ������Z�b�g
	if (st_work.pos[motion] < st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_FWD;
		dir = 1.0;
	}
	else if (st_work.pos[motion] > st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_REW;
		dir = -1.0;
	}
	else {
		st_work.motion_dir[motion] = POLICY_STOP;
		dir = 0.0;
	}

	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1�@�����ʒu�҂�
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 0.0;																// �\�莞��
		pelement->_v = 0.0;

		//�҂��ʒu(�ڕW�܂ł̋����j�v�Z
		double ta_other, a_other, v_other, d_max_other;
		bool is_slew_first;
		if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
			is_slew_first = true;
		}

		if (motion == ID_BOOM_H) {
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;
			d_max_other= PI360;

			if (is_slew_first) {
				if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				else							//���v���Ԃ����莲�̉������Ԃ�蒷��
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�

		}
		else if (motion == ID_SLEW) {
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;
			d_max_other = pCraneStat->spec.boom_pos_max;
			if (is_slew_first) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�
			}
			else {
				if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				else							//���v���Ԃ����莲�̉������Ԃ�蒷��
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
			}
		}
		else {
			pelement->type = CTR_TYPE_TIME_WAIT;
			a_other = 1.0;
			v_other = 0.0;
			ta_other = v_other / a_other;
			d_max_other = 0.0;
		}

		//�N���ʑ��i�����U�ꂪ�傫���Ƃ��ɉ������J�n����ʑ��j

	}

	//Step 2�@�������葬
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// �������葬�o��
		pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
		pelement->_v = dir * v_half;													// �n�[�t���x
		pelement->_p = (pelement - 1)->_p + dir * v_half * ( 0.5 * ta_half + tc_half);	// �ڕW�ʒu
	}
	//Step 3�@�葬TOP���x
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP���x�葬�o��
		pelement->_t = ta_half + (d - dmin) / v_top;									// �g�b�v���x�o�͎���
		pelement->_v = dir * v_top;														// �g�b�v���x
		pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top *  ta + v_half * tc_half);	// �ڕW�ʒu
	}
	//Step 4�@�������葬
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// �������葬�o��
		pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
		pelement->_v = dir * v_half;													// �n�[�t���x
		pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// �ڕW�ʒu
	}

	//Step 5�@��~
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = ta_half;															// ��������
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	}
	//Step 6�@�����~�ҋ@
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//�p�^�[���o�͎�����p
	}
	return AS_PTN_OK;
};
/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@��`												*/
/****************************************************************************/
int CPolicy::set_recipe1ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_ELEMENT pelement;
	double T = pCraneStat->T;
	double a = st_work.a[motion];
	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];
	double ta = v_top / a;
	double dmin = v_top * ta;
	double d = st_work.dist_for_target[motion];	//�ړ�����
	double initial_sway = st_work.r[motion];
	double t_expected = 2.0 * ta ;	//�\��ړ�����
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;
	double dir;
	//�ړ������Z�b�g
	if (st_work.pos[motion] < st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_FWD;
		dir = 1.0;
	}
	else if (st_work.pos[motion] > st_work.pos_target[motion]) {
		st_work.motion_dir[motion] = POLICY_REW;
		dir = -1.0;
	}
	else {
		st_work.motion_dir[motion] = POLICY_STOP;
		dir = 0.0;
	}

	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_1ACCDEC;

	//Step 1�@�����ʒu�҂�
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 0.0;																// �\�莞��
		pelement->_v = 0.0;

		//�҂��ʒu(�ڕW�܂ł̋����j�v�Z
		double ta_other, a_other, v_other, d_max_other;
		bool is_slew_first;
		if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
			is_slew_first = true;
		}

		if (motion == ID_BOOM_H) {
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;
			d_max_other = PI360;

		if (is_slew_first) {
			if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			else							//���v���Ԃ����莲�̉������Ԃ�蒷��
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
		}
		else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�

	}
	else if (motion == ID_SLEW) {
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
		ta_other = v_other / a_other;
		d_max_other = pCraneStat->spec.boom_pos_max;
		if (is_slew_first) {
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�
		}
		else {
			if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			else							//���v���Ԃ����莲�̉������Ԃ�蒷��
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
		}
	}
	else {
		pelement->type = CTR_TYPE_TIME_WAIT;
		a_other = 1.0;
		v_other = 0.0;
		ta_other = v_other / a_other;
		d_max_other = 0.0;
	}

	//�N���ʑ��i�����U�ꂪ�傫���Ƃ��ɉ������J�n����ʑ��j

	}

	//Step 2�@�葬TOP���x
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP���x�葬�o��
		pelement->_t = ta + (d - dmin) / v_top;											// �g�b�v���x�o�͎���
		pelement->_v = dir * v_top;														// �g�b�v���x
		pelement->_p = st_work.pos_target[motion] - dir * 0.5 * ta * v_top;				// �ڕW�ʒu
	}
	//Step 3�@��~
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = ta;																// ��������
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	}
	//Step 4�@�����~�ҋ@
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
	pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//�p�^�[���o�͎�����p
	}
	return AS_PTN_OK;
}

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@3STEP												*/
/****************************************************************************/
int CPolicy::set_recipe3step(LPST_MOTION_RECIPE precipe, int motion) { return AS_PTN_NG; };


/****************************************************************************/
/*   �^�X�N�ݒ�^�u�p�l���E�B���h�E�̃R�[���o�b�N�֐�                       */
/****************************************************************************/
LRESULT CALLBACK CPolicy::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

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
void CPolicy::set_panel_tip_txt()
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
void CPolicy::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};


