#include "CPolicy.h"
#include "CAgent.h"
#include "CEnvironment.h"
#include "CClientService.h"
#include "CHelper.h"

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

//定周期処理手順3　表示,信号出力処理
void CPolicy::output() {
	//共有メモリ出力処理
	memcpy_s(pPolicyInf, sizeof(ST_POLICY_INFO), &PolicyInf_workbuf, sizeof(ST_POLICY_INFO));
	//タスクパネルへの表示出力
	wostrs << L" --Scan " << dec << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;
};

/****************************************************************************/
/*　　COMMAND 処理															*/
/****************************************************************************/
// AGENTからのコマンド要求処理
LPST_COMMAND_BLOCK CPolicy::req_command() {

	if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status != STAT_ACTIVE) {							// Job実行中でない
		if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_STANDBY) {				// 半自動準備完了（自動開始入力済）
			return create_semiauto_command(&(pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active]));	//　半自動コマンドを作成してポインタを返す
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_ACTIVE) {			// 半自動実行中
			return &(PolicyInf_workbuf.command_list.commands[PolicyInf_workbuf.command_list.current_step]);		//実行中コマンドのポインタを返す
		}
		else if (pCSInf->job_list.job[pCSInf->job_list.i_job_active].status == STAT_STANDBY) {					// JOB準備完了（クライアントからのJOB受信済）
			return create_job_command(&(pCSInf->job_list.job[pCSInf->job_list.i_job_active]));					// JOBコマンドを作成してポインタを返す
		}
		else if (pCSInf->job_list.semiauto[pCSInf->job_list.i_semiauto_active].status == STAT_WAITING) {		// 半自動要求待ち（自動開始入力待ち）
			return NULL;																						//NULLポインタを返す
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
/****************************************************************************/
/*　　移動パターンレシピ生成												*/
/****************************************************************************/
/* # 起伏レシピ　*/
int CPolicy::set_receipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_BOOM_H;
	int ptn = 0;							//移動パターン

	double D = pwork->dist_for_target[id]; //残り移動距離
	
	/*### 作成パターンタイプの判定 ###*/
	//FBありなしと１回のインチングで移動可能な距離かで区別
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	double dist_inch_max;							
	if (pwork->vmax[id]/ pwork->a[id] > pwork->T) {								//振れ周期以内の加速時間
		dist_inch_max = pwork->a[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/α
	}

	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### パターン作成 ###*/
	precipe->n_step = 0;														// ステップクリア

	/*### STEP0  待機　###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// 巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{										
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D = D;																	// 残り距離変更なし

	}break;
	 
	case PTN_FBSWAY_FULL:														// 巻旋回,位置位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し、減衰位相到達
	{	
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D = D;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 速度ステップ出力　2段分###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:												// 出力するノッチ速度を計算して設定
	case PTN_FBSWAY_FULL:														
	{
		double d_step = D;													//ステップでの移動距離
		double v_top=0.0;													//ステップ速度用
		double check_d;
		int n=0, i;

		// #Step1 １段目
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];

			int k = (int)(v_top / st_com_work.a[id] / st_com_work.T);					// k > 0 →　加速時間が振れ周期以上
			check_d = v_top * (1 + k) * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//一周期振れ止め距離以上有り
			else continue;			//次のノッチへ
		}

		if (n) {																		//一周期振れ止めの移動距離有でステップ追加
			pelement = &(precipe->steps[precipe->n_step++]);							//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_POS;											//位置到達待ち

			double ta = v_top / st_com_work.a[id];
			pelement->_t = (double)n * st_com_work.T - ta;								// 振れ周期　-　減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n *  st_com_work.T - 0.5 * ta);
			pelement->_p =  (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置

			D -= d_move;																// 残り距離更新
		}

		
	

		//  #Step1２段目
		d_step = D;
		for (i -= 1;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//一周期移動以上の距離有り
			else continue;			//次のノッチへ
		}
		
		if (n) {																		//一周期以上の移動距離有でステップ追加
			pelement = &(precipe->steps[precipe->n_step++]);							//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_POS;											//位置到達待ち

			double td = ((pelement - 1)->_v - v_top) / st_com_work.a[id];				// 減速時間 ステップ速度までの減速時間
			pelement->_t = (double)n * st_com_work.T + td;								// 振れ周期　+　2段目までの減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n * st_com_work.T + td) + 0.5 * td * ((pelement - 1)->_v - v_top) ;
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置

			D -= d_move;																// 残り距離更新
		}


		//  #Step2 停止
		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
		pelement->_t = (pelement - 1)->_v / st_com_work.a[id];							//減速時間
		pelement->_v = 0.0;																// 速度0
		double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置

		D -= d_move;																	// 残り距離更新
	}break;

	case PTN_NO_FBSWAY_2INCH:													//台形部無いケースはスキップ
	case PTN_FBSWAY_AS:
	{
		D = D;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	{
		double v_inch = sqrt(0.5 * D * st_com_work.a[id]);
		double ta = v_inch/st_com_work.a[id];
		double v_top;
		for (int i = NOTCH_MAX;i > 1;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			if (v_inch > pCraneStat->spec.notch_spd_f[id][i - 1])break;
			else continue;			//次のノッチへ
		}

		//STEP3
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		pelement->_v = v_top;													// ノッチ速度
		double d_move = 0.5 * v_inch * ta;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		D -= d_move;															

		//STEP4
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		} 
		
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta + tc;													// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		D -= d_move;															

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		pelement->_v = v_top;													// ノッチ速度
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		D -= d_move;

		//STEP6
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D -= d_move;

	}break;

	case PTN_FBSWAY_AS:															//振れFBあるパターンはスキップ
	case PTN_FBSWAY_FULL:														
	{
		D = D;																	//残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FINE_POS;										// 微小位置決め
		pelement->_t = FINE_POS_TIMELIMIT;										// 位置合わせ最大継続時間
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// １ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D = 0;																	// 残り距離変更なし

	}break;

	case PTN_FBSWAY_AS:
	case PTN_FBSWAY_FULL:														// 巻、旋回,位置位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し、減衰位相到達
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FB_SWAY_POS;									// FB振れ止め/位置決め
		pelement->_t = st_com_work.T * 4.0;										// 振れ4周期分
		pelement->_v = 0.0;														// 振れ止めロジックで決定
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D = 0;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	return POLICY_PTN_OK;
}
/* # 旋回レシピ　*/
int CPolicy::set_receipe_semiauto_slw(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_SLEW;
	int ptn = 0;							//移動パターン

	double D = pwork->dist_for_target[id]; //残り移動距離

	/*### 作成パターンタイプの判定 ###*/
	//FBありなしと１回のインチングで移動可能な距離かで区別
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	double dist_inch_max;
	if (pwork->vmax[id] / pwork->a[id] > pwork->T) {							//振れ周期以内の加速時間
		dist_inch_max = pwork->a[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax[id] * pwork->vmax[id] / pwork->a[id];//V^2/α
	}

	if (is_fbtype) {
		if (D > dist_inch_max) ptn = PTN_NO_FBSWAY_FULL;
		else ptn = PTN_NO_FBSWAY_2INCH;
	}
	else {
		if (D > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}

	/*### パターン作成 ###*/
	precipe->n_step = 0;														// ステップクリア

	/*### STEP0  待機　###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// 巻、引込位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  引込：引き出し時は条件無し、引き込み時は引込位置が目標＋Xｍ以下
	case PTN_NO_FBSWAY_2INCH:
	case PTN_FBSWAY_AS:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置現在位置
		CHelper::fit_slew_axis(&(pelement->_p));								//目標位置の校正
		D = D;																	// 残り距離変更なし

	}break;

	case PTN_FBSWAY_FULL:														// 巻旋回,位置位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  引込：引き出し時は条件無し、引き込み時は引込位置が目標＋Xｍ以下
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_AND_PH;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));								// 目標位置の校正
		D = D;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 速度ステップ出力　1,2段 ###*/

	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													// 旋回はFBなしの時は振れ止めパターン
	{
		double d_step = D;													//ステップでの移動距離
		double v_top = 0.0;													//ステップ速度用
		double check_d;
		int n = 0, i;

		// #Step1 １段目
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];

			int k = (int)(v_top / st_com_work.a[id] / st_com_work.T);					// k > 0 →　加速時間が振れ周期以上
			check_d = v_top * (1+k) * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//一周期振れ止め距離以上有り
			else continue;			//次のノッチへ
		}

		if (n) {																		//一周期振れ止めの移動距離有でステップ追加
			pelement = &(precipe->steps[precipe->n_step++]);							//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_POS;											//位置到達待ち

			double ta = v_top / st_com_work.a[id];
			pelement->_t = (double)n * st_com_work.T - ta;								// 振れ周期　-　減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n * st_com_work.T - 0.5 * ta);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置
			CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

			D -= d_move;																// 残り距離更新
		}

		//  #Step1２段目
		d_step = D;
		for (i -= 1;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//一周期移動以上の距離有り
			else continue;			//次のノッチへ
		}

		if (n) {																		//一周期以上の移動距離有でステップ追加
			pelement = &(precipe->steps[precipe->n_step++]);							//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_POS;											//位置到達待ち

			double td = ((pelement - 1)->_v - v_top) / st_com_work.a[id];				// 減速時間 ステップ速度までの減速時間
			pelement->_t = (double)n * st_com_work.T + td;								// 振れ周期　+　2段目までの減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n * st_com_work.T + td) + 0.5 * td * ((pelement - 1)->_v - v_top);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置
			CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

			D -= d_move;																// 残り距離更新
		}

		//  #Step2 停止
		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
		pelement->_t = (pelement - 1)->_v / st_com_work.a[id];							//減速時間
		pelement->_v = 0.0;																// 速度0
		double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

		D -= d_move;																	// 残り距離更新
	}break;

	case PTN_FBSWAY_FULL:														// 旋回はFBありの時は振れ止め無し1段のみ：減速タイミングで調整
	{
		double d_step = D;																//ステップでの移動距離
		double v_top = 0.0;																//ステップ速度用
		double check_d;
		int n = 0, i;

		// #Step1　１段目
		for (i = NOTCH_MAX;i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * v_top / st_com_work.a[id] + SPD_FB_DELAY_TIME * v_top;
			if (check_d < d_step) break;
			else continue;																//次のノッチへ
		}

		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_POS;												//位置到達待ち
		double ta = v_top / st_com_work.a[id];
		double tc = (D - v_top * ta) / v_top;
		pelement->_t = ta + tc;															// 振れ周期　-　減速時間
		pelement->_v = v_top;															// 速度0
		double d_move = v_top * (tc + 0.5 * ta);
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D -= d_move;																	// 残り距離更新

		//  #Step2 停止
		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
		pelement->_t = ta;																//減速時間
		pelement->_v = 0.0;																// 速度0
		pelement->_p = st_com_work.target.pos[id];											// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D -= d_move;																	// 残り距離更新
	}break;

	case PTN_NO_FBSWAY_2INCH:													//台形部無いケースはスキップ
	case PTN_FBSWAY_AS:
	{
		D = D;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	{
		double v_inch = sqrt(0.5 * D * st_com_work.a[id]);
		double ta = v_inch / st_com_work.a[id];
		double v_top;
		for (int i = NOTCH_MAX;i > 1;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			if (v_inch > pCraneStat->spec.notch_spd_f[id][i - 1])break;
			else continue;			//次のノッチへ
		}

		//STEP3
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		pelement->_v = v_top;													// ノッチ速度
		double d_move = 0.5 * v_inch * ta;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D -= d_move;

		//STEP4
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta + tc;													// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D -= d_move;

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		pelement->_v = v_top;													// ノッチ速度
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D -= d_move;

		//STEP6
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {
			int n = (int)(-tc / st_com_work.T) + 1;
			tc += (double)n * st_com_work.T;
		}

		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));								//目標位置の校正
		D -= d_move;

	}break;

	case PTN_FBSWAY_AS:															//振れFBあるパターンはスキップ
	case PTN_FBSWAY_FULL:
	{
		D = D;																	//残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NO_FBSWAY_FULL:													//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NO_FBSWAY_2INCH:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FINE_POS;										// 微小位置決め
		pelement->_t = FINE_POS_TIMELIMIT;										// 位置合わせ最大継続時間
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// １ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D = 0;																	// 残り距離変更なし

	}break;

	case PTN_FBSWAY_AS:
	case PTN_FBSWAY_FULL:														//巻、旋回,位置位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し、減衰位相到達
	{
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FB_SWAY_POS;									// FB振れ止め/位置決め
		pelement->_t = st_com_work.T * 4.0;										// 振れ4周期分
		pelement->_v = 0.0;														// 振れ止めロジックで決定
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));								//目標位置の校正
		D = 0;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}


	return POLICY_PTN_OK;
}
/* # 巻レシピ　*/
int CPolicy::set_receipe_semiauto_mh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	LPST_MOTION_STEP pelement;
	int id = ID_HOIST;
	int ptn = 0;							//移動パターン

	double D = pwork->dist_for_target[id]; //残り移動距離

	/*### 作成パターンタイプの判定 ###*/
	//FBありなしと１回のインチングで移動可能な距離かで区別
	if (pwork->a[id] == 0.0) return POLICY_PTN_NG;

	/*### パターン作成 ###*/
	precipe->n_step = 0;													// ステップクリア

	/*### STEP0  待機　###*/												// 引込、旋回位置待ち　巻上時：条件無し　巻下時： 引込・旋回共が目標位置の指定範囲内

	pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_WAIT_POS_OTHERS;								// 他軸位置待ち
	pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
	pelement->_v = 0.0;														// 速度0
	pelement->_p = pwork->pos[id];											// 目標位置
	D = D;																	// 残り距離変更なし

	/*### STEP1、2 速度ステップ出力　###*/

	double d_step = D;																//ステップでの移動距離
	double v_top = 0.0;																//ステップ速度用
	double check_d;
	int n = 0, i;

	// #Step1　１段目
	for (i = NOTCH_MAX;i > 0;i--) {
		v_top = pCraneStat->spec.notch_spd_f[id][i];
		check_d = v_top * v_top / st_com_work.a[id] + SPD_FB_DELAY_TIME * v_top;
		if (check_d < d_step) break;
		else continue;																//次のノッチへ
	}

	pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_VOUT_POS;												//位置到達待ち
	double ta = v_top / st_com_work.a[id];
	double tc = (D - v_top * ta) / v_top;
	pelement->_t = ta + tc;															// 振れ周期　-　減速時間
	pelement->_v = v_top;															// 速度0
	double d_move = v_top * (tc + 0.5 * ta);
	pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
	D -= d_move;																	// 残り距離更新

	//  #Step2 停止
	pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
	pelement->_t = ta;																//減速時間
	pelement->_v = 0.0;																// 速度0
	pelement->_p = st_com_work.target.pos[id];											// 目標位置
	CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
	D -= d_move;																	// 残り距離更新


	/*### STEP7  ###*/
	pelement = &(precipe->steps[precipe->n_step++]);								// ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_FINE_POS;												// 微小位置決め
	pelement->_t = FINE_POS_TIMELIMIT;												// 位置合わせ最大継続時間
	pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];						// １ノッチ速度
	pelement->_p = st_com_work.target.pos[id];										// 目標位置
	D = 0;																			// 残り距離変更なし
	return POLICY_PTN_OK;
}


LPST_COMMAND_BLOCK CPolicy::create_semiauto_command(LPST_JOB_SET pjob) {							//実行する半自動コマンドをセットする
	
	LPST_COMMAND_BLOCK lp_semiauto_com = &(pPolicyInf->command_list.commands[0]);					//コマンドブロックのポインタセット
	
	lp_semiauto_com->no = COM_NO_SEMIAUTO;
	lp_semiauto_com->type = AUTO_TYPE_SEMIAUTO;

	for (int i = 0;i < MOTION_ID_MAX;i++) lp_semiauto_com->is_active_axis[i]= false;
	lp_semiauto_com->is_active_axis[ID_HOIST] = true;
	lp_semiauto_com->is_active_axis[ID_SLEW] = true;
	lp_semiauto_com->is_active_axis[ID_BOOM_H] = true;
	
	set_com_workbuf(pjob->target[0], AUTO_TYPE_SEMIAUTO);											//半自動パターン作成用データ取り込み

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
	case AUTO_TYPE_SEMIAUTO: {

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
				else if(temp_d > 0.0) {
					st_com_work.motion_dir[i] = ID_FWD;												//移動方向
					st_com_work.dist_for_target[i] = temp_d;										//移動距離
				}
				else{
					st_com_work.motion_dir[i] = ID_STOP;											//移動方向
					st_com_work.dist_for_target[i] = 0.0;											//移動距離
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

#if 0
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

#endif

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


