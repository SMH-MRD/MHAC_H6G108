#include "CPolicy.h"
#include "CAgent.h"

//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem*  pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
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
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/

int (CPolicy::*pfunc_set_recipe[N_AS_PTN])(LPST_MOTION_RECIPE, int);

static CAgent* pAgent;

void CPolicy::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
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
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
void CPolicy::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//定周期処理手順1　外部信号入力
void CPolicy::input() {

	
	
	
	
	return;

};

//定周期処理手順2　メイン処理
void CPolicy::main_proc() {
	
	return;

}

//定周期処理手順3　信号出力処理
void CPolicy::output() {
	
	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

int CPolicy::set_pp_th0(int motion) {

	return 0;
}


/****************************************************************************/
/*　　COMMAND 処理															*/
/****************************************************************************/
// Command バッファのインデックス更新
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

// Command生成更新
LPST_COMMAND_SET CPolicy::generate_command(int type, double* ptarget_pos) {

	LPST_COMMAND_SET lp_com = next_command(type); 

	//パターン計算ベースデータの設定
	set_pattern_cal_base(type, ID_BOOM_H);		
	set_pattern_cal_base(type, ID_SLEW);

	//振れ止めパターンの判定
	pPolicyInf->auto_ctrl_ptn[ID_BOOM_H] = judge_auto_ctrl_ptn(type, ID_BOOM_H);
	pPolicyInf->auto_ctrl_ptn[ID_SLEW] = judge_auto_ctrl_ptn(type, ID_SLEW);

	//パターンの計算
	if(AS_PTN_NG == set_recipe(lp_com, ID_BOOM_H, pPolicyInf->auto_ctrl_ptn[ID_BOOM_H]))
		return NULL;
	if(AS_PTN_NG == set_recipe(lp_com, ID_SLEW  , pPolicyInf->auto_ctrl_ptn[ID_SLEW]  ))
		return NULL;

	if (lp_com != NULL) {		//Command Set OKで　AGENTの目標位置更新
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
/*　　パターン計算用の素材データ計算,セット									*/
/****************************************************************************/
int CPolicy::set_pattern_cal_base(int auto_type, int motion) {

	//目標位置設定
	if (auto_type == POLICY_TYPE_SEMI) {
		st_work.pos_target[motion] = pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][motion];
	}
	else if (auto_type == POLICY_TYPE_JOB) {
		st_work.pos_target[motion] = pPLC_IO->status.pos[motion];
	}
	else {
		st_work.pos_target[motion] = pPLC_IO->status.pos[motion] + pAgentInf->dist_for_stop[motion];
	}
	
	//目標位置までの距離設定
	st_work.dist_for_target[motion] = pPLC_IO->status.pos[motion] - st_work.pos_target[motion];
	if (st_work.dist_for_target[motion] < 0.0) st_work.dist_for_target[motion] *= -1.0;

	//最大速度
	st_work.vmax[motion] = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];

	//加速度
	st_work.a[motion] = pCraneStat->spec.accdec[motion][FWD][ACC];

	//加速時間
	st_work.acc_time2Vmax[motion] = st_work.vmax[motion] / st_work.a[motion];

	//加速時振れ中心
	st_work.pp_th0[motion][ACC] = pCraneStat->spec.accdec[motion][FWD][ACC] / pCraneStat->w2;
	st_work.pp_th0[motion][DEC] = pCraneStat->spec.accdec[motion][FWD][DEC] / pCraneStat->w2;
	if (motion == ID_SLEW) { //旋回の加速度はRθで計算
		double R = pPLC_IO->status.pos[ID_BOOM_H];
		st_work.pp_th0[motion][ACC] *= R;
		st_work.pp_th0[motion][DEC] *= R;
	}

	//振れ振幅評価値
	st_work.r[motion] = sqrt(pSway_IO->amp2[motion]);

	if (st_work.r[motion] > st_work.pp_th0[motion][ACC]) st_work.is_sway_over_r0[motion] = true;
	else st_work.is_sway_over_r0[motion] = false;

	//現在位置、速度
	st_work.pos[motion] = pPLC_IO->status.pos[motion];
	st_work.v[motion] = pPLC_IO->status.v_fb[motion];

	//AGENTスキャンタイム
	st_work.agent_scan_ms = pAgent->inf.cycle_ms;

	return 0;
}


/****************************************************************************/
/*　　1STEP,2STEP振れ止めパターンのゲイン（加速時間(角度）計算				*/
/****************************************************************************/
void CPolicy::set_as_gain(int motion, int as_type) {

	double a,r,w,l,r0, vmax, max_th_of_as, acc_time2Vmax;

	//最大速度による加速時間制限
	r = st_work.r[motion];	//振幅評価値
	r0 = st_work.pp_th0[motion][ACC];	//加速時振中心
	w = pCraneStat->w;
	a = st_work.a[motion];
	vmax = st_work.vmax[motion];
	acc_time2Vmax = st_work.acc_time2Vmax[motion];
	l = pCraneStat->mh_l;

	if (as_type == AS_PTN_1STEP) {	// 1STEP
		max_th_of_as = r0 * 4.0; //1 STEPのロジックで制御可能な振れ振幅限界値　4r0
		//ゲイン計算用R0を設定（上限リミット）
		if (r > max_th_of_as) r = max_th_of_as;
		if (r > acc_time2Vmax * w) r = acc_time2Vmax * w; //ωtmax

		st_work.as_gain_phase[motion] = acos(1 - 0.5 * r / r0);
		st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		//最大速度による加速時間制限
		if (st_work.as_gain_time[motion] > acc_time2Vmax) {
			st_work.as_gain_time[motion] = acc_time2Vmax;
			st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
		}
	}
	else if (as_type == AS_PTN_2PN) { //2STEP round type
		
		if (r < r0) {//振れ振幅が加速振れ内側
			//振れ振幅に応じてゲインを設定（２回目で減衰させる。1回目は振れ維持。開始位相で調整
			st_work.as_gain_phase[motion] = acos(0.6*r/(r+r0));
			st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		}
		else {//振れ振幅が加速振れ外側
			//2回目のインチングで振れ止め（待ち時間調整タイプ）用
			st_work.as_gain_phase[motion] = acos(r / (r + r0));
			st_work.as_gain_time[motion] = st_work.as_gain_phase[motion] / w;
		}
		//最大速度による加速時間制限
		if (st_work.as_gain_time[motion] > acc_time2Vmax) {
			st_work.as_gain_time[motion] = acc_time2Vmax;
			st_work.as_gain_phase[motion] = st_work.as_gain_time[motion] * w;
		}
	}
	else if (as_type == AS_PTN_2PP) { //2STEP one way

		double dist_for_target = st_work.pos_target[motion] - pPLC_IO->status.pos[motion];
		if (dist_for_target < 0.0) dist_for_target *= -1.0;
	
		//1回のインチング移動距離　d = a*t^2　移動距離S=2dよりt=√(S/2a)
		st_work.as_gain_time[motion] = sqrt(0.5 * dist_for_target /a);

		//最大速度による加速時間制限
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
/*　　自動（振れ止め）の移動パターンタイプの判定							*/
/****************************************************************************/
int CPolicy::judge_auto_ctrl_ptn(int auto_type, int motion) {
	
	int ptn = AS_PTN_0;

	set_pp_th0(motion);																//位相平面の回転中心計算

	double T = pCraneStat->T;
	double R = pPLC_IO->status.pos[ID_BOOM_H];
	double vmax = st_work.vmax[motion];
	double a = pCraneStat->spec.accdec[motion][FWD][ACC];
	double d_from_target = st_work.dist_for_target[motion];

	double d_min_ptn2ad = vmax * (T + vmax / a);

	if (sqrt(pSway_IO->amp2[motion]) > st_work.pp_th0[motion][ACC])					//振れ振幅 >　加速時の振れ中心
		ptn = AS_PTN_1STEP;
	else if (d_from_target > d_min_ptn2ad)											//目標位置までの距離 > 2Stepパターンの最小距離
		ptn = AS_PTN_2ACCDEC; 
	else if(d_from_target < pCraneStat->spec.as_pos_level[motion][ID_LV_TRIGGER])	//目標位置までの距離 < 位置決めトリガ距離
		ptn = AS_PTN_2PN;
	else if (d_from_target < 2.0*vmax*vmax/a)										//目標位置までの距離 > 2回の最大速度インチング距離		
		ptn = AS_PTN_2PP;
	else 
		ptn = AS_PTN_1ACCDEC;														//いずれでもない時は台形パターン

	return ptn; 
}

/****************************************************************************/
/*　　移動パターンタイプの計算,セット										*/
/****************************************************************************/
int CPolicy::set_recipe(LPST_COMMAND_SET pcom, int motion, int ptn) {
	//動作初期化
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
/*　　振れ止めパターン　１stepタイプの計算,セット							*/
/****************************************************************************/
int CPolicy::set_recipe1step(LPST_MOTION_RECIPE precipe, int motion) { 
	
	LPST_MOTION_ELEMENT pelement;

	st_work.motion_dir[motion] = POLICY_UNKNOWN;										//移動方向不定(移動方向が実行時に決まる）
	set_as_gain(motion, AS_PTN_1STEP);													//振れ止めゲイン計算
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_1STEP;
		
	//Step 1　位相待ち
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT 早く到達する方を待つ
		pelement->_p = st_work.pos[motion];												// 目標位置(開始時位置）
		pelement->_t = 2.0*st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;
		pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion];			// プラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion] - PI180;	// マイナス加速用位相

	}
	//Step 2　加速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.a[motion] * pelement->_t;								// 目標変化速度
	}
	//Step 3　減速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為			
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.v[motion];												// 開始時速度
	}
	//Step 4　動作停止待機
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = st_work.v[motion];												// 開始時速度 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->motion[i].time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->motion[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_1STEP;					//パターン出力時判定用
	}

	return AS_PTN_OK; 
};

/****************************************************************************/
/*　　振れ止めパターン　2stepその場振れ止めセット							*/
/****************************************************************************/
int CPolicy::set_recipe2pn(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_ELEMENT pelement;

	st_work.motion_dir[motion] = POLICY_UNKNOWN;										//移動方向不定(移動方向が実行時に決まる）
	set_as_gain(motion, AS_PTN_2PN);													//振れ止めゲイン計算
	precipe->n_step = 0;																//ステップ数
	precipe->motion_type = AS_PTN_2PN;

	//Step 1　位相待ち(振れ振幅維持）
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT 早く到達する方を待つ
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）
		pelement->_t = 2.0 * st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;

		//起動位相
		double temp_d;
		if (st_work.r[motion] < st_work.r0[motion])										//初期振れが加速振れより小（予定）
			temp_d = st_work.r[motion] / st_work.r0[motion] * (cos(st_work.as_gain_phase[motion]) - 1);
		else																			//初期振れが加速振れより小（予定外）
			temp_d = cos(st_work.as_gain_phase[motion]) - 1;

		double start_phase = acos(temp_d) + st_work.as_gain_phase[motion];

		pelement->opt_d[MOTHION_OPT_PHASE_F] = start_phase;								//プラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = -PI180 + start_phase;					//マイナス加速用位相
	}
	//Step 2　加速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.a[motion] * pelement->_t;								// 目標変化速度
	}
	//Step 3　減速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為			
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.v[motion];												// 開始時速度
	}
	//Step 4　位相待ち(振れ振幅減衰）
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 開始時と反対方向の位相を待つ
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為
		pelement->_t = 2.0 * st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;

		pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;										// プラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;									// マイナス加速用位相
	}
	//Step 5　加速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.a[motion] * precipe->motion[1]._t;						// 目標変化速度
	}
	//Step 6　減速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_p = st_work.pos_target[motion];										// 開始時位置			
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.v[motion];												// 開始時速度
	}
	//Step 7　動作停止待機
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 開始時位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->motion[i].time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->motion[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PN;						//パターン出力時判定用
	}

	return AS_PTN_OK;

};

/****************************************************************************/
/*　　振れ止めパターン　2step小移動										*/
/****************************************************************************/
int CPolicy::set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_ELEMENT pelement;
	double dir;

	//移動方向セット
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

	set_as_gain(motion, AS_PTN_2PP);													//振れ止めゲイン計算
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_2PP;

	//Step 1　位相待ち(振れ振幅維持）
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;									// 位相待ち
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 2.0 * st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;

		//起動位相
		double temp_d;
		if (st_work.r[motion] < st_work.r0[motion])										// 初期振れが加速振れより小（予定）
			temp_d = st_work.r[motion] / st_work.r0[motion] * (cos(st_work.as_gain_phase[motion]) - 1);
		else																			// 初期振れが加速振れより小（予定外）
			temp_d = cos(st_work.as_gain_phase[motion]) - 1;

		double start_phase = acos(temp_d) + st_work.as_gain_phase[motion];

		pelement->opt_d[MOTHION_OPT_PHASE_F] = start_phase;								// プラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = -PI180 + start_phase;					// マイナス加速用位相
	}
	//Step 2　加速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// 目標変化速度
		pelement->_p = (pelement-1)->_p + 0.5 * pelement->_v * pelement->_t;			// 目標位置
	}
	//Step 3　減速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement-1)->_v * pelement->_t;		// 目標位置
	}
	//Step 4　位相待ち
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DOUBLE_PHASE_WAIT;									// 2POINT 早く到達する方を待つ
		pelement->_p = (pelement - 1)->_p;												// 目標位置(現在位置がセットされる）
		pelement->_t = 2.0 * st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;

		pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;										//phase1 プラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;									//phase2 マイナス加速用位相
	}
	//Step 5　加速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// 目標変化速度
		pelement->_p = (pelement - 1)->_p + 0.5 * pelement->_v * pelement->_t;			// 目標位置
	}
	//Step 6　減速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement - 1)->_v * pelement->_t;	// 目標位置
	}
	//Step 7　動作停止待機
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PP;			//パターン出力時判定用
	}

	return AS_PTN_OK;
};

/****************************************************************************/
/*　　振れ止めパターン　2段加減速											*/
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
	if (ta > T) tc_half += T;										//加速時間が振れ周期よりも長いときは＋T
	double dmin = v_top * ta + 2.0 * tc_half * v_half;
	double d = st_work.pos[motion] - st_work.pos_target[motion];	//移動距離
	if (d < 0.0) d *= -1.0;
	double initial_sway = st_work.r[motion];

	double t_expected = 2.0 * (ta + tc_half);	//予定移動時間
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//移動方向セット
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

	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1　他軸位置待ち
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 0.0;																// 予定時間
		pelement->_v = 0.0;

		//待ち位置(目標までの距離）計算
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
				if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				else							//所要時間が相手軸の加速時間より長い
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる

		}
		else if (motion == ID_SLEW) {
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;
			d_max_other = pCraneStat->spec.boom_pos_max;
			if (is_slew_first) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる
			}
			else {
				if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				else							//所要時間が相手軸の加速時間より長い
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

		//起動位相（初期振れが大きいときに加速を開始する位相）

	}

	//Step 2　加速中定速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// 加速中定速出力
		pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
		pelement->_v = dir * v_half;													// ハーフ速度
		pelement->_p = (pelement - 1)->_p + dir * v_half * ( 0.5 * ta_half + tc_half);	// 目標位置
	}
	//Step 3　定速TOP速度
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP速度定速出力
		pelement->_t = ta_half + (d - dmin) / v_top;									// トップ速度出力時間
		pelement->_v = dir * v_top;														// トップ速度
		pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top *  ta + v_half * tc_half);	// 目標位置
	}
	//Step 4　減速中定速
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// 加速中定速出力
		pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
		pelement->_v = dir * v_half;													// ハーフ速度
		pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// 目標位置
	}

	//Step 5　停止
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = ta_half;															// 減速時間
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = st_work.pos_target[motion];										// 目標位置
	}
	//Step 6　動作停止待機
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//パターン出力時判定用
	}
	return AS_PTN_OK;
};
/****************************************************************************/
/*　　振れ止めパターン　台形												*/
/****************************************************************************/
int CPolicy::set_recipe1ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_ELEMENT pelement;
	double T = pCraneStat->T;
	double a = st_work.a[motion];
	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5];
	double ta = v_top / a;
	double dmin = v_top * ta;
	double d = st_work.dist_for_target[motion];	//移動距離
	double initial_sway = st_work.r[motion];
	double t_expected = 2.0 * ta ;	//予定移動時間
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;
	double dir;
	//移動方向セット
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

	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_1ACCDEC;

	//Step 1　他軸位置待ち
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 0.0;																// 予定時間
		pelement->_v = 0.0;

		//待ち位置(目標までの距離）計算
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
			if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			else							//所要時間が相手軸の加速時間より長い
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
		}
		else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる

	}
	else if (motion == ID_SLEW) {
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
		ta_other = v_other / a_other;
		d_max_other = pCraneStat->spec.boom_pos_max;
		if (is_slew_first) {
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる
		}
		else {
			if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			else							//所要時間が相手軸の加速時間より長い
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

	//起動位相（初期振れが大きいときに加速を開始する位相）

	}

	//Step 2　定速TOP速度
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP速度定速出力
		pelement->_t = ta + (d - dmin) / v_top;											// トップ速度出力時間
		pelement->_v = dir * v_top;														// トップ速度
		pelement->_p = st_work.pos_target[motion] - dir * 0.5 * ta * v_top;				// 目標位置
	}
	//Step 3　停止
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = ta;																// 減速時間
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = st_work.pos_target[motion];										// 目標位置
	}
	//Step 4　動作停止待機
	{	pelement = &(precipe->motion[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
	pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		pelement->time_count = (int)(precipe->motion[i]._t * 1000) / (int)st_work.agent_scan_ms;
		pelement->opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//パターン出力時判定用
	}
	return AS_PTN_OK;
}

/****************************************************************************/
/*　　振れ止めパターン　3STEP												*/
/****************************************************************************/
int CPolicy::set_recipe3step(LPST_MOTION_RECIPE precipe, int motion) { return AS_PTN_NG; };


/****************************************************************************/
/*   タスク設定タブパネルウィンドウのコールバック関数                       */
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
/*　　タスク設定パネルボタンのテキストセット					            */
/****************************************************************************/
void CPolicy::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};


