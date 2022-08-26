#include "CSCADA.h"

//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;
extern CSharedMem* pSimulationStatusObj;

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
/****************************************************************************/
CSCADA::CSCADA() {
	pPLC_IO = NULL;
	pCraneStat = NULL;
	pRemoteIO = NULL;
	pSway_IO = NULL;
	pSimStat = NULL;
	pCSInf = NULL;
	pPolicyInf = NULL;
	pAgentInf = NULL;
}
CSCADA::~CSCADA() {

}


/****************************************************************************/
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/
void CSCADA::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pRemoteIO = (LPST_REMOTE_IO)(pRemoteIO_Obj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());
	pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());
	pCSInf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	
	//チャートデータバッファクリア
	memset(&chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS], 0, sizeof(ST_CHART_PLOT));
	memset(&chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS], 0, sizeof(ST_CHART_PLOT));
	memset(&chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS], 0, sizeof(ST_CHART_PLOT));
	memset(&chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS], 0, sizeof(ST_CHART_PLOT));
	//チャート100％値デフォルト値セット
	for (int i = 0; i < SCAD_N_CHART_PER_WND;i++) {
		for (int j = 0; j < SCAD_N_GRAPH_PAR_CHART;j++) {
			chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].d100[i][j] = 1.0;
			chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[i][j] = 1.0;
			chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].i100[i][j] = 1;
			chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].i100[i][j] = 1;
		}
		chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].b100 = true;
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].b100 = true;
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].b100 = true;
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].b100 = true;
	}
	//チャート100％値ポインタバッファセット
	for (int i = 0; i < SCAD_N_CHART_PER_WND;i++) {
		for (int j = 0; j < SCAD_N_GRAPH_PAR_CHART;j++) {
			chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].pd100[i][j] = &chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].d100[i][j];
			chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd100[i][j] = &chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[i][j];
			chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].pd100[i][j] = &chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].d100[i][j];
			chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].pd100[i][j] = &chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].d100[i][j];
			chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].pi100[i][j] = &chart_plot_buf[SCAD_CHART_WND1][SCAD_X_AXIS].i100[i][j];
			chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pi100[i][j] = &chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].i100[i][j];
			chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].pi100[i][j] = &chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].i100[i][j];
			chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].pi100[i][j] = &chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].i100[i][j];
		}
	}

	CMKChart::init_chartfunc();


	set_panel_tip_txt();
	return;
};

/****************************************************************************/
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
void CSCADA::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//定周期処理手順1　外部信号入力
void CSCADA::input() {

	return;

};

//定周期処理手順2　メイン処理
void CSCADA::main_proc() {

	//チャートデータセット
	/*
	if (chart_plot_ptn != SCAD_CHART_PTN0) {
		if (CMKChart::is_chart_open(MK_CHART1) || CMKChart::is_chart_open(MK_CHART2)){
			set_chart_data(chart_plot_ptn);
		}
	}
	*/
	return;
}

//定周期処理手順3　信号出力処理
void CSCADA::output() {

	wostrs << L" V: mh " << ((LPST_PLC_IO)pPLCioObj->get_pMap())->status.v_fb[ID_HOIST];
	wostrs << L" --Scan " << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   タスク設定タブパネルウィンドウのコールバック関数                       */
/****************************************************************************/
LRESULT CALLBACK CSCADA::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

	switch (msg) {
	case WM_COMMAND:
		switch (LOWORD(wp)) {
		case IDC_TASK_FUNC_RADIO1:
		{
			inf.panel_func_id = LOWORD(wp); set_panel_tip_txt(); set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			reset_panel_item_pb(hDlg);
		}break;
		case IDC_TASK_FUNC_RADIO2:
		case IDC_TASK_FUNC_RADIO3:
		case IDC_TASK_FUNC_RADIO4:
		case IDC_TASK_FUNC_RADIO5:
		case IDC_TASK_FUNC_RADIO6:
			inf.panel_func_id = LOWORD(wp); set_panel_tip_txt(); set_PNLparam_value(0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
			reset_panel_item_pb(hDlg);
			break;

		case IDC_TASK_ITEM_RADIO1:
		{
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				setup_chart(MK_CHART1);	//プロットデータのポインタをライブラリのバッファにセット
				CMKChart::open_chart(MK_CHART1, inf.hWnd_opepane); 
			}
			if (IsDlgButtonChecked(hDlg, IDC_TASK_ITEM_RADIO1) == BST_CHECKED) {
				SendMessage(GetDlgItem(hDlg, IDC_TASK_ITEM_RADIO1), BM_SETCHECK, BST_UNCHECKED, 0L);
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;
		}
		case IDC_TASK_ITEM_RADIO2:
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				setup_chart(MK_CHART2);	//プロットデータのポインタをライブラリのバッファにセット
				CMKChart::open_chart(MK_CHART2, inf.hWnd_opepane);
			}
			if (IsDlgButtonChecked(hDlg, IDC_TASK_ITEM_RADIO2) == BST_CHECKED) {
				SendMessage(GetDlgItem(hDlg, IDC_TASK_ITEM_RADIO2), BM_SETCHECK, BST_UNCHECKED, 0L);
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;
		case IDC_TASK_ITEM_RADIO3:
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				chart_plot_ptn = SCAD_CHART_PTN0;
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;
		case IDC_TASK_ITEM_RADIO4:
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				chart_plot_ptn = SCAD_CHART_PTN1;
				set_chart_data(SCAD_CHART_PTN1);
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;
		case IDC_TASK_ITEM_RADIO5:
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				chart_plot_ptn = SCAD_CHART_PTN2;
				set_chart_data(SCAD_CHART_PTN2);
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
			break;
		case IDC_TASK_ITEM_RADIO6:
			if (inf.panel_func_id == IDC_TASK_FUNC_RADIO1) {
				chart_plot_ptn = SCAD_CHART_PTN3;
				set_chart_data(SCAD_CHART_PTN3);
			}
			inf.panel_type_id = LOWORD(wp);set_panel_tip_txt();  SetFocus(GetDlgItem(inf.hWnd_opepane, IDC_TASK_EDIT1));
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
void CSCADA::set_panel_tip_txt()
{
	wstring wstr_type; wstring wstr;
	switch (inf.panel_func_id) {
	case IDC_TASK_FUNC_RADIO1: {
		wstr = L"Type for CHART \n\r 1:OPEN Time 2:OPEN Scat 3:NO PLOT \n\r 4:PLOT1 5:PLOT2 6:PLOT3";
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
		wstr = L"Func6 \n\r 1:?? 2:?? 3:?? 4:?? 5:?? 6:??";
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
void CSCADA::set_panel_pb_txt() {

	WCHAR str_func01[] = L"CHART";
	SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO1, (LPCWSTR)str_func01);

	return;
};

//****************************************************************************
//　　チャートモジュールにプロットデータのバッファポインタをセット
//	引数：
//		int iwnd	：開いているチャートウィンドウのID (0,1)
//		int ichart	：ウィンドウ内のチャートのID(0,1)
//		int igraph	：チャート内のグラフ要素ID(0-3)
//		int ibool	：グラフ要素がON/OFFの時の要素ID(0-3)
//		boolis_x	：チャートのタイプが散布図の時、x軸用はtrueをセット
//		int type	：データのタイプ(double,int,bool)
//****************************************************************************
int CSCADA::set_graph_item(int iwnd, int ichart, int igraph, int ibool, bool is_x, int type) {

	if (type == MK_DATA_TYPE_DOUBLE) {
		if (is_x) {
			CMKChart::set_double_data(&chart_plot_buf[iwnd][SCAD_X_AXIS].pd[ichart][igraph], iwnd, ichart, igraph,
				&chart_plot_buf[iwnd][SCAD_X_AXIS].pd100[ichart][igraph], true);
		}
		else {
			CMKChart::set_double_data(&chart_plot_buf[iwnd][SCAD_Y_AXIS].pd[ichart][igraph], iwnd, ichart, igraph,
				&chart_plot_buf[iwnd][SCAD_Y_AXIS].pd100[ichart][igraph], false);
		}
		return MK_DATA_TYPE_DOUBLE;
	}
	else if (type == MK_DATA_TYPE_INT) {
		if (is_x) {
			CMKChart::set_int_data(&chart_plot_buf[iwnd][SCAD_X_AXIS].pi[ichart][igraph], iwnd, ichart, igraph,
				&chart_plot_buf[iwnd][SCAD_X_AXIS].pi100[ichart][igraph], true);
		}
		else {
			CMKChart::set_int_data(&chart_plot_buf[iwnd][SCAD_Y_AXIS].pi[ichart][igraph], iwnd, ichart, igraph,
				&chart_plot_buf[iwnd][SCAD_Y_AXIS].pi100[ichart][igraph], false);
		}
		return MK_DATA_TYPE_INT;
	}
	else if (type == MK_DATA_TYPE_BOOL) {
		if (is_x) {
			CMKChart::set_bool_data(&chart_plot_buf[iwnd][SCAD_X_AXIS].pb[ichart][igraph][ibool], iwnd, ichart, igraph, ibool, 
				&chart_plot_buf[iwnd][SCAD_X_AXIS].pb100[ichart][igraph][ibool]);

		}
		else {
			CMKChart::set_bool_data(&chart_plot_buf[iwnd][SCAD_Y_AXIS].pb[ichart][igraph][ibool], iwnd, ichart, igraph,ibool,
				&chart_plot_buf[iwnd][SCAD_Y_AXIS].pb100[ichart][igraph][ibool]);
		}
		return MK_DATA_TYPE_BOOL;
	}
	else return -1;
}

//****************************************************************************
//　　チャートモジュールにプロットデータのバッファポインタをセット
//****************************************************************************
int CSCADA::setup_chart(int iwnd) {

	for (int ichart = 0;ichart < SCAD_N_CHART_PER_WND;ichart++)
		for (int igraph = 0;igraph < SCAD_N_GRAPH_PAR_CHART;igraph++)
			for (int ibool = 0;ibool < SCAD_N_BOOL_PAR_GRAPH;ibool++)
			{
				set_graph_item(iwnd, ichart, igraph, ibool, true, MK_DATA_TYPE_DOUBLE);
				set_graph_item(iwnd, ichart, igraph, ibool, false, MK_DATA_TYPE_DOUBLE);
				set_graph_item(iwnd, ichart, igraph, ibool, true, MK_DATA_TYPE_INT);
				set_graph_item(iwnd, ichart, igraph, ibool, false, MK_DATA_TYPE_INT);
				set_graph_item(iwnd, ichart, igraph, ibool, true, MK_DATA_TYPE_BOOL);
				set_graph_item(iwnd, ichart, igraph, ibool, false, MK_DATA_TYPE_BOOL);
			}

	return 0;
}
//****************************************************************************
//　　プロットデータをバッファにセット
//****************************************************************************
int CSCADA::set_chart_data(int iptn) {

	//現在の設定をクリア
//	CMKChart::clear_set_data();
	
	if (iptn == SCAD_CHART_PTN1) {
	//Window1 Time Chart
	//Chart1

		//Graph1
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH1, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH1, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH1] = &(pAgentInf->v_ref[ID_HOIST]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH1] = pCraneStat->spec.notch_spd_f[ID_HOIST][NOTCH_5];
		//Graph2
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH2, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH2, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = &(pPLC_IO->status.v_fb[ID_HOIST]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = pCraneStat->spec.notch_spd_f[ID_HOIST][NOTCH_5];
		//Graph3
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH3, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH3, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH3] = &(pPLC_IO->status.v_fb[ID_BOOM_H]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH3] = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];


	//Chart2
		//Graph1
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH1, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH1, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = &(pPLC_IO->status.v_fb[ID_BOOM_H]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];

		//Graph2
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH2, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH2, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH2] = &(pAgentInf->v_ref[ID_BOOM_H]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH2] = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];

		//Graph3
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH3, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH3, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH3] = &(pPLC_IO->status.v_fb[ID_SLEW]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH3] = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];

		//Graph4
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH4, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH4, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH4] = &(pAgentInf->v_ref[ID_SLEW]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH4] = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];


	//Window2 Scatter
		//Chart1
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART1, SCAD_CHART_GRAPH4, true);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH4] = &(pCraneStat->sw_stat.sw[ID_BOOM_H].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH4] = PI15;

		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART1, SCAD_CHART_GRAPH4, false);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH4] = &(pCraneStat->sw_stat.sw[ID_SLEW].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH4] = PI15;


		//Chart2
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART1, SCAD_CHART_GRAPH2, true);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = &(pCraneStat->sw_stat.sw[ID_SLEW].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = PI15;

		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART1, SCAD_CHART_GRAPH2, false);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = &(pCraneStat->sw_stat.sw[ID_BOOM_H].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH2] = PI15;

	}
	else if (iptn == SCAD_CHART_PTN2) {
	//Window1 Time Chart
	//Chart1
		//Graph1
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH1, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART1, SCAD_CHART_GRAPH1, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART1][SCAD_CHART_GRAPH1] = &(pAgentInf->v_ref[ID_HOIST]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART1][SCAD_CHART_GRAPH1] = pCraneStat->spec.notch_spd_f[ID_HOIST][NOTCH_5];

	//Chart2	
		//Graph2
		CMKChart::set_data_type(MK_DATA_TYPE_TIME, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH2, true);
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND1, SCAD_CHART_CHART2, SCAD_CHART_GRAPH2, false);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH2] = &(pAgentInf->v_ref[ID_BOOM_H]);
		chart_plot_buf[SCAD_CHART_WND1][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH2] = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];

	//Window2 Scatter
	//Chart1
	//Chart2
		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART2, SCAD_CHART_GRAPH1, true);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = &(pCraneStat->sw_stat.sw[ID_SLEW].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_X_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = PI15;

		CMKChart::set_data_type(MK_DATA_TYPE_DOUBLE, SCAD_CHART_WND2, SCAD_CHART_CHART2, SCAD_CHART_GRAPH1, false);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].pd[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = &(pCraneStat->sw_stat.sw[ID_BOOM_H].th);
		chart_plot_buf[SCAD_CHART_WND2][SCAD_Y_AXIS].d100[SCAD_CHART_CHART2][SCAD_CHART_GRAPH1] = PI15;
	}
	else if (iptn == SCAD_CHART_PTN3) {
		;
	}
	else;

	return 0;
}