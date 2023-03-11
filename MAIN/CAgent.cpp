#include "CAgent.h"
#include "CPolicy.h"
#include "CEnvironment.h"

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

static CPolicy* pPolicy;
static CEnvironment* pEnv;

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
	pCSInf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pAgentInf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap());
	pSway_IO = (LPST_SWAY_IO)(pSwayIO_Obj->get_pMap());

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnv = (CEnvironment*)VectpCTaskObj[g_itask.environment];
	
	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;

	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;

	set_panel_tip_txt();

	inf.is_init_complete = true;
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

//定周期処理手順1　外部信号入力加工処理
void CAgent::input() {
	
	//ジョブリストのチェック →　コマンドの取り込み
	if (can_job_trigger()) {														//ジョブ可否判定
		if (pCSInf->job_list.hot_job_status & STAT_STANDBY) {						//実行待ちジョブあり
			pCom = pPolicy->req_command();											//コマンド取り込み
			if (pCom != NULL) {														//コマンドステータス初期化											
				AgentInf_workbuf.auto_on_going = pCom->type;						//JOB or SEMIAUTO
				startup_command(pCom);												//取り込んだコマンド実行用変数を初期化してステータスを実行中にする
			}
		}
	}



	return;

};

//定周期処理手順2　メイン処理
void CAgent::main_proc() {

	//実行中の自動モードセット
	if ((pCSInf->antisway_mode == false) && (pCSInf->auto_mode == false)) {	//自動OFF　振れ止めモードOFF
		AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	}
	//j実行中コマンド有
	else if (pCom != NULL) {												//実行中コマンド有
		if (AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) {
			if (pCSInf->antisway_mode == L_ON)
				AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;
			else
				AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
		}

		if(pCom->type == AUTO_TYPE_JOB)		AgentInf_workbuf.auto_on_going |= AUTO_TYPE_JOB;
	    else								AgentInf_workbuf.auto_on_going |= AUTO_TYPE_SEMIAUTO;
	}
	//振れ止め未完状態
	else if (AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) {
		if (pCSInf->antisway_mode == L_ON)
			AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;
		else
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
	}
	else {
		AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	}

	update_motion_setting();				//PLCがPC指令で動作する軸の選択,
	update_auto_control();					//自動実行モードの設定と自動レシピのセット

	//PLCへの出力計算
	set_ref_mh();							//巻き速度指令
	set_ref_gt();							//走行速度指令
	set_ref_slew();							//旋回速度指令
	set_ref_bh();							//引込速度指令
	update_pb_lamp_com();					//PB LAMP出力

	return;

}

//定周期処理手順3　信号出力処理
/****************************************************************************/
/*   信号出力	共有メモリ出力												*/
/****************************************************************************/
void CAgent::output() {

	//自動関連報告処理
	

	//共有メモリ出力処理
	memcpy_s(pAgentInf, sizeof(ST_AGENT_INFO), &AgentInf_workbuf, sizeof(ST_AGENT_INFO));

	//タスクパネルへの表示出力
	wostrs << L" #SL TG:" << fixed<<setprecision(3) << AgentInf_workbuf.auto_pos_target.pos[ID_SLEW];
	wostrs << L",GAP: " << pEnv->cal_dist4target(ID_SLEW,false);

	wostrs << L"#BH TG: " << AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H];
	wostrs << L",GAP: " << pEnv->cal_dist4target(ID_BOOM_H, false);

	wostrs << L",ActiveSet: " << dbg_mont[0];
	
	wostrs <<  L" --Scan " << inf.period;;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   JOB関連処理															*/
/****************************************************************************/
//ジョブの起動可否判定
bool CAgent::can_job_trigger() { 
	if (AgentInf_workbuf.auto_on_going | CODE_TYPE_JOB) return false;
	return true; 
}

//コマンド完了判定
bool CAgent::is_command_completed(LPST_COMMAND_BLOCK pCom) {

	bool ans = true;
	
	if (pCom == NULL) return true;
	
	for (int i = 0;i < MOTION_ID_MAX;i++) {
		if (pCom->is_active_axis[i] == true) {
			if((pCom->motion_stat[i].status & MOTION_COMPLETE)) ans = false;
			break;
		}
	}

	return ans;
}

/****************************************************************************/
/*　　PC制御選択セット処理													*/
/****************************************************************************/
int CAgent::update_motion_setting() {
	/*

		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
	*/

	if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
	}
	else if((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO)||(AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
	}
	else if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
	}
	else {
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
	}

	AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;

	//PLCへのPC選択指令セット
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE) {
		AgentInf_workbuf.pc_ctrl_mode |= (BITSEL_HOIST | BITSEL_GANTRY | BITSEL_BOOM_H | BITSEL_SLEW);
	}
	else {
		AgentInf_workbuf.pc_ctrl_mode = 0;
		if (AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_HOIST;
		if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_GANTRY;
		if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_BOOM_H;
		if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_SLEW;
	}

	return 0;
}

/****************************************************************************/
/*	自動関連設定					*/
/****************************************************************************/
//振れ止め完了判定
int CAgent::check_as_completion() {

	int check = AS_COMPLETE_0;

	//振れ振幅、位置ずれともに完了レベル以内
	if ((pEnv->cal_sway_r_amp2_m() < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
		&& (pEnv->cal_dist4target(ID_BOOM_H, true) < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_COMPLE]))

		check |= AS_COMPLETE_BH;


	if ((pEnv->cal_sway_th_amp2_m() < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
		&& (pEnv->cal_dist4target(ID_SLEW, true) < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_COMPLE]))

		check |= AS_COMPLETE_SLEW;


	//振れ振幅、位置ずれともに起動判定レベル以内　かつ　振れ止め完了フラグON
	if (pAgentInf->antisway_comple_status | AS_COMPLETE_BH) {
		if ((pEnv->cal_sway_r_amp2_m() < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_TRIGGER])
			&& (pEnv->cal_dist4target(ID_BOOM_H, true) < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_TRIGGER]))

			check |= AS_COMPLETE_BH;
	}

	if (pAgentInf->antisway_comple_status | AS_COMPLETE_SLEW) {
		if ((pEnv->cal_sway_th_amp2_m() < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_TRIGGER])
			&& (pEnv->cal_dist4target(ID_SLEW, true) < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_TRIGGER]))

			check |= AS_COMPLETE_SLEW;
	}


	pAgentInf->antisway_comple_status = check;

	return check;
}

int CAgent::set_receipe_as_bh(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};
int CAgent::set_receipe_as_slw(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};

void CAgent::set_as_workbuf() {
	return;;
}

//振れ止めコマンドセット
int CAgent::setup_as_command() {

	pAgentInf->comset_as.is_active_axis[ID_HOIST] = false;
	pAgentInf->comset_as.is_active_axis[ID_SLEW] = true;
	pAgentInf->comset_as.is_active_axis[ID_BOOM_H] = true;

	set_as_workbuf(); //振れ止めパターン作成用データ取り込み

	set_receipe_as_bh(&(AgentInf_workbuf.comset_as.recipe[ID_BOOM_H]), true, &st_as_work);
	set_receipe_as_slw(&(AgentInf_workbuf.comset_as.recipe[ID_SLEW]), true, &st_as_work);

	return 0;
}


//自動関連各種設定・振れ止めレシピ設定
int CAgent::update_auto_control() {

	dbg_mont[0] = 0;//@@@ debug/

	/*### 振れ止め完了判定　###*/
	AgentInf_workbuf.antisway_comple_status = check_as_completion();
					
	/*### 目標位置設定　###*/

	//自動（振れ止め）モードでない
	if ((pCSInf->auto_mode == false) && (pCSInf->antisway_mode == false)){
		//目標位置＝現在位置
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			AgentInf_workbuf.auto_pos_target.pos[i] = pPLC_IO->status.pos[i];
			AgentInf_workbuf.auto_pos_target.is_held[i] = false;
		}
	}
	//自動実行　マニュアルモード
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
		//目標位置保持フラグ無し＝現在位置　目標位置保持フラグあり＝目標位置変更なし現在位置
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
			//ノッチ入りで目標保持フラグクリア
			if(pCraneStat->is_notch_0[i] == false)	AgentInf_workbuf.auto_pos_target.is_held[i] = false;

			//目標位置保持フラグ切りで現在位置
			if(AgentInf_workbuf.auto_pos_target.is_held[i] == false) AgentInf_workbuf.auto_pos_target.pos[i] = pPLC_IO->status.pos[i];

		}
	}
	//自動実行　振れ止め実行、ジョブ実行時は変更なし
	else {
		;//コマンド起動時に目標位置と目標位置保持フラグを設定する。
	}

	/*### 振れ止めコマンド設定　###*/
	
	if (pCSInf->antisway_mode == L_ON){	//振れ止めモード
		//振れ止め未完で振れ止めコマンドが実行中でなければ、振れ止めコマンド設定
		if((AgentInf_workbuf.antisway_comple_status != AS_ALL_COMPLETE) &&  (AgentInf_workbuf.comset_as.com_status == STAT_REQ_WAIT)){	//振れ止め用コマンドセットが要求待ち
				setup_as_command();
		}
	}

	return 0;
};


/****************************************************************************/
/*   巻指令出力処理		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_HOIST) {
		if (AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
		else if ((AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_SEMIAUTO)) {
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_HOIST] = cal_step(pCom, ID_HOIST);
		}
		else
			AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
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
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_GANTRY) {
		if (AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_GANTRY] = pCraneStat->notch_spd_ref[ID_GANTRY];
		else if ((AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_GANTRY] == AUTO_TYPE_SEMIAUTO)) {
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_GANTRY] = cal_step(pCom, ID_GANTRY);
		}

		else
			AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
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

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_SLEW) {										//制御PC指令選択ON
		if (AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_MANUAL)							//マニュアルモード
			AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];

		else if((AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_JOB)||
				(AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_SEMIAUTO)){					//自動運転中
			if(pCom == NULL)	AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pCom, ID_SLEW);
		}
		else if (AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_FB_ANTI_SWAY){				//振れ止め中
			if (pAgentInf->antisway_comple_status | AS_COMPLETE_SLEW)								//振れ止め完了
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else																					//振れ止め未完
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(&pAgentInf->comset_as, ID_SLEW);	
		}
		else {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
		} 
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

	LPST_COMMAND_BLOCK pcom;

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_BOOM_H) {									//制御PC指令選択ON
		
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL)						//マニュアルモード
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];

		else if ((AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_SEMIAUTO)) {					//自動運転中
			if (pCom == NULL)	AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom, ID_BOOM_H);
		}
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_FB_ANTI_SWAY) {			//振れ止め中
			if(pAgentInf->antisway_comple_status | AS_COMPLETE_BH )									//振れ止め完了
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else 																					//振れ止め未完
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(&pAgentInf->comset_as, ID_BOOM_H);
		}
		else {
			AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
		}
	}
	else {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
	}

	return 0;
}

/****************************************************************************/
/*  コマンドブロック実行前初期化処理                                        */
/*  実行管理ステータスのクリアとコマンド実行中ステータスセット				*/
/****************************************************************************/
int CAgent::startup_command(LPST_COMMAND_BLOCK pcom) {

	for (int i = 0; i < MOTION_ID_MAX;i++) {						//各軸の実行ステータスの初期化
		if (AgentInf_workbuf.auto_active[i] == AUTO_TYPE_MANUAL) {
			pcom->motion_stat[i].status = MOTION_COMPLETE;
		}
		else {
			if (pCom->is_active_axis[i] == true) {
				pcom->motion_stat[i].status = MOTION_STANDBY;
				pcom->motion_stat[i].iAct = 0;
				pcom->motion_stat[i].step_act_count = 0;
				pcom->motion_stat[i].elapsed = 0;
				pcom->motion_stat[i].error_code = 0;

				for (int k = 0;k < pcom->recipe[i].n_step;k++) {
					pcom->recipe[i].steps[k].status = STEP_STANDBY;
					pcom->recipe[i].steps[k].time_count = 0;
				}
			}
			else pcom->motion_stat[i].status = MOTION_COMPLETE;
		}
	}
	pcom->com_status = STAT_ACTIVE;									//コマンドステータス実行中へ
	GetLocalTime(&(pcom->time_start));								//開始時間セット

	return 0;
}

/****************************************************************************/
/*   STEP処理		                                                        */
/****************************************************************************/
double CAgent::cal_step(LPST_COMMAND_BLOCK pCom,int motion) {


	double v_out = 0.0;

	LPST_MOTION_RECIPE precipe = &pCom->recipe[motion];
	LPST_MOTION_STAT pmotion_stat = &pCom->motion_stat[motion];
	LPST_MOTION_STEP pStep = &precipe->steps[pmotion_stat->iAct];


	if (pmotion_stat->status & MOTION_COMPLETE) {		 //レシピ出力完了済
		return 0.0;
	}
	else if (pStep->status == STAT_STANDBY) {   //ステップ起動時
		pmotion_stat->step_act_count= 1;
		pStep->status = STAT_ACTIVE;			//ステップ実行中ステップ出力中に更新
	}
	else	pmotion_stat->step_act_count++;			//ステップ実行カウントインクリメント



	switch (pStep->type) {
#if 0
		//#	時間待機
	case CTR_TYPE_TIME_WAIT: {
		pstat->direction = AGENT_STOP;
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = pStep->_v;
	}break;
		//#	振れ位相待ち２方向
	case CTR_TYPE_DOUBLE_PHASE_WAIT: {
		pstat->direction = AGENT_NA;	//完了まで移動方向未定
		bool is_step_end = false;
		v_out = pStep->_v;

		//異常完了判定
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//指定範囲外　-π～π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//指定範囲外　-π～π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
			pStep->status = PTN_STEP_FIN;is_step_end = true;
		}
		if (is_step_end) {
			pstat->direction = AGENT_STOP;
			v_out = 0.0;
			break;
		}


		//メイン判定
		//正転開始用位相判定
		double chk_ph = PI360;
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_F] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if (pStep->opt_d[MOTHION_OPT_PHASE_F] > pSway_IO->ph[motion]) {
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F];
		}

		if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_FWD;
			if (motion == ID_SLEW)
				v_out += 0.0;
			break;
		}

		//逆転開始用位相判定
		chk_ph = PI360;
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_R] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];	
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if(pStep->opt_d[MOTHION_OPT_PHASE_R] > pSway_IO->ph[motion]){
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R];
		}
		if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_REW;
			if (motion == ID_SLEW)
				v_out += 0.0;
			break;
		}

	}break;
	//#	振れ位相待ち1方向
	case CTR_TYPE_SINGLE_PHASE_WAIT: {

		bool is_step_end = false;
		v_out = pStep->_v;

		//異常完了判定
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//指定範囲外　-π～π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//指定範囲外　-π～π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
			pStep->status = PTN_STEP_FIN;is_step_end = true;
		}
		if (is_step_end) {
			pstat->direction = AGENT_STOP;
			v_out = 0.0;
			break;
		}


		//メイン判定
		//開始用位相判定
		int dir;
		double target_ph;

		if (motion == ID_SLEW) {
			if ((pAgentInf->pos_target[motion] - pPLC_IO->status.pos[motion]) < -PI180) {
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else if ((pAgentInf->pos_target[motion] - pPLC_IO->status.pos[motion]) > PI180) {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
			else if (pPLC_IO->status.pos[motion] < pAgentInf->pos_target[motion]) {
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
		}
		else {
			if (pPLC_IO->status.pos[motion] < pAgentInf->pos_target[motion]) {//目標位置より手前
				dir = AGENT_FWD;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_F];
			}
			else {
				dir = AGENT_REW;
				target_ph = pStep->opt_d[MOTHION_OPT_PHASE_R];
			}
		}
		
		double chk_ph = PI360;
		if ((target_ph <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
			chk_ph = pSway_IO->ph[motion] - target_ph;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((target_ph >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
			chk_ph = target_ph - pSway_IO->ph[motion];
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if (target_ph > pSway_IO->ph[motion]) {
			chk_ph = target_ph - pSway_IO->ph[motion];
		}
		else {
			chk_ph = pSway_IO->ph[motion] - target_ph;
		}

		if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = PTN_STEP_FIN;
			pstat->direction = dir;
			break;
		}

	}break;

	//#	振れ止め移動起動タイミング調整（初期振れが大きいとき加速開始タイミングを調整）
	case CTR_TYPE_ADJUST_MOTION_TRIGGER: {
		v_out = 0.0;
	//	double rad_acc = pEnv->cal_arad_acc(motion, FWD);		//加速振れ角
	//	double rad_acc2 =pEnv-> rad_acc * rad_acc;				//加速振れ角２乗
		double rad_acc2 =pEnv->cal_arad2(motion,FWD);			//加速振れ角２乗

		if (sqrt(pSway_IO->rad_amp2[motion]) > rad_acc2) { // 振れ角振幅が加速振れ角より大きい
			if (AgentInf_workbuf.gap_from_target[motion] > 0) {//目標位置より現在位置が手前→進行方向＋
				if ((pSway_IO->ph[motion] < PI180) && (pSway_IO->ph[motion] > PI90))
					pStep->status = PTN_STEP_FIN;
			}
			else {
				if ((pSway_IO->ph[motion] < 0.0) && (pSway_IO->ph[motion] > -PI90))
					pStep->status = PTN_STEP_FIN;
			}
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_ON;
		}
		else if (AgentInf_workbuf.sway_amp2m[motion] > AGENT_CHECK_LARGE_SWAY_m2) {
			if (AgentInf_workbuf.gap_from_target[motion] > 0) {//目標位置より現在位置が手前→進行方向＋
				if ((pSway_IO->ph[motion] < PI90) && (pSway_IO->ph[motion] > 0.0))
					pStep->status = PTN_STEP_FIN;
			}
			else {
				if ((pSway_IO->ph[motion] < -PI90) && (pSway_IO->ph[motion] > -PI180))
					pStep->status = PTN_STEP_FIN;
			}
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_ON;
		}
		else {
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_OFF;
		}

		//タイムオーバー
		if (pstat->step_act_count >= pStep->time_count) {
			pstat->flg[MOTION_ACC_STEP_BYPASS] = L_OFF;
			pStep->status = PTN_STEP_FIN;
		}

	}break;

		//#	振れ止め加速
	case CTR_TYPE_ACC_AS: {
		if (pstat->step_act_count >= pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = (double)pstat->direction * pStep->_v;

	}break;

	case CTR_TYPE_ACC_V: {
		if (pstat->step_act_count >= pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = pStep->_v;

	}break;
	//#	減速
	case  CTR_TYPE_DEC_V: {
		if (pStep->_v < 0.0) {
			if (pPLC_IO->status.v_fb[motion] >= pStep->_v) pStep->status = PTN_STEP_FIN;
		}
		else if (pStep->_v > 0.0) {
			if (pPLC_IO->status.v_fb[motion] <= pStep->_v) pStep->status = PTN_STEP_FIN;
		}
		else;
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;

	}break;
	//#	定速（加減速→定速）
	case CTR_TYPE_CONST_V_TIME: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;
	}break;
	//#	定速（加減速→定速）
	case CTR_TYPE_CONST_V_ACC_STEP:
	{
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if(pstat->flg[MOTION_ACC_STEP_BYPASS] == L_ON )	//初期振れが大きいとき1回の加速にするフラグ
			pStep->status = PTN_STEP_FIN;
		v_out = pStep->_v;
	}break;
	case CTR_TYPE_CONST_V_DEC_STEP:
	{
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pstat->flg[MOTION_DEC_STEP_BYPASS] == L_ON)	//初期振れが大きいとき1回の減速にするフラグ
			pStep->status = PTN_STEP_FIN;
		v_out = pStep->_v;
	}break;

	//#	定速（加減速→定速）
	case CTR_TYPE_CONST_V_TOP_STEP:
	{
		double chk_d = pStep->_p - pPLC_IO->status.pos[motion];
		if (chk_d < 0.0) chk_d *= -1.0; //STEP目標位置までの距離

		if ((pStep->_v >= 0.0) && (pPLC_IO->status.pos[motion] >= pStep->_p)) {			//前進　目標位置到達
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if ((pStep->_v <= 0.0) && (pPLC_IO->status.pos[motion] <= pStep->_p)) {	//後進　目標位置到達
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if (chk_d < AgentInf_workbuf.dist_for_stop[motion]) {						// STEP目標位置までの距離が減速距離以下

			double rad_acc2 = pEnv->cal_arad2(motion, FWD);	//加速振れ角2乗
			if (pEnv->is_sway_larger_than_accsway(motion)) {							// 振れが加速振れより大
				if (AgentInf_workbuf.gap_from_target[motion] > 0) {						//目標位置より現在位置が手前→進行方向＋
					if ((pSway_IO->ph[motion] < PI180) && (pSway_IO->ph[motion] > PI90))//π/2-πの位相なら完了->減速に入る
						pStep->status = PTN_STEP_FIN;
				}
				else {																	//目標位置より現在位置が後ろ→進行方向-
					if ((pSway_IO->ph[motion] < 0.0) && (pSway_IO->ph[motion] > -PI90))	//0- -π/2の位相なら完了->減速に入る
						pStep->status = PTN_STEP_FIN;
				}
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_ON;
			}
			else if (AgentInf_workbuf.sway_amp2m[motion] > AGENT_CHECK_LARGE_SWAY_m2) {
				if (AgentInf_workbuf.gap_from_target[motion] > 0) {//目標位置より現在位置が手前→進行方向＋
					if ((pSway_IO->ph[motion] < PI90) && (pSway_IO->ph[motion] > 0.0))
						pStep->status = PTN_STEP_FIN;
				}
				else {
					if ((pSway_IO->ph[motion] < -PI90) && (pSway_IO->ph[motion] > -PI180))
						pStep->status = PTN_STEP_FIN;
				}
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_ON;
			}
			else;
		}
		else {
			if (pstat->step_act_count >= pStep->time_count) {
				pStep->status = PTN_STEP_FIN;
				pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
			}
		}

		v_out = pStep->_v;

	}break;

	//#	定速（加減速→定速）
	case CTR_TYPE_FINE_POSITION: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pAgentInf->gap2_from_target[motion] < pCraneStat->spec.as_pos2_level[motion][ID_LV_COMPLE])
			pStep->status = PTN_STEP_FIN;

		double dir = 0.0;
		if (pPLC_IO->status.pos[motion] > AgentInf_workbuf.pos_target[motion]) dir = -1.0;
		else dir = 1.0;

		v_out = (double)dir * pStep->_v;
	}break;

	//#	相手軸の位置待ち
	case CTR_TYPE_OTHER_POS_WAIT: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (motion == ID_SLEW) {
			if (pAgentInf->gap2_from_target[ID_BOOM_H] < pStep->opt_d[MOTHION_OPT_WAIT_POS])
				pStep->status = PTN_STEP_FIN;
		}
		else if (motion == ID_BOOM_H) {
			if (pAgentInf->gap2_from_target[ID_SLEW] < pStep->opt_d[MOTHION_OPT_WAIT_POS])
				pStep->status = PTN_STEP_FIN;
		}
		else {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = 0.0;

	}break;
#endif
	default:
		v_out = 0.0;
		break;
}

	if (pStep->status & STEP_FIN) {
		pmotion_stat->iAct++;
		if (pmotion_stat->iAct >= precipe->n_step) pmotion_stat->status = MOTION_COMPLETE;
	}

	return v_out;
}

/****************************************************************************/
/*  PB指令更新	(非常停止,主幹PB他）										*/
/****************************************************************************/

void CAgent::update_pb_lamp_com() {
	//操作PB(取り敢えずPLC入力値取り込み（PLC IOにてOFF DELAY組み込み済）
	AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] = pPLC_IO->ui.PB[ID_PB_ESTOP];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_ON];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE_OFF];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_ON];
	AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] = pPLC_IO->ui.PB[ID_PB_CTRL_SOURCE2_OFF];
	AgentInf_workbuf.PLC_PB_com[ID_PB_FAULT_RESET] = pPLC_IO->ui.PB[ID_PB_FAULT_RESET];
	
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

