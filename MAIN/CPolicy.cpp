#include "CPolicy.h"
#include "CAgent.h"
#include "CEnvironment.h"
#include "CClientService.h"
#include "CHelper.h"

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

//����������菇3�@�\��,�M���o�͏���
void CPolicy::output() {
	//���L�������o�͏���
	memcpy_s(pPolicyInf, sizeof(ST_POLICY_INFO), &PolicyInf_workbuf, sizeof(ST_POLICY_INFO));
	//�^�X�N�p�l���ւ̕\���o��
	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;
};

/****************************************************************************/
/*�@�@COMMAND ����															*/
/****************************************************************************/
// AGENT����̃R�}���h�v������
LPST_COMMAND_BLOCK CPolicy::req_command() {

	if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status != STAT_ACTIVE) {							// Job���s���łȂ�
		if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_STANDBY) {				// ���������������i�����J�n���͍ρj
			return create_semiauto_command(&(pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active]));	//�@�������R�}���h���쐬���ă|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_ACTIVE) {			// ���������s��
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);		//���s���R�}���h�̃|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status == STAT_STANDBY) {					// JOB���������i�N���C�A���g�����JOB��M�ρj
			return create_job_command(&(pCSInf->job_list.job[pCSInf->job_list.i_job_active]));					// JOB�R�}���h���쐬���ă|�C���^��Ԃ�
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_WAITING) {		// �������v���҂��i�����J�n���͑҂��j
			return NULL;																						//NULL�|�C���^��Ԃ�
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
/****************************************************************************/
/*�@�@�ړ��p�^�[�����V�s����												*/
/****************************************************************************/
/* # �N�����V�s�@*/
int CPolicy::set_receipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_BOOM_H;
	int ptn = 0;							//�ړ��p�^�[��

	double D = pwork->dist_for_target[id]; //�c��ړ�����
	
	/*### �쐬�p�^�[���^�C�v�̔��� ###*/
	//FB����Ȃ��ƂP��̃C���`���O�ňړ��\�ȋ������ŋ��
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	double dist_inch_max;							
	if (pwork->vmax[id]/ pwork->a[id] > pwork->T) {								//�U������ȓ��̉�������
		dist_inch_max = pwork->a[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/��
	}

	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### �p�^�[���쐬 ###*/
	precipe->n_step = 0;														// �X�e�b�v�N���A

	/*### STEP0  �ҋ@�@###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// ���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{										
		pelement = &(precipe->steps[precipe->n_step++]);						//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu
		D = D;																	// �c�苗���ύX�Ȃ�

	}break;
	 
	case PTN_FBSWAY_FULL:														// ������,�ʒu�ʑ��҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏��������A�����ʑ����B
	{	
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu
		D = D;																	// �c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 ���x�X�e�b�v�o�́@2�i��###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:												// �o�͂���m�b�`���x���v�Z���Đݒ�
	case PTN_FBSWAY_FULL:														
	{
		double d_step = D;													//�X�e�b�v�ł̈ړ�����
		double v_top=0.0;													//�X�e�b�v���x�p
		double check_d;
		int n=0, i;

		// #Step1 �P�i��
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];

			int k = (int)(v_top / st_com_work.a[id] / st_com_work.T);					// k > 0 ���@�������Ԃ��U������ȏ�
			check_d = v_top * (1 + k) * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//������U��~�ߋ����ȏ�L��
			else continue;			//���̃m�b�`��
		}

		if (n) {																		//������U��~�߂̈ړ������L�ŃX�e�b�v�ǉ�
			pelement = &(precipe->steps[precipe->n_step++]);							//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
			pelement->type = CTR_TYPE_VOUT_POS;											//�ʒu���B�҂�

			double ta = v_top / st_com_work.a[id];
			pelement->_t = (double)n * st_com_work.T - ta;								// �U������@-�@��������

			pelement->_v = v_top;														// ���x0

			double d_move = v_top * ((double)n *  st_com_work.T - 0.5 * ta);
			pelement->_p =  (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// �ڕW�ʒu

			D -= d_move;																// �c�苗���X�V
		}

		
	

		//  #Step1�Q�i��
		d_step = D;
		for (i -= 1;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//������ړ��ȏ�̋����L��
			else continue;			//���̃m�b�`��
		}
		
		if (n) {																		//������ȏ�̈ړ������L�ŃX�e�b�v�ǉ�
			pelement = &(precipe->steps[precipe->n_step++]);							//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
			pelement->type = CTR_TYPE_VOUT_POS;											//�ʒu���B�҂�

			double td = ((pelement - 1)->_v - v_top) / st_com_work.a[id];				// �������� �X�e�b�v���x�܂ł̌�������
			pelement->_t = (double)n * st_com_work.T + td;								// �U������@+�@2�i�ڂ܂ł̌�������

			pelement->_v = v_top;														// ���x0

			double d_move = v_top * ((double)n * st_com_work.T + td) + 0.5 * td * ((pelement - 1)->_v - v_top) ;
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// �ڕW�ʒu

			D -= d_move;																// �c�苗���X�V
		}


		//  #Step2 ��~
		pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_V;												//���x���B�҂�
		pelement->_t = (pelement - 1)->_v / st_com_work.a[id];							//��������
		pelement->_v = 0.0;																// ���x0
		double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu

		D -= d_move;																	// �c�苗���X�V
	}break;

	case PTN_NO_FBSWAY_2INCH:													//��`�������P�[�X�̓X�L�b�v
	case PTN_FBSWAY_AS:
	{
		D = D;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	{
		double v_inch = sqrt(0.5 * D * st_com_work.a[id]);
		double ta = v_inch/st_com_work.a[id];
		double v_top;
		for (int i = NOTCH_MAX;i > 1;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			if (v_inch > pCraneStat->spec.notch_spd_f[id][i - 1])break;
			else continue;			//���̃m�b�`��
		}

		//STEP3
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// ��������
		pelement->_v = v_top;													// �m�b�`���x
		double d_move = 0.5 * v_inch * ta;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		D -= d_move;															

		//STEP4
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		} 
		
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta + tc;													// �ʑ��҂���~����
		pelement->_v = 0.0;														// �m�b�`���x
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		D -= d_move;															

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// ��������
		pelement->_v = v_top;													// �m�b�`���x
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		D -= d_move;

		//STEP6
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// �ʑ��҂���~����
		pelement->_v = 0.0;														// �m�b�`���x
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		D -= d_move;

	}break;

	case PTN_FBSWAY_AS:															//�U��FB����p�^�[���̓X�L�b�v
	case PTN_FBSWAY_FULL:														
	{
		D = D;																	//�c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_FINE_POS;										// �����ʒu����
		pelement->_t = FINE_POS_TIMELIMIT;										// �ʒu���킹�ő�p������
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// �P�m�b�`���x
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		D = 0;																	// �c�苗���ύX�Ȃ�

	}break;

	case PTN_FBSWAY_AS:
	case PTN_FBSWAY_FULL:														// ���A����,�ʒu�ʑ��҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏��������A�����ʑ����B
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_FB_SWAY_POS;									// FB�U��~��/�ʒu����
		pelement->_t = st_com_work.T * 4.0;										// �U��4������
		pelement->_v = 0.0;														// �U��~�߃��W�b�N�Ō���
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		D = 0;																	// �c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}

	return POLICY_PTN_OK;
}
/* # ���񃌃V�s�@*/
int CPolicy::set_receipe_semiauto_slw(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_SLEW;
	int ptn = 0;							//�ړ��p�^�[��

	double D = pwork->dist_for_target[id]; //�c��ړ�����

	/*### �쐬�p�^�[���^�C�v�̔��� ###*/
	//FB����Ȃ��ƂP��̃C���`���O�ňړ��\�ȋ������ŋ��
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	double dist_inch_max;
	if (pwork->vmax[id] / pwork->a[id] > pwork->T) {							//�U������ȓ��̉�������
		dist_inch_max = pwork->a[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/��
	}

	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### �p�^�[���쐬 ###*/
	precipe->n_step = 0;														// �X�e�b�v�N���A

	/*### STEP0  �ҋ@�@###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// ���A�����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  �����F�����o�����͏��������A�������ݎ��͈����ʒu���ڕW�{X���ȉ�
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu���݈ʒu
		CHelper::fit_slew_axis(&(pelement->_p));								//�ڕW�ʒu�̍Z��
		D = D;																	// �c�苗���ύX�Ȃ�

	}break;

	case PTN_FBSWAY_FULL:														// ������,�ʒu�ʑ��҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  �����F�����o�����͏��������A�������ݎ��͈����ʒu���ڕW�{X���ȉ�
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// �����ʒu�҂�
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
		pelement->_v = 0.0;														// ���x0
		pelement->_p = pwork->pos[id];											// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));								// �ڕW�ʒu�̍Z��
		D = D;																	// �c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 ���x�X�e�b�v�o�́@1,2�i ###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// �����FB�Ȃ��̎��͐U��~�߃p�^�[��
	{
		double d_step = D;													//�X�e�b�v�ł̈ړ�����
		double v_top = 0.0;													//�X�e�b�v���x�p
		double check_d;
		int n = 0, i;

		// #Step1 �P�i��
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];

			int k = (int)(v_top / st_com_work.a[id] / st_com_work.T);					// k > 0 ���@�������Ԃ��U������ȏ�
			check_d = v_top * (1+k) * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//������U��~�ߋ����ȏ�L��
			else continue;			//���̃m�b�`��
		}

		if (n) {																		//������U��~�߂̈ړ������L�ŃX�e�b�v�ǉ�
			pelement = &(precipe->steps[precipe->n_step++]);							//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
			pelement->type = CTR_TYPE_VOUT_POS;											//�ʒu���B�҂�

			double ta = v_top / st_com_work.a[id];
			pelement->_t = (double)n * st_com_work.T - ta;								// �U������@-�@��������

			pelement->_v = v_top;														// ���x0

			double d_move = v_top * ((double)n * st_com_work.T - 0.5 * ta);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// �ڕW�ʒu
			CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��

			D -= d_move;																// �c�苗���X�V
		}

		//  #Step1�Q�i��
		d_step = D;
		for (i -= 1;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//������ړ��ȏ�̋����L��
			else continue;			//���̃m�b�`��
		}

		if (n) {																		//������ȏ�̈ړ������L�ŃX�e�b�v�ǉ�
			pelement = &(precipe->steps[precipe->n_step++]);							//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
			pelement->type = CTR_TYPE_VOUT_POS;											//�ʒu���B�҂�

			double td = ((pelement - 1)->_v - v_top) / st_com_work.a[id];				// �������� �X�e�b�v���x�܂ł̌�������
			pelement->_t = (double)n * st_com_work.T + td;								// �U������@+�@2�i�ڂ܂ł̌�������

			pelement->_v = v_top;														// ���x0

			double d_move = v_top * ((double)n * st_com_work.T + td) + 0.5 * td * ((pelement - 1)->_v - v_top);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// �ڕW�ʒu
			CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��

			D -= d_move;																// �c�苗���X�V
		}

		//  #Step2 ��~
		pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_V;												//���x���B�҂�
		pelement->_t = (pelement - 1)->_v / st_com_work.a[id];							//��������
		pelement->_v = 0.0;																// ���x0
		double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��

		D -= d_move;																	// �c�苗���X�V
	}break;

	case PTN_FBSWAY_FULL:														// �����FB����̎��͐U��~�ߖ���1�i�̂݁F�����^�C�~���O�Œ���
	{
		double d_step = D;																//�X�e�b�v�ł̈ړ�����
		double v_top = 0.0;																//�X�e�b�v���x�p
		double check_d;
		int n = 0, i;

		// #Step1�@�P�i��
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * v_top / st_com_work.a[id] + SPD_FB_DELAY_TIME * v_top;
			if (check_d < d_step) break;
			else continue;																//���̃m�b�`��
		}

		pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_POS;												//�ʒu���B�҂�
		double ta = v_top / st_com_work.a[id];
		double tc = (D - v_top * ta) / v_top;
		pelement->_t = ta + tc;															// �U������@-�@��������
		pelement->_v = v_top;															// ���x0
		double d_move = v_top * (tc + 0.5 * ta);
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D -= d_move;																	// �c�苗���X�V

		//  #Step2 ��~
		pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_V;												//���x���B�҂�
		pelement->_t = ta;																//��������
		pelement->_v = 0.0;																// ���x0
		pelement->_p = st_com_work.target.pos[id];											// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D -= d_move;																	// �c�苗���X�V
	}break;

	case PTN_NO_FBSWAY_2INCH:													//��`�������P�[�X�̓X�L�b�v
	case PTN_FBSWAY_AS:
	{
		D = D;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	{
		double v_inch = sqrt(0.5 * D * st_com_work.a[id]);
		double ta = v_inch / st_com_work.a[id];
		double v_top;
		for (int i = NOTCH_MAX;i > 1;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			if (v_inch > pCraneStat->spec.notch_spd_f[id][i - 1])break;
			else continue;			//���̃m�b�`��
		}

		//STEP3
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// ��������
		pelement->_v = v_top;													// �m�b�`���x
		double d_move = 0.5 * v_inch * ta;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D -= d_move;

		//STEP4
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta + tc;													// �ʑ��҂���~����
		pelement->_v = 0.0;														// �m�b�`���x
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D -= d_move;

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// ��������
		pelement->_v = v_top;													// �m�b�`���x
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D -= d_move;

		//STEP6
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_VOUT_TIME;									// �����p���x�o��
		pelement->_t = ta;														// �ʑ��҂���~����
		pelement->_v = 0.0;														// �m�b�`���x
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));								//�ڕW�ʒu�̍Z��
		D -= d_move;

	}break;

	case PTN_FBSWAY_AS:															//�U��FB����p�^�[���̓X�L�b�v
	case PTN_FBSWAY_FULL:
	{
		D = D;																	//�c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//���A����ʒu�҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏�������
	case PTN_NO_FBSWAY_2INCH:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_FINE_POS;										// �����ʒu����
		pelement->_t = FINE_POS_TIMELIMIT;										// �ʒu���킹�ő�p������
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// �P�m�b�`���x
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
		D = 0;																	// �c�苗���ύX�Ȃ�

	}break;

	case PTN_FBSWAY_AS:
	case PTN_FBSWAY_FULL:														//���A����,�ʒu�ʑ��҂��@�����ʒu�F���ڕW����-Xm�@�ȏ�ɂȂ�����  ����F�����o�����͖ڕW�܂ł̋�����X�x�ȉ��A�������ݎ��͏��������A�����ʑ����B
	{
		pelement = &(precipe->steps[precipe->n_step++]);						//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
		pelement->type = CTR_TYPE_FB_SWAY_POS;									// FB�U��~��/�ʒu����
		pelement->_t = st_com_work.T * 4.0;										// �U��4������
		pelement->_v = 0.0;														// �U��~�߃��W�b�N�Ō���
		pelement->_p = st_com_work.target.pos[id];								// �ڕW�ʒu
		CHelper::fit_slew_axis(&(pelement->_p));								//�ڕW�ʒu�̍Z��
		D = 0;																	// �c�苗���ύX�Ȃ�
	}break;
	default:return POLICY_PTN_NG;
	}


	return POLICY_PTN_OK;
}
/* # �����V�s�@*/
int CPolicy::set_receipe_semiauto_mh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_HOIST;
	int ptn = 0;							//�ړ��p�^�[��

	double D = pwork->dist_for_target[id]; //�c��ړ�����

	/*### �쐬�p�^�[���^�C�v�̔��� ###*/
	//FB����Ȃ��ƂP��̃C���`���O�ňړ��\�ȋ������ŋ��
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	/*### �p�^�[���쐬 ###*/
	precipe->n_step = 0;													// �X�e�b�v�N���A

	/*### STEP0  �ҋ@�@###*/												// �����A����ʒu�҂��@���㎞�F���������@�������F �����E���񋤂��ڕW�ʒu�̎w��͈͓�

	pelement = &(precipe->steps[precipe->n_step++]);						//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
	pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// �����ʒu�҂�
	pelement->_t = TIME_LIMIT_ERROR_DETECT;									// �^�C���I�[�o�[���~�b�g�l
	pelement->_v = 0.0;														// ���x0
	pelement->_p = pwork->pos[id];											// �ڕW�ʒu
	D = D;																	// �c�苗���ύX�Ȃ�

	/*### STEP1�A2 ���x�X�e�b�v�o�́@###*/

	double d_step = D;																//�X�e�b�v�ł̈ړ�����
	double v_top = 0.0;																//�X�e�b�v���x�p
	double check_d;
	int n = 0, i;

	// #Step1�@�P�i��
	for (i = NOTCH_MAX;i > 0;i--) {
		v_top = pCraneStat->spec.notch_spd_f[id][i];
		check_d = v_top * v_top / st_com_work.a[id] + SPD_FB_DELAY_TIME * v_top;
		if (check_d < d_step) break;
		else continue;																//���̃m�b�`��
	}

	pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
	pelement->type = CTR_TYPE_VOUT_POS;												//�ʒu���B�҂�
	double ta = v_top / st_com_work.a[id];
	double tc = (D - v_top * ta) / v_top;
	pelement->_t = ta + tc;															// �U������@-�@��������
	pelement->_v = v_top;															// ���x0
	double d_move = v_top * (tc + 0.5 * ta);
	pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// �ڕW�ʒu
	D -= d_move;																	// �c�苗���X�V

	//  #Step2 ��~
	pelement = &(precipe->steps[precipe->n_step++]);								//�X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
	pelement->type = CTR_TYPE_VOUT_V;												//���x���B�҂�
	pelement->_t = ta;																//��������
	pelement->_v = 0.0;																// ���x0
	pelement->_p = st_com_work.target.pos[id];											// �ڕW�ʒu
	CHelper::fit_slew_axis(&(pelement->_p));										//�ڕW�ʒu�̍Z��
	D -= d_move;																	// �c�苗���X�V


	/*### STEP7  ###*/
	pelement = &(precipe->steps[precipe->n_step++]);								// �X�e�b�v�̃|�C���^�Z�b�g���Ď��X�e�b�v�p�ɃJ�E���g�A�b�v
	pelement->type = CTR_TYPE_FINE_POS;												// �����ʒu����
	pelement->_t = FINE_POS_TIMELIMIT;												// �ʒu���킹�ő�p������
	pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];						// �P�m�b�`���x
	pelement->_p = st_com_work.target.pos[id];										// �ڕW�ʒu
	D = 0;																			// �c�苗���ύX�Ȃ�
	return POLICY_PTN_OK;
}


LPST_COMMAND_BLOCK CPolicy::create_semiauto_command(LPST_JOB_SET pjob) {							//���s���锼�����R�}���h���Z�b�g����
	
	LPST_COMMAND_BLOCK lp_semiauto_com = &(pPolicyInf->command_list.commands[0]);					//�R�}���h�u���b�N�̃|�C���^�Z�b�g
	
	lp_semiauto_com->no = COM_NO_SEMIAUTO;
	lp_semiauto_com->type = AUTO_TYPE_SEMIAUTO;

	for (int i = 0;i < MOTION_ID_MAX;i++) lp_semiauto_com->is_active_axis[i]= false;
	lp_semiauto_com->is_active_axis[ID_HOIST] = true;
	lp_semiauto_com->is_active_axis[ID_SLEW] = true;
	lp_semiauto_com->is_active_axis[ID_BOOM_H] = true;
	
	set_com_workbuf(pjob->target[0], AUTO_TYPE_SEMIAUTO);											//�������p�^�[���쐬�p�f�[�^��荞��

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
	case AUTO_TYPE_SEMIAUTO: {

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
				else if(temp_d > 0.0) {
					st_com_work.motion_dir[i] = ID_FWD;												//�ړ�����
					st_com_work.dist_for_target[i] = temp_d;										//�ړ�����
				}
				else{
					st_com_work.motion_dir[i] = ID_STOP;											//�ړ�����
					st_com_work.dist_for_target[i] = 0.0;											//�ړ�����
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

#if 0
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

#endif

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


