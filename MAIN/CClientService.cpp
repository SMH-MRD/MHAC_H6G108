#include "CClientService.h"


//-���L�������I�u�W�F�N�g�|�C���^:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
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

	for (int i = 0;i < N_PLC_PB;i++) PLC_PBs_last[i] = false;

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnvironment = (CEnvironment*)VectpCTaskObj[g_itask.environment];


	set_panel_tip_txt();

	inf.is_init_complete = true;
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

//����������菇1�@�O���M������


void CClientService::input() {

	//�����J�nPB
	if (pPLC_IO->ui.PB[ID_PB_AUTO_START])CS_workbuf.plc_pb[ID_PB_AUTO_START]++;
	else CS_workbuf.plc_pb[ID_PB_AUTO_START] = 0;


	return;

};

//����������菇2�@���C������

static DWORD PLC_Dbg_last_input = 0;

void CClientService::main_proc() {

	/*### ���[�h�Ǘ� ###*/
	//�U��~�߃��[�h�Z�b�g
	if (pPLC_IO->ui.PB[ID_PB_ANTISWAY_OFF]) CS_workbuf.auto_standby = L_OFF;
	else if (pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON]) CS_workbuf.auto_standby = L_ON;
	else;
	
	/*### ���������� ###*/
	//�������ݒ�X�V,�������W���u�Z�b�g
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
		}
		else if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_SELECT_TIME) {						 //�������ڕW�ݒ�
			if (i == CS_workbuf.semi_auto_selected){//�ݒ蒆�̃{�^���������������
				CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
				update_semiauto_joblist(CS_SEMIAUTO_LIST_CLEAR,i);
			}
			else {
				CS_workbuf.semi_auto_selected = i;
				update_semiauto_joblist(CS_SEMIAUTO_LIST_ADD, i);

			}
		}
		else;

	}

	//�������ݒ����
	if (pPLC_IO->ui.PB[ID_PB_AUTO_RESET])
		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;


	/*### Job���� ###*/


	return;

}

//����������菇3�@�M���o�͏���
void CClientService::output() {

/*### �����֘A�����v�\���@###*/
	//�U��~�߃����v�@�����J�n�����v(JOB���s���_��,JOB���s���łȂ��o�^JOB�L�œ_�ŁA���̑������j
	if (CS_workbuf.auto_standby) {//�������[�h��
		//�����J�n�����v
		if ((pAgent_Inf->auto_on_going == AUTO_TYPE_JOB) || (pAgent_Inf->auto_on_going == AUTO_TYPE_SEMI_AUTO)) {//JOB���s��
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;//�}�j���A��
		}
		else {
			if ((CS_workbuf.job_list.job_wait_n + CS_workbuf.job_list.semiauto_wait_n)>0) {						//�ҋ@JOB�����鎞�_��
				if (inf.act_count % PLC_IO_LAMP_FLICKER_COUNT > PLC_IO_LAMP_FLICKER_CHANGE) CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
				else CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
			}
			else {												//JOB�����ŏ���
				CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
			}
		}
		//�U��~�߃����v
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_OFF] = L_OFF;
		if (pAgent_Inf->auto_on_going == AUTO_TYPE_MANUAL) {
			CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_ON;
		}
		else {//�U��~�ߋN�����͓_��
			if (inf.act_count % PLC_IO_LAMP_FLICKER_COUNT > PLC_IO_LAMP_FLICKER_CHANGE) CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_ON;
			else CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_OFF;
		}
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_OFF] = L_ON;

		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}


	//�����������v	LAMP�@�J�E���g�l�@0�F�����@�J�E���g�l%PLC_IO_LAMP_FLICKER_COUNT�@���@PLC_IO_LAMP_FLICKER_CHANGE�ȉ���OFF,�ȏ��ON
	for (int i = 0;i < SEMI_AUTO_TG_CLR;i++) {
		if (i == CS_workbuf.semi_auto_selected) {	//�������I�𒆂�PB
			if ((CS_workbuf.semiauto_pb[i] > SEMI_AUTO_TG_SELECT_TIME) && (CS_workbuf.semiauto_pb[i] < SEMI_AUTO_TG_RESET_TIME)) {//�ڕW�ʒu�X�V���͓_��
				if ((CS_workbuf.semiauto_lamp[i] % PLC_IO_LAMP_FLICKER_COUNT) > PLC_IO_LAMP_FLICKER_CHANGE)
					CS_workbuf.semiauto_lamp[i] = L_OFF;
				else 
					CS_workbuf.semiauto_lamp[i] = L_ON;
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

	wostrs << L" --Scan " << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};
/****************************************************************************/
/*   �������֘A																*/
/****************************************************************************/
int CClientService:: update_semiauto_list(int command, int code){
	switch (command) {
	case CS_CLEAR_SEMIAUTO: {	//�������W���u�N���A
		CS_workbuf.job_list.semiauto_wait_n = 0;
		CS_workbuf.job_list.i_semiauto_next = 0;
		CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_next].n_step = 0;
		return ID_OK;
	}break;
	case CS_ADD_SEMIAUTO: {	//�X�V
		if ((code < SEMI_AUTO_TG1) || (code >= SEMI_AUTO_TG_CLR)) {
			return ID_NG;
		}
		else {
			CS_workbuf.job_list.semiauto_wait_n = 1;
			CS_workbuf.job_list.i_semiauto_next = 0;
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_next].n_step = JOB_N_STEP_SEMIAUTO;
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_next].target[0] = CS_workbuf.semi_auto_setting_target[code];

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
	case CS_CLEAR_SEMIAUTO: {	//�W���u�N���A
		CS_workbuf.job_list.job_wait_n = 0;
		CS_workbuf.job_list.i_job_next = 0;
		CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_next].n_step = 0;
		return ID_OK;
	}break;
	case CS_ADD_SEMIAUTO: {	//�X�V
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
