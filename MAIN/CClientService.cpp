#include "CClientService.h"


//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pOTEioObj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfsObj;
extern CSharedMem* pAgentInfObj;
extern CSharedMem* pJobIO_Obj;

extern vector<void*>	VectpCTaskObj;	//タスクオブジェクトのポインタ
extern ST_iTask g_itask;

/****************************************************************************/
/*   コンストラクタ　デストラクタ                                           */
/****************************************************************************/
CClientService::CClientService() {
	pPLC_IO = NULL;
	pCraneStat = NULL;
}

CClientService::~CClientService() {

}


/****************************************************************************/
/*   タスク初期化処理                                                       */
/* 　メインスレッドでインスタンス化した後に呼びます。                       */
/****************************************************************************/
static BOOL PLC_PBs_last[N_PLC_PB];

void CClientService::init_task(void* pobj) {

	//共有メモリ構造体のポインタセット
	pPLC_IO = (LPST_PLC_IO)(pPLCioObj->get_pMap());
	pCraneStat = (LPST_CRANE_STATUS)(pCraneStatusObj->get_pMap()); 
	pCSinf = (LPST_CS_INFO)(pCSInfObj->get_pMap());
	pAgent_Inf = (LPST_AGENT_INFO)(pAgentInfObj->get_pMap());
	pOTE_IO = (LPST_OTE_IO)(pOTEioObj->get_pMap());
	pJob_IO = (LPST_JOB_IO)(pJobIO_Obj->get_pMap());

	for (int i = 0;i < N_PLC_PB;i++) PLC_PBs_last[i] = false;

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnvironment = (CEnvironment*)VectpCTaskObj[g_itask.environment];


	set_panel_tip_txt();


	CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
	CS_workbuf.command_type = COM_TYPE_NON;							//PICK, GRND, PARK

	for (int i = 0;i < N_JOB_LIST;i++) {
		pJob_IO->job_list[i].i_job_hot = 0;
		pJob_IO->job_list[i].n_hold_job = 0;
	}

	inf.is_init_complete = true;
	CS_workbuf.job_set_event = CS_JOBSET_EVENT_CLEAR;

	return;
};

/****************************************************************************/
/*   タスク定周期処理                                                       */
/* 　タスクスレッドで毎周期実行される関数			　                      */
/****************************************************************************/
void CClientService::routine_work(void* param) {
	input();
	main_proc();
	output();
};

//# 定周期処理手順1　外部信号入力
/***  JOB関連入力信号処理（自動開始PB)   ***/
/****************************************************************************/
/*  入力処理																*/
/****************************************************************************/

void CClientService::input() {
	/*### 初期化処理（CraneStatの立ち上がり後にSpec取り込み） ###*/
	if (pCraneStat->env_act_count < 10){//Environmentが立ち上がり後に初期値取り込み
		// 半自動目標位置デフォルト値の取り込み
		for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
			CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST]	= pCraneStat->spec.semi_target[i][ID_HOIST];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H]	= pCraneStat->spec.semi_target[i][ID_BOOM_H];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW]		= pCraneStat->spec.semi_target[i][ID_SLEW];
		}
	}

	//PLC入力処理
	parce_onboard_input(CS_NORMAL_OPERATION_MODE);
	//操作端末からの入力処理 操作端末モードでは、操作系機上入力の内容を上書き
	if (can_ote_activate()) {
		parce_ote_imput(CS_NORMAL_OPERATION_MODE);
	}

	return;
};

//# 操作入力取り込み処理
static int as_pb_last = 0, auto_pb_last = 0, set_z_pb_last = 0, set_xy_pb_last = 0;
static int park_pb_last = 0, pick_pb_last = 0, grnd_pb_last = 0;					//自動指定入力前回値保持
static int mhp1_pb_last = 0, mhp2_pb_last = 0, mhm1_pb_last = 0, mhm2_pb_last = 0;	//目標位置補正入力前回値保持
static int slp1_pb_last = 0, slp2_pb_last = 0, slm1_pb_last = 0, slm2_pb_last = 0;	//目標位置補正入力前回値保持
static int bhp1_pb_last = 0, bhp2_pb_last = 0, bhm1_pb_last = 0, bhm2_pb_last = 0;	//目標位置補正入力前回値保持
static int semi_auto_selected_last = SEMI_AUTO_TG_CLR,job_set_event_last;

int CClientService::parce_onboard_input(int mode) { 




	/*### モード管理 ###*/
	//振れ止めモードセット
	if (pCraneStat->operation_mode & OPERATION_MODE_REMOTE) {
		if ((pOTE_IO->rcv_msg_u.body.pb[ID_PB_ANTISWAY_ON]) && (as_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.antisway_mode == L_OFF)
				CS_workbuf.antisway_mode = L_ON;
			else
				CS_workbuf.antisway_mode = L_OFF;
		}
		as_pb_last = pOTE_IO->rcv_msg_u.body.pb[ID_PB_ANTISWAY_ON];
	}
	else {
		if ((pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON]) && (as_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.antisway_mode == L_OFF)
				CS_workbuf.antisway_mode = L_ON;
			else
				CS_workbuf.antisway_mode = L_OFF;
		}
		as_pb_last = pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON];
	}


	INT16* pUIpb;
	INT16* pUIpb_semiauto;

	if (pCraneStat->operation_mode & OPERATION_MODE_REMOTE) {
		pUIpb = pOTE_IO->rcv_msg_u.body.pb;
		pUIpb_semiauto = pOTE_IO->ui.PBsemiauto;
	}
	else {
		pUIpb = pPLC_IO->ui.PB;
		pUIpb_semiauto = pPLC_IO->ui.PBsemiauto;
	}

#if 0
		//自動モードセット
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_MODE]) && (auto_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.auto_mode == L_OFF)
				CS_workbuf.auto_mode = L_ON;
			else
				CS_workbuf.auto_mode = L_OFF;
		}
	auto_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_MODE];


	//半自動選択設定,目標位置設定
	if (CS_workbuf.auto_mode == L_ON) {

		//半自動目標設定
		//目標位置未確定時のみ更新可能
		if ((CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) && (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED)) {
			for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
				//PB ON時間カウント 半自動リセット時間まで
				if (pPLC_IO->ui.PBsemiauto[i] <= 0) CS_workbuf.semiauto_pb[i] = 0;
				else if (CS_workbuf.semiauto_pb[i] < SEMI_AUTO_TG_RESET_TIME) CS_workbuf.semiauto_pb[i]++;
				else;

				//目標設定
				if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_RESET_TIME) {//半自動目標位置設定値更新
					CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];

					CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

					CS_workbuf.tg_sel_trigger_z = L_ON, CS_workbuf.tg_sel_trigger_xy = L_ON;
				}
				else if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_SELECT_TIME) {						//半自動目標設定
					if (i == CS_workbuf.semi_auto_selected) {											//設定中のボタンを押したら解除
						CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];

						CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
					}
					else {
						//半自動選択ボタン取り込み
						CS_workbuf.semi_auto_selected = i;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

						CS_workbuf.tg_sel_trigger_z = L_ON, CS_workbuf.tg_sel_trigger_xy = L_ON;
					}
				}
				else;
			}
			if (pCraneStat->operation_mode & OPERATION_MODE_REMOTE) {//遠隔端末モード
				if (CS_workbuf.semi_auto_selected == 5) {
					CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[2] / 1000.0;
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[1] / 1000.0;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[0] / 1000.0;
				}
			}
		}
		//Z目標位置補正　対応軸目標未確定時のみ更新可能
		if (CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) {//目標確定設定OFF時補正可能
			if ((pPLC_IO->ui.PB[ID_PB_MH_P1]) && (mhp1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_P2]) && (mhp2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_M1]) && (mhm1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_MH_M2]) && (mhm2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
		}
		//XY目標位置補正　対応軸目標未確定時のみ更新可能
		if (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED) {//目標確定設定OFF時補正可能
			if ((pPLC_IO->ui.PB[ID_PB_BH_P1]) && (bhp1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_BH_P2]) && (bhp2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_BH_M1]) && (bhm1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_BH_M2]) && (bhm2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}

			//旋回は角度に変換
			if ((pPLC_IO->ui.PB[ID_PB_SL_P1]) && (slp1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_P2]) && (slp2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_M1]) && (slm1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_M2]) && (slm2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
		}

		//半自動解除ボタン入力で設定クリア
		if (pPLC_IO->ui.PB[ID_PB_AUTO_RESET]) {
			CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
		}


		//半自動目標設定入力検出状態セット（画面クリック入力）
		if (CS_workbuf.tg_sel_trigger_z) {
			if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_DEFAULT) CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_ACTIVE;
		}
		if (CS_workbuf.tg_sel_trigger_xy) {
			if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_DEFAULT) CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_ACTIVE;
		}

		//目標位置確定セット
		//#### ジョブ登録イベントセット
		job_set_event_last = CS_workbuf.job_set_event;

		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z]) && (set_z_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_z & CS_SEMIAUTO_TG_SEL_FIXED) {
				CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
				CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
			}
			else {
				if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_ACTIVE) CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_FIXED;

				if (CS_workbuf.target_set_xy & CS_SEMIAUTO_TG_SEL_FIXED) {	//XY方向確定済
					CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_STANDBY;
				}
			}
		}
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY]) && (set_xy_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_xy & CS_SEMIAUTO_TG_SEL_FIXED) {
				CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
				CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
			}
			else {
				if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_ACTIVE) CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_FIXED;

				if (CS_workbuf.target_set_z & CS_SEMIAUTO_TG_SEL_FIXED) {	//Z方向確定済
					CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_STANDBY;
				}
			}
		}

		if ((semi_auto_selected_last != SEMI_AUTO_TG_CLR) && (CS_workbuf.semi_auto_selected == SEMI_AUTO_TG_CLR)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
		}

		//半自動JOB完了で目標確定クリア
		if ((job_set_event_last == CS_JOBSET_EVENT_SEMI_STANDBY) && (CS_workbuf.job_set_event != CS_JOBSET_EVENT_SEMI_STANDBY)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
		}

		//自動コマンド選択設定
		if ((pPLC_IO->ui.PB[ID_PB_PARK]) && (park_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.command_type == COM_TYPE_PARK)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_PARK;
		}
		park_pb_last = pPLC_IO->ui.PB[ID_PB_PARK];

		if ((pPLC_IO->ui.PB[ID_PB_PICK]) && (pick_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.command_type == COM_TYPE_PICK)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_PICK;
		}
		pick_pb_last = pPLC_IO->ui.PB[ID_PB_PICK];

		if ((pPLC_IO->ui.PB[ID_PB_GRND]) && (grnd_pb_last == 0)) { // PB入力立ち上がりK
			if (CS_workbuf.command_type == COM_TYPE_GRND)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_GRND;
		}
		grnd_pb_last = pPLC_IO->ui.PB[ID_PB_GRND];

		//自動起動PB
		if (pPLC_IO->ui.PB[ID_PB_AUTO_START])CS_workbuf.ui_pb[ID_PB_AUTO_START]++;
		else CS_workbuf.ui_pb[ID_PB_AUTO_START] = 0;
		//JOB起動処理

		if (CS_workbuf.ui_pb[ID_PB_AUTO_START] == AUTO_START_CHECK_TIME) {
			//半自動がスタンバイ状態
			if (pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status == STAT_STANDBY) {
				pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_TRIGED;
			}
			//JOBがスタンバイ状態
			else if (pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status == STAT_STANDBY) {
				pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status = STAT_TRIGED;
			}
			else;
		}
	}
	else {
		//自動モードでないときは半自動設定クリア
		CS_workbuf.tg_sel_trigger_z = L_OFF;
		CS_workbuf.tg_sel_trigger_xy = L_OFF;
		CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;

		CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
		CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
		CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];

		if ((semi_auto_selected_last != SEMI_AUTO_TG_CLR) && (CS_workbuf.semi_auto_selected == SEMI_AUTO_TG_CLR)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
		}
	}


	//前回値保持
	set_z_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z];
	set_xy_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY];
	mhp1_pb_last = pPLC_IO->ui.PB[ID_PB_MH_P1];
	mhp2_pb_last = pPLC_IO->ui.PB[ID_PB_MH_P2];
	mhm1_pb_last = pPLC_IO->ui.PB[ID_PB_MH_M1];
	mhm2_pb_last = pPLC_IO->ui.PB[ID_PB_MH_M2];
	bhp1_pb_last = pPLC_IO->ui.PB[ID_PB_BH_P1];
	bhp2_pb_last = pPLC_IO->ui.PB[ID_PB_BH_P2];
	bhm1_pb_last = pPLC_IO->ui.PB[ID_PB_BH_M1];
	bhm2_pb_last = pPLC_IO->ui.PB[ID_PB_BH_M2];
	slp1_pb_last = pPLC_IO->ui.PB[ID_PB_SL_P1];
	slp2_pb_last = pPLC_IO->ui.PB[ID_PB_SL_P2];
	slm1_pb_last = pPLC_IO->ui.PB[ID_PB_SL_M1];
	slm2_pb_last = pPLC_IO->ui.PB[ID_PB_SL_M2];
	semi_auto_selected_last = CS_workbuf.semi_auto_selected;
#endif
	//自動モードセット
	if ((pUIpb[ID_PB_AUTO_MODE]) && (auto_pb_last == 0)) { // PB入力立ち上がり
		if (CS_workbuf.auto_mode == L_OFF)
			CS_workbuf.auto_mode = L_ON;
		else
			CS_workbuf.auto_mode = L_OFF;
	}
	auto_pb_last = pUIpb[ID_PB_AUTO_MODE];


	//半自動選択設定,目標位置設定
	if (CS_workbuf.auto_mode == L_ON) {

		//半自動目標設定
		//目標位置未確定時のみ更新可能
		if ((CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) && (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED)) {
			for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
				//PB ON時間カウント 半自動リセット時間まで
				if (pUIpb_semiauto[i] <= 0) CS_workbuf.semiauto_pb[i] = 0;
				else if (CS_workbuf.semiauto_pb[i] < SEMI_AUTO_TG_RESET_TIME) CS_workbuf.semiauto_pb[i]++;
				else;

				//目標設定
				if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_RESET_TIME) {//半自動目標位置設定値更新
					CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST]	= pPLC_IO->status.pos[ID_HOIST];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H]	= pPLC_IO->status.pos[ID_BOOM_H];
					CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW]		= pPLC_IO->status.pos[ID_SLEW];

					CS_workbuf.semi_auto_selected_target.pos[ID_HOIST]		= CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H]		= CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW]		= CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

					CS_workbuf.tg_sel_trigger_z = L_ON, CS_workbuf.tg_sel_trigger_xy = L_ON;
				}
				else if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_SELECT_TIME) {						//半自動目標設定
					if (i == CS_workbuf.semi_auto_selected) {											//設定中のボタンを押したら解除
						CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;	
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST]	= pPLC_IO->status.pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW]	= pPLC_IO->status.pos[ID_SLEW];

						CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
					}
					else {
						//半自動選択ボタン取り込み
						CS_workbuf.semi_auto_selected = i;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST]	= CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW]	= CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];

						CS_workbuf.tg_sel_trigger_z = L_ON, CS_workbuf.tg_sel_trigger_xy = L_ON;
					}
				}
				else;
			}
			if (pCraneStat->operation_mode & OPERATION_MODE_REMOTE) {//遠隔端末モード
				if (CS_workbuf.semi_auto_selected == 5) {
					CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[2]/1000.0;
					CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[1] / 1000.0;
					CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = (double)pOTE_IO->rcv_msg_u.body.tg_pos1[0] / 1000.0;
				}
			}
		}
		//Z目標位置補正　対応軸目標未確定時のみ更新可能
		if (CS_workbuf.target_set_z != CS_SEMIAUTO_TG_SEL_FIXED) {//目標確定設定OFF時補正可能
			if ((pUIpb[ID_PB_MH_P1]) && (mhp1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pUIpb[ID_PB_MH_P2]) && (mhp2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pUIpb[ID_PB_MH_M1]) && (mhm1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
			if ((pUIpb[ID_PB_MH_M2]) && (mhm2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_z = L_ON;
			}
		}
		//XY目標位置補正　対応軸目標未確定時のみ更新可能
		if (CS_workbuf.target_set_xy != CS_SEMIAUTO_TG_SEL_FIXED) {//目標確定設定OFF時補正可能
			if ((pUIpb[ID_PB_BH_P1]) && (bhp1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_BH_P2]) && (bhp2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_BH_M1]) && (bhm1_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_100mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_BH_M2]) && (bhm2_pb_last == 0)) {
				CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_1000mm;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}

			//旋回は角度に変換
			if ((pUIpb[ID_PB_SL_P1]) && (slp1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_SL_P2]) && (slp2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_SL_M1]) && (slm1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
			if ((pUIpb[ID_PB_SL_M2]) && (slm2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
				CS_workbuf.tg_sel_trigger_xy = L_ON;
			}
		}

		//半自動解除ボタン入力で設定クリア
		if (pUIpb[ID_PB_AUTO_RESET]) {
			CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
		}


		//半自動目標設定入力検出状態セット（画面クリック入力）
		if (CS_workbuf.tg_sel_trigger_z) {
			if(CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_DEFAULT) CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_ACTIVE;
		}
		if (CS_workbuf.tg_sel_trigger_xy) {
			if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_DEFAULT) CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_ACTIVE;
		}

		//目標位置確定セット
		//#### ジョブ登録イベントセット
		job_set_event_last = CS_workbuf.job_set_event;

		if ((pUIpb[ID_PB_AUTO_SET_Z]) && (set_z_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_z & CS_SEMIAUTO_TG_SEL_FIXED) {
				CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
				CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
			}
			else {
				if(CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_ACTIVE) CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_FIXED;

				if (CS_workbuf.target_set_xy & CS_SEMIAUTO_TG_SEL_FIXED) {	//XY方向確定済
					CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_STANDBY;
				}
			}
		}
		if ((pUIpb[ID_PB_AUTO_SET_XY]) && (set_xy_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_xy & CS_SEMIAUTO_TG_SEL_FIXED) {
				CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
				CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
			}
			else {
				if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_ACTIVE) CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_FIXED;

				if (CS_workbuf.target_set_z & CS_SEMIAUTO_TG_SEL_FIXED) {	//Z方向確定済
					CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_STANDBY;
				}
			}
		}

		if ((semi_auto_selected_last != SEMI_AUTO_TG_CLR) && (CS_workbuf.semi_auto_selected == SEMI_AUTO_TG_CLR)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
		}

		//半自動JOB完了で目標確定クリア
		if ((job_set_event_last == CS_JOBSET_EVENT_SEMI_STANDBY) && (CS_workbuf.job_set_event != CS_JOBSET_EVENT_SEMI_STANDBY)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
		}

		//自動コマンド選択設定
		if ((pUIpb[ID_PB_PARK]) && (park_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.command_type == COM_TYPE_PARK)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_PARK;
		}
		park_pb_last = pUIpb[ID_PB_PARK];

		if ((pUIpb[ID_PB_PICK]) && (pick_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.command_type == COM_TYPE_PICK)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_PICK;
		}
		pick_pb_last = pUIpb[ID_PB_PICK];

		if ((pUIpb[ID_PB_GRND]) && (grnd_pb_last == 0)) { // PB入力立ち上がりK
			if (CS_workbuf.command_type == COM_TYPE_GRND)
				CS_workbuf.command_type = COM_TYPE_NON;
			else
				CS_workbuf.command_type = COM_TYPE_GRND;
		}
		grnd_pb_last = pUIpb[ID_PB_GRND];

		//自動起動PB
		if (pUIpb[ID_PB_AUTO_START])CS_workbuf.ui_pb[ID_PB_AUTO_START]++;
		else CS_workbuf.ui_pb[ID_PB_AUTO_START] = 0;
		//JOB起動処理
			
		if (CS_workbuf.ui_pb[ID_PB_AUTO_START] == AUTO_START_CHECK_TIME) {
			//半自動がスタンバイ状態
			if (pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status == STAT_STANDBY) {
				pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_TRIGED;
			}
			//JOBがスタンバイ状態
			else if (pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status == STAT_STANDBY) {
				pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status = STAT_TRIGED;
			}
			else;
		}
	}
	else {
	//自動モードでないときは半自動設定クリア
		CS_workbuf.tg_sel_trigger_z		= L_OFF;
		CS_workbuf.tg_sel_trigger_xy	= L_OFF;
		CS_workbuf.target_set_z			= CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.target_set_xy		= CS_SEMIAUTO_TG_SEL_DEFAULT;
		CS_workbuf.semi_auto_selected	= SEMI_AUTO_TG_CLR;

		CS_workbuf.semi_auto_selected_target.pos[ID_HOIST]	= pPLC_IO->status.pos[ID_HOIST];
		CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
		CS_workbuf.semi_auto_selected_target.pos[ID_SLEW]	= pPLC_IO->status.pos[ID_SLEW];

		if ((semi_auto_selected_last != SEMI_AUTO_TG_CLR) && (CS_workbuf.semi_auto_selected == SEMI_AUTO_TG_CLR)) {
			CS_workbuf.tg_sel_trigger_z = L_OFF, CS_workbuf.tg_sel_trigger_xy = L_OFF;
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_DEFAULT;
			CS_workbuf.job_set_event = CS_JOBSET_EVENT_SEMI_SEL_CLEAR;
		}
	}


	//前回値保持
	set_z_pb_last	= pUIpb[ID_PB_AUTO_SET_Z];
	set_xy_pb_last	= pUIpb[ID_PB_AUTO_SET_XY];
	mhp1_pb_last	= pUIpb[ID_PB_MH_P1];
	mhp2_pb_last	= pUIpb[ID_PB_MH_P2];
	mhm1_pb_last	= pUIpb[ID_PB_MH_M1];
	mhm2_pb_last	= pUIpb[ID_PB_MH_M2];
	bhp1_pb_last	= pUIpb[ID_PB_BH_P1];
	bhp2_pb_last	= pUIpb[ID_PB_BH_P2];
	bhm1_pb_last	= pUIpb[ID_PB_BH_M1];
	bhm2_pb_last	= pUIpb[ID_PB_BH_M2];
	slp1_pb_last	= pUIpb[ID_PB_SL_P1];
	slp2_pb_last	= pUIpb[ID_PB_SL_P2];
	slm1_pb_last	= pUIpb[ID_PB_SL_M1];
	slm2_pb_last	= pUIpb[ID_PB_SL_M2];
	semi_auto_selected_last = CS_workbuf.semi_auto_selected;

	return 0; 
}

//# 操作端末入力取り込み処理
int CClientService::parce_ote_imput(int mode) {
	return 0;
}

//# 操作端末有効判断
int CClientService::can_ote_activate() {
	if (pPLC_IO->ui.PB[ID_PB_REMOTE_MODE]) {
		return L_ON;
	}
	else {
		return L_OFF;
	}
}

/****************************************************************************/
/*  メイン処理																*/
/****************************************************************************/
void CClientService::main_proc() {

	
	//＃＃＃ジョブイベント処理
	//半自動登録処理

	if (CS_workbuf.auto_mode == L_OFF) {
		//自動モードOFFでジョブホールド数クリア
		pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job = pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job = 0;
	}

	//イベント処理
	switch (CS_workbuf.job_set_event) {
	case CS_JOBSET_EVENT_SEMI_SEL_CLEAR: {
		int i_job = pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot;
		LPST_JOB_SET p_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[i_job];
		//		if (p_job->status == STAT_STANDBY) p_job->status = STAT_REQ_WAIT;
		p_job->status = STAT_REQ_WAIT;
		pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job--;
		if (pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job < 0) pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job = 0;

		//イベントクリア
		CS_workbuf.job_set_event = CS_JOBSET_EVENT_CLEAR;
	}break;
	case CS_JOBSET_EVENT_SEMI_STANDBY: {										//半自動STANBY状態入り
		//*　半自動は、複数JOBの事前登録なし
				//現在のJOBバッファ
		int i_job = pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot;
		LPST_JOB_SET p_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[i_job];

		if (p_job->status != STAT_STANDBY) {//現在のJOB 起動待ちでない
			//次のバッファへ
			i_job = ++pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot;
			p_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[i_job];
			
			if (i_job >= JOB_HOLD_MAX) {
				pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot = i_job = 0;
				p_job->status = STAT_ABOTED;
			}
			else {
				p_job->status = STAT_STANDBY;
			}

			//JOB LIST更新
			pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job++;	//減算は、POLICY呼び出しのupdate_job_status()の完了報告で実行
		}
		else {//現在のJOB既に起動待ち→ステータスホールド
			//現バッファを更新
			//i_job = ++CS_workbuf.job_list[ID_JOBTYPE_SEMI].i_job_hot;
			//p_job = &CS_workbuf.job_list[ID_JOBTYPE_SEMI].job[i_job];
			p_job->status = STAT_STANDBY;
		}

		 //JOB SET内容セット
		p_job->list_id = ID_JOBTYPE_SEMI;
		p_job->id = i_job;
		set_semi_recipe(p_job);							//JOBレシピセット
	
		//イベントクリア
		CS_workbuf.job_set_event = CS_JOBSET_EVENT_CLEAR;
	}break;
	case CS_JOBSET_EVENT_JOB_STANDBY: {							//JOB　STANDBY状態入り JOBはCLIENTからのJOB受信時にSTANDBY
		//*　JOBは、複数JOBの事前登録可能 
		if (pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job < JOB_REGIST_MAX - 1) {
			//バッファへの追加場所を評価
			int i_job = pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot + pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job;
			if (i_job > JOB_REGIST_MAX) i_job = i_job % JOB_REGIST_MAX;
			LPST_JOB_SET p_job = &pJob_IO->job_list[ID_JOBTYPE_JOB].job[i_job];


			//JOB LIST更新
			pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job++;	//減算は、POLICY呼び出しのupdate_job_status()の完了報告で実行

			//JOB SET内容セット
			p_job->status = STAT_STANDBY;
			p_job->list_id = ID_JOBTYPE_SEMI;
			p_job->id = i_job;
			set_job_recipe(p_job);							//JOBレシピセット

			CS_workbuf.job_set_event = CS_JOBSET_EVENT_CLEAR;
		}
		else {//登録制限数越えは、無視。警報表示必要
			CS_workbuf.job_set_event = CS_JOBSET_EVENT_JOB_OVERFLOW;
		}
	}break;
	case CS_JOBSET_EVENT_SEMI_TRIG: {							//半自動起動PBトリガあり
		int i_job = pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot;
		LPST_JOB_SET p_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[i_job];

		if (p_job->status == STAT_STANDBY) {//JOB起動待ち状態
			p_job->status = STAT_TRIGED;	//JOBトリガ状態にステータス更新(STANDBYでレシピ設定済　AGENT実行待ち
		}
	}break;
	case CS_JOBSET_EVENT_JOB_TRIG: {
		int i_job = pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot;
		LPST_JOB_SET p_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[i_job];

		if (p_job->status == STAT_STANDBY) {//JOB起動待ち状態
			p_job->status = STAT_TRIGED;	//JOBトリガ状態にステータス更新(STANDBYでレシピ設定済　AGENT実行待ち
		}
	}break;
	default:break;
	}

	//現在アクティブなJOB
	if ((pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job > 0) &&
		(pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status != STAT_ACTIVE)){
		CS_workbuf.p_active_job = &pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot];
	}
	else if ((pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job > 0) &&
			 (pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status != STAT_ACTIVE)) {
		CS_workbuf.p_active_job = &pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot];
	}
	else
		CS_workbuf.p_active_job = NULL;


	return;
}

//ジョブのレシピセット
LPST_JOB_SET CClientService::set_job_recipe(LPST_JOB_SET pjob_set) {
	LPST_JOB_SET pjob = NULL;
	pjob_set->type = ID_JOBTYPE_JOB;
	return pjob;
}

//半自動のレシピセット
LPST_JOB_SET CClientService::set_semi_recipe(LPST_JOB_SET pjob_set) {
	//JOBのコマンド数　半自動は１
	pjob_set->n_com = 1;
	pjob_set->type = ID_JOBTYPE_SEMI;
	//目標位置セット
	pjob_set->recipe[0].target.pos[ID_HOIST] = CS_workbuf.semi_auto_selected_target.pos[ID_HOIST];
	pjob_set->recipe[0].target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H];
	pjob_set->recipe[0].target.pos[ID_SLEW] = CS_workbuf.semi_auto_selected_target.pos[ID_SLEW];

	return pjob_set;
}

int CClientService::perce_client_message(LPST_CLIENT_COM_RCV_MSG pmsg) {
	return 0;
}

/****************************************************************************/
/*  出力処理																*/
/*  ジョブ関連ランプ表示他													*/
/****************************************************************************/
void CClientService::output() {

/*### 自動関連ランプ表示　###*/

	//振れ止めランプ
	if (CS_workbuf.antisway_mode == L_ON) {
		CS_workbuf.ui_lamp[ID_PB_ANTISWAY_ON] = L_ON;
	}
	else {//振れ止め起動中は点滅
		CS_workbuf.ui_lamp[ID_PB_ANTISWAY_ON] = L_OFF;
	}

	//自動ランプ
	if (CS_workbuf.auto_mode == L_ON) {
		CS_workbuf.ui_lamp[ID_PB_AUTO_MODE] = L_ON;
	}
	else {
		CS_workbuf.ui_lamp[ID_PB_AUTO_MODE] = L_OFF;
	}

	//起動ランプ
	//ホットなジョブがアクティブ→点灯
	if ((pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status & STAT_ACTIVE)||
		(pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status & STAT_ACTIVE)){
		CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_ON;
	}
	//ホットなジョブがスタンバイ→点滅
	else if((pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status & STAT_STANDBY)||
		(pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status & STAT_STANDBY)) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_ON;
		else
			CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_OFF;
	}
	//ホットなジョブがサスペンド→点滅
	else if ((pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status & STAT_SUSPENDED) ||
		(pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status & STAT_SUSPENDED)) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_ON;
		else
			CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_OFF;
	}
	else {
		CS_workbuf.ui_lamp[ID_PB_AUTO_START] = L_OFF;
	}

	//目標設定ランプ　目標設定確定ランプ



	if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_FIXED) {
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z] = L_ON;
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z_FIXED] = L_ON;
	}
	else if (CS_workbuf.target_set_z == CS_SEMIAUTO_TG_SEL_ACTIVE) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z] = L_ON;
		else
			CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z] = L_OFF;

		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z_FIXED] = L_OFF;
	}
	else {
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_Z_FIXED] = L_OFF;
	}

	if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_FIXED) {
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY] = L_ON;
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY_FIXED] = L_ON;
	}
	else if (CS_workbuf.target_set_xy == CS_SEMIAUTO_TG_SEL_ACTIVE) {
		if (inf.total_act % LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY] = L_ON;
		else
			CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY_FIXED] = L_OFF;
	}
	else {
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_AUTO_SET_XY_FIXED] = L_OFF;
	}
	//自動コマンドランプ
	if (CS_workbuf.command_type == COM_TYPE_PICK) {
		CS_workbuf.ui_lamp[ID_PB_PICK] = L_ON;
		CS_workbuf.ui_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == COM_TYPE_GRND) {
		CS_workbuf.ui_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_GRND] = L_ON;
		CS_workbuf.ui_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == COM_TYPE_PARK) {
		CS_workbuf.ui_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_PARK] = L_ON;
	}
	else {
		CS_workbuf.ui_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.ui_lamp[ID_PB_PARK] = L_OFF;
	}

	//半自動ランプ
	for (int i = 0;i < SEMI_AUTO_TG_CLR;i++) {
		if (i == CS_workbuf.semi_auto_selected) {	//半自動選択中のPB
			if(CS_workbuf.semiauto_pb[i]){
				if ((CS_workbuf.semiauto_pb[i] >= SEMI_AUTO_TG_RESET_TIME)||(CS_workbuf.semiauto_pb[i]< SEMI_AUTO_TG_SELECT_TIME * 4))
					CS_workbuf.semiauto_lamp[i] = L_ON;
				else if ((CS_workbuf.semiauto_pb[i] % (SEMI_AUTO_TG_SELECT_TIME*2)) > SEMI_AUTO_TG_SELECT_TIME)
					CS_workbuf.semiauto_lamp[i] = L_ON;
				else
					CS_workbuf.semiauto_lamp[i] = L_OFF;
			}
			else {//目標位置確定で点灯
				CS_workbuf.semiauto_lamp[i] = L_ON;
			}
		}
		else {	//半自動選択中でないPB
			CS_workbuf.semiauto_lamp[i] = L_OFF;
		}

		CS_workbuf.ui_lamp[ID_PB_SEMI_AUTO_S1 + i] = CS_workbuf.semiauto_lamp[i];//表示ランプバッファへコピー
	}




	//ブレーキ状態
	for (int i = 0;i < MOTION_ID_MAX;i++) {
		CS_workbuf.ui_lamp[ID_LAMP_HST_BRK + i] = pPLC_IO->status.brk[i];
	}




	//共有メモリ出力
	memcpy_s(pCSinf, sizeof(ST_CS_INFO), &CS_workbuf, sizeof(ST_CS_INFO));



	//タスクパネル表示出力
	{
		wostrs << L" AS=" << CS_workbuf.antisway_mode << L",AUTO=" << CS_workbuf.auto_mode;

		int status;
		if (CS_workbuf.p_active_job != NULL) {
			status = CS_workbuf.p_active_job->status;
			wostrs << L",JOB TYPE=" << CS_workbuf.p_active_job->list_id << L", id=" << CS_workbuf.p_active_job->id;
		}
		else status = STAT_NA;

		if ((pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job <= 0) && (pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job <= 0)) {
			wostrs << L" >JOB: 0 hold";
		}
		else if ((CS_workbuf.p_active_job != NULL) &&
			(CS_workbuf.p_active_job->list_id == ID_JOBTYPE_JOB) &&
			(pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job > 0)) {
			wostrs << L" >JOB: " << pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job << L" iHOT-> ";
			if (status & STAT_STANDBY)		wostrs << L"STANDBY";
			else if (status & STAT_ACTIVE)	wostrs << L"ACTIVE";
			else if (status & STAT_TRIGED)	wostrs << L"TRIGED";
			else if (status & STAT_SUSPENDED)	wostrs << L"SUSPEND";
			else if (status & STAT_REQ_WAIT)	wostrs << L"WAIT REQ";
			else wostrs << L"OUT OF SERV";

		}
		else if ((CS_workbuf.p_active_job != NULL) &&
			(CS_workbuf.p_active_job->list_id == ID_JOBTYPE_SEMI) &&
			(pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job > 0)) {
			wostrs << L" >SEMIAUTO: " << pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job << L" iHOT-> ";
			if (status & STAT_STANDBY)		wostrs << L"STANDBY";
			else if (status & STAT_ACTIVE)	wostrs << L"ACTIVE";
			else if (status & STAT_TRIGED)	wostrs << L"TRIGED";
			else if (status & STAT_SUSPENDED)	wostrs << L"SUSPEND";
			else if (status & STAT_REQ_WAIT)	wostrs << L"WAIT REQ";
			else wostrs << L"OUT OF SERV";
		}
		else  wostrs << L" >NO JOB REQUEST ";

		wostrs << L" --Scan " << inf.period;

		tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	}
	return;

};

/****************************************************************************/
/*   他タスクからのアクセス関数												*/
/****************************************************************************/
//### AGENTからの実行待ちJOB問い合わせ応答
LPST_JOB_SET CClientService::get_next_job() {
	
	//HOT　JOBのステータスがトリガ状態のものを返信
	
	//半自動チェック
	int job_status = pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status;

	switch(job_status){
	case STAT_TRIGED:
		//レシピをセットしてポインタを返す
		 return &(pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot]);
	
	case STAT_ACTIVE:
	case STAT_SUSPENDED:
	case STAT_STANDBY:
	case STAT_ABOTED:
	case STAT_END:
	case STAT_REQ_WAIT:
		//実行待ち(TRIGGER）状態以外はスルー
		break;
	}

	//JOBチェック
	job_status = pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot].status;

	switch (job_status) {
	case STAT_TRIGED:
		//レシピをセットしてポインタを返す
		return &(pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_JOB].i_job_hot]);

	case STAT_ACTIVE:
	case STAT_SUSPENDED:
	case STAT_STANDBY:
	case STAT_ABOTED:
	case STAT_END:
	case STAT_REQ_WAIT:
		//実行待ち(TRIGGER）状態以外はスルー
		break;
	}
	//実行待ち無ければヌルリターン
	return NULL;
}

//### POLICYからのJOB Status更新依頼
int CClientService::update_job_status(LPST_JOB_SET pjobset, int fb_code) {

	if (pjobset->list_id == ID_JOBTYPE_JOB) {
		switch (fb_code) {
		case STAT_END: {
			pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job--;
			pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_END;

			job_report2client(pjobset, STAT_END);

			return STAT_ACK;
			break;
		}
		case STAT_ABNORMAL_END: {
			pJob_IO->job_list[ID_JOBTYPE_JOB].n_hold_job--;
			pJob_IO->job_list[ID_JOBTYPE_JOB].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_END;

			job_report2client(pjobset, STAT_END);

			return STAT_ACK;
		}break;
		case STAT_ACTIVE :break;
		case STAT_ABOTED:break;
		default:break;
		}
	}
	else if (pjobset->list_id == ID_JOBTYPE_SEMI) {
		switch (fb_code) {
		case STAT_END: {
			//正常完了時JOBのホールド数を0クリア
			pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job = 0;
			pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_END;

	//		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;		//半自動設定クリア
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_CLEAR;	//目標確定解除
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_CLEAR;	//目標確定解除

			job_report2client(pjobset, STAT_END);

			return STAT_ACK;
		}break;
		case STAT_ABNORMAL_END: {
			//異常完了時JOBのホールド数を0クリア
			pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job = 0;
			pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_ABNORMAL_END;

	//		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;		//半自動設定クリア
			CS_workbuf.target_set_z = CS_SEMIAUTO_TG_SEL_CLEAR;	//目標確定解除
			CS_workbuf.target_set_xy = CS_SEMIAUTO_TG_SEL_CLEAR;	//目標確定解除

			job_report2client(pjobset, STAT_ABNORMAL_END);

			return STAT_ACK;
		}break;
		case STAT_ACTIVE: {
			//ジョブ実行報告は、ステータスの更新のみ
			pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_ACTIVE;

			job_report2client(pjobset, STAT_ACTIVE);

			return STAT_ACK;
		}break;
		case STAT_ABOTED: {
			//ジョブキャンセル時（自動OFF等）JOBのホールド数を0クリア
			pJob_IO->job_list[ID_JOBTYPE_SEMI].n_hold_job = 0;
			pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_ABOTED;

			return STAT_ACK;
		}break;
		case STAT_SUSPENDED: {
			//中断時（手動介入）の場合は、ステータスをスタンバイ状態に戻す
			pJob_IO->job_list[ID_JOBTYPE_SEMI].job[pJob_IO->job_list[ID_JOBTYPE_SEMI].i_job_hot].status = STAT_STANDBY;

			return STAT_ACK;
		}break;
		default:return STAT_LOGICAL_ERROR;
		}
	}

	return STAT_LOGICAL_ERROR;
}

int CClientService::job_report2client(LPST_JOB_SET pjobset, int fb_code) {       //Jobの実行状況報告

	//後日検討　CLIENT通信アンサバック,ログ記録等
	if (pjobset->list_id == ID_JOBTYPE_JOB) {
		switch (fb_code) {
		case STAT_END: {
		}break;
		case STAT_ABNORMAL_END:break;
		case STAT_ACTIVE:break;
		case STAT_ABOTED:break;
		default:break;
		}
	}
	else if (pjobset->list_id == ID_JOBTYPE_SEMI) {
		switch (fb_code) {
		case STAT_END: {
		}break;
		case STAT_ABNORMAL_END: {
		}break;
		case STAT_ACTIVE: {
		}break;
		case STAT_ABOTED: {
		}break;
		case STAT_SUSPENDED: {
		}break;
		default:return STAT_LOGICAL_ERROR;
		}
	}

	return STAT_SUCCEED;
	return STAT_NA;
}


/****************************************************************************/
/*   半自動関連処理															*/
/****************************************************************************/


/****************************************************************************/
/*   JOB関連																*/
/****************************************************************************/

/****************************************************************************/
/*   タスク設定タブパネルウィンドウのコールバック関数                       */
/****************************************************************************/
LRESULT CALLBACK CClientService::PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp) {

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
void CClientService::set_panel_tip_txt()
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
void CClientService::set_panel_pb_txt() {

	//WCHAR str_func06[] = L"DEBUG";

	//SetDlgItemText(inf.hWnd_opepane, IDC_TASK_FUNC_RADIO6, (LPCWSTR)str_func06);

	return;
};
