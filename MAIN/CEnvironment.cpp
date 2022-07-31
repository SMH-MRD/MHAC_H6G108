#include "CEnvironment.h"

//-���L�������I�u�W�F�N�g�|�C���^:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pSimulationStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem*  pCSInfObj;
extern CSharedMem* pPolicyInfmandStatusObj;
extern CSharedMem* pAgentInfObj;

/****************************************************************************/
/*   �R���X�g���N�^�@�f�X�g���N�^                                           */
/****************************************************************************/
CEnvironment::CEnvironment() {
	pCraneStat = NULL;
	pPLC_IO = NULL;
	pSway_IO = NULL;
	pRemoteIO = NULL;
	pSimStat = NULL;
}

CEnvironment::~CEnvironment() {

}


/****************************************************************************/
/*   �^�X�N����������                                                       */
/* �@���C���X���b�h�ŃC���X�^���X��������ɌĂт܂��B                       */
/****************************************************************************/
void CEnvironment::init_task(void* pobj) {

	//���L�N���[���X�e�[�^�X�\���̂̃|�C���^�Z�b�g
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pRemoteIO = (LPST_REMOTE_IO)(pRemoteIO_Obj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());
	pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());
	
	//�N���[���d�l�Z�b�g
	stWorkCraneStat.spec = this->spec;

	//�U��Z���T�J�����d�l�Z�b�g
	//  �������
	stWorkCraneStat.sw_stat.cam[ID_SLEW].D0 = spec.Csw[SID_CAM1][SID_X][SID_D0];
	stWorkCraneStat.sw_stat.cam[ID_SLEW].H0 = spec.Csw[SID_CAM1][SID_X][SID_H0];
	stWorkCraneStat.sw_stat.cam[ID_SLEW].l0 = spec.Csw[SID_CAM1][SID_X][SID_l0];
	stWorkCraneStat.sw_stat.cam[ID_SLEW].ph0 = spec.Csw[SID_CAM1][SID_X][SID_ph0];

	stWorkCraneStat.sw_stat.cam[ID_BOOM_H].D0 = spec.Csw[SID_CAM1][SID_Y][SID_D0];
	stWorkCraneStat.sw_stat.cam[ID_BOOM_H].H0 = spec.Csw[SID_CAM1][SID_Y][SID_H0];
	stWorkCraneStat.sw_stat.cam[ID_BOOM_H].l0 = spec.Csw[SID_CAM1][SID_Y][SID_l0];
	stWorkCraneStat.sw_stat.cam[ID_BOOM_H].ph0 = spec.Csw[SID_CAM1][SID_Y][SID_ph0];

	set_panel_tip_txt();
	return;
};

/****************************************************************************/
/*   �^�X�N���������                                                       */
/* �@�^�X�N�X���b�h�Ŗ��������s�����֐�			�@                      */
/****************************************************************************/
void CEnvironment::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//����������菇1�@�O���M������
void CEnvironment::input(){
	

	return;

};

//����������菇2�@���C������

void CEnvironment::main_proc() {

	//���C���E�B���h�E��Tweet���b�Z�[�W�X�V
	tweet_update();

	//�w���V�[�J�E���^�Z�b�g
	stWorkCraneStat.env_act_count = inf.total_act;

	//�T�u�v���Z�X�`�F�b�N
	chk_subproc();

	//���[�h�Z�b�g
	mode_set();


	//�m�b�`�w�ߏ�ԃZ�b�g
	parse_notch_com();

	//�ʒu���Z�b�g
	pos_set();
	
	//�U����Z�b�g
	parse_sway_stat(ID_SLEW);
	parse_sway_stat(ID_BOOM_H);

	return;
}

//����������菇3�@�M���o�͏���

void CEnvironment::output() {

	

	//���L�������o��
	memcpy_s(pCraneStat, sizeof(ST_CRANE_STATUS), &stWorkCraneStat, sizeof(ST_CRANE_STATUS));
	
	return;

}; 

/****************************************************************************/
/*�@�@�m�b�`���͐M���𑬓x�w�߂ɕϊ����Ď�荞��				            */
/****************************************************************************/
int CEnvironment::parse_notch_com() {

	int* p_notch;
	if (stWorkCraneStat.operation_mode & OPERATION_MODE_REMOTE) p_notch = pRemoteIO->PLCui.notch_pos;
	else p_notch = pPLC_IO->ui.notch_pos;


	if (pPLC_IO->ui.notch_pos[ID_HOIST] < 0) stWorkCraneStat.notch_spd_ref[ID_HOIST] = stWorkCraneStat.spec.notch_spd_r[ID_HOIST][iABS(p_notch[ID_HOIST])];
	else stWorkCraneStat.notch_spd_ref[ID_HOIST] = stWorkCraneStat.spec.notch_spd_f[ID_HOIST][iABS(p_notch[ID_HOIST])];


	if (pPLC_IO->ui.notch_pos[ID_GANTRY] < 0) stWorkCraneStat.notch_spd_ref[ID_GANTRY] = stWorkCraneStat.spec.notch_spd_r[ID_GANTRY][iABS(p_notch[ID_GANTRY])];
	else stWorkCraneStat.notch_spd_ref[ID_GANTRY] = stWorkCraneStat.spec.notch_spd_f[ID_GANTRY][iABS(p_notch[ID_GANTRY])];


	if (pPLC_IO->ui.notch_pos[ID_BOOM_H] < 0) stWorkCraneStat.notch_spd_ref[ID_BOOM_H] = stWorkCraneStat.spec.notch_spd_r[ID_BOOM_H][iABS(p_notch[ID_BOOM_H])];
	else stWorkCraneStat.notch_spd_ref[ID_BOOM_H] = stWorkCraneStat.spec.notch_spd_f[ID_BOOM_H][iABS(p_notch[ID_BOOM_H])];


	if (pPLC_IO->ui.notch_pos[ID_SLEW] < 0) stWorkCraneStat.notch_spd_ref[ID_SLEW] = stWorkCraneStat.spec.notch_spd_r[ID_SLEW][iABS(p_notch[ID_SLEW])];
	else stWorkCraneStat.notch_spd_ref[ID_SLEW] = stWorkCraneStat.spec.notch_spd_f[ID_SLEW][iABS(p_notch[ID_SLEW])];

	return 0;

};
/****************************************************************************/
/*�@ ����p�U���Ԍv�Z											            */
/****************************************************************************/
int CEnvironment::parse_sway_stat(int ID) {

	double th, dth, L, dph, ddph, dthw;

	double D = stWorkCraneStat.sw_stat.cam[ID].D0;
	double  l0 = stWorkCraneStat.sw_stat.cam[ID].l0;
	double  H = stWorkCraneStat.sw_stat.cam[ID].H0 + l0;
	double  ph0 = stWorkCraneStat.sw_stat.cam[ID].ph0;
	
	stWorkCraneStat.mh_l = L = pCraneStat->rc.z - pCraneStat->rl.z;//���[�v��
	
	//�p���g��
	if (stWorkCraneStat.mh_l>1.0) {	//���[�v������
		stWorkCraneStat.sw_stat.sw[ID].w = sqrt(GA / stWorkCraneStat.mh_l);
	}
	else {
		stWorkCraneStat.sw_stat.sw[ID].w = 3.13;//1m���[�v�����̊p���g��
	}

	//����
	stWorkCraneStat.sw_stat.sw[ID].T = PI360 / stWorkCraneStat.sw_stat.sw[ID].w;
	
	//�U�p�@�U�p���x�@�U���@�ʑ��@
	//    �J�����ʒu����̐U��p���J�������o�p�{��t�I�t�Z�b�g  
	th = pSway_IO->rad[ID] + ph0;
	//    �J�����ʒu�ƒݓ_����̐U��p�� 	
	dph = asin((D * cos(th) - H * sin(th)) / L);
	//    �ݓ_����̐U��p 	
	stWorkCraneStat.sw_stat.sw[ID].th = th + dph;

	//    �J�����ʒu����̐U��p���x 	
	dth = pSway_IO->w[ID] ;
	//    �J�����ʒu�ƒݓ_����̐U��p���x��
	ddph = 1.0 - (D * sin(th) + H * cos(th)) / (L * cos(dph));
	//    �ݓ_����̐U��p���x 	
	stWorkCraneStat.sw_stat.sw[ID].dth = dth * ddph;

	//    �ݓ_����̐U��p���x/�ց@�ʑ�����Y���l 
	dthw = stWorkCraneStat.sw_stat.sw[ID].dth / stWorkCraneStat.sw_stat.sw[ID].w;

	//    �ݓ_����̐U��p�U��
	stWorkCraneStat.sw_stat.sw[ID].amp2 = stWorkCraneStat.sw_stat.sw[ID].th * stWorkCraneStat.sw_stat.sw[ID].th
											+ dthw * dthw;
	//    �ʑ����ʏ�̈ʑ��@0�����
	if (abs(stWorkCraneStat.sw_stat.sw[ID].th) < 0.000001) {
		if(dthw < 0.0)	stWorkCraneStat.sw_stat.sw[ID].ph = -PI90;
		else 	stWorkCraneStat.sw_stat.sw[ID].ph = PI90;
	}
	else {
		stWorkCraneStat.sw_stat.sw[ID].ph = dthw / stWorkCraneStat.sw_stat.sw[ID].th;
	}
	// atan�́A-��/2�`��/2�̕\���@���@-�΁`�΂ŕ\������ 
	if (stWorkCraneStat.sw_stat.sw[ID].th < 0.0) {
		if (dthw < 0.0) stWorkCraneStat.sw_stat.sw[ID].ph -= PI180;
		else stWorkCraneStat.sw_stat.sw[ID].ph += PI180;
	}
		
	return 0;
}
/****************************************************************************/
/*�@ ����p�U���Ԍv�Z											            */
/****************************************************************************/
int CEnvironment::mode_set() {
	//�����[�g���[�h�Z�b�g
	if (pPLC_IO->ui.PB[ID_PB_REMOTE_MODE])stWorkCraneStat.operation_mode |= OPERATION_MODE_REMOTE;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_REMOTE;

	//�V�~�����[�^���[�h�Z�b�g
	if (pSimStat->mode & SIM_ACTIVE_MODE)stWorkCraneStat.operation_mode |= OPERATION_MODE_SIMULATOR;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_SIMULATOR;

	//PLC�f�o�b�O���[�h�Z�b�g
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE)stWorkCraneStat.operation_mode |= OPERATION_MODE_PLC_DBGIO;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_PLC_DBGIO;

	return 0;

}
/****************************************************************************/
/*�@ �ʒu���Z�b�g											            */
/****************************************************************************/
int CEnvironment::pos_set() {
	//�N���[����_��x,y,z���΍��W
	stWorkCraneStat.rc0.x = pPLC_IO->status.pos[ID_GANTRY];	//���s�ʒu
	stWorkCraneStat.rc0.y = 0.0;							//���񒆐S�_
	stWorkCraneStat.rc0.z = 0.0;							//���s���[������

	//�N���[���ݓ_�̃N���[����_�Ƃ�x,y,z���΍��W
	stWorkCraneStat.rc.x = pPLC_IO->status.pos[ID_BOOM_H] * cos(pPLC_IO->status.pos[ID_SLEW]);
	stWorkCraneStat.rc.y = pPLC_IO->status.pos[ID_BOOM_H] * sin(pPLC_IO->status.pos[ID_SLEW]);
	stWorkCraneStat.rc.z = spec.boom_high;

	//�ׂ݉̃N���[����_�Ƃ�x, y, z���΍��W
	stWorkCraneStat.rl.x = pCraneStat->rc.x;
	stWorkCraneStat.rl.y = pCraneStat->rc.y;
	stWorkCraneStat.rl.z = pPLC_IO->status.pos[ID_HOIST];

	//�Ɍ�����
	if (stWorkCraneStat.rc0.x < spec.gantry_pos_min) stWorkCraneStat.is_rev_endstop[ID_GANTRY] = true;
	else stWorkCraneStat.is_rev_endstop[ID_GANTRY] = false;
	if (stWorkCraneStat.rc0.x > spec.gantry_pos_max) stWorkCraneStat.is_fwd_endstop[ID_GANTRY] = true;
	else stWorkCraneStat.is_fwd_endstop[ID_GANTRY] = false;

	if (stWorkCraneStat.rl.z < spec.hoist_pos_min) stWorkCraneStat.is_rev_endstop[ID_HOIST] = true;
	else stWorkCraneStat.is_rev_endstop[ID_HOIST] = false;
	if (stWorkCraneStat.rl.z > spec.hoist_pos_max) stWorkCraneStat.is_fwd_endstop[ID_HOIST] = true;
	else stWorkCraneStat.is_fwd_endstop[ID_HOIST] = false;

	if (pPLC_IO->status.pos[ID_BOOM_H] < spec.boom_pos_min) stWorkCraneStat.is_rev_endstop[ID_BOOM_H] = true;
	else stWorkCraneStat.is_rev_endstop[ID_BOOM_H] = false;
	if (pPLC_IO->status.pos[ID_BOOM_H] > spec.boom_pos_max) stWorkCraneStat.is_fwd_endstop[ID_BOOM_H] = true;
	else stWorkCraneStat.is_fwd_endstop[ID_BOOM_H] = false;

	return 0;

}
/****************************************************************************/
/*�@�@�T�u�v���Z�X�̏�Ԋm�F			            */
/****************************************************************************/
static DWORD plc_io_helthy_NGcount = 0;
static DWORD plc_io_helthy_count_last = 0;
static DWORD sim_helthy_NGcount = 0;
static DWORD sim_helthy_count_last = 0;
static DWORD sway_helthy_NGcount = 0;
static DWORD sway_helthy_count_last = 0;

void CEnvironment::chk_subproc() {

	//PLC IF
	if (plc_io_helthy_count_last == pPLC_IO->helthy_cnt) plc_io_helthy_NGcount++;
	else plc_io_helthy_NGcount = 0;
	if (plc_io_helthy_NGcount > PLC_IO_HELTHY_NG_COUNT) stWorkCraneStat.subproc_stat.is_plcio_join = false;
	else stWorkCraneStat.subproc_stat.is_plcio_join = true;
	plc_io_helthy_count_last = pPLC_IO->helthy_cnt;

	//SWAY IF
	if (sway_helthy_count_last == pSway_IO->helthy_cnt) sway_helthy_NGcount++;
	else sway_helthy_NGcount = 0;
	if (sway_helthy_NGcount > SWAY_HELTHY_NG_COUNT) stWorkCraneStat.subproc_stat.is_sway_join = false;
	else stWorkCraneStat.subproc_stat.is_sway_join = true;
	sway_helthy_count_last = pSway_IO->helthy_cnt;

	//SIM
	if (sim_helthy_count_last == pSimStat->helthy_cnt) sim_helthy_NGcount++;
	else sim_helthy_NGcount = 0;
	if (sim_helthy_NGcount >SIM_HELTHY_NG_COUNT) stWorkCraneStat.subproc_stat.is_sim_join = false;
	else stWorkCraneStat.subproc_stat.is_sim_join = true;
	sim_helthy_count_last = pSimStat->helthy_cnt;

	return;

};

/****************************************************************************/
/*�@�@���C���E�B���h�E��Tweet���b�Z�[�W�X�V          			            */
/****************************************************************************/
void CEnvironment::tweet_update() {

	//PLC
	if (stWorkCraneStat.subproc_stat.is_plcio_join == true) {
		if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE) wostrs << L" #PLC:DBG";
		else wostrs << L" #PLC:PLC";
/*
		if (pPLC_IO->status.ctrl[ID_WORD_CTRL_SOURCE_ON] == L_ON) wostrs << L" ,! PW:ON";
		else wostrs << L",! PW:OFF";

		if (pPLC_IO->status.ctrl[ID_WORD_CTRL_REMOTE] == L_ON) wostrs << L",@ RMT";
		else wostrs << L",@CRANE";
*/
	}
	else wostrs << L" # PLC:NG";

	//SWAY
	if (stWorkCraneStat.subproc_stat.is_sway_join == true) {
		if (pSway_IO->proc_mode & SWAY_IF_SIM_DBG_MODE) wostrs << L" #SWY:SIM";
		else wostrs << L" #SWY:CAM";
	}
	else wostrs << L" #SWY:NG";

	//SIM
	if (stWorkCraneStat.subproc_stat.is_sim_join == true) {
		if (pSimStat->mode & SIM_ACTIVE_MODE) wostrs << L" #SIM:ACT";
		else wostrs << L" #SIM:STP";
	}
	else wostrs << L" #SIM:OUT";

	wostrs << L" --Scan " << inf.period;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();

	return;

};

/****************************************************************************/
/*   �^�X�N�ݒ�^�u�p�l���E�B���h�E�̃R�[���o�b�N�֐�                       */
/****************************************************************************/
LRESULT CALLBACK CEnvironment::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

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
void CEnvironment::set_panel_tip_txt()
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
void CEnvironment::set_panel_pb_txt() {

//	WCHAR str_func06[] = L"DEBUG";

//	SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};