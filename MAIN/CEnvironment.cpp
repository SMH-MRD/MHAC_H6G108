#include "CEnvironment.h"

//-共有メモリオブジェクトポインタ:
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
/*   コンストラクタ　デストラクタ                                           */
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
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/
void CEnvironment::init_task(void* pobj) {

	//共有クレーンステータス構造体のポインタセット
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pRemoteIO = (LPST_REMOTE_IO)(pRemoteIO_Obj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());
	pSimStat = (LPST_SIMULATION_STATUS)(pSimulationStatusObj->get_pMap());
	
	//クレーン仕様セット
	stWorkCraneStat.spec = this->spec;

	//振れセンサカメラ仕様セット
	//  旋回方向
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
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
void CEnvironment::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//定周期処理手順1　外部信号入力
void CEnvironment::input(){
	

	return;

};

//定周期処理手順2　メイン処理

void CEnvironment::main_proc() {

	//メインウィンドウのTweetメッセージ更新
	tweet_update();

	//ヘルシーカウンタセット
	stWorkCraneStat.env_act_count = inf.total_act;

	//サブプロセスチェック
	chk_subproc();

	//モードセット
	mode_set();


	//ノッチ指令状態セット
	parse_notch_com();

	//位置情報セット
	pos_set();
	
	//振れ情報セット
	parse_sway_stat(ID_SLEW);
	parse_sway_stat(ID_BOOM_H);

	return;
}

//定周期処理手順3　信号出力処理

void CEnvironment::output() {

	

	//共有メモリ出力
	memcpy_s(pCraneStat, sizeof(ST_CRANE_STATUS), &stWorkCraneStat, sizeof(ST_CRANE_STATUS));
	
	return;

}; 

/****************************************************************************/
/*　　ノッチ入力信号を速度指令に変換して取り込み				            */
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
/*　 制御用振れ状態計算											            */
/****************************************************************************/
int CEnvironment::parse_sway_stat(int ID) {

	double th, dth, L, dph, ddph, dthw;

	double D = stWorkCraneStat.sw_stat.cam[ID].D0;
	double  l0 = stWorkCraneStat.sw_stat.cam[ID].l0;
	double  H = stWorkCraneStat.sw_stat.cam[ID].H0 + l0;
	double  ph0 = stWorkCraneStat.sw_stat.cam[ID].ph0;
	
	stWorkCraneStat.mh_l = L = pCraneStat->rc.z - pCraneStat->rl.z;//ロープ長
	
	//角周波数
	if (stWorkCraneStat.mh_l>1.0) {	//ロープ長下限
		stWorkCraneStat.sw_stat.sw[ID].w = sqrt(GA / stWorkCraneStat.mh_l);
	}
	else {
		stWorkCraneStat.sw_stat.sw[ID].w = 3.13;//1mロープ長時の角周波数
	}

	//周期
	stWorkCraneStat.sw_stat.sw[ID].T = PI360 / stWorkCraneStat.sw_stat.sw[ID].w;
	
	//振角　振角速度　振幅　位相　
	//    カメラ位置からの振れ角＝カメラ検出角＋取付オフセット  
	th = pSway_IO->rad[ID] + ph0;
	//    カメラ位置と吊点からの振れ角差 	
	dph = asin((D * cos(th) - H * sin(th)) / L);
	//    吊点からの振れ角 	
	stWorkCraneStat.sw_stat.sw[ID].th = th + dph;

	//    カメラ位置からの振れ角速度 	
	dth = pSway_IO->w[ID] ;
	//    カメラ位置と吊点からの振れ角速度差
	ddph = 1.0 - (D * sin(th) + H * cos(th)) / (L * cos(dph));
	//    吊点からの振れ角速度 	
	stWorkCraneStat.sw_stat.sw[ID].dth = dth * ddph;

	//    吊点からの振れ角速度/ω　位相平面Y軸値 
	dthw = stWorkCraneStat.sw_stat.sw[ID].dth / stWorkCraneStat.sw_stat.sw[ID].w;

	//    吊点からの振れ角振幅
	stWorkCraneStat.sw_stat.sw[ID].amp2 = stWorkCraneStat.sw_stat.sw[ID].th * stWorkCraneStat.sw_stat.sw[ID].th
											+ dthw * dthw;
	//    位相平面上の位相　0割回避
	if (abs(stWorkCraneStat.sw_stat.sw[ID].th) < 0.000001) {
		if(dthw < 0.0)	stWorkCraneStat.sw_stat.sw[ID].ph = -PI90;
		else 	stWorkCraneStat.sw_stat.sw[ID].ph = PI90;
	}
	else {
		stWorkCraneStat.sw_stat.sw[ID].ph = dthw / stWorkCraneStat.sw_stat.sw[ID].th;
	}
	// atanは、-π/2～π/2の表現　→　-π～πで表現する 
	if (stWorkCraneStat.sw_stat.sw[ID].th < 0.0) {
		if (dthw < 0.0) stWorkCraneStat.sw_stat.sw[ID].ph -= PI180;
		else stWorkCraneStat.sw_stat.sw[ID].ph += PI180;
	}
		
	return 0;
}
/****************************************************************************/
/*　 制御用振れ状態計算											            */
/****************************************************************************/
int CEnvironment::mode_set() {
	//リモートモードセット
	if (pPLC_IO->ui.PB[ID_PB_REMOTE_MODE])stWorkCraneStat.operation_mode |= OPERATION_MODE_REMOTE;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_REMOTE;

	//シミュレータモードセット
	if (pSimStat->mode & SIM_ACTIVE_MODE)stWorkCraneStat.operation_mode |= OPERATION_MODE_SIMULATOR;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_SIMULATOR;

	//PLCデバッグモードセット
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE)stWorkCraneStat.operation_mode |= OPERATION_MODE_PLC_DBGIO;
	else stWorkCraneStat.operation_mode &= ~OPERATION_MODE_PLC_DBGIO;

	return 0;

}
/****************************************************************************/
/*　 位置情報セット											            */
/****************************************************************************/
int CEnvironment::pos_set() {
	//クレーン基準点のx,y,z相対座標
	stWorkCraneStat.rc0.x = pPLC_IO->status.pos[ID_GANTRY];	//走行位置
	stWorkCraneStat.rc0.y = 0.0;							//旋回中心点
	stWorkCraneStat.rc0.z = 0.0;							//走行レール高さ

	//クレーン吊点のクレーン基準点とのx,y,z相対座標
	stWorkCraneStat.rc.x = pPLC_IO->status.pos[ID_BOOM_H] * cos(pPLC_IO->status.pos[ID_SLEW]);
	stWorkCraneStat.rc.y = pPLC_IO->status.pos[ID_BOOM_H] * sin(pPLC_IO->status.pos[ID_SLEW]);
	stWorkCraneStat.rc.z = spec.boom_high;

	//吊荷のクレーン基準点とのx, y, z相対座標
	stWorkCraneStat.rl.x = pCraneStat->rc.x;
	stWorkCraneStat.rl.y = pCraneStat->rc.y;
	stWorkCraneStat.rl.z = pPLC_IO->status.pos[ID_HOIST];

	//極限判定
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
/*　　サブプロセスの状態確認			            */
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
/*　　メインウィンドウのTweetメッセージ更新          			            */
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
/*   タスク設定タブパネルウィンドウのコールバック関数                       */
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
/*　　タスク設定パネルボタンのテキストセット					            */
/****************************************************************************/
void CEnvironment::set_panel_pb_txt() {

//	WCHAR str_func06[] = L"DEBUG";

//	SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};