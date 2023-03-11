#include "CClientService.h"


//-���L�������I�u�W�F�N�g�|�C���^:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pOTEioObj;
extern CSharedMem*  pCSInfObj;
extern CSharedMem* pPolicyInfsObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;	//�^�X�N�I�u�W�F�N�g�̃|�C���^
extern ST_iTask g_itask;

/****************************************************************************/
/*   �R���X�g���N�^�@�f�X�g���N�^                                           */
/****************************************************************************/
CClientService::CClientService() {
	pPLC_IO = NULL;
	pCraneStat = NULL;
}

CClientService::~CClientService() {

}


/****************************************************************************/
/*   �^�X�N����������                                                       */
/* �@���C���X���b�h�ŃC���X�^���X��������ɌĂт܂��B                       */
/****************************************************************************/
static BOOL PLC_PBs_last[N_PLC_PB];

void CClientService::init_task(void* pobj) {

	//���L�������\���̂̃|�C���^�Z�b�g
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap()); 
	pCSinf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pAgent_Inf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pOTE_IO = (LPST_OTE_IO)(pOTEioObj->get_pMap());

	for (int i = 0;i < N_PLC_PB;i++) PLC_PBs_last[i] = false;

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnvironment = (CEnvironment*)VectpCTaskObj[g_itask.environment];


	set_panel_tip_txt();

	inf.is_init_complete = true;
	CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
	CS_workbuf.semiauto_status = STAT_OUT_OF_SERVICE;
	CS_workbuf.command_type = NON_COM;							//PICK, GRND, PARK



	return;
};

/****************************************************************************/
/*   �^�X�N���������                                                       */
/* �@�^�X�N�X���b�h�Ŗ��������s�����֐�			�@                      */
/****************************************************************************/
void CClientService::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//# ����������菇1�@�O���M������
/***  JOB�֘A���͐M�������i�����J�nPB)   ***/
/****************************************************************************/
/*  ���͏���																*/
/****************************************************************************/

void CClientService::input() {
	/*### �����������iCraneStat�̗����オ����Spec��荞�݁j ###*/
	if (pCraneStat->env_act_count < 10){//Environment�������オ���ɏ����l��荞��
		// �������ڕW�ʒu�̎�荞��
		for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
			CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST] = pCraneStat->spec.semi_target[i][ID_HOIST];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H] = pCraneStat->spec.semi_target[i][ID_BOOM_H];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW] = pCraneStat->spec.semi_target[i][ID_SLEW];
		}
	}

	//PLC���͏���
	parce_onboard_input(CS_NORMAL_OPERATION_MODE);
	//����[������
	if (can_ote_activate()) {
		parce_ote_imput(CS_NORMAL_OPERATION_MODE);
	}
	return;
};

static int as_pb_last = 0, auto_pb_last = 0, set_z_pb_last = 0, set_xy_pb_last = 0;
static int park_pb_last = 0, pick_pb_last = 0, grnd_pb_last = 0;
static int mhp1_pb_last = 0, mhp2_pb_last = 0, mhm1_pb_last = 0, mhm2_pb_last = 0;
static int slp1_pb_last = 0, slp2_pb_last = 0, slm1_pb_last = 0, slm2_pb_last = 0;
static int bhp1_pb_last = 0, bhp2_pb_last = 0, bhm1_pb_last = 0, bhm2_pb_last = 0;

int CClientService::parce_onboard_input(int mode) { 
	//�����N��PB
	if (pPLC_IO->ui.PB[ID_PB_AUTO_START])CS_workbuf.plc_pb[ID_PB_AUTO_START]++;
	else CS_workbuf.plc_pb[ID_PB_AUTO_START] = 0;

	/*### ���[�h�Ǘ� ###*/
	//�U��~�߃��[�h�Z�b�g
	if ((pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON]) && (as_pb_last == 0)) { // PB���͗����オ��
		if (CS_workbuf.antisway_mode == L_OFF)
			CS_workbuf.antisway_mode = L_ON;
		else
			CS_workbuf.antisway_mode = L_OFF;
	}
	as_pb_last = pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON];

	//�������[�h�Z�b�g
	if ((pPLC_IO->ui.PB[ID_PB_AUTO_MODE]) && (auto_pb_last == 0)) { // PB���͗����オ��
		if (CS_workbuf.auto_mode == L_OFF)
			CS_workbuf.auto_mode = L_ON;
		else
			CS_workbuf.auto_mode = L_OFF;
	}
	auto_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_MODE];


	if (CS_workbuf.auto_mode == L_ON) {
		//�ڕW�ʒu�m��Z�b�g
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z]) && (set_z_pb_last == 0)) { // PB���͗����オ��
			if (CS_workbuf.target_set_z & CS_SEMIAUTO_TG_SEL_FIXED)
				CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			else
				CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_FIXED;
		}
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY]) && (set_xy_pb_last == 0)) { // PB���͗����オ��
			if (CS_workbuf.target_set_xy & CS_SEMIAUTO_TG_SEL_FIXED)
				CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
			else
				CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_FIXED;
		}

		bool tg_sel_trigger_z = false, tg_sel_trigger_xy = false;

		//�������ڕW�ݒ�
		if ((CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) && (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED)) {//�ڕW�ʒu���m�莞�̂ݍX�V�\
			for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
				//PB ON���ԃJ�E���g ���������Z�b�g���Ԃ܂�
				if (pPLC_IO->ui.PBsemiauto[i] <= 0) CS_workbuf.semiauto_pb[i] = 0;
				else if (CS_workbuf.semiauto_pb[i] < SEMI_AUTO_TG_RESET_TIME) CS_workbuf.semiauto_pb[i]++;
				else;

				//�ڕW�ݒ�
				if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_RESET_TIME) {//�������ڕW�ʒu�ݒ�l�X�V
					CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];

					CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

					tg_sel_trigger_z = true, tg_sel_trigger_xy = true;
				}
				else if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_SELECT_TIME) {						 //�������ڕW�ݒ�
					if (i == CS_workbuf.semi_auto_selected) {//�ݒ蒆�̃{�^���������������
						CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];

						tg_sel_trigger_z = false, tg_sel_trigger_xy = false;
					}
					else {
						//�������I���{�^����荞��
						CS_workbuf.semi_auto_selected = i;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

						tg_sel_trigger_z = true, tg_sel_trigger_xy = true;
					}
				}
				else;
			}
		}

		if (CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) {//�ڕW�m��ݒ�OFF���␳�\
			if ((pPLC_IO->ui.PB[ID_PB_MH_P1]) && (mhp1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_100mm;
				tg_sel_trigger_z = true;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_P2]) && (mhp2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_1000mm;
				tg_sel_trigger_z = true;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_M1]) && (mhm1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_100mm;
				tg_sel_trigger_z = true;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_M2]) && (mhm2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_1000mm;
				tg_sel_trigger_z = true;
			}

			if (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED) {//�ڕW�m��ݒ�OFF���␳�\
				if ((pPLC_IO->ui.PB[ID_PB_BH_P1]) && (bhp1_pb_last == 0)) {
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_100mm;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_BH_P2]) && (bhp2_pb_last == 0)) {
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_1000mm;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_BH_M1]) && (bhm1_pb_last == 0)) {
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_100mm;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_BH_M2]) && (bhm2_pb_last == 0)) {
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_1000mm;
					tg_sel_trigger_xy = true;
				}

				//����͊p�x�ɕϊ�
				if ((pPLC_IO->ui.PB[ID_PB_SL_P1]) && (slp1_pb_last == 0)) {
					double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_SL_P2]) && (slp2_pb_last == 0)) {
					double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_SL_M1]) && (slm1_pb_last == 0)) {
					double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
					tg_sel_trigger_xy = true;
				}
				if ((pPLC_IO->ui.PB[ID_PB_SL_M2]) && (slm2_pb_last == 0)) {
					double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
					tg_sel_trigger_xy = true;
				}
			}
		}


		//�������ڕW�ݒ�L��ԃZ�b�g
		if (tg_sel_trigger_z)CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_ACTIVE;
		if (tg_sel_trigger_xy)CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_ACTIVE;

		//�����������{�^�����͂Őݒ�N���A
		if (pPLC_IO->ui.PB[ID_PB_AUTO_RESET]) {
			CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
			tg_sel_trigger_z = false, tg_sel_trigger_xy = false;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
		}

	}
	else {
	//�������[�h�łȂ��Ƃ��͔������ݒ�N���A
		CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
		CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
		CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
		CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];
	}


	//�O��l�ێ�
	set_z_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z];
	set_xy_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY];
	mhp1_pb_last = pPLC_IO->ui.PB[ID_PB_MH_P1];
	mhp2_pb_last = pPLC_IO->ui.PB[ID_PB_MH_P2];
	mhm1_pb_last = pPLC_IO->ui.PB[ID_PB_MH_M1];
	mhm2_pb_last = pPLC_IO->ui.PB[ID_PB_MH_M2];
	bhp1_pb_last = pPLC_IO->ui.PB[ID_PB_BH_P1];
	bhp2_pb_last = pPLC_IO->ui.PB[ID_PB_BH_P2];
	bhm1_pb_last = pPLC_IO->ui.PB[ID_PB_BH_M1];
	bhm2_pb_last = pPLC_IO->ui.PB[ID_PB_BH_M2];
	slp1_pb_last = pPLC_IO->ui.PB[ID_PB_SL_P1];
	slp2_pb_last = pPLC_IO->ui.PB[ID_PB_SL_P2];
	slm1_pb_last = pPLC_IO->ui.PB[ID_PB_SL_M1];
	slm2_pb_last = pPLC_IO->ui.PB[ID_PB_SL_M2];


	//�����R�}���h�Z�b�g
	if ((pPLC_IO->ui.PB[ID_PB_PARK]) && (park_pb_last == 0)) { // PB���͗����オ��
		if (CS_workbuf.command_type == PARK_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = PARK_COM;
	}
	park_pb_last = pPLC_IO->ui.PB[ID_PB_PARK];

	if ((pPLC_IO->ui.PB[ID_PB_PICK]) && (pick_pb_last == 0)) { // PB���͗����オ��
		if (CS_workbuf.command_type == PICK_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = PICK_COM;
	}
	pick_pb_last = pPLC_IO->ui.PB[ID_PB_PICK];

	if ((pPLC_IO->ui.PB[ID_PB_GRND]) && (grnd_pb_last == 0)) { // PB���͗����オ��K
		if (CS_workbuf.command_type == GRND_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = GRND_COM;
	}
	grnd_pb_last = pPLC_IO->ui.PB[ID_PB_GRND];

	return 0; 
}

int CClientService::parce_ote_imput(int mode) {
	return 0;
}
int CClientService::can_ote_activate() {
	if (pPLC_IO->ui.PB[ID_PB_REMOTE_MODE]) {
		return L_ON;
	}
	else {
		return L_OFF;
	}
}

//# ����������菇2�@���C������
/***  ���[�h�Ǘ�,JOB/�������Ď��X�V����   ***/


/****************************************************************************/
/*  ���C������																*/
/****************************************************************************/
void CClientService::main_proc() {

	//JOB�o�^�����i�����J�nPB�����j

	//�������W���u�X�e�[�^�X�Z�b�g
	if (CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].status == STAT_ACTIVE){
		CS_workbuf.semiauto_status = STAT_ACTIVE;
	}
	else if ((CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_FIXED) && (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_FIXED)) {
		if(CS_workbuf.semiauto_status != STAT_ACTIVE) CS_workbuf.semiauto_status = STAT_STANDBY;
	}
	else if(CS_workbuf.auto_mode){
		CS_workbuf.semiauto_status = STAT_REQ_WAIT;
	}
	else {
		CS_workbuf.semiauto_status = STAT_OUT_OF_SERVICE;
	}



	
	if (CS_workbuf.plc_pb[ID_PB_AUTO_START] == AUTO_START_CHECK_TIME) {
		if(CS_workbuf.semi_auto_selected != SEMI_AUTO_TG_CLR) update_semiauto_list(CS_ADD_SEMIAUTO, AUTO_TYPE_SEMIAUTO | COM_TYPE_PARK, CS_workbuf.semi_auto_selected);
		else update_semiauto_list(CS_CLEAR_SEMIAUTO, AUTO_TYPE_MANUAL, CS_workbuf.semi_auto_selected);
	}



	/*### Job���� ###*/
	//���݂̃W���u�̏󋵃Z�b�g
	judge_job_list_status();

	return;

}

//# ����������菇3�@�M���o��, ��ʃ��X�|���X����
//�W���u�֘A�����v�\����
/****************************************************************************/
/*  �o�͏���																*/
/****************************************************************************/
void CClientService::output() {

/*### �����֘A�����v�\���@###*/

	//�U��~�߃����v
	if (CS_workbuf.antisway_mode == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_ON;
	}
	else {//�U��~�ߋN�����͓_��
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_OFF;
	}

	//���������v
	if (CS_workbuf.auto_mode == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_MODE] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_MODE] = L_OFF;
	}

	//�N�������v

	if (CS_workbuf.job_list.hot_job_status & STAT_ACTIVE) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
	}
	else if ((CS_workbuf.semiauto_status & STAT_STANDBY) || (CS_workbuf.job_list.hot_job_status & STAT_STANDBY)) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
		else
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}
	else if (CS_workbuf.job_list.hot_job_status & STAT_SUSPEND)  {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
		else
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}

	//�ڕW�ݒ胉���v
	if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_FIXED) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_ON;
	}
	else if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_ACTIVE) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_ON;
		else
			CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_OFF;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_OFF;
	}

	if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_FIXED) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_ON;
	}
	else if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_ACTIVE) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_ON;
		else
			CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_OFF;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_OFF;
	}
	//�����R�}���h�����v
	if (CS_workbuf.command_type == PICK_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_ON;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == GRND_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_ON;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == PARK_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}

	//�����������v
	for (int i = 0;i < SEMI_AUTO_TG_CLR;i++) {
		if (i == CS_workbuf.semi_auto_selected) {	//�������I�𒆂�PB
			if(CS_workbuf.semiauto_pb[i]){
				if ((CS_workbuf.semiauto_pb[i] >= SEMI_AUTO_TG_RESET_TIME)||(CS_workbuf.semiauto_pb[i]< SEMI_AUTO_TG_SELECT_TIME * 4))
					CS_workbuf.semiauto_lamp[i] = L_ON;
				else if ((CS_workbuf.semiauto_pb[i] % (SEMI_AUTO_TG_SELECT_TIME*2)) > SEMI_AUTO_TG_SELECT_TIME)
					CS_workbuf.semiauto_lamp[i] = L_ON;
				else
					CS_workbuf.semiauto_lamp[i] = L_OFF;
			}
			else {//�ڕW�ʒu�m��œ_��
				CS_workbuf.semiauto_lamp[i] = L_ON;
			}
		}
		else {	//�������I�𒆂łȂ�PB
			CS_workbuf.semiauto_lamp[i] = L_OFF;
		}
	}


	//���L�������o��
	memcpy_s(pCSinf, sizeof(ST_CS_INFO), &CS_workbuf, sizeof(ST_CS_INFO));

	wostrs << L" AS: " << CS_workbuf.antisway_mode << L" AUTO: " << CS_workbuf.auto_mode;
	
	int job_stat = CS_workbuf.job_list.hot_job_status;

	if (job_stat & JOB_TYPE_JOB) {
		wostrs << L" >JOB: ";
		if (job_stat & STAT_STANDBY) wostrs << L"STANDBY";
		else if (job_stat & STAT_ACTIVE) wostrs << L"ACTIVE";
		else if (job_stat & STAT_SUSPEND) wostrs << L"SUSPEND";
		else if (job_stat & STAT_REQ_WAIT) wostrs << L"WAITING";
		else wostrs << L"OUT OF SERV";
	}
	else if (job_stat & JOB_TYPE_SEMIAUTO) {
		wostrs << L" >SEMIAUTO: ";
		if (job_stat & STAT_STANDBY) wostrs << L"STANDBY";
		else if (job_stat & STAT_ACTIVE) wostrs << L"ACTIVE";
		else if (job_stat & STAT_SUSPEND) wostrs << L"SUSPEND";
		else if (job_stat & STAT_REQ_WAIT) wostrs << L"WAITING";
		else wostrs << L"OUT OF SERV";
	}
	else  wostrs << L" >JOB WAITING ";

	wostrs << L" --Scan " << inf.period;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   JOB LIST�֘A															*/
/****************************************************************************/
int CClientService::judge_job_list_status() {


	//�W���u���X�g�X�e�[�^�X�Z�b�g
	if (CS_workbuf.auto_mode == L_OFF) {
		CS_workbuf.job_list.hot_job_status = STAT_OUT_OF_SERVICE;
	}
	else if (CS_workbuf.job_list.hot_job_status & STAT_ACTIVE) {
		CS_workbuf.job_list.hot_job_status &= JOB_TYPE_MASK;
		if (CS_workbuf.job_list.hot_job_status & JOB_TYPE_JOB) {
			CS_workbuf.job_list.hot_job_status |= CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].status;
		}
		else if (CS_workbuf.job_list.hot_job_status & JOB_TYPE_SEMIAUTO) {
			CS_workbuf.job_list.hot_job_status |= CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].status;
		}
		else {
			CS_workbuf.job_list.hot_job_status = STAT_ABOTED;
		}
	}
	else if (CS_workbuf.semiauto_status & STAT_STANDBY) {
		CS_workbuf.job_list.hot_job_status = STAT_OUT_OF_SERVICE;
		CS_workbuf.job_list.hot_job_status |= JOB_TYPE_SEMIAUTO;
		CS_workbuf.job_list.hot_job_status |= STAT_STANDBY;
	}
	else {
		CS_workbuf.job_list.hot_job_status |= STAT_REQ_WAIT;
	}

	return 0;
}

/****************************************************************************/
/*   �������֘A����															*/
/*   command	:�ǉ�,�폜��												*/
/*   type		:��������ށ@PARK,PICK,GRAND								*/
/*   code		:����������w��R�[�h�@�I�𒆔�����PB  						*/
/****************************************************************************/
int CClientService:: update_semiauto_list(int command, int type, int code){
	switch (command) {
	case CS_CLEAR_SEMIAUTO: {	//�������W���u�N���A
		CS_workbuf.job_list.n_semiauto_hold = 0;
		CS_workbuf.job_list.i_semiauto_active = 0;
		CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].status = STAT_STANDBY;
		CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].type = type;
		return ID_OK;
	}break;
	case CS_ADD_SEMIAUTO: {	//�X�V
		if ((code < SEMI_AUTO_S1) || (code >= SEMI_AUTO_TG_CLR)) {
			return ID_NG;
		}
		else {
			CS_workbuf.job_list.n_semiauto_hold = 1;
			CS_workbuf.job_list.i_semiauto_active = 0;		//���ʔ�������id=0�̂ݎg�p
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].n_command = JOB_N_STEP_SEMIAUTO;
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].target[0] = CS_workbuf.semi_auto_setting_target[code];
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].type = type;

			return ID_OK;
		}
	}break;
	default:
		return ID_NA;
		break;
	}
}

/****************************************************************************/
/*   JOB�֘A																*/
/****************************************************************************/
int CClientService::update_job_list(int command, int code) {
	switch (command) {
	case CS_CLEAR_JOB: {	//�W���u�N���A
		CS_workbuf.job_list.n_job_hold = 0;
		CS_workbuf.job_list.i_job_active = 0;
		CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].n_command = 0;
		CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].status = STAT_REQ_WAIT;
		return ID_OK;
	}break;
	case CS_ADD_JOB: {	//�X�V
		return ID_NG;
	}break;
	default:
		return ID_NA;
		break;
	}
}
/****************************************************************************/
/*   �^�X�N�ݒ�^�u�p�l���E�B���h�E�̃R�[���o�b�N�֐�                       */
/****************************************************************************/
LRESULT CALLBACK CClientService::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

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
void CClientService::set_panel_tip_txt()
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
void CClientService::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};
