#include "CPolicy.h"
#include "CAgent.h"
#include "CEnvironment.h"
#include "CClientService.h"

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

static CAgent* pAgent;
static CEnvironment* pEnvironment;
static CClientService* pCS;

void CPolicy::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
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
	//共有メモリ出力処理
	memcpy_s(pPolicyInf, sizeof(ST_POLICY_INFO), &PolicyInf_workbuf, sizeof(ST_POLICY_INFO));

	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;
};

/****************************************************************************/
/*　　COMMAND 処理															*/
/****************************************************************************/
// AGENTからのコマンド要求処理
LPST_COMMAND_BLOCK CPolicy::get_command() {

	if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status != REQ_ACTIVE) {							// Job実行中でない
		if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_STANDBY) {			// 半自動準備完了（自動開始入力済）
			return create_semiauto_command();																//　半自動コマンドを作成してポインタを返す
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_ACTIVE) {		// 半自動実行中
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);	//実行中コマンドのポインタを返す
		}
		else if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status == REQ_STANDBY) {				// JOB準備完了（クライアントからのJOB受信済）
			return create_job_command();																	// JOBコマンドを作成してポインタを返す
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == REQ_WAITING) {		// 半自動要求待ち（自動開始入力待ち）
			return NULL;																					//NULLポインタを返す
		}
		else {
			return NULL;
		}
	}
	else {
		if ((PolicyInf_workbuf.command_list.job_type == AUTO_TYPE_JOB)										// コマンドリスト内容がJOB
			&& (PolicyInf_workbuf.command_list.job_id == pCSInf->job_list.i_job_active)){					// コマンドリストの対象Jobが実行中Jobと一致
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);	//実行中コマンドのポインタを返す
		}
		else {																								//実行中jobとセット中のコマンドが一致しない　→　異常
			return NULL;
		}
	}
	return	NULL;
};

int CPolicy::set_receipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_BOOM_H;
	int ptn = 0;

	double D = pwork->dist_for_target[id]; //残り移動距離
	
	/*### 作成パターンタイプの判定 ###*/
	//FBありなしと１回のインチングで移動可能な距離かで区別
	if (pwork->a[id] == 0.0) return -1;
	double dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/α
	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### パターン作成 ###*/
	/*### STEP0  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:		//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{										
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
	}break;

	case PTN_FBSWAY_FULL:			//巻、旋回,位置位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し、減衰位相到達
	{	
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
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
	if (ta > T) tc_half += T;										//加速時間が振れ周期よりも長いときは＋T
	double dmin = v_top * ta + 2.0 * tc_half * v_half;
	double d = st_work.pos[motion] - st_work.pos_target[motion];	//移動距離
	if (d < 0.0) d *= -1.0;

	double t_expected = 2.0 * (ta + tc_half);	//予定移動時間
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//移動方向セット
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
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1　他軸位置待ち
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->_p = st_work.pos[motion];												// 現在位置
	pelement->_t = 0.0;																// 予定時間
	pelement->_v = 0.0;

	//待ち位置(目標までの距離）計算
	double ta_other, a_other, v_other, d_max_other;
	bool is_slew_first = false;

	//動作が引込方向か引出方向か判定して旋回、引込どちらを先に動かすか決定する
	if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
		is_slew_first = true;	//引出時は、半径が小さいうちに先に旋回を掛ける
	}

	if (motion == ID_BOOM_H) {	//引込動作用
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
		ta_other = v_other / a_other;					//加速時間最大値
		d_max_other = PI360;								//待ち無しとなる条件

		if (is_slew_first) {
			//				if (ta_other > t_expected)		//予定時間が相手軸の減速時間より短い時、相手の目標までの距離が減速停止距離内に入ったら動作開始
			//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
			//				else							//所要時間が相手軸の加速時間より長い
			//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

							//当面固定値にする
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
		}
		else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる

	}
	else if (motion == ID_SLEW) {	//旋回動作用
		pelement->type = CTR_TYPE_OTHER_POS_WAIT;
		a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
		v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
		ta_other = v_other / a_other;					//加速時間最大値
		d_max_other = pCraneStat->spec.boom_pos_max;	//待ち無しとなる条件
		if (is_slew_first == true) {
			pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる
		}
		else {
			//			if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
			//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
			//			else							//所要時間が相手軸の加速時間より長い
			//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

						//当面減速距離値にする
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
	//Step 2　起動調整（初期振れが大きいとき加速開始タイミングを調整）
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP速度定速出力
	pelement->_t = pCraneStat->T;													// 最大１周期待機
	pelement->_v = 0.0;																// トップ速度
	pelement->_p = st_work.pos[motion];												// 目標位置
	}
	//Step 3　加速中定速
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// 加速中定速出力
	pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
	pelement->_v = dir * v_half;													// ハーフ速度
	pelement->_p = (pelement - 1)->_p + dir * v_half * (0.5 * ta_half + tc_half);	// 目標位置
	}
	//Step 4　定速TOP速度
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP速度定速出力
	pelement->_t = ta_half + (d - dmin) / v_top;									// トップ速度出力時間
	pelement->_v = dir * v_top;														// トップ速度
	pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top * ta + v_half * tc_half);	// 目標位置
	}
	//Step 5　減速中定速
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// 加速中定速出力
	pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
	pelement->_v = dir * v_half;													// ハーフ速度
	pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// 目標位置
	}

	//Step 6　停止
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
	pelement->_t = ta_half;															// 減速時間
	pelement->_v = 0.0;																// 目標現在速度
	pelement->_p = st_work.pos_target[motion];										// 目標位置
	}
	//Step 7　動作停止待機
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
	pelement->_p = st_work.pos_target[motion];										// 目標位置
	pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
	pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//パターン出力時判定用
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


LPST_COMMAND_BLOCK CPolicy::create_semiauto_command(LPST_JOB_SET pjob) {	//実行する半自動コマンドをセットする
	
	LPST_COMMAND_BLOCK lp_semiauto_com = pPolicyInf->command_list[0];

	set_com_workbuf(pjob->target[0], AUTO_TYPE_SEMI_AUTO);											//半自動パターン作成用データ取り込み

	set_receipe_semiauto_bh(pjob->type, &(lp_semiauto_com->recipe[ID_BOOM_H]), true, &st_com_work);
	set_receipe_semiauto_slw(pjob->type, &(lp_semiauto_com->recipe[ID_SLEW]), true, &st_com_work);
	set_receipe_semiauto_mh(pjob->type, &(lp_semiauto_com->recipe[ID_HOIST]), true, &st_com_work);

	return lp_semiauto_com;

};

LPST_COMMAND_BLOCK CPolicy::create_job_command(LPST_JOB_SET pjob) {		//実行するJOBコマンドをセットする

	LPST_COMMAND_BLOCK lp_job_com = NULL;

	return lp_job_com;
};        

/****************************************************************************/
/*　　コマンドパターン計算用の素材データ計算,セット									*/
/*　　目標位置,目標までの距離,最大速度,加速度,加速時間,加速振れ中心,振れ振幅*/
/****************************************************************************/
LPST_POLICY_WORK CPolicy::set_com_workbuf(ST_POS_TARGETS targets, int type) {

	st_com_work.agent_scan_ms = pAgent->inf.cycle_ms;;                 //AGENTタスクのスキャンタイム
	switch(type){
	case AUTO_TYPE_SEMI_AUTO: {

		st_com_work.target = targets;																//目標位置



		double temp_d;
		for (int i = 0; i < MOTION_ID_MAX; i++) {
			if ((i == ID_HOIST) || (i == ID_BOOM_H) || (i == ID_SLEW)) {
				st_com_work.pos[i] = pPLC_IO->status.pos[i];										//現在位置
				st_com_work.v[i] = pPLC_IO->status.v_fb[i];											//現在速度
				temp_d = st_com_work.target.pos[i] - st_com_work.pos[i];

				if (i == ID_SLEW) {		//旋回は、180を越えるときは逆方向が近い
					if (temp_d > PI180)	temp_d -= PI360;
					else if (temp_d < -PI180) temp_d += PI360;
					else;
				}

				if (temp_d < 0.0) {
					st_com_work.motion_dir[i] = ID_REV;												//移動方向
					st_com_work.dist_for_target[i] = -1.0 * temp_d;									//移動距離
				}
				else {
					st_com_work.motion_dir[i] = ID_FWD;												//移動方向
					st_com_work.dist_for_target[i] = temp_d;										//移動距離
				}
				st_com_work.a[i] = pCraneStat->spec.accdec[i][st_com_work.motion_dir[i]][ID_ACC];	//動作軸加速度
				st_com_work.vmax[i]= pCraneStat->spec.notch_spd_f[i][NOTCH_MAX - 1];				//最大速度
				st_com_work.acc_time2Vmax[MOTION_ID_MAX] = st_com_work.vmax[i]/ st_com_work.a[i];   //最大加速時間
			}
			if ((i == ID_BOOM_H) || (i == ID_SLEW)) {
				st_com_work.a_hp[i] = pEnvironment->cal_hp_acc(i, st_com_work.motion_dir[i]);		//吊点の加速度
				st_com_work.pp_th0[i][ACC] = pEnvironment->cal_arad_acc(i, FWD);					//加速時振れ中心
				st_com_work.pp_th0[i][DEC] = pEnvironment->cal_arad_dec(i, REV);					//減速時振れ中心
				st_com_work.sway_amp[i] = pSway_IO->rad_amp2[i];									//振れ振幅2乗
				st_com_work.sway_amp[i] = sqrt(pSway_IO->rad_amp2[i]);		;						//振れ振幅
			}

		}

		//巻きの目標位置が上の時は、巻上後に旋回引き込み動作をするので目標位置の周期でパターンを作る
		if (targets.pos[ID_HOIST] > st_com_work.pos[ID_HOIST]) {
			st_com_work.T = pEnvironment->cal_T(targets.pos[ID_HOIST]);								//振れ周期
			st_com_work.w = pEnvironment->cal_w(targets.pos[ID_HOIST]);								//振れ角周波数
			st_com_work.w2 = pEnvironment->cal_w2(targets.pos[ID_HOIST]);							//振れ角周波数2乗
		}
		else {
			st_com_work.T = pCraneStat->T;															//振れ周期
			st_com_work.w = pCraneStat->w;															//振れ角周波数
			st_com_work.w2 = pCraneStat->w2;														//振れ角周波数2乗
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
/*　　1STEP,2STEP振れ止めパターンのゲイン（加速時間(角度）計算				*/
/****************************************************************************/
void CPolicy::set_as_gain(int motion, int as_type) {

	double a,r,w,l,r0, vmax, max_th_of_as, acc_time2Vmax;

	//最大速度による加速時間制限
	r = sqrt(pSway_IO->rad_amp2[motion]);			//振幅角評価値　rad
	r0 = pEnvironment->cal_arad_acc(motion,FWD);	 //加速時振中心
	w = pCraneStat->w;								//振れ角加速度
	a = st_work.a[motion];							//ここの加速度SLEWはrad/s2で良い（半径未考慮）　r0振れ中心は半径考慮済
	vmax = st_work.vmax[motion];					//ここの速度SLEWはrad/sで良い（半径未考慮）
	acc_time2Vmax = st_work.acc_time2Vmax[motion];	//加速時間最大値
	l = pCraneStat->mh_l;

	if (as_type == AS_PTN_1STEP){	// 1STEP
		max_th_of_as = r0 * 2.0; //1 STEPのロジックで制御可能な振れ振幅限界値　2r0
		//ゲイン計算用R0を設定（上限リミット）
		if (r >= max_th_of_as) {
			st_work.as_gain_phase[motion] = PI180;
		}
		else {
			st_work.as_gain_phase[motion] = acos(1 - 0.5 * r / r0);
		}
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
			st_work.as_gain_phase[motion] = acos(r0/(r+r0));
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
		if (motion == ID_SLEW) {
			if (st_work.dist_for_target[motion] > PI180) st_work.dist_for_target[motion] -= PI360;
			else if (st_work.dist_for_target[motion] < -PI180) st_work.dist_for_target[motion] += PI360;
			else;
		}

		if (dist_for_target < 0.0) dist_for_target *= -1.0;

	
		//1回のインチング移動距離　目標までの距離S=2d　　d = a*t^2　t=√(S/2a)
		st_work.as_gain_time[motion] = sqrt(0.5 * dist_for_target /a);
		if (st_work.as_gain_time[motion] > acc_time2Vmax) //最大速度による加速時間制限
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
/*　　自動（振れ止め）の動作パターンタイプの判定							*/
/****************************************************************************/
int CPolicy::judge_auto_ctrl_ptn(int auto_type, int motion) {
	
	int ptn = AS_PTN_0;
	double r0 = st_work.pp_th0[motion][ACC];	//加速時振中心

	double T = pCraneStat->T;
	double w = pCraneStat->w;
	double R = pPLC_IO->status.pos[ID_BOOM_H];
	double vmax = st_work.vmax[motion];
	double a = pCraneStat->spec.accdec[motion][FWD][ACC];
	double d_from_target = st_work.dist_for_target[motion];
	double d_min_ptn2ad = vmax * (T + vmax / a);
	double d_max_2pp = 2.0 * a * T * T;


	if (sqrt(pSway_IO->rad_amp2[motion]) > 2.0 * st_work.pp_th0[motion][ACC])					//振れ振幅 >　加速振れ*2
		ptn = AS_PTN_1STEP;
	else if (d_from_target > d_min_ptn2ad)													//目標位置までの距離 > 2Stepパターンの最小距離
		ptn = AS_PTN_2ACCDEC; 
	else if(d_from_target > d_max_2pp)
		ptn = AS_PTN_1ACCDEC;																//いずれでもない時は台形パターン
	else if (pSway_IO->rad_amp2[motion] > pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE])	//振れ振幅 >　加速時の振れ中心
		ptn = AS_PTN_1STEP;
	else 							
		ptn = AS_PTN_2PP;
	return ptn; 
}



}
/****************************************************************************/
/*　　振れ止めパターン　１stepタイプの計算,セット							*/
/****************************************************************************/
int CPolicy::set_recipe1step(LPST_MOTION_RECIPE precipe, int motion) { 
	
	LPST_MOTION_STEP pelement;

	st_work.motion_dir[motion] = POLICY_NA;										//移動方向不定(移動方向が実行時に決まる）
	set_as_gain(motion, AS_PTN_1STEP);													//振れ止めゲイン計算
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_1STEP;
		
	//Step 1　位相待ち
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;										// 目標に向かう方向の位相を待つ
		pelement->_p = st_work.pos[motion];													// 目標位置(開始時位置）
		pelement->_t = 2.0*st_work.T;														// 予定時間(２周期以内）
		pelement->_v = 0.0;
		if (motion == ID_SLEW) {															//旋回は、振れ方向と移動方向の向きが逆
			if (st_work.as_gain_phase[motion] > PI180) {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = 0;									// プラス加速用位相
				pelement->opt_d[MOTHION_OPT_PHASE_R] = PI180;								// マイナス加速用位相
			}
			else {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion]-PI180;	// プラス加速用位相
				pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion];		// マイナス加速用位相
			}
		}
		else {
			if (st_work.as_gain_phase[motion] > PI180) {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = PI180;								// プラス加速用位相
				pelement->opt_d[MOTHION_OPT_PHASE_R] = 0;									// マイナス加速用位相
			}
			else {
				pelement->opt_d[MOTHION_OPT_PHASE_F] = st_work.as_gain_phase[motion];		// プラス加速用位相
				pelement->opt_d[MOTHION_OPT_PHASE_R] = st_work.as_gain_phase[motion] - PI180;	// マイナス加速用位相
			}
		}
	}
	//Step 2　加速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_AS;												// 振れ止め加速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.a[motion] * pelement->_t;								// 目標変化速度
	}
	//Step 3　減速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為			
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = st_work.v[motion];												// 開始時速度
	}
	//Step 4　動作停止待機
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置(開始時位置）移動方向不定の為
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = st_work.v[motion];												// 開始時速度 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_1STEP;					//パターン出力時判定用
	}

	return AS_PTN_OK; 
};



/****************************************************************************/
/*　　振れ止めパターン　2step小移動										*/
/****************************************************************************/
int CPolicy::set_recipe2pp(LPST_MOTION_RECIPE precipe, int motion) {

	LPST_MOTION_STEP pelement;
	double dir;

	//移動方向セット
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

	set_as_gain(motion, AS_PTN_2PP);													//振れ止めゲイン計算
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_2PP;

	//Step 1　位相待ち(振れ振幅維持）
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_SINGLE_PHASE_WAIT;									// 位相待ち
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 2.0 * st_work.T;													// 予定時間(２周期以内）
		pelement->_v = 0.0;

		//起動位相
		pelement->opt_d[MOTHION_OPT_PHASE_R] = 0.0;					//マイナスプラス加速用位相
		pelement->opt_d[MOTHION_OPT_PHASE_F] = PI180;					//プラス加速用位相

	}
	//Step 2　加速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_V;												// 振れ止め加速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// 目標変化速度
		pelement->_p = (pelement-1)->_p + 0.5 * pelement->_v * pelement->_t;			// 目標位置
	}
	//Step 3　減速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement-1)->_v * pelement->_t;		// 目標位置
	}
	//Step 4　時間待ち
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間待ち
		pelement->_p = (pelement - 1)->_p;												// 目標位置
//		pelement->_t = PI180 / st_work.w - 2.0 * st_work.as_gain_time[motion];			// 待機時間
		double ph = PI180 - 2.0 * st_work.as_gain_phase[motion];
		if (ph < 0.0) ph += PI360;
		pelement->_t = ph / st_work.w;

		pelement->_v = 0.0;
	}
	//Step 5　加速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ACC_V;												// 振れ止め加速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = dir * st_work.a[motion] * pelement->_t;							// 目標変化速度
		pelement->_p = (pelement - 1)->_p + 0.5 * pelement->_v * pelement->_t;			// 目標位置
	}
	//Step 6　減速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = st_work.as_gain_time[motion];									// 振れ止めゲイン
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = (pelement - 1)->_p + 0.5 * (pelement - 1)->_v * pelement->_t;	// 目標位置
	}
	//Step 7　微小位置合わせ
	{	pelement = &(precipe->steps[precipe->n_step++]);
	pelement->type = CTR_TYPE_FINE_POSITION;											// 微小移動
	pelement->_p = st_work.pos_target[motion];											// 開始時位置
	pelement->_t = PTN_FINE_POS_LIMIT_TIME;												// パターン出力調整時間
	pelement->_v = pCraneStat->spec.notch_spd_f[motion][NOTCH_1];						// １ノッチ速度	
	}
	//Step 8　動作停止待機
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2PP;			//パターン出力時判定用
	}

	return AS_PTN_OK;
};

/****************************************************************************/
/*　　振れ止めパターン　2段加減速											*/
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
	if (ta > T) tc_half += T;										//加速時間が振れ周期よりも長いときは＋T
	double dmin = v_top * ta + 2.0 * tc_half * v_half;
	double d = st_work.pos[motion] - st_work.pos_target[motion];	//移動距離
	if (d < 0.0) d *= -1.0;

	double t_expected = 2.0 * (ta + tc_half);	//予定移動時間
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;

	double dir;

	//移動方向セット
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
	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_2ACCDEC;

	//Step 1　他軸位置待ち
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 0.0;																// 予定時間
		pelement->_v = 0.0;

		//待ち位置(目標までの距離）計算
		double ta_other, a_other, v_other, d_max_other;
		bool is_slew_first=false;

		//動作が引込方向か引出方向か判定して旋回、引込どちらを先に動かすか決定する
		if (st_work.pos_target[ID_BOOM_H] > st_work.pos[ID_BOOM_H] + 1.0) {
			is_slew_first = true;	//引出時は、半径が小さいうちに先に旋回を掛ける
		}

		if (motion == ID_BOOM_H) {	//引込動作用
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];		
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;					//加速時間最大値
			d_max_other= PI360;								//待ち無しとなる条件

			if (is_slew_first) {
//				if (ta_other > t_expected)		//予定時間が相手軸の減速時間より短い時、相手の目標までの距離が減速停止距離内に入ったら動作開始
//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
//				else							//所要時間が相手軸の加速時間より長い
//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

				//当面固定値にする
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる

		}
		else if (motion == ID_SLEW) {	//旋回動作用
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;					//加速時間最大値
			d_max_other = pCraneStat->spec.boom_pos_max;	//待ち無しとなる条件
			if (is_slew_first==true) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる
			}
			else {
	//			if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
	//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
	//			else							//所要時間が相手軸の加速時間より長い
	//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);
				
				//当面減速距離値にする
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
	//Step 2　起動調整（初期振れが大きいとき加速開始タイミングを調整）
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP速度定速出力
		pelement->_t = pCraneStat->T;													// 最大１周期待機
		pelement->_v = 0.0;																// トップ速度
		pelement->_p = st_work.pos[motion];												// 目標位置
	}
	//Step 3　加速中定速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_ACC_STEP;										// 加速中定速出力
		pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
		pelement->_v = dir * v_half;													// ハーフ速度
		pelement->_p = (pelement - 1)->_p + dir * v_half * ( 0.5 * ta_half + tc_half);	// 目標位置
	}
	//Step 4　定速TOP速度
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP速度定速出力
		pelement->_t = ta_half + (d - dmin) / v_top;									// トップ速度出力時間
		pelement->_v = dir * v_top;														// トップ速度
		pelement->_p = st_work.pos_target[motion] - dir * (0.5 * v_top *  ta + v_half * tc_half);	// 目標位置
	}
	//Step 5　減速中定速
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_DEC_STEP;										// 加速中定速出力
		pelement->_t = ta_half + tc_half;												// ハーフ速度出力時間
		pelement->_v = dir * v_half;													// ハーフ速度
		pelement->_p = st_work.pos_target[motion] - dir * v_half * 0.5 * ta_half;		// 目標位置
	}

	//Step 6　停止
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = ta_half;															// 減速時間
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = st_work.pos_target[motion];										// 目標位置
	}
	//Step 7　動作停止待機
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//パターン出力時判定用
	}
	return AS_PTN_OK;
};
/****************************************************************************/
/*　　振れ止めパターン　台形												*/
/****************************************************************************/
int CPolicy::set_recipe1ad(LPST_MOTION_RECIPE precipe, int motion) {
	LPST_MOTION_STEP pelement;
	double T = pCraneStat->T;
	double w = pCraneStat->w;
	double a = st_work.a[motion];
	double v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_5]; 
	//半周期以上の移動時間となるTop速度設定
	for (int i = 0; i < NOTCH_MAX; i++) {
		int k = (NOTCH_MAX - i) - 1;
		v_top = pCraneStat->spec.notch_spd_f[motion][k];
		if((v_top * (T/2.0 + v_top / a) < st_work.dist_for_target[motion]))
			break;
	}
	
	//0ノッチ速度の時は下限速度（１ノッチ速度）
	if (v_top < pCraneStat->spec.notch_spd_f[motion][NOTCH_1])v_top = pCraneStat->spec.notch_spd_f[motion][NOTCH_1];

	double ta = v_top / a;
	double dmin = v_top * ta;
	double d = st_work.dist_for_target[motion];	//移動距離
	double t_expected = 2.0 * ta ;	//予定移動時間
	if (st_work.dist_for_target[motion] > dmin) t_expected += (st_work.dist_for_target[motion] - dmin) / v_top;
	double dir;

	//移動方向セット
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


	precipe->n_step = 0;																//ステップ数初期化
	precipe->motion_type = AS_PTN_1ACCDEC;

	//待ち位置(目標までの距離）計算
	double ta_other, a_other, v_other, d_max_other;
	bool is_slew_first = false;

	//Step 1　他軸位置待ち
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->_p = st_work.pos[motion];												// 現在位置
		pelement->_t = 0.0;																// 予定時間
		pelement->_v = 0.0;

		if (motion == ID_BOOM_H) {	//引込動作用
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_SLEW][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_SLEW][NOTCH_5];
			ta_other = v_other / a_other;					//加速時間最大値
			d_max_other = PI360;								//待ち無しとなる条件

			if (is_slew_first) {
				//				if (ta_other > t_expected)		//予定時間が相手軸の減速時間より短い時、相手の目標までの距離が減速停止距離内に入ったら動作開始
				//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * v_other;
				//				else							//所要時間が相手軸の加速時間より長い
				//					pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

								//当面固定値にする
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = PI15;
			}
			else 	pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる

		}
		else if (motion == ID_SLEW) {	//旋回動作用
			pelement->type = CTR_TYPE_OTHER_POS_WAIT;
			a_other = pCraneStat->spec.accdec[ID_BOOM_H][FWD][ACC];
			v_other = pCraneStat->spec.notch_spd_f[ID_BOOM_H][NOTCH_5];
			ta_other = v_other / a_other;					//加速時間最大値
			d_max_other = pCraneStat->spec.boom_pos_max;	//待ち無しとなる条件
			if (is_slew_first == true) {
				pelement->opt_d[MOTHION_OPT_WAIT_POS] = d_max_other;	//待ち無しとなる
			}
			else {
				//			if (ta_other > t_expected)		//所要時間が相手軸の加速時間より短い
				//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * t_expected * t_expected * a_other;
				//			else							//所要時間が相手軸の加速時間より長い
				//				pelement->opt_d[MOTHION_OPT_WAIT_POS] = 0.5 * ta_other * ta_other + v_other * (ta_other - t_expected);

							//当面減速距離値にする
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
	//Step 2　起動調整（初期振れが大きいとき加速開始タイミングを調整）
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_ADJUST_MOTION_TRIGGER;								// TOP速度定速出力
		pelement->_t = pCraneStat->T;													// 最大１周期待機
		pelement->_v = 0.0;																// トップ速度
		pelement->_p = st_work.pos[motion];												// 目標位置
	}

	//Step 3　定速TOP速度
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_CONST_V_TOP_STEP;										// TOP速度定速出力
		pelement->_t = ta + (d - dmin) / v_top;											// トップ速度出力時間
		pelement->_v = dir * v_top;														// トップ速度
		pelement->_p = st_work.pos_target[motion] - dir * 0.5 * ta * v_top;				// 目標位置
		if (pelement->_p > PI180) pelement->_p -= PI360;
		else if(pelement->_p < -PI180) pelement->_p += PI360;
	}
	//Step 4　停止
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_DEC_V;												// 開始時速度に減速
		pelement->_t = ta;																// 減速時間
		pelement->_v = 0.0;																// 目標現在速度
		pelement->_p = st_work.pos_target[motion];										// 目標位置
	}
	//Step 5　動作停止待機
	{	pelement = &(precipe->steps[precipe->n_step++]);
		pelement->type = CTR_TYPE_TIME_WAIT;											// 時間調整
		pelement->_p = st_work.pos_target[motion];										// 目標位置
		pelement->_t = PTN_CONFIRMATION_TIME;											// パターン出力調整時間
		pelement->_v = 0.0;																// 
	}

	//予定時間をAGENTのスキャンカウント値に変換,パターンタイプセット
	for (int i = 0; i < precipe->n_step; i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000) / (int)st_work.agent_scan_ms;
		precipe->steps[i].opt_i[MOTHION_OPT_AS_TYPE] = AS_PTN_2ACCDEC;			//パターン出力時判定用
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


