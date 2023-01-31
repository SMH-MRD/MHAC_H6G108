#include "CPolicy.h"
#include "CAgent.h"
#include "CEnvironment.h"
#include "CClientService.h"

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

static CAgent* pAgent;
static CEnvironment* pEnvironment;
static CClientService* pCS;

void CPolicy::init_task(void* pobj) {

	//���L�������\���̂̃|�C���^�Z�b�g
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pRemoteIO = (LPST_REMOTE_IO)(pRemoteIO_Obj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());
	pCSInf = (LPST_CS_INFO)(pCSInfObj->get_pMap());

	pAgent = (CAgent*)VectpCTaskObj[g_itask.agent];
	pEnvironment = (CEnvironment*)VectpCTaskObj[g_itask.environment];
	pCS = (CClientService*)VectpCTaskObj[g_itask.client];

	set_panel_tip_txt();

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
	//���L�������o�͏���
	memcpy_s(pPolicyInf, sizeof(ST_POLICY_INFO), &PolicyInf_workbuf, sizeof(ST_POLICY_INFO));

	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;
};

/****************************************************************************/
/*�@�@COMMAND ����															*/
/****************************************************************************/
// AGENT����̃R�}���h�v������
LPST_COMMAND_BLOCK CPolicy::get_command() {

	if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status != REQ_ACTIVE) {							// Job���s���łȂ�
		if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_STANDBY) {			// ���������������i�����J�n���͍ρj
			return create_semiauto_command();																//�@�������R�}���h���쐬���ă|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_ACTIVE) {		// ���������s��
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);	//���s���R�}���h�̃|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status == REQ_STANDBY) {				// JOB���������i�N���C�A���g�����JOB��M�ρj
			return create_job_command();																	// JOB�R�}���h���쐬���ă|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_WAITING) {		// �������v���҂��i�����J�n���͑҂��j
			return NULL;																					//NULL�|�C���^��Ԃ�
		}
		else {
			return NULL;
		}
	}
	else {
		if ((PolicyInf_workbuf.command_list.job_type == AUTO_TYPE_JOB)										// �R�}���h���X�g���e��JOB
			&& (PolicyInf_workbuf.command_list.job_id == pCSInf->job_list.i_job_active)){					// �R�}���h���X�g�̑Ώ�Job�����s��Job�ƈ�v
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);	//���s���R�}���h�̃|�C���^��Ԃ�
		}
		else {																								//���s��job�ƃZ�b�g���̃R�}���h����v���Ȃ��@���@�ُ�
			return NULL;
		}
	}
	return	NULL;
};

int CPolicy::set_receipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_BOOM_H;
	int ptn = 0;

	double D = pwork->dist_for_target[id]; //�c��ړ�����
	
	/*### �쐬�p�^�[���^�C�v�̔��� ###*/
	//FB����Ȃ��ƂP��̃C���`���O�ňړ��\�ȋ������ŋ��
	if (pwork->a[id] == 0.0) return -1;
	double dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/��
	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### �p�^�[���쐬 ###*/
	/*### STEP0  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:		//���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{										
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu
	}break;

	case PTN_FBSWAY_FULL:			//���A����,�ʒu�ʑ��҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏��������A�����ʑ����B
	{	
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu
	}break;
	default:return -1;
	}

	/*### STEP1  ###*/
	/*### STEP1+ ###*/
	/*### STEP2  ###*/
	/*### STEP3  ###*/
	/*### STEP4  ###*/
	/*### STEP5  ###*/
	/*### STEP6  ###*/
	/*### STEP7  ###*/



	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];
	double v_half = pCraneStat->spec.notch_spd_f[motion][NOTCH_3];
	double ta = v_top / a;
	double ta_half = ta / 2.0;
	double tc_half = (T - ta) / 2.0;
	if (ta > T) tc_half += T;										//�������Ԃ��U��������������Ƃ��́{T
	double dmin = v_top * ta + 2.0 * tc_half * v_half;
	double d = st_work.pos[motion] - st_work.pos_target[motion];	//�ړ�����
	if (d < 0.0) d *= -1.0;

	double t_expected = 2.0 * (ta + tc_half);	//�\��ړ�����
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//�ړ������Z�b�g
	if (motion == ID_SLEW) {
		if ((st_work.pos_target[motion] - st_work.pos[motion]) < -PI180) {
			st_work.motion_dir[motion] = POLICY_FWD;
			dir = 1.0;
		}
		else if ((st_work.pos_target[motion] - st_work.pos[motion]) > PI180) {
			st_work.motion_dir[motion] = POLICY_REW;
			dir = -1.0;
		}
		else if (st_work.pos[motion] < st_work.pos_target[motion]) {
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
	}
	else {
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
	}
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1�@�����ʒu�҂�
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->_p = st_work.pos[motion];												// ���݈ʒu
	pelement->_t = 0.0;																// �\�莞��
	pelement->_v = 0.0;

	//�҂��ʒu(�ڕW�܂ł̋����j�v�Z
	double ta_other, a_other, v_other, d_max_other;
	bool is_slew_first = false;

	//���삪�������������o���������肵�Đ���A�����ǂ�����ɓ����������肷��
	if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
		is_slew_first = true;	//���o���́A���a�������������ɐ�ɐ�����|����
	}

	if (motion == ID_BOOM_H) {	//��������p
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
		ta_other = v_other / a_other;					//�������ԍő�l
		d_max_other = PI360;								//�҂������ƂȂ����

		if (is_slew_first) {
			//				if (ta_other > t_expected)		//�\�莞�Ԃ����莲�̌������Ԃ��Z�����A����̖ڕW�܂ł̋�����������~�������ɓ������瓮��J�n
			//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
			//				else							//���v���Ԃ����莲�̉������Ԃ�蒷��
			//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

							//���ʌŒ�l�ɂ���
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
		}
		else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�

	}
	else if (motion == ID_SLEW) {	//���񓮍�p
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
		ta_other = v_other / a_other;					//�������ԍő�l
		d_max_other = pCraneStat->spec.boom_pos_max;	//�҂������ƂȂ����
		if (is_slew_first == true) {
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�
		}
		else {
			//			if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
			//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			//			else							//���v���Ԃ����莲�̉������Ԃ�蒷��
			//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

						//���ʌ��������l�ɂ���
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;

		}
	}
	else {
		pelement->type = CTR_TYPE_TIME_WAIT;
		a_other = 1.0;
		v_other = 0.0;
		ta_other = v_other / a_other;
		d_max_other = 0.0;
	}
	}
	//Step 2�@�N�������i�����U�ꂪ�傫���Ƃ������J�n�^�C�~���O�𒲐��j
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP���x�葬�o��
	pelement->_t = pCraneStat->T;													// �ő�P�����ҋ@
	pelement->_v = 0.0;																// �g�b�v���x
	pelement->_p = st_work.pos[motion];												// �ڕW�ʒu
	}
	//Step 3�@�������葬
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// �������葬�o��
	pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
	pelement->_v = dir * v_half;													// �n�[�t���x
	pelement->_p = (pelement - 1)->_p + dir * v_half * (0.5 * ta_half + tc_half);	// �ڕW�ʒu
	}
	//Step 4�@�葬TOP���x
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP���x�葬�o��
	pelement->_t = ta_half + (d - dmin) / v_top;									// �g�b�v���x�o�͎���
	pelement->_v = dir * v_top;														// �g�b�v���x
	pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top * ta + v_half * tc_half);	// �ڕW�ʒu
	}
	//Step 5�@�������葬
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// �������葬�o��
	pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
	pelement->_v = dir * v_half;													// �n�[�t���x
	pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// �ڕW�ʒu
	}

	//Step 6�@��~
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
	pelement->_t = ta_half;															// ��������
	pelement->_v = 0.0;																// �ڕW���ݑ��x
	pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	}
	//Step 7�@�����~�ҋ@
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
	pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
	pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//�p�^�[���o�͎�����p
	}
	return AS_PTN_OK;







	return 0;
}
int CPolicy::set_receipe_semiauto_slw(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {
	return 0;
}
int CPolicy::set_receipe_semiauto_mh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {
	return 0;
}


LPST_COMMAND_BLOCK CPolicy::create_semiauto_command(LPST_JOB_SET pjob) {	//���s���锼�����R�}���h���Z�b�g����
	
	LPST_COMMAND_BLOCK lp_semiauto_com = pPolicyInf->command_list[0];

	set_com_workbuf(pjob->target[0], AUTO_TYPE_SEMI_AUTO);											//�������p�^�[���쐬�p�f�[�^��荞��

	set_receipe_semiauto_bh(pjob->type, &(lp_semiauto_com->recipe[ID_BOOM_H]), true, &st_com_work);
	set_receipe_semiauto_slw(pjob->type, &(lp_semiauto_com->recipe[ID_SLEW]), true, &st_com_work);
	set_receipe_semiauto_mh(pjob->type, &(lp_semiauto_com->recipe[ID_HOIST]), true, &st_com_work);

	return lp_semiauto_com;

};

LPST_COMMAND_BLOCK CPolicy::create_job_command(LPST_JOB_SET pjob) {		//���s����JOB�R�}���h���Z�b�g����

	LPST_COMMAND_BLOCK lp_job_com = NULL;

	return lp_job_com;
};        

/****************************************************************************/
/*�@�@�R�}���h�p�^�[���v�Z�p�̑f�ރf�[�^�v�Z,�Z�b�g									*/
/*�@�@�ڕW�ʒu,�ڕW�܂ł̋���,�ő呬�x,�����x,��������,�����U�ꒆ�S,�U��U��*/
/****************************************************************************/
LPST_POLICY_WORK CPolicy::set_com_workbuf(ST_POS_TARGETS targets, int type) {

	st_com_work.agent_scan_ms = pAgent->inf.cycle_ms;;                 //AGENT�^�X�N�̃X�L�����^�C��
	switch(type){
	case AUTO_TYPE_SEMI_AUTO: {

		st_com_work.target = targets;																//�ڕW�ʒu



		double temp_d;
		for (int i = 0; i < MOTION_ID_MAX; i++) {
			if ((i == ID_HOIST) || (i == ID_BOOM_H) || (i == ID_SLEW)) {
				st_com_work.pos[i] = pPLC_IO->status.pos[i];										//���݈ʒu
				st_com_work.v[i] = pPLC_IO->status.v_fb[i];											//���ݑ��x
				temp_d = st_com_work.target.pos[i] - st_com_work.pos[i];

				if (i == ID_SLEW) {		//����́A180���z����Ƃ��͋t�������߂�
					if (temp_d > PI180)	temp_d -= PI360;
					else if (temp_d < -PI180) temp_d += PI360;
					else;
				}

				if (temp_d < 0.0) {
					st_com_work.motion_dir[i] = ID_REV;												//�ړ�����
					st_com_work.dist_for_target[i] = -1.0 * temp_d;									//�ړ�����
				}
				else {
					st_com_work.motion_dir[i] = ID_FWD;												//�ړ�����
					st_com_work.dist_for_target[i] = temp_d;										//�ړ�����
				}
				st_com_work.a[i] = pCraneStat->spec.accdec[i][st_com_work.motion_dir[i]][ID_ACC];	//���쎲�����x
				st_com_work.vmax[i]= pCraneStat->spec.notch_spd_f[i][NOTCH_MAX - 1];				//�ő呬�x
				st_com_work.acc_time2Vmax[MOTION_ID_MAX] = st_com_work.vmax[i]/ st_com_work.a[i];   //�ő��������
			}
			if ((i == ID_BOOM_H) || (i == ID_SLEW)) {
				st_com_work.a_hp[i] = pEnvironment->cal_hp_acc(i, st_com_work.motion_dir[i]);		//�ݓ_�̉����x
				st_com_work.pp_th0[i][ACC] = pEnvironment->cal_arad_acc(i, FWD);					//�������U�ꒆ�S
				st_com_work.pp_th0[i][DEC] = pEnvironment->cal_arad_dec(i, REV);					//�������U�ꒆ�S
				st_com_work.sway_amp[i] = pSway_IO->rad_amp2[i];									//�U��U��2��
				st_com_work.sway_amp[i] = sqrt(pSway_IO->rad_amp2[i]);		;						//�U��U��
			}

		}

		//�����̖ڕW�ʒu����̎��́A�����ɐ���������ݓ��������̂ŖڕW�ʒu�̎����Ńp�^�[�������
		if (targets.pos[ID_HOIST] > st_com_work.pos[ID_HOIST]) {
			st_com_work.T = pEnvironment->cal_T(targets.pos[ID_HOIST]);								//�U�����
			st_com_work.w = pEnvironment->cal_w(targets.pos[ID_HOIST]);								//�U��p���g��
			st_com_work.w2 = pEnvironment->cal_w2(targets.pos[ID_HOIST]);							//�U��p���g��2��
		}
		else {
			st_com_work.T = pCraneStat->T;															//�U�����
			st_com_work.w = pCraneStat->w;															//�U��p���g��
			st_com_work.w2 = pCraneStat->w2;														//�U��p���g��2��
		}
		break;
	}
	case AUTO_TYPE_JOB: {
		return NULL;
		break;
	}
	default:return NULL;

	}
	return &st_com_work;
}

/****************************************************************************/
/*�@�@1STEP,2STEP�U��~�߃p�^�[���̃Q�C���i��������(�p�x�j�v�Z				*/
/****************************************************************************/
void CPolicy::set_as_gain(int motion, int as_type) {

	double a,r,w,l,r0, vmax, max_th_of_as, acc_time2Vmax;

	//�ő呬�x�ɂ��������Ԑ���
	r = sqrt(pSway_IO->rad_amp2[motion]);			//�U���p�]���l�@rad
	r0 = pEnvironment->cal_arad_acc(motion,FWD);	 //�������U���S
	w = pCraneStat->w;								//�U��p�����x
	a = st_work.a[motion];							//�����̉����xSLEW��rad/s2�ŗǂ��i���a���l���j�@r0�U�ꒆ�S�͔��a�l����
	vmax = st_work.vmax[motion];					//�����̑��xSLEW��rad/s�ŗǂ��i���a���l���j
	acc_time2Vmax = st_work.acc_time2Vmax[motion];	//�������ԍő�l
	l = pCraneStat->mh_l;

	if (as_type == AS_PTN_1STEP){	// 1STEP
		max_th_of_as = r0 * 2.0; //1 STEP�̃��W�b�N�Ő���\�ȐU��U�����E�l�@2r0
		//�Q�C���v�Z�pR0��ݒ�i������~�b�g�j
		if (r >= max_th_of_as) {
			st_work.as_gain_phase[motion] = PI180;
		}
		else {
			st_work.as_gain_phase[motion] = acos(1 - 0.5 * r / r0);
		}
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
			st_work.as_gain_phase[motion] = acos(r0/(r+r0));
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
		if (motion == ID_SLEW) {
			if (st_work.dist_for_target[motion] > PI180) st_work.dist_for_target[motion] -= PI360;
			else if (st_work.dist_for_target[motion] < -PI180) st_work.dist_for_target[motion] += PI360;
			else;
		}

		if (dist_for_target < 0.0) dist_for_target *= -1.0;

	
		//1��̃C���`���O�ړ������@�ڕW�܂ł̋���S=2d�@�@d = a*t^2�@t=��(S/2a)
		st_work.as_gain_time[motion] = sqrt(0.5 * dist_for_target /a);
		if (st_work.as_gain_time[motion] > acc_time2Vmax) //�ő呬�x�ɂ��������Ԑ���
			st_work.as_gain_time[motion] = acc_time2Vmax;

		st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
	}
	else {
		st_work.as_gain_phase[motion] = 0.0;
		st_work.as_gain_time[motion] = 0.0;
	}
	return;
}

/****************************************************************************/
/*�@�@�����i�U��~�߁j�̓���p�^�[���^�C�v�̔���							*/
/****************************************************************************/
int CPolicy::judge_auto_ctrl_ptn(int auto_type, int motion) {
	
	int ptn = AS_PTN_0;
	double r0 = st_work.pp_th0[motion][ACC];	//�������U���S

	double T = pCraneStat->T;
	double w = pCraneStat->w;
	double R = pPLC_IO->status.pos[ID_BOOM_H];
	double vmax = st_work.vmax[motion];
	double a = pCraneStat->spec.accdec[motion][FWD][ACC];
	double d_from_target = st_work.dist_for_target[motion];
	double d_min_ptn2ad = vmax * (T + vmax / a);
	double d_max_2pp = 2.0 * a * T * T;


	if (sqrt(pSway_IO->rad_amp2[motion]) > 2.0 * st_work.pp_th0[motion][ACC])					//�U��U�� >�@�����U��*2
		ptn = AS_PTN_1STEP;
	else if (d_from_target > d_min_ptn2ad)													//�ڕW�ʒu�܂ł̋��� > 2Step�p�^�[���̍ŏ�����
		ptn = AS_PTN_2ACCDEC; 
	else if(d_from_target > d_max_2pp)
		ptn = AS_PTN_1ACCDEC;																//������ł��Ȃ����͑�`�p�^�[��
	else if (pSway_IO->rad_amp2[motion] > pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE])	//�U��U�� >�@�������̐U�ꒆ�S
		ptn = AS_PTN_1STEP;
	else 							
		ptn = AS_PTN_2PP;
	return ptn; 
}



}
/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@�Pstep�^�C�v�̌v�Z,�Z�b�g							*/
/****************************************************************************/
int CPolicy::set_recipe1step(LPST_MOTION_RECIPE precipe, int motion) { 
	
	LPST_MOTION_STEP pelement;

	st_work.motion_dir[motion] = POLICY_NA;										//�ړ������s��(�ړ����������s���Ɍ��܂�j
	set_as_gain(motion, AS_PTN_1STEP);													//�U��~�߃Q�C���v�Z
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_1STEP;
		
	//Step 1�@�ʑ��҂�
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;										// �ڕW�Ɍ����������̈ʑ���҂�
		pelement->_p = st_work.pos[motion];													// �ڕW�ʒu(�J�n���ʒu�j
		pelement->_t = 2.0*st_work.T;														// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;
		if (motion == ID_SLEW) {															//����́A�U������ƈړ������̌������t
			if (st_work.as_gain_phase[motion] > PI180) {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;									// �v���X�����p�ʑ�
				pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;								// �}�C�i�X�����p�ʑ�
			}
			else {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion]-PI180;	// �v���X�����p�ʑ�
				pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion];		// �}�C�i�X�����p�ʑ�
			}
		}
		else {
			if (st_work.as_gain_phase[motion] > PI180) {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = PI180;								// �v���X�����p�ʑ�
				pelement->opt_d[MOTHION_OPT_PHASE_R] = 0;									// �}�C�i�X�����p�ʑ�
			}
			else {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion];		// �v���X�����p�ʑ�
				pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion] - PI180;	// �}�C�i�X�����p�ʑ�
			}
		}
	}
	//Step 2�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// �U��~�߉���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.a[motion] * pelement->_t;								// �ڕW�ω����x
	}
	//Step 3�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�			
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = st_work.v[motion];												// �J�n�����x
	}
	//Step 4�@�����~�ҋ@
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu(�J�n���ʒu�j�ړ������s��̈�
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = st_work.v[motion];												// �J�n�����x 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_1STEP;					//�p�^�[���o�͎�����p
	}

	return AS_PTN_OK; 
};



/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@2step���ړ�										*/
/****************************************************************************/
int CPolicy::set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_STEP pelement;
	double dir;

	//�ړ������Z�b�g
	if (motion == ID_SLEW) {
		if ((st_work.pos_target[motion] - st_work.pos[motion]) < -PI180) {
			st_work.motion_dir[motion] = POLICY_FWD;
			dir = 1.0;
		}
		else if ((st_work.pos_target[motion] - st_work.pos[motion]) > PI180) {
			st_work.motion_dir[motion] = POLICY_REW;
			dir = -1.0;
		}
		else if (st_work.pos[motion] < st_work.pos_target[motion]) {
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
	}
	else {
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
	}

	set_as_gain(motion, AS_PTN_2PP);													//�U��~�߃Q�C���v�Z
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_2PP;

	//Step 1�@�ʑ��҂�(�U��U���ێ��j
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;									// �ʑ��҂�
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 2.0 * st_work.T;													// �\�莞��(�Q�����ȓ��j
		pelement->_v = 0.0;

		//�N���ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_R] = 0.0;					//�}�C�i�X�v���X�����p�ʑ�
		pelement->opt_d[MOTHION_OPT_PHASE_F] = PI180;					//�v���X�����p�ʑ�

	}
	//Step 2�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_V;												// �U��~�߉���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// �ڕW�ω����x
		pelement->_p = (pelement-1)->_p + 0.5 * pelement->_v * pelement->_t;			// �ڕW�ʒu
	}
	//Step 3�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement-1)->_v * pelement->_t;		// �ڕW�ʒu
	}
	//Step 4�@���ԑ҂�
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���ԑ҂�
		pelement->_p = (pelement - 1)->_p;												// �ڕW�ʒu
//		pelement->_t = PI180 / st_work.w - 2.0 * st_work.as_gain_time[motion];			// �ҋ@����
		double ph = PI180 - 2.0 * st_work.as_gain_phase[motion];
		if (ph < 0.0) ph += PI360;
		pelement->_t = ph / st_work.w;

		pelement->_v = 0.0;
	}
	//Step 5�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_V;												// �U��~�߉���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// �ڕW�ω����x
		pelement->_p = (pelement - 1)->_p + 0.5 * pelement->_v * pelement->_t;			// �ڕW�ʒu
	}
	//Step 6�@����
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = st_work.as_gain_time[motion];									// �U��~�߃Q�C��
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement - 1)->_v * pelement->_t;	// �ڕW�ʒu
	}
	//Step 7�@�����ʒu���킹
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_FINE_POSITION;											// �����ړ�
	pelement->_p = st_work.pos_target[motion];											// �J�n���ʒu
	pelement->_t = PTN_FINE_POS_LIMIT_TIME;												// �p�^�[���o�͒�������
	pelement->_v = pCraneStat->spec.notch_spd_f[motion][NOTCH_1];						// �P�m�b�`���x	
	}
	//Step 8�@�����~�ҋ@
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PP;			//�p�^�[���o�͎�����p
	}

	return AS_PTN_OK;
};

/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@2�i������											*/
/****************************************************************************/
int CPolicy::set_recipe2ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_STEP pelement;
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

	double t_expected = 2.0 * (ta + tc_half);	//�\��ړ�����
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//�ړ������Z�b�g
	if (motion == ID_SLEW) {
		if ((st_work.pos_target[motion] - st_work.pos[motion]) < -PI180) {
			st_work.motion_dir[motion] = POLICY_FWD;
			dir = 1.0;
		}
		else if ((st_work.pos_target[motion] - st_work.pos[motion]) > PI180) {
			st_work.motion_dir[motion] = POLICY_REW;
			dir = -1.0;
		}
		else if (st_work.pos[motion] < st_work.pos_target[motion]) {
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
	}
	else {
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
	}
	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1�@�����ʒu�҂�
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 0.0;																// �\�莞��
		pelement->_v = 0.0;

		//�҂��ʒu(�ڕW�܂ł̋����j�v�Z
		double ta_other, a_other, v_other, d_max_other;
		bool is_slew_first=false;

		//���삪�������������o���������肵�Đ���A�����ǂ�����ɓ����������肷��
		if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
			is_slew_first = true;	//���o���́A���a�������������ɐ�ɐ�����|����
		}

		if (motion == ID_BOOM_H) {	//��������p
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];		
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;					//�������ԍő�l
			d_max_other= PI360;								//�҂������ƂȂ����

			if (is_slew_first) {
//				if (ta_other > t_expected)		//�\�莞�Ԃ����莲�̌������Ԃ��Z�����A����̖ڕW�܂ł̋�����������~�������ɓ������瓮��J�n
//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
//				else							//���v���Ԃ����莲�̉������Ԃ�蒷��
//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

				//���ʌŒ�l�ɂ���
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�

		}
		else if (motion == ID_SLEW) {	//���񓮍�p
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;					//�������ԍő�l
			d_max_other = pCraneStat->spec.boom_pos_max;	//�҂������ƂȂ����
			if (is_slew_first==true) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�
			}
			else {
	//			if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
	//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
	//			else							//���v���Ԃ����莲�̉������Ԃ�蒷��
	//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
				
				//���ʌ��������l�ɂ���
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other *  v_other;
			
			}
		}
		else {
			pelement->type = CTR_TYPE_TIME_WAIT;
			a_other = 1.0;
			v_other = 0.0;
			ta_other = v_other / a_other;
			d_max_other = 0.0;
		}
	}
	//Step 2�@�N�������i�����U�ꂪ�傫���Ƃ������J�n�^�C�~���O�𒲐��j
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP���x�葬�o��
		pelement->_t = pCraneStat->T;													// �ő�P�����ҋ@
		pelement->_v = 0.0;																// �g�b�v���x
		pelement->_p = st_work.pos[motion];												// �ڕW�ʒu
	}
	//Step 3�@�������葬
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// �������葬�o��
		pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
		pelement->_v = dir * v_half;													// �n�[�t���x
		pelement->_p = (pelement - 1)->_p + dir * v_half * ( 0.5 * ta_half + tc_half);	// �ڕW�ʒu
	}
	//Step 4�@�葬TOP���x
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP���x�葬�o��
		pelement->_t = ta_half + (d - dmin) / v_top;									// �g�b�v���x�o�͎���
		pelement->_v = dir * v_top;														// �g�b�v���x
		pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top *  ta + v_half * tc_half);	// �ڕW�ʒu
	}
	//Step 5�@�������葬
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// �������葬�o��
		pelement->_t = ta_half + tc_half;												// �n�[�t���x�o�͎���
		pelement->_v = dir * v_half;													// �n�[�t���x
		pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// �ڕW�ʒu
	}

	//Step 6�@��~
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = ta_half;															// ��������
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	}
	//Step 7�@�����~�ҋ@
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//�p�^�[���o�͎�����p
	}
	return AS_PTN_OK;
};
/****************************************************************************/
/*�@�@�U��~�߃p�^�[���@��`												*/
/****************************************************************************/
int CPolicy::set_recipe1ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_STEP pelement;
	double T = pCraneStat->T;
	double w = pCraneStat->w;
	double a = st_work.a[motion];
	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5]; 
	//�������ȏ�̈ړ����ԂƂȂ�Top���x�ݒ�
	for (int i = 0; i < NOTCH_MAX; i++) {
		int k = (NOTCH_MAX - i) - 1;
		v_top = pCraneStat->spec.notch_spd_f[motion][k];
		if((v_top * (T/2.0 + v_top / a) < st_work.dist_for_target[motion]))
			break;
	}
	
	//0�m�b�`���x�̎��͉������x�i�P�m�b�`���x�j
	if (v_top < pCraneStat->spec.notch_spd_f[motion][NOTCH_1])v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_1];

	double ta = v_top / a;
	double dmin = v_top * ta;
	double d = st_work.dist_for_target[motion];	//�ړ�����
	double t_expected = 2.0 * ta ;	//�\��ړ�����
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;
	double dir;

	//�ړ������Z�b�g
	if (motion == ID_SLEW) {
		if ((st_work.pos_target[motion] - st_work.pos[motion] ) < -PI180 ) {
			st_work.motion_dir[motion] = POLICY_FWD;
			dir = 1.0;
		}
		else if ((st_work.pos_target[motion] - st_work.pos[motion]) > PI180) {
			st_work.motion_dir[motion] = POLICY_REW;
			dir = -1.0;
		}
		else if (st_work.pos[motion] < st_work.pos_target[motion]) {
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
	}
	else {
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
	}


	precipe->n_step = 0;																//�X�e�b�v��������
	precipe->motion_type = AS_PTN_1ACCDEC;

	//�҂��ʒu(�ڕW�܂ł̋����j�v�Z
	double ta_other, a_other, v_other, d_max_other;
	bool is_slew_first = false;

	//Step 1�@�����ʒu�҂�
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// ���݈ʒu
		pelement->_t = 0.0;																// �\�莞��
		pelement->_v = 0.0;

		if (motion == ID_BOOM_H) {	//��������p
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;					//�������ԍő�l
			d_max_other = PI360;								//�҂������ƂȂ����

			if (is_slew_first) {
				//				if (ta_other > t_expected)		//�\�莞�Ԃ����莲�̌������Ԃ��Z�����A����̖ڕW�܂ł̋�����������~�������ɓ������瓮��J�n
				//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
				//				else							//���v���Ԃ����莲�̉������Ԃ�蒷��
				//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

								//���ʌŒ�l�ɂ���
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�

		}
		else if (motion == ID_SLEW) {	//���񓮍�p
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;					//�������ԍő�l
			d_max_other = pCraneStat->spec.boom_pos_max;	//�҂������ƂȂ����
			if (is_slew_first == true) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//�҂������ƂȂ�
			}
			else {
				//			if (ta_other > t_expected)		//���v���Ԃ����莲�̉������Ԃ��Z��
				//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				//			else							//���v���Ԃ����莲�̉������Ԃ�蒷��
				//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

							//���ʌ��������l�ɂ���
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;

			}
		}
		else {
			pelement->type = CTR_TYPE_TIME_WAIT;
			a_other = 1.0;
			v_other = 0.0;
			ta_other = v_other / a_other;
			d_max_other = 0.0;
		}
}
	//Step 2�@�N�������i�����U�ꂪ�傫���Ƃ������J�n�^�C�~���O�𒲐��j
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP���x�葬�o��
		pelement->_t = pCraneStat->T;													// �ő�P�����ҋ@
		pelement->_v = 0.0;																// �g�b�v���x
		pelement->_p = st_work.pos[motion];												// �ڕW�ʒu
	}

	//Step 3�@�葬TOP���x
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP���x�葬�o��
		pelement->_t = ta + (d - dmin) / v_top;											// �g�b�v���x�o�͎���
		pelement->_v = dir * v_top;														// �g�b�v���x
		pelement->_p = st_work.pos_target[motion] - dir * 0.5 * ta * v_top;				// �ڕW�ʒu
		if (pelement->_p > PI180) pelement->_p -= PI360;
		else if(pelement->_p < -PI180) pelement->_p += PI360;
	}
	//Step 4�@��~
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// �J�n�����x�Ɍ���
		pelement->_t = ta;																// ��������
		pelement->_v = 0.0;																// �ڕW���ݑ��x
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
	}
	//Step 5�@�����~�ҋ@
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// ���Ԓ���
		pelement->_p = st_work.pos_target[motion];										// �ڕW�ʒu
		pelement->_t = PTN_CONFIRMATION_TIME;											// �p�^�[���o�͒�������
		pelement->_v = 0.0;																// 
	}

	//�\�莞�Ԃ�AGENT�̃X�L�����J�E���g�l�ɕϊ�,�p�^�[���^�C�v�Z�b�g
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//�p�^�[���o�͎�����p
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


