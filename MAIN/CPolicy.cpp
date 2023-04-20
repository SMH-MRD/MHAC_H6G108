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
extern CSharedMem* pOTEioObj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;
extern CSharedMem* pJobIO_Obj;


extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
/****************************************************************************/
CPolicy::CPolicy() {
	pPolicyInf = NULL;
	pPLC_IO = NULL;
	pCraneStat = NULL;
	pOTE_IO = NULL;
	pSway_IO = NULL;
	pCSInf = NULL;
	pAgentInf = NULL;
	memset(&st_com_work, 0, sizeof(ST_POLICY_WORK));
	memset(&PolicyInf_workbuf, 0, sizeof(ST_POLICY_INFO));
}

CPolicy::~CPolicy() {

}

/****************************************************************************/
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/

static CAgent* pAgent=NULL;
static CEnvironment* pEnvironment = NULL;
static CClientService* pCS = NULL;

void CPolicy::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
	pPolicyInf = (LPST_POLICY_INFO)(pPolicyInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pOTE_IO = (LPST_OTE_IO)(pOTEioObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());
	pCSInf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pJob_IO = (LPST_JOB_IO)(pJobIO_Obj->get_pMap());

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
LPST_COMMAND_SET CPolicy::req_command(LPST_JOB_SET pjob_set) {


	int _i_hot_com = pjob_set->i_hot_com;
	LPST_COMMAND_SET pcom_set;

	if (pjob_set == NULL) return NULL;	//NULL jobにはNULLリターン

	if (pjob_set->status & STAT_TRIGED) {							//JOBのステータスが実行待ち
		_i_hot_com = pjob_set->i_hot_com = 0;						//起動時は、実行レシピののインデックスは、0
		pcom_set = setup_job_command(&(pjob_set->recipe[_i_hot_com]), pjob_set->list_id);
		pjob_set->recipe[_i_hot_com].status = STAT_STANDBY;			//コマンドステータス更新
	
		pCS->update_job_status(pjob_set, STAT_ACTIVE);				//JOBのステータスをACTIVEに更新
	}
	else if (pjob_set->status & STAT_ACTIVE) {						//JOB実行中からの呼び出し＝ 次のレシピ実行待ち
		if (pjob_set->i_hot_com < (pjob_set->n_com - 1)) {			//次のレシピ有
			_i_hot_com = pjob_set->i_hot_com++;
			pcom_set = setup_job_command(&(pjob_set->recipe[_i_hot_com]), pjob_set->list_id);
			pjob_set->recipe[_i_hot_com].status = STAT_STANDBY;		//コマンドステータス更新
		}
		else {														//次レシピ無　既に完了済 
			_i_hot_com = pjob_set->i_hot_com;
			pcom_set = NULL;										//次レシピ無し→NULLリターン
			pjob_set->recipe[_i_hot_com].status = STAT_END;			//コマンドステータス更新
			pCS->update_job_status(pjob_set, STAT_END);				//JOBのステータスをNORMAL　END更新
		}
	}
	else {
		pcom_set= NULL;
		pCS->update_job_status(pjob_set, STAT_ABOTED);				//JOBのステータスをABOTE更新　正常完了の時はupdate_command_status()から報告
	}


	if (pcom_set != NULL) {
		//### コマンドコードセット
		pcom_set->com_code.i_list = pjob_set->list_id;
		pcom_set->com_code.i_job = pjob_set->id;
		pcom_set->com_code.i_recipe = _i_hot_com;
	}
	return pcom_set;

};

// AGENTからのコマンド実行報告処理
int CPolicy::update_command_status(LPST_COMMAND_SET pcom, int code) {

	if (pcom == NULL)return STAT_NAK;
	LPST_JOB_SET pjob_set = &pJob_IO->job_list[pcom->com_code.i_list].job[pcom->com_code.i_job];//紐付きJOB

	//コマンドコードとJOB＿SETの内容に不整合でエラー
	int _i_recipe = pcom->com_code.i_recipe;
	if (_i_recipe != pjob_set->i_hot_com) return NULL;

	LPST_COM_RECIPE pcom_recipe = &pjob_set->recipe[_i_recipe];
	switch (code) {
	case STAT_END: {
		pcom_recipe->status = code;								//コマンドのステータスを報告内容に更新
		pCS->update_job_status(pjob_set, STAT_END);				//JOBのステータス更新
	}break;
	case STAT_ABNORMAL_END: {
		pcom_recipe->status = code;								//コマンドのステータスを報告内容に更新
		pCS->update_job_status(pjob_set, STAT_ABNORMAL_END);	//JOBのステータス更新
	}break;
	case STAT_ACTIVE: {
		pCS->update_job_status(pjob_set, STAT_ACTIVE);			//JOBのステータス更新
		pcom_recipe->status = code;								//コマンドのステータスを報告内容に更新
	}break;
	case STAT_ABOTED: {
		pcom_recipe->status = code;								//コマンドのステータスを報告内容に更新
		pCS->update_job_status(pjob_set, STAT_ABOTED);			//JOBのステータス更新
	}break;
	case STAT_SUSPENDED: {
		pcom_recipe->status = code;								//コマンドのステータスを報告内容に更新
		pCS->update_job_status(pjob_set, STAT_SUSPENDED);		//JOBのステータス更新
	}break;
	default: break;
	}
	//return req_command(pjob_set);
	return STAT_ACK;
}

LPST_COMMAND_SET CPolicy::setup_job_command(LPST_COM_RECIPE pcom_recipe, int type) {							//実行する半自動コマンドをセットする

	LPST_COMMAND_SET pcom_set = &pcom_recipe->comset;

	//半自動は、巻、旋回、引込が対象
	for (int i = 0;i < MOTION_ID_MAX;i++) pcom_set->active_motion[i] = L_OFF;
	if (type == ID_JOBTYPE_SEMI) {
		pcom_set->active_motion[ID_HOIST] = L_ON;
		pcom_set->active_motion[ID_SLEW] = L_ON;
		pcom_set->active_motion[ID_BOOM_H] = L_ON;
	}
	else if (type == ID_JOBTYPE_JOB) {
		pcom_set->active_motion[ID_HOIST] = L_ON;
		pcom_set->active_motion[ID_SLEW] = L_ON;
		pcom_set->active_motion[ID_BOOM_H] = L_ON;
	}
	else;

	set_com_workbuf(pcom_recipe->target);	//半自動パターン作成作業用構造体（st_com_work）にデータ取り込み

	bool is_fb_antisway = false;
	if (pCSInf->antisway_mode == L_ON) {
		is_fb_antisway = true;
	}
	//コマンドセットに目標位置セット
	pcom_set->target = st_com_work.target;
	//旋回,引込,巻のレシピセット　set_recipe_semiauto_bh(JOBタイプ,レシピアドレス,isFBタイプ,レシピ設定条件バッファアドレス
	set_recipe_semiauto_bh(ID_JOBTYPE_SEMI, &(pcom_set->recipe[ID_BOOM_H]), is_fb_antisway, &st_com_work);
	set_recipe_semiauto_slw(ID_JOBTYPE_SEMI, &(pcom_set->recipe[ID_SLEW]), is_fb_antisway, &st_com_work);
	set_recipe_semiauto_mh(ID_JOBTYPE_SEMI, &(pcom_set->recipe[ID_HOIST]), is_fb_antisway, &st_com_work);

	return pcom_set;
};

/****************************************************************************/
/*　　移動パターンレシピ生成												*/
/****************************************************************************/
/* # 引込レシピ　*/
int CPolicy::set_recipe_semiauto_bh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	//#レシピ条件セット
	//軸ID
	int id = precipe->axis_id = ID_BOOM_H;
	
	//移動方向
	precipe->direction = pwork->motion_dir[id];

	double dist_inch_max;
	if (pwork->vmax_abs[id] / pwork->a_abs[id] > pwork->T) {							//最大速度までの加速時間が振れ周期より大きい時は振れ周期分の加速時間がインチング最大距離
		dist_inch_max = pwork->a_abs[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax_abs[id] * pwork->vmax_abs[id] / pwork->a_abs[id];	//最大速度までの加速時間が振れ周期より小さい時V^2/α
	}

	//作成パターンのタイプ   FBありなしと１回のインチングで移動可能な距離かで区別
	double D_abs = pwork->dist_for_target_abs[id];										//残り移動距離
	
	int ptn = 0;
	if (is_fbtype) {															//インチング最大距離の計算 移動距離がインチング最大距離より小さいとき,FB有はFB振れ止め、FB無しは2回インチング移動
		if (D_abs > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}
	else {
		if (D_abs > dist_inch_max) ptn = PTN_NON_FBSWAY_FULL;
		else ptn = PTN_NON_FBSWAY_2INCH;
	}
	precipe->motion_type =ptn;

	LPST_MOTION_STEP pelement;
	
	
	//加速度が0.0はエラー　0割り防止
	if (pwork->a_abs[id] == 0.0) return POLICY_PTN_NG;								


	/*### パターン作成 ###*/
	precipe->n_step = 0;														// ステップクリア

	/*### STEP0  待機　###*/

	switch (ptn) {

	case PTN_NON_FBSWAY_FULL:	//FB無のフルパターン
	case PTN_NON_FBSWAY_2INCH:	//FB無のインチングパターン
	case PTN_FBSWAY_AS:			//FB振れ止めパターン
	{
		// 巻位置待ち　巻き位置：巻目標高さ-Xm　以上になったら						
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_HST;									// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D_abs = D_abs;															// 残り距離変更なし


		// 旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し						
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_SLW;									// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D_abs = D_abs;															// 残り距離変更なし


	}break;
	 
	case PTN_FBSWAY_FULL:		//FB有のフルパターン
	{																			// 巻旋回位置＋位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し、減衰位相到達		

		// 巻位置待ち　巻き位置：巻目標高さ-Xm　以上になったら						
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_HST;									// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D_abs = D_abs;															// 残り距離変更なし


		// 旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し						
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_SLW;									// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D_abs = D_abs;															// 残り距離変更なし

		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_PH_SINGLE;								// 振れ位相待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		D_abs = D_abs;															// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 速度ステップ出力　2段分###*/
	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:	//FB無のフルパターン
	case PTN_FBSWAY_FULL:		//FB有のフルパターン		
	//台形パターン部 減速を２段階にして距離を調整
	{																			// 出力するノッチ速度を計算して設定
		
		double d_step = D_abs;													//ステップでの移動距離
		double v_top_abs=0.0;													//ステップ速度用
		double check_d_abs;
		double check_d_inch_abs;	//インチング距離
		int n=0, i;

		// #Step1-1 ２段構成になるときの１段目
		// まずは、移動距離が1周期振れ止めと出来るTop速度を求める
		for (i = (NOTCH_MAX - 1);i > 0;i--) {
			v_top_abs = pCraneStat->spec.notch_spd_f[id][i];
			n = (int)(D_abs / (v_top_abs * st_com_work.T));				//1周期振れ止めの移動距離　=nTV
			if (n) {
				pelement = &(precipe->steps[precipe->n_step++]);		//ステップのポインタセットして次ステップ用にカウントアップ
				pelement->type = CTR_TYPE_VOUT_POS;						//位置到達待ちステップ出力
				pelement->_t = (double)n  * st_com_work.T;				// n x 振れ周期

				double d_move_abs = (double)n * v_top_abs * st_com_work.T - 0.5 * v_top_abs * v_top_abs / st_com_work.a_abs[id];	// 台形移動距離 -減速距離
				if (precipe->direction == ID_REV) {
					pelement->_p = (pelement - 1)->_p - d_move_abs;					// 目標位置
					pelement->_v = -v_top_abs;										// 出力速度
				}
				else {
					pelement->_p = (pelement - 1)->_p + d_move_abs;					// 目標位置
					pelement->_v = v_top_abs;										// 出力速度
				}
				D_abs -= d_move_abs;												// 残り距離更新
				break;
			}
		}
		//  #Step1-2　2段目　残り移動距離が１周期移動距離以上あるとき2段目追加
		// ノッチ選択は続きから
		for (;i > 0;i--) {
			v_top_abs = pCraneStat->spec.notch_spd_f[id][i];
			n = (int)(D_abs / (v_top_abs * st_com_work.T));				//1周期振れ止めの移動距離　=nTV
			if (n) {
				pelement = &(precipe->steps[precipe->n_step++]);		//ステップのポインタセットして次ステップ用にカウントアップ
				pelement->type = CTR_TYPE_VOUT_POS;						//位置到達待ちステップ出力
				pelement->_t = (double)n * st_com_work.T;				// n x 振れ周期

				double d_move_abs = (double)n * v_top_abs * st_com_work.T ;	// 定速移動距離（振れ周期の整数倍移動）
				if (precipe->direction == ID_REV) {
					pelement->_p = (pelement - 1)->_p - d_move_abs;					// 目標位置
					pelement->_v = -v_top_abs;										// 出力速度
				}
				else {
					pelement->_p = (pelement - 1)->_p + d_move_abs;					// 目標位置
					pelement->_v = v_top_abs;										// 出力速度
				}
				D_abs -= d_move_abs;												// 残り距離更新
				break;
			}
		}

		//  #Step2 停止
		if (v_top_abs > 0.0) {
			pelement = &(precipe->steps[precipe->n_step++]);				//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_V;								//速度到達待ち
			pelement->_t = v_top_abs / st_com_work.a_abs[id];				//減速時間
			pelement->_v = 0.0;												// 速度0

			double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
			pelement->_p = (pelement - 1)->_p + d_move;						// 目標位置

			if (d_move < 0.0) D_abs += d_move;								// 残り距離更新
			else D_abs -= d_move;
		}
																			
	}break;

	//台形部無いケースはスキップ
	case PTN_NON_FBSWAY_2INCH:	//FB無のインチングパターン												
	case PTN_FBSWAY_AS:			//FB振れ止めパターン	
	{
		D_abs = D_abs;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  2回インチング移動部　###*/
	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:	//FB無のフルパターン	
	case PTN_NON_FBSWAY_2INCH:	//FB無のインチングパターン	
	{
		double v_inch = sqrt(0.5 * D_abs * st_com_work.a_abs[id]);
		double ta = v_inch/st_com_work.a_abs[id];
		double v_top_abs;
		for (int i = (NOTCH_MAX-1);i > 1;i--) {	
			v_top_abs = pCraneStat->spec.notch_spd_f[id][i];
			if (v_inch > pCraneStat->spec.notch_spd_f[id][i - 1])break;
			else continue;			//次のノッチへ
		}

		double d_move_abs = 0.5 * ta * v_inch;

		//STEP3
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		if (precipe->direction == ID_REV) {
			pelement->_p = (pelement - 1)->_p - d_move_abs;					// 目標位置
			pelement->_v = -v_top_abs;										// 出力速度
		}
		else {
			pelement->_p = (pelement - 1)->_p + d_move_abs;					// 目標位置
			pelement->_v = v_top_abs;										// 出力速度
		}
		D_abs -= d_move_abs;															

		//STEP4
		double tc = 0.5 * st_com_work.T - 2.0 * ta;
		if (tc < 0.0) {

			int n = (-tc / st_com_work.T)+1;
			tc = tc + (double)(n* st_com_work.T);
		} 
		
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta + tc;													// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		if (precipe->direction == ID_REV) {
			pelement->_p = (pelement - 1)->_p - d_move_abs;					// 目標位置
		}
		else {
			pelement->_p = (pelement - 1)->_p + d_move_abs;					// 目標位置
		}
		D_abs -= d_move_abs;

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		if (precipe->direction == ID_REV) {
			pelement->_p = (pelement - 1)->_p - d_move_abs;					// 目標位置
			pelement->_v = -v_top_abs;										// 出力速度
		}
		else {
			pelement->_p = (pelement - 1)->_p + d_move_abs;					// 目標位置
			pelement->_v = v_top_abs;										// 出力速度
		}
		D_abs -= d_move_abs;

		//STEP6
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D_abs = 0.0;

	}break;

	case PTN_FBSWAY_AS:															//振れFBあるパターンはスキップ
	case PTN_FBSWAY_FULL:														
	{
		D_abs = D_abs;																	//残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:	//FB無のフルパターン													
	case PTN_NON_FBSWAY_2INCH:	//FB無のインチングパターン	
																				{//微小位置決め
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FINE_POS;										// 微小位置決め
		pelement->_t = FINE_POS_TIMELIMIT;										// 位置合わせ最大継続時間
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// １ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D_abs = 0.0;															// 残り距離変更なし

	}break;

	case PTN_FBSWAY_AS:			//FB振止パターン	
	case PTN_FBSWAY_FULL:		//FB有のフルパターン	
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FB_SWAY_POS;									// FB振れ止め/位置決め
		pelement->_t = st_com_work.T * 4.0;										// 振れ4周期分
		pelement->_v = 0.0;														// 振れ止めロジックで決定
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		D_abs = 0;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	//時間条件のスキャンカウント値セット
	for (int i = 0;i < precipe->n_step;i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t / pwork->agent_scan);
	}


	return POLICY_PTN_OK;
}

/* # 旋回レシピ　*/
int CPolicy::set_recipe_semiauto_slw(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	//#レシピ条件セット
	//軸ID
	int id = precipe->axis_id = ID_SLEW;

	//移動方向
	precipe->direction = pwork->motion_dir[id];

	double dist_inch_max;
	if (pwork->vmax_abs[id] / pwork->a_abs[id] > pwork->T) {					//最大速度までの加速時間が振れ周期より大きい時は振れ周期分の加速時間がインチング最大距離
		dist_inch_max = pwork->a_abs[id] * pwork->T * pwork->T;
	}
	else {
		dist_inch_max = pwork->vmax_abs[id] * pwork->vmax_abs[id] / pwork->a_abs[id];		//最大速度までの加速時間が振れ周期より小さい時V^2/α
	}

	//作成パターンのタイプ   FBありなしと１回のインチングで移動可能な距離かで区別
	double D_abs = pwork->dist_for_target_abs[id];										//残り移動距離
	int ptn = 0;
	if (is_fbtype) {															//インチング最大距離の計算 移動距離がインチング最大距離より小さいとき,FB有はFB振れ止め、FB無しは2回インチング移動
		if (D_abs > dist_inch_max) ptn = PTN_FBSWAY_FULL;
		else ptn = PTN_FBSWAY_AS;
	}
	else {
		if (D_abs > dist_inch_max) ptn = PTN_NON_FBSWAY_FULL;
		else ptn = PTN_NON_FBSWAY_2INCH;
	}
	precipe->motion_type = ptn;

	LPST_MOTION_STEP pelement;


	//加速度が0.0はエラー　0割り防止
	if (pwork->a_abs[id] == 0.0) return POLICY_PTN_NG;


	/*### パターン作成 ###*/
	precipe->n_step = 0;														// ステップクリア

	/*### STEP0  待機　###*/

	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:		//FB無のフルパターン
	case PTN_NON_FBSWAY_2INCH:		//FB無のインチング
	case PTN_FBSWAY_AS:				//FB振止
	{																			// 巻、引込位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  引込：引き出し時は条件無し、引き込み時は引込位置が目標＋Xｍ以下
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_HST;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置現在位置
		CHelper::fit_slew_axis(&(pelement->_p));								//目標位置の校正（-180°～180°の表現にする
		D_abs = D_abs;																	// 残り距離変更なし

	}break;

	case PTN_FBSWAY_FULL: 			//FB有のフルパターン
	{																			// 巻旋回,位置＋位相待ち　巻き位置：巻目標高さ-Xm　以上になったら  引込：引き出し時は条件無し、引き込み時は引込位置が目標＋Xｍ以下
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_PH_SINGLE;								// 他軸位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));								// 目標位置の校正（-180°～180°の表現にする
		D_abs = D_abs;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP1,2 速度ステップ出力　1,2段 ###*/
	/*### 旋回ではFBパターンでは、振れ周期ステップパターンは無し、減速タイミングを調整し最後にFB振れ止めパターンを入れる ###*/

	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:												// 旋回はFBなしの時のみ振れ周期パターン
	{
		double d_step = D_abs;													//ステップでの移動距離
		double v_top = 0.0;													//ステップ速度用
		double check_d;
		int n = 0, i;

		// #Step1-1 １段目
		for (i = (NOTCH_MAX-1);i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];

			int k = (int)(v_top / st_com_work.a_abs[id] / st_com_work.T);					// k > 0 →　加速時間が振れ周期以上
			check_d = v_top * (1.0+ (double)k) * st_com_work.T;
			if (check_d == 0.0) break;

			n = int(d_step / check_d);
			if (n) break;			//一周期振れ止め距離以上有り
			else continue;			//次のノッチへ
		}

		if (n) {																		//一周期振れ止めの移動距離有でステップ追加
			pelement = &(precipe->steps[precipe->n_step++]);							//ステップのポインタセットして次ステップ用にカウントアップ
			pelement->type = CTR_TYPE_VOUT_POS;											//位置到達待ち

			double ta = v_top / st_com_work.a_abs[id];
			pelement->_t = (double)n * st_com_work.T - ta;								// 振れ周期　-　減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n * st_com_work.T - 0.5 * ta);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置
			CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

			D_abs -= d_move;																// 残り距離更新
		}

		//  #Step1-2２段目
		d_step = D_abs;
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

			double td = ((pelement - 1)->_v - v_top) / st_com_work.a_abs[id];				// 減速時間 ステップ速度までの減速時間
			pelement->_t = (double)n * st_com_work.T + td;								// 振れ周期　+　2段目までの減速時間

			pelement->_v = v_top;														// 速度0

			double d_move = v_top * ((double)n * st_com_work.T + td) + 0.5 * td * ((pelement - 1)->_v - v_top);
			pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;	// 目標位置
			CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

			D_abs -= d_move;																// 残り距離更新
		}

		//  #Step2 停止
		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
		pelement->_t = (pelement - 1)->_v / st_com_work.a_abs[id];							//減速時間
		pelement->_v = 0.0;																// 速度0
		double d_move = 0.5 * (pelement - 1)->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正

		D_abs -= d_move;																	// 残り距離更新
	}break;

	case PTN_FBSWAY_FULL:																// 旋回はFBありの時は振れ止め無し1段のみ：減速タイミングで調整
	{
		double d_step = D_abs;																//ステップでの移動距離
		double v_top = 0.0;																//ステップ速度用
		double check_d;
		int n = 0, i;

		// #Step1　１段目
		for (i = (NOTCH_MAX-1);i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * v_top / st_com_work.a_abs[id] + SPD_FB_DELAY_TIME * v_top;
			if (check_d < d_step) break;
			else continue;																//次のノッチへ
		}

		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_POS;												//位置到達待ち
		double ta = v_top / st_com_work.a_abs[id];
		double tc = (D_abs - v_top * ta) / v_top;
		pelement->_t = ta + tc;															// 振れ周期　-　減速時間
		pelement->_v = v_top;															// 速度0
		double d_move = v_top * (tc + 0.5 * ta);
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D_abs -= d_move;																	// 残り距離更新

		//  #Step2 停止
		pelement = &(precipe->steps[precipe->n_step++]);								//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_V;												//速度到達待ち
		pelement->_t = ta;																//減速時間
		pelement->_v = 0.0;																// 速度0
		pelement->_p = st_com_work.target.pos[id];											// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D_abs -= d_move;																	// 残り距離更新
	}break;

	case PTN_NON_FBSWAY_2INCH:													//台形部無いケースはスキップ
	case PTN_FBSWAY_AS:
	{
		D_abs = D_abs;
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP3,4,5,6  FB無しのインチング移動パターン　###*/
	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:													
	case PTN_NON_FBSWAY_2INCH:
	{
		double v_inch = sqrt(0.5 * D_abs * st_com_work.a_abs[id]);
		double ta = v_inch / st_com_work.a_abs[id];
		double v_top;
		for (int i = (NOTCH_MAX-1);i > 1;i--) {
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
		D_abs -= d_move;

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
		D_abs -= d_move;

		//STEP5
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 加速時間
		pelement->_v = v_top;													// ノッチ速度
		pelement->_p = (pelement - 1)->_p + (double)st_com_work.motion_dir[id] * d_move;// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D_abs -= d_move;

		//STEP6
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_VOUT_TIME;									// 加速用速度出力
		pelement->_t = ta;														// 位相待ち停止時間
		pelement->_v = 0.0;														// ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));								//目標位置の校正
		D_abs -= d_move;

	}break;

	case PTN_FBSWAY_AS:															//振れFBあるパターンはスキップ
	case PTN_FBSWAY_FULL:
	{
		D_abs = D_abs;																	//残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	/*### STEP7  ###*/
	switch (ptn) {
	case PTN_NON_FBSWAY_FULL:													//巻、旋回位置待ち　巻き位置：巻目標高さ-Xm　以上になったら  旋回：引き出し時は目標までの距離がX度以下、引き込み時は条件無し
	case PTN_NON_FBSWAY_2INCH:
	{
		pelement = &(precipe->steps[precipe->n_step++]);						// ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_FINE_POS;										// 微小位置決め
		pelement->_t = FINE_POS_TIMELIMIT;										// 位置合わせ最大継続時間
		pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];				// １ノッチ速度
		pelement->_p = st_com_work.target.pos[id];								// 目標位置
		CHelper::fit_slew_axis(&(pelement->_p));										//目標位置の校正
		D_abs = 0;																	// 残り距離変更なし

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
		D_abs = 0;																	// 残り距離変更なし
	}break;
	default:return POLICY_PTN_NG;
	}

	//時間条件のスキャンカウント値セット
	for (int i = 0;i < precipe->n_step;i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t / pwork->agent_scan);
	}


	return POLICY_PTN_OK;
}

/* # 巻レシピ　*/
int CPolicy::set_recipe_semiauto_mh(int jobtype, LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_POLICY_WORK pwork) {

	//#レシピ条件セット
	int id = precipe->axis_id = ID_HOIST;			//軸ID
	precipe->n_step = 0;							//ステップ数初期化
	precipe->direction = pwork->motion_dir[id];		//移動方向
	precipe->time_limit = 120000/inf.cycle_ms;		//タイムオーバーカウント
	precipe->motion_type = PTN_ORDINARY;			//作成パターンのタイプ


	//#パターン計算用データセット
	double D = pwork->dist_for_target[id];			//残り移動距 絶対値								
	if (pwork->a_abs[id] == 0.0) return POLICY_PTN_NG;	//加速度が0.0はエラー　0割り防止


	/*### パターン作成 ###*/
	LPST_MOTION_STEP pelement;

	/*### STEP0  待機　###	引込、旋回位置待ち　巻上時：条件無し　巻下時： 引込・旋回共が目標位置の指定範囲内 */
	if (precipe->direction == ID_REV) {											//巻き下げ時は、旋回、引込が目標付近着まで待機
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_SLW;									//旋回位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置＝現在位置
		D = D;																	// 残り距離変更なし

		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_POS_BH;									// 引込位置待ち
		pelement->_t = TIME_LIMIT_ERROR_DETECT;									// タイムオーバーリミット値
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置＝現在位置
		D = D;																	// 残り距離変更なし
	}
	else {																		//停止または巻き上げ時は、確認待ち
		pelement = &(precipe->steps[precipe->n_step++]);						//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->type = CTR_TYPE_WAIT_TIME;									// 待機時間待ち
		pelement->_t = TIME_LIMIT_CONFIRMATION;									// 待ち時間
		pelement->_v = 0.0;														// 速度0
		pelement->_p = pwork->pos[id];											// 目標位置＝現在位置
		D = D;																	// 残り距離変更なし
	}

	/*### STEP1 速度ステップ出力　###*/

	double v_top = 0.0;																	//ステップ速度用
	double check_d;
	int n = 0, i;

	pelement = &(precipe->steps[precipe->n_step++]);									//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_VOUT_POS;													//位置到達待ち定速出力

	double ta, tc;//加速時間,定速時間
	
	if (precipe->direction >= ID_FWD) {														//巻上時
		//ノッチ速度選択
		for (i = (NOTCH_MAX - 1);i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_f[id][i];
			check_d = v_top * v_top / st_com_work.a_abs[id] + SPD_FB_DELAY_TIME * v_top;	//インチング距離 → V^2/α　+　V*遅れ時間
			if (check_d < D) break;														//ノッチ速度確定
			else continue;																//次のノッチへ
		}
	}
	else {												//巻下時
		//ノッチ速度選択
		for (i = (NOTCH_MAX - 1);i > 0;i--) {
			v_top = pCraneStat->spec.notch_spd_r[id][i];
			check_d = v_top * v_top / st_com_work.a_abs[id] + SPD_FB_DELAY_TIME * -v_top;	//巻き下げはv_top < 0.0
			if (check_d < D) break;														//ノッチ速度確定
			else continue;																//次のノッチへ
		}
	}

	ta = v_top / st_com_work.a_abs[id]; 													//加速度は正の値
	if (ta < 0.0) ta *= -1.0;

	double dd = SPD_FB_DELAY_TIME * v_top; if (dd < 0.0)dd *= -1.0;
	tc = (D - v_top * v_top / st_com_work.a_abs[id] / 2.0 - dd)/ v_top;						//(残り距離 - インチング距離)/トップ速度
	if (tc < 0.0) tc *= -1.0;

	pelement->_t = ta + tc;																// 加速時間　＋定速時間
	pelement->_v = v_top;																// 速度

	double d4stop = v_top * (SPD_FB_DELAY_TIME + 0.5 * ta);								//停止移動距離
	pelement->_p = pwork->target.pos[ID_HOIST] -  d4stop;								// 目標位置　ターゲット位置-減速距離

	if(d4stop <0.0) D = -d4stop;
	else			D = -d4stop;// 残り距離更新	

	/*### STEP2 停止　###*/
	pelement = &(precipe->steps[precipe->n_step++]);									//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_VOUT_V;													//速度到達待ち
	pelement->_t = ta;																	//減速時間
	pelement->_v = 0.0;																	// 速度0
	pelement->_p = st_com_work.target.pos[id];											// 目標位置
	D = 0.0;																			// 残り距離更新

	/*### STEP3 位置合わせ　###*/
	pelement = &(precipe->steps[precipe->n_step++]);									// ステップのポインタセットして次ステップ用にカウントアップ
	pelement->type = CTR_TYPE_FINE_POS;													// 微小位置決め
	pelement->_t = FINE_POS_TIMELIMIT;													// 位置合わせ最大継続時間
	pelement->_v = pCraneStat->spec.notch_spd_f[id][NOTCH_1];							// １ノッチ速度
	pelement->_p = st_com_work.target.pos[id];											// 目標位置
	D = 0.0;																				// 残り距離変更なし


	//時間条件のスキャンカウント値セット
	for (int i = 0;i < precipe->n_step;i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t / pwork->agent_scan);
	}

	return POLICY_PTN_OK;
}


/****************************************************************************/
/*　　コマンドパターン計算用の素材データ計算,セット									*/
/*　　目標位置,目標までの距離,最大速度,加速度,加速時間,加速振れ中心,振れ振幅*/
/****************************************************************************/
LPST_POLICY_WORK CPolicy::set_com_workbuf(ST_POS_TARGETS target) {

	st_com_work.agent_scan_ms = pAgent->inf.cycle_ms;				//AGENTタスクのスキャンタイム
	st_com_work.agent_scan = 0.001 * (double)st_com_work.agent_scan_ms;
	st_com_work.target = target;									//目標位置

	double temp_d;
	for (int i = 0; i < MOTION_ID_MAX; i++) {
		//現在位置
		st_com_work.pos[i] = pPLC_IO->status.pos[i];
		//現在速度
		st_com_work.v[i] = pPLC_IO->status.v_fb[i];
		//移動距離　方向
		st_com_work.dist_for_target[i] = st_com_work.target.pos[i] - st_com_work.pos[i];
		if (i == ID_SLEW) {		//旋回は、180を越えるときは逆方向が近い
			if (st_com_work.dist_for_target[ID_SLEW] > PI180)	st_com_work.dist_for_target[ID_SLEW] -= PI360;
			else if (st_com_work.dist_for_target[ID_SLEW] < -PI180) st_com_work.dist_for_target[ID_SLEW] += PI360;
			else;
		}
	
		if (st_com_work.dist_for_target[i] < 0.0) {
			st_com_work.motion_dir[i] = ID_REV;
			st_com_work.dist_for_target_abs[i] = -1.0 * st_com_work.dist_for_target[i];
		}
		else if (st_com_work.dist_for_target[i] > 0.0) {
			st_com_work.motion_dir[i] = ID_FWD;
			st_com_work.dist_for_target_abs[i] = st_com_work.dist_for_target[i];
		}
		else {
			st_com_work.motion_dir[i] = ID_STOP;
			st_com_work.dist_for_target_abs[i] = 0.0;
		}


		//動作軸加速度
		st_com_work.a_abs[i] = pCraneStat->spec.accdec[i][FWD][ACC];

		//最大速度
		st_com_work.vmax_abs[i] = pCraneStat->spec.notch_spd_f[i][NOTCH_MAX - 1];

		//最大加速時間
		st_com_work.acc_time2Vmax[i] = st_com_work.vmax_abs[i] / st_com_work.a_abs[i];

		if ((i == ID_BOOM_H) || (i == ID_SLEW)) {
			//吊点の加速度
			st_com_work.a_hp_abs[i] = pEnvironment->cal_hp_acc(i, st_com_work.motion_dir[i]);
			if (st_com_work.a_hp_abs[i] < 0.0) st_com_work.a_hp_abs[i] *= -1.0;

			//加速時振れ中心
			st_com_work.pp_th0[i][ACC] = pEnvironment->cal_arad_acc(i, FWD);
			//減速時振れ中心
			st_com_work.pp_th0[i][DEC] = pEnvironment->cal_arad_dec(i, REV);
		}
	}

	//巻きの目標位置が上の時は、巻上後に旋回引き込み動作をするので目標位置の周期でパターンを作る
	if (target.pos[ID_HOIST] > st_com_work.pos[ID_HOIST]) {
		st_com_work.T = pEnvironment->cal_T(target.pos[ID_HOIST]);								//振れ周期
		st_com_work.w = pEnvironment->cal_w(target.pos[ID_HOIST]);								//振れ角周波数
		st_com_work.w2 = pEnvironment->cal_w2(target.pos[ID_HOIST]);							//振れ角周波数2乗
	}
	else {
		st_com_work.T = pCraneStat->T;															//振れ周期
		st_com_work.w = pCraneStat->w;															//振れ角周波数
		st_com_work.w2 = pCraneStat->w2;
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


