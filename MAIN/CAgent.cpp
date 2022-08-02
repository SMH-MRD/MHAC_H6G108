#include "CAgent.h"
#include "CPolicy.h"
#include "CPolicy.h"

//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
/****************************************************************************/
CAgent::CAgent() {
	pPolicyInf = NULL;
	pPLC_IO = NULL;
	pCraneStat = NULL;
}

CAgent::~CAgent() {

}


/****************************************************************************/
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/
void CAgent::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());

	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;
	for (int i = 0;i < N_PLC_LAMP;i++) AgentInf_workbuf.PLC_LAMP_com[i] = 0;

	set_panel_tip_txt();
	return;
};

/****************************************************************************/
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
void CAgent::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//定周期処理手順1　外部信号入力
void CAgent::input() {

	return;

};

//定周期処理手順2　メイン処理
void CAgent::main_proc() {

	set_lamp();

	return;

}

//定周期処理手順3　信号出力処理
void CAgent::output() {

//PLCへの出力計算
	set_ref_mh();			//巻き速度指令
	set_ref_gt();			//走行速度指令
	set_ref_slew();			//旋回速度指令
	set_ref_bh();			//引込速度指令
	update_pb_lamp_com();	//PB LAMP出力


		//共有メモリ出力処理
	memcpy_s(pAgentInf, sizeof(ST_AGENT_INFO), &AgentInf_workbuf, sizeof(ST_AGENT_INFO));

	wostrs << L" #Ref:" << fixed<<setprecision(3);
	wostrs << L"MH " << AgentInf_workbuf.v_ref[ID_HOIST];
	wostrs << L",GT " << AgentInf_workbuf.v_ref[ID_GANTRY];
	wostrs << L",SL " << AgentInf_workbuf.v_ref[ID_SLEW];
	wostrs << L",BH " << AgentInf_workbuf.v_ref[ID_BOOM_H];

	wostrs <<  L" --Scan " << inf.period;;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   巻指令出力処理		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (pPolicyInf->pc_ctrl_mode & BITSEL_HOIST) {
		AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
	}
	else {
		AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
	}
	return 0; 
}
/****************************************************************************/
/*   走行指令出力処理		                                                */
/****************************************************************************/
int CAgent::set_ref_gt(){
	if (pPolicyInf->pc_ctrl_mode & BITSEL_GANTRY) {
		AgentInf_workbuf.v_ref[ID_GANTRY] = pCraneStat->notch_spd_ref[ID_GANTRY];
	}
	else {
		AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
	}
	return 0;
}
/****************************************************************************/
/*   旋回指令出力処理		                                                */
/****************************************************************************/
int CAgent::set_ref_slew(){
	if (pPolicyInf->pc_ctrl_mode & BITSEL_SLEW) {
		AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];
	}
	else {
		AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
	}
	
	return 0;
}
/****************************************************************************/
/*   引込指令出力処理		                                                */
/****************************************************************************/
int CAgent::set_ref_bh(){
	if (pPolicyInf->pc_ctrl_mode & BITSEL_BOOM_H) {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
	}
	else {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
	}
	return 0;
}

/****************************************************************************/
/*   Job受付                                                                */
/****************************************************************************/
int CAgent::receipt_job(LPST_JOB_SET pjob, int to_do) {
	return 0;
};
/****************************************************************************/
/*   Command受付															*/
/****************************************************************************/
int CAgent::receipt_com(int type,int target) {
	switch(type){
	case COM_PB_SET:
		AgentInf_workbuf.PLC_PB_com[target] = AGENT_PB_OFF_DELAY;
		break;
	case COM_LAMP_ON:
		AgentInf_workbuf.PLC_LAMP_com[target] = PLC_IO_LAMP_FLICKER_CHANGE;
		break;
	case COM_LAMP_OFF:
		AgentInf_workbuf.PLC_LAMP_com[target] = 0;
		break;
	case COM_LAMP_FLICKER:
		AgentInf_workbuf.PLC_LAMP_com[target] = PLC_IO_LAMP_FLICKER_COUNT;
	default:
		break;
	}
	return 0;
};
/****************************************************************************/
/*  PB,ランプ出力内容セット														*/
/****************************************************************************/
void CAgent::set_lamp() {
	if (pPolicyInf->antisway_mode & BITSEL_COMMON) {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_ON;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_OFF;
	}
	else {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_OFF;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_ON;
	}
	return;
};  
/****************************************************************************/
/*  PB,ランプ指令更新														*/
/****************************************************************************/
void CAgent::update_pb_lamp_com() {
	//PB
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]--;

	//LAMP
	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] >PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8]++;
	else;

	return;
};
/****************************************************************************/
/*   タスク設定タブパネルウィンドウのコールバック関数                       */
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

			//サンプルとしていろいろな型で読み込んで表示している
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
/*   タスク設定パネルの操作ボタン説明テキスト設定関数                       */
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
/*　　タスク設定パネルボタンのテキストセット					            */
/****************************************************************************/
void CAgent::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};

