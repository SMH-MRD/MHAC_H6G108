#include "CAgent.h"
#include "CPolicy.h"

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
	
	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;
	for (int i = 0;i < N_PLC_LAMP;i++) AgentInf_workbuf.PLC_LAMP_com[i] = 0;
	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;

	for (int i = 0;i < NUM_OF_AS_AXIS;i++) ph_chk_range[i] = PHASE_CHECK_RANGE;
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

	parse_indata();
	return;

};

//定周期処理手順2　メイン処理
void CAgent::main_proc() {

	update_auto_setting();	//自動実行モードの設定と自動レシピのセット
	set_pc_control();		//PLCがPC指令で動作する軸の選択

	//PLCへの出力計算
	set_ref_mh();			//巻き速度指令
	set_ref_gt();			//走行速度指令
	set_ref_slew();			//旋回速度指令
	set_ref_bh();			//引込速度指令
	update_pb_lamp_com();	//PB LAMP出力

	return;

}

//定周期処理手順3　信号出力処理
/****************************************************************************/
/*   信号出力	共有メモリ出力								*/
/****************************************************************************/
void CAgent::output() {

	//共有メモリ出力処理
	memcpy_s(pAgentInf, sizeof(ST_AGENT_INFO), &AgentInf_workbuf, sizeof(ST_AGENT_INFO));

	wostrs << L" #Ref:" << fixed<<setprecision(3);
	wostrs << L"MH " << AgentInf_workbuf.v_ref[ID_HOIST];
	wostrs << L",GT " << AgentInf_workbuf.v_ref[ID_GANTRY];
	wostrs << L",SL " << AgentInf_workbuf.v_ref[ID_SLEW];
	wostrs << L",BH " << AgentInf_workbuf.v_ref[ID_BOOM_H];

	wostrs <<  L" --Scan " << inf.period;;

	tweet2owner(wostrs.str()); wostrs.str(L""); wostrs.clear();
	return;

};

/****************************************************************************/
/*   入力信号の分析															*/
/*   減速距離,　0速判定,　目標までの距離,　自動完了判定用データ				*/
/****************************************************************************/
int CAgent::parse_indata() {

	//0速チェック,減速距離計算
	for (int i = 0; i < NUM_OF_AS_AXIS; i++) {
		//0速チェック
		if ((pPLC_IO->status.v_fb[i] >= pCraneStat->spec.notch_spd_f[i][NOTCH_1] * SPD0_CHECK_RETIO) ||
			(pPLC_IO->status.v_fb[i] <= pCraneStat->spec.notch_spd_r[i][NOTCH_1] * SPD0_CHECK_RETIO)) {//1ノッチの10％速度以上
			AgentInf_workbuf.is_spdfb_0[i] = false;	//0速でない
		}
		else if (pCraneStat->is_notch_0[i] == false) {//ノッチ0で無い
			AgentInf_workbuf.is_spdfb_0[i] = false;
		}
		else {
			AgentInf_workbuf.is_spdfb_0[i] = true;
		}

		//減速距離
		if (pPLC_IO->status.v_fb[i] < 0.0) {
			AgentInf_workbuf.dist_for_stop[i]
				= pPLC_IO->status.v_fb[i] * (-0.5 * pPLC_IO->status.v_fb[i] / pCraneStat->spec.accdec[i][ID_REV][ID_DEC] + pCraneStat->spec.delay_time[i][ID_DELAY_CNT_DEC]);

		}
		else {
			AgentInf_workbuf.dist_for_stop[i]
				= pPLC_IO->status.v_fb[i] * (-0.5 * pPLC_IO->status.v_fb[i] / pCraneStat->spec.accdec[i][ID_FWD][ID_DEC] + pCraneStat->spec.delay_time[i][ID_DELAY_CNT_DEC]);
		}
	}

	//自動完了条件
	double k = pCraneStat->mh_l * pCraneStat->mh_l;
	AgentInf_workbuf.sway_amp2m[ID_BOOM_H] = k * pSway_IO->amp2[ID_BOOM_H];
	AgentInf_workbuf.sway_amp2m[ID_SLEW] = k * pSway_IO->amp2[ID_SLEW];

	for(int i=0;i<MOTION_ID_MAX;i++) //目標位置までの距離
		AgentInf_workbuf.gap_from_target[i] = AgentInf_workbuf.pos_target[i] - pPLC_IO->status.pos[i];

	AgentInf_workbuf.gap2_from_target[ID_BOOM_H] = AgentInf_workbuf.gap_from_target[ID_BOOM_H] * AgentInf_workbuf.gap_from_target[ID_BOOM_H];
	AgentInf_workbuf.gap2_from_target[ID_SLEW] = AgentInf_workbuf.gap_from_target[ID_SLEW]* AgentInf_workbuf.gap_from_target[ID_SLEW];

	return 0;
}

/****************************************************************************/
/*　　PC制御選択セット処理													*/
/****************************************************************************/
int CAgent::set_pc_control() {

	//デバッグモード要求
	if (pPLC_IO->mode & PLC_IF_PC_DBG_MODE) {
		AgentInf_workbuf.pc_ctrl_mode |= (BITSEL_HOIST | BITSEL_GANTRY | BITSEL_BOOM_H | BITSEL_SLEW);
	}
	else {
		AgentInf_workbuf.pc_ctrl_mode = 0;
		if(AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_HOIST;
		if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_GANTRY;
		if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_BOOM_H;
		if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BITSEL_SLEW;
	}

	return 0;
}

/****************************************************************************/
/*   巻指令出力処理		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_HOIST) {
		if (AgentInf_workbuf.auto_active[ID_HOIST] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_HOIST] = pCraneStat->notch_spd_ref[ID_HOIST];
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

	LPST_COMMAND_SET pcom;

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_SLEW) {
		if (AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];
		else if(AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_JOB){
			pcom = &(pPolicyInf->job_com[pPolicyInf->i_jobcom]);
			AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
		}
		else {
			pcom = &(pPolicyInf->com[pPolicyInf->i_com]);
			if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_END) {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_STANDBY) {
				pcom->motion_stat[ID_SLEW].status = COMMAND_STAT_ACTIVE;
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else if (pcom->motion_stat[ID_SLEW].status == COMMAND_STAT_ACTIVE) {
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
			}
			else {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}

			AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pcom, ID_SLEW);
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

	if (AgentInf_workbuf.pc_ctrl_mode & BITSEL_BOOM_H) {
		
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL)
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_JOB) {
			pcom = &(pPolicyInf->job_com[pPolicyInf->i_jobcom]);
			AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pcom,ID_BOOM_H);
		}
		else {
			pcom = &(pPolicyInf->com[pPolicyInf->i_com]);
			if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_END) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
			else if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_STANDBY) {
				pcom->motion_stat[ID_BOOM_H].status = COMMAND_STAT_ACTIVE;
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
			else if (pcom->motion_stat[ID_BOOM_H].status == COMMAND_STAT_ACTIVE) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom,ID_BOOM_H);
			}
			else {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
		}
	}
	else {
		AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
	}

	return 0;
}

/****************************************************************************/
/*   コマンドステータス初期化                                               */
/****************************************************************************/
int CAgent::cleanup_command(LPST_COMMAND_SET pcom) {
	pcom->com_status = COMMAND_STAT_STANDBY;
	for (int i = 0; i < MOTION_ID_MAX;i++) {
		if (AgentInf_workbuf.auto_active[i] == AUTO_TYPE_MANUAL) {
			pcom->motion_stat[i].status = COMMAND_STAT_END;
		}
		else {
			pcom->motion_stat[i].status = COMMAND_STAT_STANDBY;
		}
		pcom->motion_stat[i].iAct = 0;
		pcom->motion_stat[i].step_act_count = 0;
		pcom->motion_stat[i].direction = AGENT_STOP;
		for (int j = 0; j < MOTION_STAT_FLG_N; j++) {
			pcom->motion_stat[i].flg[j] = L_OFF;
		}
	}
	GetLocalTime(&(pcom->time_start));	//開始時間セット
	return 0;
}

/****************************************************************************/
/*   STEP処理		                                                        */
/****************************************************************************/
double CAgent::cal_step(LPST_COMMAND_SET pCom,int motion) {

	double v_out = 0.0;

	LPST_MOTION_RECIPE precipe = &pCom->recipe[motion];
	LPST_MOTION_STAT pstat = &pCom->motion_stat[motion];
	LPST_MOTION_STEP pStep = &precipe->steps[pstat->iAct];

	if (pStep->status == COMMAND_STAT_STANDBY) {
		pstat->step_act_count= 1;
		pStep->status = COMMAND_STAT_ACTIVE;
	}
	else	pstat->step_act_count++;	//ステップ実行カウントインクリメント

	pStep->status = PTN_STEP_ON_GOING;

	switch (pStep->type) {
		//#	時間待機
	case CTR_TYPE_TIME_WAIT: {
		pstat->direction = AGENT_STOP;
		if (pstat->step_act_count > pStep->time_count) {
			pStep->status = PTN_STEP_FIN;
		}
		v_out = pStep->_v;
		return v_out;
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
		if (pSway_IO->amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
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
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F] - PI180;	//+で現在値が目標追い越し
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_F] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion] - PI180;	//+で現在値が目標追い越し
		}
		else {
			double chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];	//+で現在値が目標追い越し
		}
		if (chk_ph < 0)chk_ph *= -1.0;			//角度差絶対値
		if (chk_ph >= PI180) chk_ph -= PI360;	//小さい方の角度差

		if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_FWD;
			break;
		}

		//逆転開始用位相判定

		if ((pStep->opt_d[MOTHION_OPT_PHASE_R] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
			chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R] - PI180;	//+で現在値が目標追い越し
		}
		else if ((pStep->opt_d[MOTHION_OPT_PHASE_R] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
			chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion] - PI180;	//+で現在値が目標追い越し
		}
		else {
			double chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];	//+で現在値が目標追い越し
		}
		if (chk_ph < 0)chk_ph *= -1.0;			//角度差絶対値
		if (chk_ph >= PI180) chk_ph -= PI360;	//小さい方の角度差
		if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = PTN_STEP_FIN;
			pstat->direction = AGENT_REW;
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
		if (pSway_IO->amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {
			pStep->status = PTN_STEP_FIN;is_step_end = true;
		}
		if (is_step_end) {
			pstat->direction = AGENT_STOP;
			v_out = 0.0;
			break;
		}

		//メイン判定
		double chk_ph = PI360;
		if (pAgentInf->gap_from_target[motion] > 0.0) {	//目標位置よりも現在位置が手前
			//正転開始用位相判定

			if ((pStep->opt_d[MOTHION_OPT_PHASE_F] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
				chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_F] - PI180;	//+で現在値が目標追い越し
			}
			else if ((pStep->opt_d[MOTHION_OPT_PHASE_F] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
				chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion] - PI180;	//+で現在値が目標追い越し
			}
			else {
				double chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_F] - pSway_IO->ph[motion];	//+で現在値が目標追い越し
			}
			if (chk_ph < 0)chk_ph *= -1.0;			//角度差絶対値
			if (chk_ph >= PI180) chk_ph -= PI360;	//小さい方の角度差

			if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
				pStep->status = PTN_STEP_FIN;
				pstat->direction = AGENT_FWD;
				break;
			}
		}
		else {
			//逆転開始用位相判定

			if ((pStep->opt_d[MOTHION_OPT_PHASE_R] <= 0) && (pSway_IO->ph[motion] >= 0)) {	//目標が負　現在値が正
				chk_ph = pSway_IO->ph[motion] - pStep->opt_d[MOTHION_OPT_PHASE_R] - PI180;	//+で現在値が目標追い越し
			}
			else if ((pStep->opt_d[MOTHION_OPT_PHASE_R] >= 0) && (pSway_IO->ph[motion] <= 0)) {	//目標が正　現在値が負
				chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion] - PI180;	//+で現在値が目標追い越し
			}
			else {
				double chk_ph = pStep->opt_d[MOTHION_OPT_PHASE_R] - pSway_IO->ph[motion];	//+で現在値が目標追い越し
			}
			if (chk_ph < 0)chk_ph *= -1.0;			//角度差絶対値
			if (chk_ph >= PI180) chk_ph -= PI360;	//小さい方の角度差
			if (chk_ph < ph_chk_range[motion]) {	//目標位相に到達
				pStep->status = PTN_STEP_FIN;
				pstat->direction = AGENT_REW;
				break;
			}
		}

	}break;
	//#	振れ止め移動起動タイミング調整
	case CTR_TYPE_ADJUST_MOTION_TRIGGER: {
		v_out = 0.0;
		if (sqrt(pSway_IO->amp2[motion]) > pCraneStat->r0[motion]) {
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
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
	}break;
	//#	振れ止め加速
	case CTR_TYPE_ACC_AS: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;

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
		v_out = (double)pstat->direction * pStep->_v;
	}break;
	case CTR_TYPE_CONST_V_DEC_STEP:
	{
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pstat->flg[MOTION_DEC_STEP_BYPASS] == L_ON)	//初期振れが大きいとき1回の減速にするフラグ
			pStep->status = PTN_STEP_FIN;
		v_out = (double)pstat->direction * pStep->_v;
	}break;

	//#	定速（加減速→定速）
	case CTR_TYPE_CONST_V_TOP_STEP:
	{
		double chk_d = pStep->_p - pPLC_IO->status.pos[motion];
		if (chk_d < 0.0) chk_d *= -1.0; //STEP目標位置までの距離

		if ((pStep->_v >= 0.0) && (pPLC_IO->status.pos[motion] >= pStep->_p)) {
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if ((pStep->_v <= 0.0) && (pPLC_IO->status.pos[motion] <= pStep->_p)) {
			pStep->status = PTN_STEP_FIN;
			pstat->flg[MOTION_DEC_STEP_BYPASS] = L_OFF;
		}
		else if (chk_d < AgentInf_workbuf.dist_for_stop[motion]) { // STEP目標位置までの距離が減速距離以下
			if (sqrt(pSway_IO->amp2[motion]) > pCraneStat->r0[motion]) {
				if (AgentInf_workbuf.gap_from_target[motion] > 0) {//目標位置より現在位置が手前→進行方向＋
					if ((pSway_IO->ph[motion] < PI180) && (pSway_IO->ph[motion] > PI90))
						pStep->status = PTN_STEP_FIN;
				}
				else {
					if ((pSway_IO->ph[motion] < 0.0) && (pSway_IO->ph[motion] > -PI90))
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

		v_out = (double)pstat->direction * pStep->_v;
	}break;

	//#	定速（加減速→定速）
	case CTR_TYPE_FINE_POSITION: {
		if (pstat->step_act_count >= pStep->time_count)	pStep->status = PTN_STEP_FIN;
		if (pAgentInf->gap2_from_target[motion] < pCraneStat->spec.as_pos2_level[motion][ID_LV_COMPLE])
			pStep->status = PTN_STEP_FIN;

		v_out = (double)pstat->direction * pStep->_v;
	}break;
	}

	if (pStep->status == PTN_STEP_FIN) {
		pstat->iAct++;
		pstat->step_act_count = 0;
	}
	return v_out;
}

/****************************************************************************/
/*   運転操作IF　Command受付（PB,Lamp）												*/
/****************************************************************************/
int CAgent::receipt_ope_com(int type, int target) {
	switch (type) {
	case OPE_COM_PB_SET:
		AgentInf_workbuf.PLC_PB_com[target] = AGENT_PB_OFF_DELAY;
		break;
	case OPE_COM_LAMP_ON:
		AgentInf_workbuf.PLC_LAMP_com[target] = PLC_IO_LAMP_FLICKER_CHANGE;
		break;
	case OPE_COM_LAMP_OFF:
		AgentInf_workbuf.PLC_LAMP_com[target] = 0;
		break;
	case OPE_COM_LAMP_FLICKER:
		AgentInf_workbuf.PLC_LAMP_com[target] = PLC_IO_LAMP_FLICKER_COUNT;
		break;
	case OPE_COM_SEMI_LAMP_ON:
		AgentInf_workbuf.PLC_LAMP_semiauto_com[target] = PLC_IO_LAMP_FLICKER_CHANGE;
		break;
	case OPE_COM_SEMI_LAMP_OFF:
		AgentInf_workbuf.PLC_LAMP_semiauto_com[target] = 0;
		break;
	case OPE_COM_SEMI_LAMP_FLICKER:
		AgentInf_workbuf.PLC_LAMP_semiauto_com[target] = PLC_IO_LAMP_FLICKER_CHANGE;
		break;

	default:
		break;
	}
	return 0;
};

/****************************************************************************/
/*  自動起動可否チェック													*/
/****************************************************************************/
bool CAgent::can_auto_trigger()
{
	if ((pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_ACTIVE) &&
		(pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_STANDBY) &&
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_ACTIVE)&&
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_STANDBY)) {

		if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
			//引込,旋回の0速確認で起動可
			if (AgentInf_workbuf.is_spdfb_0[ID_SLEW] && AgentInf_workbuf.is_spdfb_0[ID_BOOM_H]) {
				return true;
			}
		}
		else return true;
	}
	else {
		return false;
	}

	return false;
}

/****************************************************************************/
/*  自動完了判定															*/
/****************************************************************************/
bool CAgent::can_auto_complete() {

	//実行中または実行待ちコマンド有
	if ((pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_ACTIVE) ||
		(pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_STANDBY) ||
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_ACTIVE) ||
		(pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_STANDBY))
		return false;
	
	//振れ止めは振れ振幅小かつ目標位置到着で完了
	if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_ANTI_SWAY) {
		if ((AgentInf_workbuf.sway_amp2m[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.sway_amp2m[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE]))
			return true;
		else return false;
	}
	//半自動は目標位置到着で完了
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_SEMI_AUTO) {
		if ((AgentInf_workbuf.gap2_from_target[ID_BOOM_H] < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (AgentInf_workbuf.gap2_from_target[ID_SLEW] < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE]))
			return true;
		else return false;
	}
	else if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_JOB) {
		if (pCSInf->n_job_standby < 1) //待機ジョブ無し
			return true;
		else 
			return false;
	}
	else;

	return true;
}

/****************************************************************************/
/*  個別軸毎に実行中の自動タイプセット											*/
/****************************************************************************/
void CAgent::set_auto_active(int type) {
	if (type == AUTO_TYPE_ANTI_SWAY) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_ANTI_SWAY;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_ANTI_SWAY;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else if (type == AUTO_TYPE_SEMI_AUTO) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_SEMI_AUTO;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_SEMI_AUTO;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else if (type == AUTO_TYPE_JOB) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_JOB;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
	else {
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_TROLLY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_OP_ROOM] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_H_ASSY] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_COMMON] = AUTO_TYPE_MANUAL;
	}
}

/****************************************************************************/
/*  実行自動タイプ（全体）セット,　自動レシピセット											*/
/****************************************************************************/
int CAgent::update_auto_setting() {
	
	//自動起動処理
	if (pCraneStat->auto_standby == false) {//自動モード
		for (int i = 0; i < NUM_OF_AS_AXIS; i++) AgentInf_workbuf.pos_target[i] = pPLC_IO->status.pos[i];
		if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
			set_auto_active(AUTO_TYPE_MANUAL);
			pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_END;
			pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_END;
		}
	}
	else if ((pCraneStat->is_notch_0[ID_SLEW] == false || pCraneStat->is_notch_0[ID_BOOM_H]) == false) {
		if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
			set_auto_active(AUTO_TYPE_MANUAL);
			if (pPolicyInf->com[pPolicyInf->i_com].com_status != COMMAND_STAT_END) pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status != COMMAND_STAT_END) pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ABORT;
		}
	}
	else if(can_auto_trigger()) {
		if ((pCraneStat->semi_auto_selected != SEMI_AUTO_TG_CLR)
			&&(pCraneStat->auto_start_pb_count > AGENT_AUTO_TRIG_ACK_COUNT)) {	//半自動設定有で自動開始PB ON
			pCom = pPolicy->generate_command(AUTO_TYPE_SEMI_AUTO, AgentInf_workbuf.pos_target);
			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_SEMI_AUTO;
				set_auto_active(AUTO_TYPE_SEMI_AUTO);
				cleanup_command(pCom);
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);
			}
		}
		else if ((pCSInf->n_job_standby > 0) 
			&& (pCraneStat->auto_start_pb_count > AGENT_AUTO_TRIG_ACK_COUNT)) {
			pCom = pPolicy->generate_command(AUTO_TYPE_JOB, AgentInf_workbuf.pos_target);
			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_JOB;
				set_auto_active(AUTO_TYPE_JOB);
				cleanup_command(pCom);
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);
			}
		}
		else {
			pCom = pPolicy->generate_command(AUTO_TYPE_ANTI_SWAY, AgentInf_workbuf.pos_target);
			if (pCom != NULL) {
				AgentInf_workbuf.auto_on_going = AUTO_TYPE_ANTI_SWAY;
				set_auto_active(AUTO_TYPE_ANTI_SWAY);
				cleanup_command(pCom);
			}
			else {
				set_auto_active(AUTO_TYPE_MANUAL);
			}
		}
	}
	else;

	//コマンド実行完了判定
	if (pPolicyInf->com[pPolicyInf->i_com].com_status == COMMAND_STAT_ACTIVE) {
		bool is_active_motion_there = false;
		for (int i = 0; i < MOTION_ID_MAX;i++) {
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ERROR)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ERROR;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ABORT)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_PAUSE)
				pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_PAUSE;
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_ACTIVE) {
				is_active_motion_there = true;
				break;
			}
			if (pPolicyInf->com[pPolicyInf->i_com].motion_stat[i].status == COMMAND_STAT_STANDBY) {
				is_active_motion_there = true;
				break;
			}
		}
		if (is_active_motion_there == false)
			pPolicyInf->com[pPolicyInf->i_com].com_status = COMMAND_STAT_END;
	}

	if (pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status == COMMAND_STAT_ACTIVE) {
		bool is_active_motion_there = false;
		for (int i = 0; i < MOTION_ID_MAX;i++) {
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ERROR)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ERROR;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ABORT)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_ABORT;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_PAUSE)
				pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_PAUSE;
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_ACTIVE) {
				is_active_motion_there = true;
				break;
			}
			if (pPolicyInf->job_com[pPolicyInf->i_jobcom].motion_stat[i].status == COMMAND_STAT_STANDBY) {
				is_active_motion_there = true;
				break;
			}
		}
		if (is_active_motion_there == false)
			pPolicyInf->job_com[pPolicyInf->i_jobcom].com_status = COMMAND_STAT_END;
	}

	//自動完了判定
	if (AgentInf_workbuf.auto_on_going != AUTO_TYPE_MANUAL) {
		if (can_auto_complete()) {
			pCom = NULL;
			AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
			set_auto_active(AUTO_TYPE_MANUAL);
		}
	}
	return 0;
};

/****************************************************************************/
/*  PB,ランプ指令更新														*/
/****************************************************************************/
void CAgent::update_pb_lamp_com() {
	//PB ON状態を一定時間ホールド
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_ESTOP]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_ON] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_ON]--;
	if (AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE2_OFF] > 0)AgentInf_workbuf.PLC_PB_com[ID_PB_CTRL_SOURCE_OFF]--;

	//LAMP
	if (pCraneStat->auto_standby) {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_ON;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_OFF;
	}
	else {
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] = AGENT_LAMP_OFF;
		AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = AGENT_LAMP_ON;
	}
	
	//LAMP　カウント値　0：消灯　カウント値%PLC_IO_LAMP_FLICKER_COUNT　が　PLC_IO_LAMP_FLICKER_CHANGE以下でON,以上でOFF（PLC_IFにて出力）
	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_ON] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_ANTISWAY_OFF]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] <= 0)AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[ID_PB_AUTO_START]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG1]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG2]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3] >PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG3]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG4]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG5]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG6]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG7]++;
	else;

	if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] <= 0)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] = 0;
	else if (AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8] > PLC_IO_LAMP_FLICKER_CHANGE)AgentInf_workbuf.PLC_LAMP_com[SEMI_AUTO_TG8]++;
	else;

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

