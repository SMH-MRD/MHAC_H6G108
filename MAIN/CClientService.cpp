#include "CClientService.h"


//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pOTEioObj;
extern CSharedMem*  pCSInfObj;
extern CSharedMem* pPolicyInfsObj;
extern CSharedMem* pAgentInfObj;

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

	for (int i = 0;i < N_PLC_PB;i++) PLC_PBs_last[i] = false;

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnvironment = (CEnvironment*)VectpCTaskObj[g_itask.environment];


	set_panel_tip_txt();

	inf.is_init_complete = true;
	CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
	CS_workbuf.command_type = NON_COM;


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
		// 半自動目標位置の取り込み
		for (int i = 0; i < SEMI_AUTO_TARGET_MAX; i++) {
			CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST] = pCraneStat->spec.semi_target[i][ID_HOIST];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H] = pCraneStat->spec.semi_target[i][ID_BOOM_H];
			CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW] = pCraneStat->spec.semi_target[i][ID_SLEW];
		}
	}

	//自動開始PB
	if (pPLC_IO->ui.PB[ID_PB_AUTO_START])CS_workbuf.plc_pb[ID_PB_AUTO_START]++;
	else CS_workbuf.plc_pb[ID_PB_AUTO_START] = 0;

	//PLC入力処理
	parce_onboard_input(CS_NORMAL_MODE);
	//操作端末処理
	if (can_ote_activate()) {

	}
	return;
};

int CClientService::parce_onboard_input(int mode) { 
	return 0; 
}

int CClientService::parce_ote_imput(int mode) {
	return 0;
}
int CClientService::can_ote_activate() {
	if (pPLC_IO->ui.PB[ID_PB_REMOTE_SEL]) {
		return true;
	}
	else {
		return false;
	}
}

//# 定周期処理手順2　メイン処理
/***  モード管理,JOB/半自動監視更新処理   ***/

static int as_pb_last=0,auto_pb_last = 0, set_z_pb_last = 0, set_xy_pb_last = 0;
static int park_pb_last=0, pick_pb_last = 0, grnd_pb_last = 0;
static int mhp1_pb_last = 0, mhp2_pb_last = 0, mhm1_pb_last = 0, mhm2_pb_last = 0;
static int slp1_pb_last = 0, slp2_pb_last = 0, slm1_pb_last = 0, slm2_pb_last = 0;
static int bhp1_pb_last = 0, bhp2_pb_last = 0, bhm1_pb_last = 0, bhm2_pb_last = 0;
/****************************************************************************/
/*  メイン処理																*/
/****************************************************************************/
void CClientService::main_proc() {

	/*### モード管理 ###*/
	//振れ止めモードセット
	if ((pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON]) && (as_pb_last == 0)) { // PB入力立ち上がり
		if (CS_workbuf.antisway_mode == L_OFF)
			CS_workbuf.antisway_mode = L_ON;
		else
			CS_workbuf.antisway_mode = L_OFF;
	}
	as_pb_last = pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON];
	
	//自動モードセット
	if ((pPLC_IO->ui.PB[ID_PB_AUTO_MODE]) && (auto_pb_last == 0)) { // PB入力立ち上がり
		if (CS_workbuf.auto_mode == L_OFF)
			CS_workbuf.auto_mode = L_ON;
		else
			CS_workbuf.auto_mode = L_OFF;
	}
	auto_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_MODE];

	//目標位置確定セット
	if (CS_workbuf.auto_mode == L_ON) {
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z]) && (set_z_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_z == L_OFF)
				CS_workbuf.target_set_z = L_ON;
			else
				CS_workbuf.target_set_z = L_OFF;
		}
		if ((pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY]) && (set_xy_pb_last == 0)) { // PB入力立ち上がり
			if (CS_workbuf.target_set_xy == L_OFF)
				CS_workbuf.target_set_xy = L_ON;
			else
				CS_workbuf.target_set_xy = L_OFF;
		}
	}
	else {
		CS_workbuf.target_set_z = L_OFF;
		CS_workbuf.target_set_xy = L_OFF;
	}
	set_z_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z];
	set_xy_pb_last = pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY];


	/*### 半自動処理 ###*/
	//半自動設定操作処理
	if (CS_workbuf.auto_mode == L_ON) {
		if((CS_workbuf.target_set_z == L_OFF)&& (CS_workbuf.target_set_xy == L_OFF)){//目標位置未確定時のみ更新可能
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
				}
				else if (CS_workbuf.semiauto_pb[i] == SEMI_AUTO_TG_SELECT_TIME) {						 //半自動目標設定
					if (i == CS_workbuf.semi_auto_selected) {//設定中のボタンを押したら解除
						CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];
					}
					else {
						CS_workbuf.semi_auto_selected = i;
						CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = CS_workbuf.semi_auto_setting_target[i].pos[ID_HOIST];
						CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = CS_workbuf.semi_auto_setting_target[i].pos[ID_BOOM_H];
						CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = CS_workbuf.semi_auto_setting_target[i].pos[ID_SLEW];
					}
				}
			}
		}
	}
	else{ //自動モードでないときは半自動設定クリア
		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;
		CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];
		CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
		CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];
	}

	//半自動目標位置補正PB入力処理
	if (CS_workbuf.auto_mode == L_ON) {
		if (CS_workbuf.target_set_z == L_OFF) {//目標確定設定OFF時補正可能
			if ((pPLC_IO->ui.PB[ID_PB_MH_P1]) && (mhp1_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_100mm;
			if ((pPLC_IO->ui.PB[ID_PB_MH_P2]) && (mhp2_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] += AUTO_TG_ADJUST_1000mm;
			if ((pPLC_IO->ui.PB[ID_PB_MH_M1]) && (mhm1_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_100mm;
			if ((pPLC_IO->ui.PB[ID_PB_MH_M2]) && (mhm2_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_HOIST] -= AUTO_TG_ADJUST_1000mm;
		}
		if (CS_workbuf.target_set_xy == L_OFF) {//目標確定設定OFF時補正可能
			if ((pPLC_IO->ui.PB[ID_PB_BH_P1]) && (bhp1_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_100mm;
			if ((pPLC_IO->ui.PB[ID_PB_BH_P2]) && (bhp2_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] += AUTO_TG_ADJUST_1000mm;
			if ((pPLC_IO->ui.PB[ID_PB_BH_M1]) && (bhm1_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_100mm;
			if ((pPLC_IO->ui.PB[ID_PB_BH_M2]) && (bhm2_pb_last == 0)) CS_workbuf.semi_auto_selected_target.pos[ID_BOOM_H] -= AUTO_TG_ADJUST_1000mm;

			//旋回は角度に変換
			if ((pPLC_IO->ui.PB[ID_PB_SL_P1]) && (slp1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_P2]) && (slp2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] += rad;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_M1]) && (slm1_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_100mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
			}
			if ((pPLC_IO->ui.PB[ID_PB_SL_M2]) && (slm2_pb_last == 0)) {
				double rad = AUTO_TG_ADJUST_1000mm / pCraneStat->R;
				CS_workbuf.semi_auto_selected_target.pos[ID_SLEW] -= rad;
			}
		}
	}
	//前回値保持
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


	//自動コマンドセット
	if ((pPLC_IO->ui.PB[ID_PB_PARK]) && (park_pb_last == 0)) { // PB入力立ち上がりID_PB_GRNDID_PB_PICK
		if (CS_workbuf.command_type == PARK_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = PARK_COM;
	}
	park_pb_last = pPLC_IO->ui.PB[ID_PB_PARK];

	if ((pPLC_IO->ui.PB[ID_PB_PICK]) && (pick_pb_last == 0)) { // PB入力立ち上がりID_PB_GRNDID_PB_PICK
		if (CS_workbuf.command_type == PICK_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = PICK_COM;
	}
	pick_pb_last = pPLC_IO->ui.PB[ID_PB_PICK];

	if ((pPLC_IO->ui.PB[ID_PB_GRND]) && (grnd_pb_last == 0)) { // PB入力立ち上がりID_PB_GRNDID_PB_PICK
		if (CS_workbuf.command_type == GRND_COM)
			CS_workbuf.command_type = NON_COM;
		else
			CS_workbuf.command_type = GRND_COM;
	}
	grnd_pb_last = pPLC_IO->ui.PB[ID_PB_GRND];


	//JOB登録処理（自動開始PB処理）
	//半自動ジョブ登録
	if (CS_workbuf.plc_pb[ID_PB_AUTO_START] == AUTO_START_CHECK_TIME) {
		if(CS_workbuf.semi_auto_selected != SEMI_AUTO_TG_CLR) update_semiauto_list(CS_ADD_SEMIAUTO, AUTO_TYPE_SEMIAUTO | COM_TYPE_PARK, CS_workbuf.semi_auto_selected);
		else update_semiauto_list(CS_CLEAR_SEMIAUTO, AUTO_TYPE_MANUAL, CS_workbuf.semi_auto_selected);
	}

	//半自動設定解除
	if (pPLC_IO->ui.PB[ID_PB_AUTO_RESET])
		CS_workbuf.semi_auto_selected = SEMI_AUTO_TG_CLR;


	/*### Job処理 ###*/


	return;

}

//# 定周期処理手順3　信号出力, 上位レスポンス処理
//ジョブ関連ランプ表示他
/****************************************************************************/
/*  出力処理																*/
/****************************************************************************/
void CClientService::output() {

/*### 自動関連ランプ表示　###*/

	//振れ止めランプ
	if (CS_workbuf.antisway_mode == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_ON;
	}
	else {//振れ止め起動中は点滅
		CS_workbuf.plc_lamp[ID_PB_ANTISWAY_ON] = L_OFF;
	}

	//自動ランプ
	if (CS_workbuf.auto_mode == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_MODE] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_MODE] = L_OFF;
	}

	//起動ランプ
	if (CS_workbuf.auto_status == STAT_ACTIVE) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
	}
	else if (CS_workbuf.auto_status == STAT_STANDBY) {
		if(inf.act_count% LAMP_FLICKER_BASE_COUNT > LAMP_FLICKER_CHANGE_COUNT)
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
		else
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
	}

	//目標設定ランプ
	if (CS_workbuf.target_set_z == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_Z] = L_OFF;
	}

	if (CS_workbuf.target_set_xy == L_ON) {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_SET_XY] = L_OFF;
	}
	//自動コマンドランプ
	if (CS_workbuf.command_type == PICK_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_ON;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == GRND_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_ON;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}
	else if (CS_workbuf.command_type == PARK_COM) {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_ON;
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_PICK] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_GRND] = L_OFF;
		CS_workbuf.plc_lamp[ID_PB_PARK] = L_OFF;
	}


	//振れ止めランプ　自動開始ランプ(JOB実行中点灯,JOB実行中でなく登録JOB有で点滅、その他消灯）
	if (CS_workbuf.antisway_mode) {//自動モード時
		//自動開始ランプ
		if ((pAgent_Inf->auto_on_going == AUTO_TYPE_JOB) || (pAgent_Inf->auto_on_going == AUTO_TYPE_SEMIAUTO)) {//JOB実行中
			CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;//マニュアル
		}
		else {
			if ((CS_workbuf.job_list.job_wait_n + CS_workbuf.job_list.semiauto_wait_n)>0) {						//待機JOBがある時点滅
				if (inf.act_count % (AUTO_START_CHECK_TIME*2) > AUTO_START_CHECK_TIME) CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_ON;
				else CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
			}
			else {												//JOB無しで消灯
				CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
			}
		}
	}
	else {
		CS_workbuf.plc_lamp[ID_PB_AUTO_START] = L_OFF;
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
	}


	//共有メモリ出力
	memcpy_s(pCSinf, sizeof(ST_CS_INFO), &CS_workbuf, sizeof(ST_CS_INFO));

	wostrs << L" AS: " << CS_workbuf.antisway_mode << L" AUTO: " << CS_workbuf.auto_mode << L" --Scan " << inf.period;
	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   半自動関連処理															*/
/*   command	:追加,削除等												*/
/*   type		:半自動種類　PARK,PICK,GRAND								*/
/*   code		:半自動動作指定コード　選択中半自動PB  						*/
/****************************************************************************/
int CClientService:: update_semiauto_list(int command, int type, int code){
	switch (command) {
	case CS_CLEAR_SEMIAUTO: {	//半自動ジョブクリア
		CS_workbuf.job_list.semiauto_wait_n = 0;
		CS_workbuf.job_list.i_semiauto_active = 0;
		CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].status = STAT_STANDBY;
		CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].type = type;
		return ID_OK;
	}break;
	case CS_ADD_SEMIAUTO: {	//更新
		if ((code < SEMI_AUTO_S1) || (code >= SEMI_AUTO_TG_CLR)) {
			return ID_NG;
		}
		else {
			CS_workbuf.job_list.semiauto_wait_n = 1;
			CS_workbuf.job_list.i_semiauto_active = 0;		//当面半自動はid=0のみ使用
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].n_command = JOB_N_STEP_SEMIAUTO;
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].target[0] = CS_workbuf.semi_auto_setting_target[code];
			CS_workbuf.job_list.semiauto[CS_workbuf.job_list.i_semiauto_active].type = type;

			return ID_OK;
		}
	}break;
	default:
		return ID_NA;
		break;
	}
}

/****************************************************************************/
/*   JOB関連																*/
/****************************************************************************/
int CClientService::update_job_list(int command, int code) {
	switch (command) {
	case CS_CLEAR_JOB: {	//ジョブクリア
		CS_workbuf.job_list.job_wait_n = 0;
		CS_workbuf.job_list.i_job_active = 0;
		CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].n_command = 0;
		CS_workbuf.job_list.job[CS_workbuf.job_list.i_job_active].status = STAT_WAITING;
		return ID_OK;
	}break;
	case CS_ADD_JOB: {	//更新
		return ID_NG;
	}break;
	default:
		return ID_NA;
		break;
	}
}
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
