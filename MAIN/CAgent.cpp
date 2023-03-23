#include "CAgent.h"
#include "CPolicy.h"
#include "CEnvironment.h"
#include "CClientService.h"

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

static CClientService* pCS;
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
	pCS = (CClientService*)VectpCTaskObj[g_itask.client];
	
	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;

	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	pjob_active = NULL;

	//軸ステータスクリア
	for (int i = 0;i < MOTION_ID_MAX;i++) {
		AgentInf_workbuf.axis_status[i] = 0;
	}

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

/****************************************************************************/
/*  定周期処理手順1 信号入力　外部信号入力加工処理　モード管理				*/
/****************************************************************************/
void CAgent::input() {
	
	//# 各軸条件設定（FB0,AUTO_ENABLE...)
	{
		//PC指令可設定
		for (int i = 0;i < MOTION_ID_MAX;i++) {
			AgentInf_workbuf.axis_status[i]		|= AG_AXIS_STAT_PC_ENABLE;
		}

		//自動実行可設定
		AgentInf_workbuf.axis_status[ID_HOIST]	|= AG_AXIS_STAT_AUTO_ENABLE;
		AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_AUTO_ENABLE;
		AgentInf_workbuf.axis_status[ID_SLEW]	|= AG_AXIS_STAT_AUTO_ENABLE;

		//振れ止め自動実行可設定
		AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_ANTISWAY_ENABLE;
		AgentInf_workbuf.axis_status[ID_SLEW]	|= AG_AXIS_STAT_ANTISWAY_ENABLE;

		//速度0状態設定
		if (pEnv->is_speed_0(ID_HOIST))	AgentInf_workbuf.axis_status[ID_HOIST] |= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_HOIST] &= ~AG_AXIS_STAT_FB0;
		if(pEnv->is_speed_0(ID_BOOM_H))	AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_BOOM_H] &= ~AG_AXIS_STAT_FB0;
		if (pEnv->is_speed_0(ID_SLEW))	AgentInf_workbuf.axis_status[ID_SLEW] |= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_SLEW] &= ~AG_AXIS_STAT_FB0;
	}
	
	//# 振れ止め完了判定
	{
		//引込
		//振れ振幅、位置ずれともに完了判定レベル以内
		if ((pEnv->cal_sway_r_amp2_m() < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (pEnv->cal_dist4target(ID_BOOM_H, true) < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_COMPLE])) {

			AgentInf_workbuf.antisway_on_going |= ANTISWAY_BH_COMPLETE;
		}
		else if (pAgentInf->antisway_on_going & ANTISWAY_BH_COMPLETE) {	//振れ止め完了フラグON
			//振れ止め起動条件成立で完了フラグクリア
			if ((pEnv->cal_sway_r_amp2_m() > pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_TRIGGER])
				|| (pEnv->cal_dist4target(ID_BOOM_H, true) > pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_TRIGGER]))

				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_COMPLETE;
		}
		else {
			AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_COMPLETE;
		}

		//旋回
		if ((pEnv->cal_sway_th_amp2_m() < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
			&& (pEnv->cal_dist4target(ID_SLEW, true) < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_COMPLE])) {

			AgentInf_workbuf.antisway_on_going |= ANTISWAY_SLEW_COMPLETE;

		}
		else if (pAgentInf->antisway_on_going & ANTISWAY_SLEW_COMPLETE) {
			if ((pEnv->cal_sway_th_amp2_m() > pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_TRIGGER])
				|| (pEnv->cal_dist4target(ID_SLEW, true) > pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_TRIGGER]))

				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_COMPLETE;
		}
		else {
			AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_COMPLETE;
		}
	}

	//# 振れ止め実行モードセット antisway_on_going
	{
		if (pCSInf->antisway_mode == L_OFF) {
			AgentInf_workbuf.antisway_on_going = ANTISWAY_ALL_MANUAL;
		}
		else {
			AgentInf_workbuf.antisway_on_going |= ANTISWAY_BH_ACTIVE;
			AgentInf_workbuf.antisway_on_going |= ANTISWAY_SLEW_ACTIVE;
		}
	}

	//# 振れ止め自動モード(auto_on_going)セット
	{
		if (pCSInf->antisway_mode) AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;
		else					   AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
	}

	//# ジョブ自動モード(auto_on_going)セット →　起動時であればコマンドの取り込み
	{
		if (pCSInf->auto_mode == L_ON) {	//自動モード
			if (can_job_trigger()) { //ジョブ実行可否チェック （ジョブ実行中でない）
				pjob_active = pCS->get_next_job();							//CSにジョブ問い合わせ
				if (pjob_active == NULL) {
					pCom_hot = pPolicy->req_command(pjob_active);				//POLICYにコマンド展開依頼 pjob NULLならばNULLが帰ってくる
					pPolicy->update_command_status(pCom_hot, STAT_ACTIVE);		//コマンド実行開始報告
				}
				else {
					pCom_hot = NULL;
				}

				//自動制御モードの更新
				if ((pCom_hot == NULL)||(pjob_active == NULL)) AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_JOB_MASK;	//自動制御JOB TYPEクリア
				else if (pjob_active->type == ID_JOBTYPE_SEMI) AgentInf_workbuf.auto_on_going |= AUTO_TYPE_SEMIAUTO;
				else if (pjob_active->type == ID_JOBTYPE_JOB) AgentInf_workbuf.auto_on_going |= AUTO_TYPE_JOB;
				else;
			}
		}
		else {	//自動モードOFFでJOB部クリア
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_JOB_MASK;
			pjob_active = NULL;
			pCom_hot = NULL;
		}
	}

	return;
};

//ジョブの起動可否判定
bool CAgent::can_job_trigger() {

	//現在自動実行中でなければ起動可
	if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO))
		return false;
	else if ((pjob_active != NULL) || (pCom_hot == NULL))	//実行中のジョブがあるときは不可
		return false;

	else
		return true;

}


/****************************************************************************/
/*   定周期処理手順2　メイン処理																*/
/****************************************************************************/

static int auto_on_going_last = AUTO_TYPE_MANUAL;
static bool notch0_last[MOTION_ID_MAX];

void CAgent::main_proc() {

	//# 軸毎のモードセット(PLC指令出力軸の判定）
	{
		if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
		}
		else if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
			AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
		}
		else {//FB振れ止め
			AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
			AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		}
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
	}

	//# PLCへのPC選択指令セット
	{
		if ((pPLC_IO->mode & PLC_IF_PC_DBG_MODE) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_OPERATION)) {//デバッグモード　または　リモート手動操作モード
			AgentInf_workbuf.pc_ctrl_mode |= (BITSEL_HOIST | BITSEL_GANTRY | BITSEL_BOOM_H | BITSEL_SLEW);
		}
		else {
			AgentInf_workbuf.pc_ctrl_mode = 0;
			if (AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_HOIST;
			if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_GANTRY;
			if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_BOOM_H;
			if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_SLEW;
		}
	}
		
	//#自動目標位置設定
	{
		//JOBトリガ検出
		int auto_on_trigger = ~auto_on_going_last & AgentInf_workbuf.auto_on_going;		// 自動実行モードがOFF→ON
		int auto_off_trigger = auto_on_going_last & ~AgentInf_workbuf.auto_on_going;	// 自動実行モードがON→OFF
		//JOB,SEMIAUTO ONトリガ検出
		if ((auto_on_trigger & AUTO_TYPE_JOB) || (auto_on_trigger & AUTO_TYPE_SEMIAUTO)) {
			if (pjob_active != NULL) {	//ジョブ有でレシピの目標位置をコピー
				AgentInf_workbuf.auto_pos_target = pjob_active->recipe[pjob_active->i_hot_com].target;
			}
			else {	//ジョブ無し（ロジック異常）でレシピの目標位置を現在位置に
				for (int i = 0; i < NUM_OF_AS_AXIS; i++) AgentInf_workbuf.auto_pos_target.pos[i] = pPLC_IO->status.pos[i];
			}
		}


		if (pCom_hot == NULL) {//コマンド未実行時 
			//巻は現在位置
			AgentInf_workbuf.auto_pos_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];

			//引込、旋回はノッチ入りまたは振れ止めOFFで現在位置　0ノッチトリガで現在位置＋減速距離位置
			if ((notch0_last[ID_BOOM_H] == false) && (pCraneStat->is_notch_0[ID_BOOM_H] == true)) { //0ノッチトリガ
				AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H] + pEnv->cal_dist4stop(ID_BOOM_H, false);
			}
			else if((pCraneStat->is_notch_0[ID_BOOM_H] == false)||(pCSInf->antisway_mode!=L_ON)) {
				AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
			}
			else;

			if ((notch0_last[ID_SLEW] == false) && (pCraneStat->is_notch_0[ID_SLEW] == true)) { //0ノッチトリガ
				AgentInf_workbuf.auto_pos_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW] + pEnv->cal_dist4stop(ID_SLEW, false);
			}
			else if((pCraneStat->is_notch_0[ID_SLEW] == false)||(pCSInf->antisway_mode != L_ON)) {
				AgentInf_workbuf.auto_pos_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];
			}
			else;
		}

		//前回値保持
		auto_on_going_last = AgentInf_workbuf.auto_on_going;
		notch0_last[ID_HOIST] = pCraneStat->is_notch_0[ID_HOIST];
		notch0_last[ID_BOOM_H] = pCraneStat->is_notch_0[ID_BOOM_H];
		notch0_last[ID_SLEW] = pCraneStat->is_notch_0[ID_SLEW];
	}
	
	//#PLCへの出力計算　
	set_ref_mh();							//巻き速度指令
	set_ref_gt();							//走行速度指令
	/**********************************************************************/
	/*振れ止めレシピの生成タイミング                                      */
	/*  pCom_hot != NULL   →  ステップがCTR_TYPE_FB_SWAY_POSに入ったとき */
	/*  pCom_hot == NULL                                                  */
	/*   振れ止めON∩振れ止め完了条件未成立∩振れ止めレシピ完了時∩停止時　*/
	/**********************************************************************/
	set_ref_slew();							//旋回速度指令
	set_ref_bh();							//引込速度指令
	update_pb_lamp_com();					//PB LAMP出力

	return;
}

/****************************************************************************/
/*   定周期処理手順3　信号出力処理												*/
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



/****************************************************************************/
/*	自動関連設定					*/
/****************************************************************************/

int CAgent::set_recipe_as_bh(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};
int CAgent::set_recipe_as_slw(LPST_MOTION_RECIPE precipe, bool is_fbtype, LPST_AGENT_WORK pwork) {

	return 0;
};

void CAgent::set_as_workbuf() {
	return;;
}


/****************************************************************************/
/*   巻指令出力処理		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_HOIST) {
		if (AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
		else if ((AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_SEMIAUTO)) {
			if (pCom_hot == NULL)	AgentInf_workbuf.v_ref[ID_HOIST] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_HOIST] = cal_step(pCom_hot, ID_HOIST);
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
			if (pCom_hot == NULL)	AgentInf_workbuf.v_ref[ID_GANTRY] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_GANTRY] = cal_step(pCom_hot, ID_GANTRY);
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
			if(pCom_hot == NULL)	AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pCom_hot, ID_SLEW);
		}
		else if (AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_FB_ANTI_SWAY){				//振れ止め中
			if (AgentInf_workbuf.antisway_on_going | ANTISWAY_SLEW_COMPLETE)					//振れ止め完了
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			else																					//振れ止め未完
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(&pAgentInf->comrecipe_as.comset, ID_SLEW);	
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

	LPST_COMMAND_SET pcom;

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_BOOM_H) {									//制御PC指令選択ON
		
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL)						//マニュアルモード
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];

		else if ((AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_SEMIAUTO)) {					//自動運転中
			if (pCom_hot == NULL)	AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom_hot, ID_BOOM_H);
		}
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_FB_ANTI_SWAY) {			//振れ止め中
			if(AgentInf_workbuf.antisway_on_going | ANTISWAY_BH_COMPLETE)									//振れ止め完了
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			else 																					//振れ止め未完
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(&pAgentInf->comrecipe_as.comset, ID_BOOM_H);
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
int CAgent::startup_command(LPST_COMMAND_SET pcom) {

	for (int i = 0; i < MOTION_ID_MAX;i++) {						//各軸の実行ステータスの初期化
		if (AgentInf_workbuf.auto_active[i] == AUTO_TYPE_MANUAL) {
			pcom->motion_status[i]= MOTION_COMPLETE;
		}
		else {
			if (pcom->active_motion[i] == L_ON) {
				pcom->motion_status[i] = MOTION_STANDBY;
				pcom->recipe[i].i_hot_step = 0;
				pcom->recipe[i].motion_act_count = 0;
				pcom->recipe[i].fin_code = STAT_STANDBY;

				for (int k = 0;k < pcom->recipe[i].n_step;k++) {
					pcom->recipe[i].steps[k].status = STEP_STANDBY;
					pcom->recipe[i].steps[k].act_count = 0;
				}
			}
			else pcom->motion_status[i] = MOTION_COMPLETE;
		}
	}
	return 0;
}

/****************************************************************************/
/*   STEP処理		                                                        */
/****************************************************************************/
double CAgent::cal_step(LPST_COMMAND_SET pCom,int motion) {


	double v_out = 0.0;

	LPST_MOTION_RECIPE precipe = &pCom->recipe[motion];
	LPST_MOTION_STEP pStep = &precipe->steps[precipe->i_hot_step];


	if (pCom->motion_status[motion] & MOTION_COMPLETE) {		 //レシピ出力完了済
		return 0.0;
	}
	else if (pStep->status == STAT_STANDBY) {   //ステップ起動時
		pStep->act_count= 1;
		pStep->status = STAT_ACTIVE;			//ステップ実行中ステップ出力中に更新
	}
	else	pStep->act_count++;			//ステップ実行カウントインクリメント

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
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//指定範囲外　-π〜π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//指定範囲外　-π〜π
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
		if ((pStep->opt_d[MOTHION_OPT_PHASE_F] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_F] < -PI180)) {//指定範囲外　-π〜π
			pStep->status = PTN_STEP_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] > PI180) || (pStep->opt_d[MOTHION_OPT_PHASE_R] < -PI180)) {//指定範囲外　-π〜π
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
		pStep->act_count++;
		if (precipe->i_hot_step >= precipe->n_step) pCom->motion_status[motion] = MOTION_COMPLETE;
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

