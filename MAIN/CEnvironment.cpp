#include "CEnvironment.h"

//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pSimulationStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
/****************************************************************************/
CEnvironment::CEnvironment() {
	pCraneStat = NULL;
	pPLC_IO = NULL;
	pSway_IO = NULL;
	pRemoteIO = NULL;
	pSimStat = NULL;
	pCSInf = NULL;
	pPolicyInf = NULL;
	pAgentInf = NULL;
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
	pCSInf=(LPST_CS_INFO)(pCSInfObj->get_pMap());
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	
	//クレーン仕様セット
	stWorkCraneStat.spec = this->spec;
	stWorkCraneStat.is_tasks_standby_ok = false;


	set_panel_tip_txt();

	inf.is_init_complete = true;
	return;
};

/****************************************************************************/
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
bool CEnvironment::check_tasks_init() {

	CTaskObj* ptask;
	int n_tasks = (int)VectpCTaskObj.size();

	for (int i = 0;i < n_tasks ;i++) {
		ptask = (CTaskObj*)VectpCTaskObj[i];
		if(ptask->inf.is_init_complete == false) return false;
	}
	return true;

}

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

	//各タスクの初期化完了チェック
	if (pCraneStat->is_tasks_standby_ok == false) {
		stWorkCraneStat.is_tasks_standby_ok = check_tasks_init();
	}

	//メインウィンドウのTweetメッセージ更新
	tweet_update();

	//ヘルシーカウンタセット
	stWorkCraneStat.env_act_count = inf.total_act;

	//サブプロセスチェック
	chk_subproc();

	//システムのモードセット
	sys_mode_set();

	//ノッチ指令状態セット
	parse_notch_com();

	//位置情報セット
	pos_set();
	
	//自動用情報セット
	parse_for_auto_ctrl();

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

	//ノッチ位置配列のポインタセット
	int* p_notch;
	if (stWorkCraneStat.operation_mode & OPERATION_MODE_REMOTE) 
		p_notch = pRemoteIO->PLCui.notch_pos;
	else 
		p_notch = pPLC_IO->ui.notch_pos;

	for (int i = 0;i < MOTION_ID_MAX;i++) {
		if (*(p_notch+i) == NOTCH_0) {
			stWorkCraneStat.is_notch_0[i] = true;
			stWorkCraneStat.notch_spd_ref[i] = 0.0;
		}
		else {
			stWorkCraneStat.is_notch_0[i] = false;
			if (p_notch[i] < 0) 
				stWorkCraneStat.notch_spd_ref[i] = stWorkCraneStat.spec.notch_spd_r[i][-p_notch[i]] * 1.001;
			else 
				stWorkCraneStat.notch_spd_ref[i] = stWorkCraneStat.spec.notch_spd_f[i][p_notch[i]] * 1.001;
		}
	}

	return 0;

};
/****************************************************************************/
/*　 振れ周期関連　　											            */
/****************************************************************************/
double CEnvironment::cal_T(double pos_hst){   //振れ周期計算　巻き位置指定
	double ans = cal_w(pos_hst);
	ans = PI360 / ans;
	return ans;
};

double CEnvironment::cal_w(double pos_hst){  //振れ角周波数計算　巻き位置指定
	double ans = spec.boom_high - pos_hst;
	if (ans > 1.0) ans = GA / ans; 	//ロープ長下限より大
	else ans = GA;
	ans = sqrt(ans);
	return ans;
}; 

double CEnvironment::cal_w2(double pos_hst) {  //振れ角周波数の2乗計算　巻き位置指定
	double ans = spec.boom_high - pos_hst;
	if (ans > 1.0) ans = GA / ans; 	//ロープ長下限より大
	else ans = GA;

	return ans;
};

double CEnvironment::cal_l(double pos_hst){  //ロープ長計算　巻き位置指定
	double ans = spec.boom_high - pos_hst;
	return ans;
};

/****************************************************************************/
/*　 各種状態量計算											            */
/****************************************************************************/
double CEnvironment::get_vmax(int motion) {//最大速度
	return pCraneStat->spec.notch_spd_f[motion][NOTCH_5];
}
/****************************************************************************/
/*　システムモードセット										            */
/****************************************************************************/
int CEnvironment::sys_mode_set() {
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
/*　吊点の加減速度計算（※旋回はm/s)　オーバーロード（旋回半径現在値 or 指定）
*/
/****************************************************************************/

//旋回半径現在位置での計算
double CEnvironment::cal_hp_acc(int motion, int dir ) {

	double ans = spec.accdec[motion][dir][ACC];
	double r = stWorkCraneStat.R;

	switch (motion) {
	case ID_BOOM_H: {
		ans *= (0.0008 * r * r - 0.0626 * r + 1.9599);
	}break;
	case ID_SLEW: {
		ans *= r;
	}break;
	default:break;
	}

	return ans;      //吊点の加速度計算
}
double CEnvironment::cal_hp_dec(int motion, int dir) {

	double ans = spec.accdec[motion][dir][DEC];
	double r = stWorkCraneStat.R;

	switch (motion) {
	case ID_BOOM_H: {
		ans *= (0.0008 * r * r - 0.0626 * r + 1.9599);
	}break;
	case ID_SLEW: {
		ans *= r;
	}break;
	default:break;
	}

	return ans;      //吊点の減速度計算
}

//旋回半径を指定しての計算
double CEnvironment::cal_hp_acc(int motion, int dir,double R) {

	double ans = spec.accdec[motion][dir][ACC];
	double r = R;

	switch (motion) {
	case ID_BOOM_H: {
		ans *= (0.0008 * r * r - 0.0626 * r + 1.9599);
	}break;
	case ID_SLEW: {
		ans *= r;
	}break;
	default:break;
	}

	return ans;      //吊点の加速度計算
}
double CEnvironment::cal_hp_dec(int motion, int dir, double R) {

	double ans = spec.accdec[motion][dir][DEC];
	double r = R;

	switch (motion) {
	case ID_BOOM_H: {
		ans *= (0.0008 * r * r - 0.0626 * r + 1.9599);
	}break;
	case ID_SLEW: {
		ans *= r;
	}break;
	default:break;
	}

	return ans;      //吊点の減速度計算
}

/****************************************************************************/
/*　 加減速振れ角計算		オーバーロード（旋回半径現在値 or 指定） 　　   */
/****************************************************************************/

///旋回半径現在位置での加速振れ計算　a/g
double CEnvironment::cal_arad_acc(int motion, int dir) {     //加減速振れ振幅計算rad
	double ans = cal_hp_acc(motion, dir);
	ans /= GA;
	return ans;      //吊点の加速振れ計算

}
double CEnvironment::cal_arad_dec(int motion, int dir) {     //加減速振れ振幅計算rad
	double ans = cal_hp_dec(motion, dir);
	ans /= GA;
	return ans;      //吊点の加速振れ計算
}
double CEnvironment::cal_arad2(int motion, int dir) {     //加減速振れ振幅計算rad
	double ans = cal_hp_acc(motion, dir);
	ans /= GA;
	return (ans * ans); //吊点の加速振れ2乗計算

}


//旋回半径指定での加速振れ計算　a/g
double CEnvironment::cal_arad_acc(int motion, int dir, double R) {     //加減速振れ振幅計算rad
	double ans = cal_hp_acc(motion, dir, R);
	ans /= GA;
	return ans;      //吊点の加速振れ計算

}
double CEnvironment::cal_arad_dec(int motion, int dir, double R) {     //加減速振れ振幅計算rad
	double ans = cal_hp_dec(motion, dir, R);
	ans /= GA;
	return ans;      //吊点の加速振れ計算
}
double CEnvironment::cal_arad2(int motion, int dir, double R) {     //加減速振れ振幅計算rad
	double ans = cal_hp_acc(motion, dir, R);
	ans /= GA;
	return (ans * ans); //吊点の加速振れ2乗計算
}

bool CEnvironment::is_sway_larger_than_accsway(int motion){
	//振角振幅が加速振角よりも大きいか判定
	double rad_acc2 = cal_arad2(motion, FWD);	//加速振れ角2乗
	if (pSway_IO->rad_amp2[motion] > rad_acc2) return true;
	else return false;
}

double CEnvironment::cal_sway_r_amp2_m() { return pCraneStat->mh_l * pCraneStat->mh_l * sqrt(pSway_IO->rad_amp2[ID_BOOM_H]); }			//振れ振幅2乗半径方向 m
double CEnvironment::cal_sway_th_amp2_m() { return pCraneStat->mh_l * pCraneStat->mh_l * sqrt(pSway_IO->rad_amp2[ID_SLEW]); }			//振れ振幅2乗円周方向 m
double CEnvironment::cal_sway_x_amp2_m() { return 0.0; }																				//振れ振幅2乗x方向 m
double CEnvironment::cal_sway_y_amp2_m() { return 0.0; }																				//振れ振幅2乗y方向 m

bool CEnvironment::is_speed_0(int motion) { 

	if (pCraneStat->is_notch_0[motion] == false) return false;//ノッチ0で無い

	if ((pPLC_IO->status.v_fb[motion] >= pCraneStat->spec.notch_spd_f[motion][NOTCH_1] * SPD0_CHECK_RETIO) ||		// 0速チェック
		(pPLC_IO->status.v_fb[motion] <= pCraneStat->spec.notch_spd_r[motion][NOTCH_1] * SPD0_CHECK_RETIO)) {		//1ノッチの10％速度以上
		return false;	//0速でない
	}
	return true;
}

/****************************************************************************/
/*　 自動関連計算											         　　   */
/****************************************************************************/
//停止距離計算
double CEnvironment::cal_dist4stop(int motion, bool is_abs_answer) {
	
	int dir;
	double v = pPLC_IO->status.v_fb[motion];
	if (v < 0.0) dir = REV; else dir = FWD;
	double r = stWorkCraneStat.R;
	double dec = spec.accdec[motion][dir][DEC];


	if (motion == ID_BOOM_H) {
		dec *= (0.0008 * r * r - 0.0626 * r + 1.9599);
	}

	double dist = 0.5 * v * v / dec;

	if (is_abs_answer) {
		return dist;
	}
	else {
		if (dir == REV) return (-1.0 * dist);
		else return dist;
	}
}

//目標位置までの距離
double CEnvironment::cal_dist4target(int motion, bool is_abs_answer) {
	double dist=pPLC_IO->status.pos[motion] - pAgentInf->auto_pos_target.pos[motion];

	if (motion == ID_SLEW) {
		if (dist > PI180) dist -= PI360;
		else if (dist < -PI180) dist += PI360;
		else;
	}

	if ((is_abs_answer == true) && (dist < 0.0)) dist *= -1.0;
	return dist;
}

/****************************************************************************/
/*　 位置情報セット											            */
/****************************************************************************/
int CEnvironment::pos_set() {

	double sin_slew = sin(pPLC_IO->status.pos[ID_SLEW]);
	double cos_slew = cos(pPLC_IO->status.pos[ID_SLEW]);


	//クレーン吊点のクレーン基準点とのx,y,z相対座標
	stWorkCraneStat.rc.x = pPLC_IO->status.pos[ID_BOOM_H] * cos_slew;
	stWorkCraneStat.rc.y = pPLC_IO->status.pos[ID_BOOM_H] * sin_slew;
	stWorkCraneStat.rc.z = spec.boom_high;



	//旋回半径
	stWorkCraneStat.R = pPLC_IO->status.pos[ID_BOOM_H];
	
	//吊荷のカメラ座標での吊荷xyz相対座標　m
	stWorkCraneStat.rcam_m.x = stWorkCraneStat.mh_l * sin(pSway_IO->th[ID_SLEW]) ;
	stWorkCraneStat.rcam_m.y = stWorkCraneStat.mh_l * sin(pSway_IO->th[ID_BOOM_H]);
	stWorkCraneStat.rcam_m.z = -stWorkCraneStat.mh_l;

	//吊荷のx, y, z座標
	stWorkCraneStat.rl.x = pCraneStat->rc.x + stWorkCraneStat.rcam_m.x * sin_slew + stWorkCraneStat.rcam_m.y * cos_slew;
	stWorkCraneStat.rl.y = pCraneStat->rc.y + stWorkCraneStat.rcam_m.x * -cos_slew + stWorkCraneStat.rcam_m.y * sin_slew;
	stWorkCraneStat.rl.z = pPLC_IO->status.pos[ID_HOIST];

	//極限判定
	if (pPLC_IO->status.pos[ID_GANTRY] < spec.gantry_pos_min) stWorkCraneStat.is_rev_endstop[ID_GANTRY] = true;
	else stWorkCraneStat.is_rev_endstop[ID_GANTRY] = false;
	if (pPLC_IO->status.pos[ID_GANTRY] > spec.gantry_pos_max) stWorkCraneStat.is_fwd_endstop[ID_GANTRY] = true;
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
/*　 自動用情報セット											            */
/****************************************************************************/
int CEnvironment::parse_for_auto_ctrl() {
	//ロープ長
	stWorkCraneStat.mh_l = cal_l(pPLC_IO->status.pos[ID_HOIST]);
	//角周期
	stWorkCraneStat.w2 = cal_w2(pPLC_IO->status.pos[ID_HOIST]);
	stWorkCraneStat.w = sqrt(stWorkCraneStat.w2);
	//周期
	stWorkCraneStat.T = PI360 / stWorkCraneStat.w;
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

#if 0
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
#endif
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