#include "CAgent.h"
#include "CPolicy.h"
#include "CEnvironment.h"
#include "CClientService.h"
#include "CHelper.h"

//-共有メモリオブジェクトポインタ:
extern CSharedMem* pCraneStatusObj;
extern CSharedMem* pSwayStatusObj;
extern CSharedMem* pPLCioObj;
extern CSharedMem* pSwayIO_Obj;
extern CSharedMem* pRemoteIO_Obj;
extern CSharedMem* pCSInfObj;
extern CSharedMem* pPolicyInfObj;
extern CSharedMem* pAgentInfObj;
extern CSharedMem* pJobIO_Obj;

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
	pJob_IO = (LPST_JOB_IO)(pJobIO_Obj->get_pMap());

	pPolicy = (CPolicy*)VectpCTaskObj[g_itask.policy];
	pEnv = (CEnvironment*)VectpCTaskObj[g_itask.environment];
	pCS = (CClientService*)VectpCTaskObj[g_itask.client];

	pCom_as = &(pAgentInf->st_as_comset);
	pCom_hot = NULL;
	pjob_active = NULL;

	for (int i = 0;i < N_PLC_PB;i++) AgentInf_workbuf.PLC_PB_com[i] =0;

	AgentInf_workbuf.auto_on_going = AUTO_TYPE_MANUAL;
	pjob_active = NULL;

	//軸ステータスクリア
	for (int i = 0;i < MOTION_ID_MAX;i++) {
		AgentInf_workbuf.axis_status[i] = 0;
		AgentInf_workbuf.as_count[i] = 0;
	}
	AgentInf_workbuf.command_count = 0;

	set_panel_tip_txt();

	AgentInf_workbuf.st_as_comset.com_code.i_list = ID_JOBTYPE_ANTISWAY;//コマンドセットのタイプを振れ止めに設定

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
	
	//# 各軸条件設定axis_status（FB0,AUTO_ENABLE...)
	{

		//振れ止め自動実行可設定
		AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_ANTISWAY_ENABLE;
		AgentInf_workbuf.axis_status[ID_SLEW]	|= AG_AXIS_STAT_ANTISWAY_ENABLE;

		//速度0状態設定
		if (pEnv->is_speed_0(ID_HOIST))	AgentInf_workbuf.axis_status[ID_HOIST]	|= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_HOIST]	&= ~AG_AXIS_STAT_FB0;
		if (pEnv->is_speed_0(ID_BOOM_H))AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_BOOM_H] &= ~AG_AXIS_STAT_FB0;
		if (pEnv->is_speed_0(ID_SLEW))	AgentInf_workbuf.axis_status[ID_SLEW]	|= AG_AXIS_STAT_FB0;
		else							AgentInf_workbuf.axis_status[ID_SLEW]	&= ~AG_AXIS_STAT_FB0;
	}
	
	
	//# JOBコマンド設定
	{
		if (pCSInf->auto_mode == L_ON) {	//自動モード
			if (pjob_active == NULL) {		//ジョブ実行中でない
				pjob_active = pCS->get_next_job();								//CSにジョブ問い合わせ
				if (pjob_active != NULL) {										//CSにジョブ有
					pCom_hot = pPolicy->req_command(pjob_active);				//POLICYにコマンド展開依頼 pjob NULLならばNULLが帰ってくる
					if (pCom_hot != NULL) {
						init_comset(pCom_hot);									//コマンドのフラグ初期化
						pPolicy->update_command_status(pCom_hot, STAT_ACTIVE);	//コマンド実行開始報告
						AgentInf_workbuf.command_count++;						//コマンドレシピ作成カウント モニタ用
					}
					else {
						pjob_active = NULL;
					}
				}
				else {															//ジョブ無し
					pCom_hot = NULL;
				}
			}
			else {
				if (pCom_hot != NULL) {	//ジョブ実行中でコマンド実行中
					int com_complete = STAT_END;
					for (int i = 0;i < MOTION_ID_MAX;i++) {
						if (!(pCom_hot->motion_status[i] & STAT_END)) {//未完動作有
							com_complete = STAT_ACTIVE;
						}
						else {
							if (pCom_hot->motion_status[i] != STAT_END) {
								com_complete = pCom_hot->motion_status[i];
							}
						}
					}
					if (com_complete != STAT_ACTIVE) {								//コマンド完了
						AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_JOB_MASK;		//自動制御JOB TYPEクリア
						pPolicy->update_command_status(pCom_hot, com_complete);		//コマンド実行完了報告
						pCom_hot = NULL;											//コマンドクリア
					}
					else {
						;//現状維持
					}
				}
				else {															//ジョブ実行中でコマンド無し
					pCom_hot = pPolicy->req_command(pjob_active);				//POLICYにコマンド展開依頼 pjob NULLならばNULLが帰ってくる
					if (pCom_hot != NULL) {										//次のコマンド有
						init_comset(pCom_hot);
						pPolicy->update_command_status(pCom_hot, STAT_ACTIVE);	//コマンド実行開始報告
						AgentInf_workbuf.command_count++;						//コマンドレシピ作成カウント モニタ用
						//自動制御モードのセット
					}
					else {
						pjob_active = NULL;
					}
				}
			}
		}
		else {//自動モードOFF コマンド実行中であればABORT報告	
			if (pCom_hot != NULL) {
				pPolicy->update_command_status(pCom_hot, STAT_ABOTED);	//コマンド実行開始報告
			}
			pCom_hot = NULL;
			pjob_active = NULL;
		}
	}
		
	return;
};

/****************************************************************************/
/*   定周期処理手順2　メイン処理											*/
/****************************************************************************/

static int auto_on_going_last = AUTO_TYPE_MANUAL;
static INT32 notch0_last;
static LPST_COMMAND_SET pCom_hot_last;

void CAgent::main_proc() {


	//# 制御モードセット auto_on_going,antisway_on_going
	{
		//自動制御モードのセット
		if ((pCSInf->auto_mode == L_ON) && (pjob_active != NULL)) {//自動制御モードONで実行中JOB有
			if (pjob_active->type == ID_JOBTYPE_SEMI)		AgentInf_workbuf.auto_on_going |= AUTO_TYPE_SEMIAUTO;
			else if (pjob_active->type == ID_JOBTYPE_JOB)	AgentInf_workbuf.auto_on_going |= AUTO_TYPE_JOB;
			else;
		}
		else {
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_SEMIAUTO;
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_JOB;
		}
		
		//振れ止めモードセット
		if (pCSInf->antisway_mode == L_OFF) {
			//auto_on_going
			AgentInf_workbuf.auto_on_going &= ~AUTO_TYPE_FB_ANTI_SWAY;
			//antisway_on_going
			AgentInf_workbuf.antisway_on_going = ANTISWAY_ALL_MANUAL;
		}
		else {
			AgentInf_workbuf.auto_on_going |= AUTO_TYPE_FB_ANTI_SWAY;

			if (pCraneStat->notch0 & BIT_SEL_BH) {							//0ノッチ
				AgentInf_workbuf.antisway_on_going |= ANTISWAY_BH_ACTIVE;
				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_PAUSED;
			}
			else {															//ノッチ入り
				AgentInf_workbuf.antisway_on_going |= ANTISWAY_BH_PAUSED;
				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_ACTIVE;
			}

			if (pCraneStat->notch0 & BIT_SEL_SLW) {							//0ノッチ
				AgentInf_workbuf.antisway_on_going |= ANTISWAY_SLEW_ACTIVE;
				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_PAUSED;
			}
			else {															//ノッチ入り
				AgentInf_workbuf.antisway_on_going |= ANTISWAY_SLEW_PAUSED;
				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_ACTIVE;
			}
		}
	}

	//# 振れ止め完了状態セット　antisway_on_going
	{
		double tmp_amp2, tmp_dist;
		//引込
		//振れ振幅、位置ずれともに完了判定レベル以内
		tmp_amp2 = pEnv->cal_sway_r_amp2_m();
		tmp_dist = pEnv->cal_dist4target(ID_BOOM_H, true);
		//振幅、位置とも制御完了レベル以内
		if ((tmp_amp2 < pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_COMPLE])
			&& (tmp_dist < pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_COMPLE])) {

			AgentInf_workbuf.antisway_on_going |= ANTISWAY_BH_COMPLETE;
		}
		//振れ止め完了状態
		else if (pAgentInf->antisway_on_going & ANTISWAY_BH_COMPLETE) {	//振れ止め完了フラグON
			//振幅または位置が振れ止め起動レベル越で完了フラグクリア
			if ((pEnv->cal_sway_r_amp2_m() > pCraneStat->spec.as_m2_level[ID_BOOM_H][ID_LV_TRIGGER])
				|| (pEnv->cal_dist4target(ID_BOOM_H, true) > pCraneStat->spec.as_pos_level[ID_BOOM_H][ID_LV_TRIGGER]))

				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_COMPLETE;
		}
		else {
			AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_BH_COMPLETE;
		}

		//旋回
		tmp_amp2 = pEnv->cal_sway_th_amp2_m();
		tmp_dist = pEnv->cal_dist4target(ID_SLEW, true);
		//振幅、位置とも制御完了レベル以内
		if ((tmp_amp2 < pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_COMPLE])
			&& (tmp_dist < pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_COMPLE])) {

			AgentInf_workbuf.antisway_on_going |= ANTISWAY_SLEW_COMPLETE;

		}
		//振れ止め完了状態
		else if (pAgentInf->antisway_on_going & ANTISWAY_SLEW_COMPLETE) {
			if ((pEnv->cal_sway_th_amp2_m() > pCraneStat->spec.as_m2_level[ID_SLEW][ID_LV_TRIGGER])
				|| (pEnv->cal_dist4target(ID_SLEW, true) > pCraneStat->spec.as_pos_level[ID_SLEW][ID_LV_TRIGGER]))

				AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_COMPLETE;
		}
		else {
			AgentInf_workbuf.antisway_on_going &= ~ANTISWAY_SLEW_COMPLETE;
		}
	}

	//# 軸毎のモードセット(PLC指令出力軸の判定）
	if (AgentInf_workbuf.auto_on_going == AUTO_TYPE_MANUAL) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
	}
	else if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_SEMIAUTO) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB)) {
		AgentInf_workbuf.auto_active[ID_HOIST] = AgentInf_workbuf.auto_active[ID_BOOM_H] = AgentInf_workbuf.auto_active[ID_SLEW] = AgentInf_workbuf.auto_on_going;
	}
	else {//FB振れ止め
		if (AgentInf_workbuf.auto_on_going & AUTO_TYPE_FB_ANTI_SWAY) {
			if(pCraneStat->notch0 & BIT_SEL_BH)	AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_FB_ANTI_SWAY;
			else								AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_MANUAL;
			
			if (pCraneStat->notch0 & BIT_SEL_SLW)	AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_FB_ANTI_SWAY;
			else									AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
		}
		else {
			AgentInf_workbuf.auto_active[ID_BOOM_H] = AUTO_TYPE_MANUAL;
			AgentInf_workbuf.auto_active[ID_SLEW] = AUTO_TYPE_MANUAL;
		}
		AgentInf_workbuf.auto_active[ID_HOIST] = AUTO_TYPE_MANUAL;
		AgentInf_workbuf.auto_active[ID_GANTRY] = AUTO_TYPE_MANUAL;
	}
	//#振れ止めコマンドセットクリア
	{
		if (!(AgentInf_workbuf.auto_on_going & AUTO_TYPE_FB_ANTI_SWAY)) {
			AgentInf_workbuf.st_as_comset.motion_status[ID_BOOM_H] = STAT_END;
			AgentInf_workbuf.st_as_comset.motion_status[ID_SLEW] = STAT_END;
		}
	}

	//#自動目標位置設定
	{
		//JOBトリガ検出
		if ((pCom_hot != NULL) && (pCom_hot != pCom_hot_last)) {
			AgentInf_workbuf.auto_pos_target = pCom_hot->target;
		}
		//JOB無し
		else if (pCom_hot == NULL) {
			//巻は現在位置
			AgentInf_workbuf.auto_pos_target.pos[ID_HOIST] = pPLC_IO->status.pos[ID_HOIST];

			//引込、旋回はノッチ入りまたは振れ止めOFFで現在位置　0ノッチトリガで現在位置＋減速距離位置
			//0ノッチトリガで現在位置＋減速距離位置に設定
			if (!(notch0_last & BIT_SEL_BH) && (pCraneStat->notch0 & BIT_SEL_BH)) {
				AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H] + pEnv->cal_dist4stop(ID_BOOM_H, false);
			}
			//ノッチ入りまたは振れ止めモードOFFで現在位置
			else if (!(pCraneStat->notch0 & BIT_SEL_BH) || (pCSInf->antisway_mode != L_ON)) {
				AgentInf_workbuf.auto_pos_target.pos[ID_BOOM_H] = pPLC_IO->status.pos[ID_BOOM_H];
			}
			else;

			//0ノッチトリガで現在位置＋減速距離位置に設定
			if (!(notch0_last & BIT_SEL_SLW) && (pCraneStat->notch0 & BIT_SEL_SLW)) { //0ノッチトリガ
				AgentInf_workbuf.auto_pos_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW] + pEnv->cal_dist4stop(ID_SLEW, false);
			}
			//ノッチ入りまたは振れ止めモードOFFで現在位置
			else if (!(pCraneStat->notch0 & BIT_SEL_SLW) || (pCSInf->antisway_mode != L_ON)) {
				AgentInf_workbuf.auto_pos_target.pos[ID_SLEW] = pPLC_IO->status.pos[ID_SLEW];
			}
			else;
		}
		else;

		//# 前回値保持
		notch0_last = pCraneStat->notch0;
		pCom_hot_last = pCom_hot;

	}


	//各軸の制御状態設定	axis_status[i]
	{
		//PC指令可設定
		for (int i = 0;i < MOTION_ID_MAX;i++) {
			AgentInf_workbuf.axis_status[i] |= AG_AXIS_STAT_PC_ENABLE;
		}

		//自動実行可設定
		AgentInf_workbuf.axis_status[ID_HOIST] |= AG_AXIS_STAT_AUTO_ENABLE;
		AgentInf_workbuf.axis_status[ID_BOOM_H] |= AG_AXIS_STAT_AUTO_ENABLE;
		AgentInf_workbuf.axis_status[ID_SLEW] |= AG_AXIS_STAT_AUTO_ENABLE;
	}
	//# PLCへのPC選択指令セット
	{
		if ((pPLC_IO->mode & PLC_IF_PC_DBG_MODE) || (AgentInf_workbuf.auto_on_going & AUTO_TYPE_OPERATION)) {//デバッグモード　または　リモート手動操作モード
			AgentInf_workbuf.pc_ctrl_mode |= (BIT_SEL_HST | BIT_SEL_GNT | BIT_SEL_BH | BIT_SEL_SLW);
		}
		else {
			AgentInf_workbuf.pc_ctrl_mode = 0;
			if (AgentInf_workbuf.auto_active[ID_HOIST])  AgentInf_workbuf.pc_ctrl_mode |= BIT_SEL_HST;
			if (AgentInf_workbuf.auto_active[ID_GANTRY])  AgentInf_workbuf.pc_ctrl_mode |= BIT_SEL_GNT;
			if (AgentInf_workbuf.auto_active[ID_BOOM_H])  AgentInf_workbuf.pc_ctrl_mode |= BIT_SEL_BH;
			if (AgentInf_workbuf.auto_active[ID_SLEW])  AgentInf_workbuf.pc_ctrl_mode |= BIT_SEL_SLW;
		}
	}
		
		
	//# PLCへの出力計算　
	set_ref_mh();							//巻き速度指令
	set_ref_gt();							//走行速度指令
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
	int a = sizeof(ST_AGENT_INFO);
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
/*   巻指令出力処理		                                                    */
/****************************************************************************/
int CAgent::set_ref_mh(){
	if (AgentInf_workbuf.pc_ctrl_mode & BIT_SEL_HST) {
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
	if (AgentInf_workbuf.pc_ctrl_mode & BIT_SEL_GNT) {
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

	if (AgentInf_workbuf.pc_ctrl_mode & BIT_SEL_SLW) {										//制御PC指令選択ON
		//マニュアルモード
		if (AgentInf_workbuf.auto_active[ID_SLEW] == AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.v_ref[ID_SLEW] = pCraneStat->notch_spd_ref[ID_SLEW];
		}
		//JOB実行時
		else if((AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_JOB)||
				(AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_SEMIAUTO)){					//自動運転中
			if (pCom_hot == NULL) {
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else {
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(pCom_hot, ID_SLEW);
			}
		}
		else if (AgentInf_workbuf.auto_active[ID_SLEW] & AUTO_TYPE_FB_ANTI_SWAY) {				//振れ止め中
			if (AgentInf_workbuf.antisway_on_going & ANTISWAY_SLEW_COMPLETE){					//振れ止め完了
				AgentInf_workbuf.v_ref[ID_SLEW] = 0.0;
			}
			else {																				//振れ止め未完
				AgentInf_workbuf.v_ref[ID_SLEW] = cal_step(&AgentInf_workbuf.st_as_comset, ID_SLEW);
			}
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

	if (AgentInf_workbuf.pc_ctrl_mode & BIT_SEL_BH) {									//制御PC指令選択ON
		//マニュアルモード
		if (AgentInf_workbuf.auto_active[ID_BOOM_H] == AUTO_TYPE_MANUAL) {
			AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
		}
		//JOB実行時
		else if ((AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_JOB) ||
			(AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_SEMIAUTO)) {					
			if (pCom_hot == NULL) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = 0.0;
			}
			else {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(pCom_hot, ID_BOOM_H);
			}
		}
		//FB振れ止め
		else if (AgentInf_workbuf.auto_active[ID_BOOM_H] & AUTO_TYPE_FB_ANTI_SWAY) {	//振れ止め中
			if (!(AgentInf_workbuf.antisway_on_going & ANTISWAY_BH_ACTIVE) || (AgentInf_workbuf.antisway_on_going & ANTISWAY_BH_PAUSED)) {
				AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
			}
			else if (AgentInf_workbuf.antisway_on_going & ANTISWAY_BH_COMPLETE) {			//振れ止め完了
				AgentInf_workbuf.v_ref[ID_BOOM_H] = pCraneStat->notch_spd_ref[ID_BOOM_H];
			}
			else { 																		//振れ止め未完
				AgentInf_workbuf.v_ref[ID_BOOM_H] = cal_step(&AgentInf_workbuf.st_as_comset,ID_BOOM_H);
			}
		}
		//その他
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
int CAgent::init_comset(LPST_COMMAND_SET pcom) {
	if (pcom->com_code.i_list == ID_JOBTYPE_ANTISWAY) {
		for (int i = 0; i < MOTION_ID_MAX;i++) 
			pcom->motion_status[i] = STAT_END;
		pcom->motion_status[ID_BOOM_H]	= STAT_STANDBY;
		pcom->motion_status[ID_SLEW]	= STAT_STANDBY;
		pcom->recipe->i_hot_step = 0;
		pcom->recipe->motion_act_count = 0;
	}
	else {
		for (int i = 0; i < MOTION_ID_MAX;i++) {						//各軸の実行ステータスの初期化
			if (pcom->active_motion[i] == L_ON) pcom->motion_status[i]	= STAT_STANDBY;
			else								pcom->motion_status[i]	= STAT_END;
			pcom->recipe->i_hot_step = 0;
			pcom->recipe->motion_act_count = 0;
		}
	}
	pcom->com_status = STAT_STANDBY;
	return 0;
}
/****************************************************************************/
/*	振れ止め関連設定					*/
/****************************************************************************/

//振れ止めパターン計算用作業バッファセット
void CAgent::set_as_workbuf(int motion) {

	st_as_work.agent_scan_ms = inf.cycle_ms;						//AGENTタスクのスキャンタイム

	double temp_d;

	//現在位置
	st_as_work.pos[motion] = pPLC_IO->status.pos[motion];
	//現在速度
	st_as_work.v[motion] = pPLC_IO->status.v_fb[motion];
	//移動距離
	temp_d = AgentInf_workbuf.auto_pos_target.pos[motion] - st_as_work.pos[motion];
	if (motion == ID_SLEW) {									//旋回は、180を越えるときは逆方向が近い
		if (temp_d > PI180)	temp_d -= PI360;
		else if (temp_d < -PI180) temp_d += PI360;
		else;
	}
	st_as_work.dist_for_target[motion] = temp_d;				//移動距離
	if (temp_d < 0.0)	st_as_work.dist_for_target_abs[motion] = -1.0 * temp_d;	//移動距離絶対値
	else				st_as_work.dist_for_target_abs[motion] = temp_d;

	//動作軸加速度
	st_as_work.a_abs[motion] = pCraneStat->spec.accdec[motion][FWD][ACC];
	//最大速度
	st_as_work.vmax[motion] = pCraneStat->spec.notch_spd_f[motion][NOTCH_MAX - 1];
	//最大加速時間
	st_as_work.acc_time2Vmax[motion] = st_as_work.vmax[motion] / st_as_work.a_abs[motion];

	if ((motion == ID_BOOM_H) || (motion == ID_SLEW)) {
		//吊点の加速度
		st_as_work.a_hp_abs[motion] = pEnv->cal_hp_acc(motion, ID_FWD);
		if (st_as_work.a_hp_abs[motion] < 0.0)st_as_work.a_hp_abs[motion] *= -1.0;
		//加速時振れ中心
//		st_as_work.pp_th0[motion][ACC] = pEnv->cal_arad_acc(motion, FWD);
		//減速時振れ中心
//		st_as_work.pp_th0[motion][DEC] = pEnv->cal_arad_dec(motion, FWD);
	}

	st_as_work.T = pCraneStat->T;															//振れ周期
	st_as_work.w = pCraneStat->w;															//振れ角周波数
	st_as_work.w2 = pCraneStat->w2;


	 double rs_m;
	if (motion == ID_BOOM_H) {
		rs_m = sqrt(pEnv->cal_sway_r_amp2_m());
	}
	else if (motion == ID_SLEW) {
		rs_m = sqrt(pEnv->cal_sway_th_amp2_m());
	}
	else {
		rs_m = 0.0;
	}

	//振れ止めパターンとゲイン（加速時間）
	st_as_work.as_ptn_type[motion] = AGENT_AS_PTN_PAUSE;

	double r0 = st_as_work.a_hp_abs[motion] / GA;
	double rs = sqrt(pSway_IO->rad_amp2[motion]);


	if (st_as_work.dist_for_target_abs[motion] > pCraneStat->spec.as_pos_level[motion][ID_LV_TRIGGER]) {			//目標までの距離大→位置合わせ
		st_as_work.as_ptn_type[motion] = AGENT_AS_PTN_POS_IN;
		//st_as_work.as_gain_ta[motion] = sqrt(st_as_work.dist_for_target_abs[motion] / st_as_work.a_hp_abs[motion]);
		double cos_ph = 1 - 0.5* rs / r0;
		if (cos_ph < -1.0) {
			st_as_work.as_gain_ta[motion] = st_as_work.T / 2.0;
		}
		else {
			//			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.a_hp_abs[motion];
			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.w;
		}
	}
	else if(rs_m < pCraneStat->spec.as_m_level[motion][ID_LV_TRIGGER]){												//目標までの距離小で振れが小さい→位置合わせ
		st_as_work.as_ptn_type[motion] = AGENT_AS_PTN_POS_IN;
		//st_as_work.as_gain_ta[motion] = sqrt(st_as_work.dist_for_target_abs[motion] / st_as_work.a_hp_abs[motion]);
		double cos_ph = 1 - 0.5 * rs / r0;
		if (cos_ph < -1.0) {
			st_as_work.as_gain_ta[motion] = st_as_work.T / 2.0;
		}
		else {
//			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.a_hp_abs[motion];
			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.w;
		}
	}
	else if (rs_m > pCraneStat->spec.as_m_level[motion][ID_LV_COMPLE]) {
		st_as_work.as_ptn_type[motion] = AGENT_AS_PTN_POS_OUT;

		double cos_ph = 1 - rs / (4.0 * r0);
		if (cos_ph < -1.0) {
			st_as_work.as_gain_ta[motion] = st_as_work.T / 2.0;
		}
		else {
			//			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.a_hp_abs[motion];
			st_as_work.as_gain_ta[motion] = acos(cos_ph) / st_as_work.w;
		}
	}
	else {
		st_as_work.as_ptn_type[motion] = AGENT_AS_PTN_FINE_POS;//微小位置合わせは1ノッチ速度
		st_as_work.as_gain_ta[motion] = pCraneStat->spec.notch_spd_f[motion][1] / st_as_work.a_abs[motion];
	}

	//インチングの加速時間は振れ周期を上限とする。
	if (st_as_work.as_gain_ta[motion] > st_as_work.T) st_as_work.as_gain_ta[motion] = st_as_work.T;

	//インチングの加速時間は最大加速時間が上限。
	if (st_as_work.as_gain_ta[motion] > st_as_work.acc_time2Vmax[motion]) st_as_work.as_gain_ta[motion] = st_as_work.acc_time2Vmax[motion];


	if ((st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_POS_OUT)|| (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_FINE_POS)) {
		st_as_work.motion_dir[motion] = ID_SELECT;				//振り出しの移動方向は先に来た位相で決める
	}
	else if (st_as_work.dist_for_target[motion] < 0.0) {
		st_as_work.motion_dir[motion] = ID_REV;					//移動方向
	}
	else if (temp_d > 0.0) {
		st_as_work.motion_dir[motion] = ID_FWD;					//移動方向
	}
	else {
		st_as_work.motion_dir[motion] = ID_STOP;				//移動方向
	}


	//振れ止め起動位相
	st_as_work.as_ph[motion] = st_as_work.as_gain_ta[motion]* st_as_work.w;// 起動位相は　移動方向によって　th0 - ph = nπ(n=0 or -π）

	st_as_work.as_ph_chk_range[motion] = AGENT_AS_PH_CHECK_RANGE;

	return;
}


//振れ止めパターンレシピ計算
int CAgent::cal_as_recipe(int motion) {

	set_as_workbuf(motion);
	AgentInf_workbuf.st_as_comset.com_code.i_list = ID_JOBTYPE_ANTISWAY;//コマンドコード　振れ止めタイプセット

	/*### レシピ設定 ###*/
	LPST_MOTION_RECIPE precipe = &AgentInf_workbuf.st_as_comset.recipe[motion];
	precipe->n_step = 0;								// ステップクリア
	precipe->direction = st_as_work.motion_dir[motion];
	precipe->i_hot_step = 0;
	precipe->motion_act_count = 0;
	precipe->axis_id = motion;
	precipe->fin_code = STAT_NA;

	LPST_MOTION_STEP pelement = &precipe->steps[0];

	/*### ステップ設定 ###*/
	//#### STEP 0 起動待機
	pelement = &(precipe->steps[precipe->n_step++]);											//ステップのポインタセットして次ステップ用にカウントアップ
	pelement->_v = 0.0;																			// 速度0
	pelement->_p = st_as_work.pos[motion];														// 目標位置
	pelement->status = STAT_STANDBY;															// ステータスセット

	switch (st_as_work.as_ptn_type[motion]) {
	case AGENT_AS_PTN_POS_IN: {
		pelement->type = CTR_TYPE_WAIT_PH_SINGLE;
		pelement->_t = st_as_work.T * 2.0;														// タイムオーバーリミット値
		if (st_as_work.motion_dir[motion] == ID_REV) {
			if (motion == ID_BOOM_H) {
				pelement->opt_d[STEP_OPT_PHASE_SINGLE] = -PI180 + st_as_work.as_ph[motion];
			}
			else {
				pelement->opt_d[STEP_OPT_PHASE_SINGLE] = st_as_work.as_ph[motion];
			}
		}
		else {
			if (motion == ID_BOOM_H) {
				pelement->opt_d[STEP_OPT_PHASE_SINGLE] = st_as_work.as_ph[motion];

			}
			else {
				pelement->opt_d[STEP_OPT_PHASE_SINGLE] = -PI180 + st_as_work.as_ph[motion];
			}
		}
		CHelper::fit_ph_range_upto_pi(&(pelement->opt_d[STEP_OPT_PHASE_SINGLE]));			  //レンジを-π～πに
	}break;
	case AGENT_AS_PTN_POS_OUT: {
		pelement->type = CTR_TYPE_WAIT_PH_DOUBLE;
		pelement->_t = st_as_work.T * 2.0;														// タイムオーバーリミット値
		pelement->opt_d[STEP_OPT_PHASE_FWD] = -PI180 + st_as_work.as_ph[motion];
		CHelper::fit_ph_range_upto_pi(&(pelement->opt_d[STEP_OPT_PHASE_FWD]));					//レンジを-π～πに
		pelement->opt_d[STEP_OPT_PHASE_REV] = st_as_work.as_ph[motion];
		CHelper::fit_ph_range_upto_pi(&(pelement->opt_d[STEP_OPT_PHASE_REV]));					//レンジを-π～πに
	}break;
	case AGENT_AS_PTN_FINE_POS: {
		pelement->type = CTR_TYPE_WAIT_TIME;
		pelement->_t = 0.1;
	}break;																						// 調整時間
	default: {
		pelement->type = CTR_TYPE_WAIT_TIME;
		pelement->_t = 0.1;																		// 調整時間														// タイムオーバーリミット値
	}break;
	}

	//#### STEP 1 加速
	
	double d;

	if ((st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_POS_IN)
		|| (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_POS_OUT)
		|| (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_FINE_POS)) {

		pelement = &(precipe->steps[precipe->n_step++]);										//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->status = STAT_STANDBY;														// ステータスセット
		pelement->type = CTR_TYPE_VOUT_TIME;
		pelement->_t = st_as_work.as_gain_ta[motion];
		pelement->_v = pelement->_t * st_as_work.a_abs[motion];										//加速時間 * ノッチ速度加速度
		for (int i = (NOTCH_MAX - 1);i > 1;i--) {												//真近のノッチ速度を求める
			if (pelement->_v > pCraneStat->spec.notch_spd_f[motion][i - 1]) {
				pelement->_v = pCraneStat->spec.notch_spd_f[motion][i];
				break;
			}
			else continue;			//次のノッチへ
		}
		d = 0.5 * pelement->_v * pelement->_t;
		pelement->_p = (pelement - 1)->_p - (double)st_as_work.motion_dir[motion] * d;
	}

	//#### STEP 2 定速

	if (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_POS_IN){
		pelement = &(precipe->steps[precipe->n_step++]);										//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->status = STAT_STANDBY;														// ステータスセット
		pelement->type = CTR_TYPE_VOUT_V;
		pelement->_t = st_as_work.as_gain_ta[motion];
		pelement->_v = 0.0;																		//加速時間 * ノッチ速度加速度
		pelement->_p = AgentInf_workbuf.auto_pos_target.pos[motion];							//振れ止め目標位置
	}
	else if (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_POS_OUT){
		pelement = &(precipe->steps[precipe->n_step++]);										//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->status = STAT_STANDBY;														// ステータスセット
		pelement->type = CTR_TYPE_VOUT_V;
		pelement->_t = st_as_work.as_gain_ta[motion];
		pelement->_v = 0.0;																		//加速時間 * ノッチ速度加速度
		pelement->_p = (pelement-1)->_p;														//移動方向が不定→前ステップから変更なし
	}
	else if (st_as_work.as_ptn_type[motion] == AGENT_AS_PTN_FINE_POS) {
		pelement = &(precipe->steps[precipe->n_step++]);										//ステップのポインタセットして次ステップ用にカウントアップ
		pelement->status = STAT_STANDBY;														// ステータスセット
		pelement->type = CTR_TYPE_FINE_POS;
		pelement->_t = st_as_work.as_gain_ta[motion];
		pelement->_v = pCraneStat->spec.notch_spd_f[motion][1];									//加速時間 * ノッチ速度加速度
		pelement->_p = AgentInf_workbuf.auto_pos_target.pos[motion];							//移動方向が不定のものがあるので目標位置は異常検知用にターゲット位置にする
	}
	else;

	//時間条件のスキャンカウント値セット
	for (int i = 0;i < precipe->n_step;i++) {
		precipe->steps[i].time_count = (int)(precipe->steps[i]._t * 1000.0 / st_as_work.agent_scan_ms);
		precipe->steps[i].act_count = 0;
		//旋回はレンジを-π～πに
		if (motion == ID_SLEW)	CHelper::fit_ph_range_upto_pi(&(precipe->steps[i]._p));
	}

	return L_ON;
};

/****************************************************************************/
/*   STEP処理		                                                        */
/****************************************************************************/

double CAgent::cal_step(LPST_COMMAND_SET pCom,int motion) {

	double v_out = 0.0;

	//振れ止めコマンドの時、レシピ未作成の時は、ここでレシピセット
	if ((pCom->com_code.i_list == ID_JOBTYPE_ANTISWAY) && (pCom->motion_status[motion] & STAT_END)) {
		if (cal_as_recipe(motion) == L_OFF) {
			return 0.0;  //振れ止めパターン計算セット レシピ生成不可時は0リターン
		}
		else {
			//実行カウンタ値のクリア
			for (int i = 0;i < AgentInf_workbuf.st_as_comset.recipe[motion].n_step;i++) {
				AgentInf_workbuf.st_as_comset.recipe[motion].steps[i].act_count = 0;
			}
			//レシピ作成カウントアップ　ステータスクリア
			AgentInf_workbuf.as_count[motion]++;
			AgentInf_workbuf.st_as_comset.motion_status[motion] = STAT_STANDBY;
			AgentInf_workbuf.st_as_comset.recipe[motion].i_hot_step = 0;

		}
	}

	LPST_MOTION_RECIPE	precipe = &pCom->recipe[motion];
	LPST_MOTION_STEP	pStep = &precipe->steps[precipe->i_hot_step];

	if (pStep->status == STAT_STANDBY) {   //ステップ起動時
		pStep->act_count = 1;
		pStep->status = STAT_ACTIVE;			//ステップ実行中に更新
	}
	else	pStep->act_count++;					//ステップ実行カウントインクリメント



	switch (pStep->type) {
	//#	起動位相待ち（移動方向指定）
	case CTR_TYPE_WAIT_PH_SINGLE: {
		bool is_step_end = false;
		double chk_ph = PI360;
		double trig_point_ph = pStep->opt_d[STEP_OPT_PHASE_SINGLE];
		double ph_io = pSway_IO->ph[motion];

		//異常完了判定
		if ((trig_point_ph > PI180) || (trig_point_ph < -PI180)) {//指定範囲外　-π～π
			pStep->status = STAT_LOGICAL_ERROR;is_step_end = true;
		}
		if (pStep->act_count > pStep->time_count) {
//			pStep->status = STAT_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {//振れ振幅完了レベル
			pStep->status = STAT_END;is_step_end = true;
		}
		if (is_step_end) {
			v_out = 0.0;
			break;
		}

		//メイン判定
		if ((trig_point_ph <= 0.0) && (ph_io >= 0)) {			//目標が負　現在位相が正
			chk_ph = pSway_IO->ph[motion] - trig_point_ph;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((trig_point_ph >= 0) && (ph_io <= 0)) {			//目標が正　現在値が負
			chk_ph = trig_point_ph - ph_io;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if (trig_point_ph > ph_io) {						//目標と現在値の符号が一致
			chk_ph = trig_point_ph - ph_io;
		}
		else {																	//目標と現在値の符号が一致
			chk_ph = ph_io - trig_point_ph;
		}

		if (chk_ph < st_as_work.as_ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = STAT_END;
			break;
		}

		v_out = 0.0;

	}break;
	//#	起動位相待ち（移動方向指定）
	case CTR_TYPE_WAIT_PH_DOUBLE: {
		precipe->direction = ID_STOP;	//完了まで移動方向未定
		bool is_step_end = false;
		double ph_io = pSway_IO->ph[motion];

		//異常完了判定
		if ((pStep->opt_d[STEP_OPT_PHASE_FWD] > PI180) || (pStep->opt_d[STEP_OPT_PHASE_FWD] < -PI180)) {//指定範囲外　-π～π
			pStep->status = STAT_LOGICAL_ERROR;is_step_end = true;
		}
		if ((pStep->opt_d[STEP_OPT_PHASE_REV] > PI180) || (pStep->opt_d[STEP_OPT_PHASE_REV] < -PI180)) {//指定範囲外　-π～π
			pStep->status = STAT_LOGICAL_ERROR;is_step_end = true;
		}
		if (pStep->act_count > pStep->time_count) {
			pStep->status = STAT_TIME_OVER;is_step_end = true;
		}
		if (pSway_IO->rad_amp2[motion] < pCraneStat->spec.as_rad2_level[motion][ID_LV_COMPLE]) {//振れ振幅完了レベル
			pStep->status = STAT_END;is_step_end = true;
		}

		if (is_step_end) {
			precipe->direction = ID_STOP;
			v_out = 0.0;
			break;
		}

		//メイン判定
		//正転開始用位相判定
		double chk_ph = PI360;
		double trig_point_ph = pStep->opt_d[STEP_OPT_PHASE_FWD];
		if ((trig_point_ph <= 0.0) && (ph_io >= 0)) {			//目標が負　現在位相が正
			chk_ph = ph_io - trig_point_ph;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((trig_point_ph >= 0) && (ph_io <= 0)) {			//目標が正　現在値が負
			chk_ph = trig_point_ph - ph_io;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if (trig_point_ph > ph_io) {						//目標と現在値の符号が一致
			chk_ph = trig_point_ph - ph_io;
		}
		else {																	//目標と現在値の符号が一致
			chk_ph = ph_io - trig_point_ph;
		}

		if (chk_ph < st_as_work.as_ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = STAT_END;
			precipe->direction = ID_REV;
			break;
		}

		//逆転開始用位相判定
		chk_ph = PI360;
		trig_point_ph = pStep->opt_d[STEP_OPT_PHASE_REV];
		if ((trig_point_ph <= 0) && (ph_io >= 0)) {	//目標が負　現在値が正
			chk_ph = ph_io - trig_point_ph;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if ((trig_point_ph >= 0) && (ph_io <= 0)) {	//目標が正　現在値が負
			chk_ph = trig_point_ph - ph_io;
			if (chk_ph >= PI180) chk_ph = PI360 - chk_ph;	//小さい方の角度差
		}
		else if (trig_point_ph > ph_io) {
			chk_ph = trig_point_ph - ph_io;
		}
		else {
			chk_ph = ph_io - trig_point_ph;
		}
		if (chk_ph < st_as_work.as_ph_chk_range[motion]) {	//目標位相に到達
			pStep->status = STAT_END;
			precipe->direction = ID_FWD;
			break;
		}

		v_out = 0.0;

	}break;
	//#	時間待機
	case CTR_TYPE_WAIT_TIME: {
		pStep->status = STAT_ACTIVE;
		if (pStep->act_count >= pStep->time_count) {
			pStep->status = STAT_END;
		}
		v_out = pStep->_v;
	}break;
	//#	指定時間速度出力
	case CTR_TYPE_VOUT_TIME: {
		pStep->status = STAT_ACTIVE;
		if (pStep->act_count >= pStep->time_count) {
			pStep->status = STAT_END;
		}

		if (precipe->direction == ID_FWD) {
			if (pStep->_v < 0.0) v_out = pStep->_v * -1.0;
			else v_out = pStep->_v;
		}
		else if (precipe->direction == ID_REV) {
			if (pStep->_v > 0.0) v_out = pStep->_v * -1.0;
			else v_out = pStep->_v;
		}
		else v_out = 0.0;

	}break;
	//#	振れ止め位置合わせ
	case CTR_TYPE_FB_SWAY_POS: {
		pStep->status = STAT_ACTIVE;
		if (pStep->act_count > pStep->time_count) {
			pStep->status = STAT_END;
		}
		v_out = cal_step(&AgentInf_workbuf.st_as_comset,motion);
		if(pCom->motion_status[motion]== STAT_END) pStep->status = STAT_END;
	}break;
	//#	巻位置到達待ち
	case CTR_TYPE_WAIT_POS_HST: {				//巻き位置が目標-αより上の位置に来るまで待機
		if (pEnv->cal_dist4target(ID_HOIST, false) < AGENT_CHECK_HST_POS_CLEAR_RANGE) {
			pStep->status = STAT_END;
		}
		else {
			pStep->status = STAT_ACTIVE;
		}
		v_out = 0.0;
	
	}break;
	//#	引込位置到達待ち	
	case CTR_TYPE_WAIT_POS_BH: {
		double d = pEnv->cal_dist4target(ID_BOOM_H, true);
		if (motion == ID_SLEW) {
			if (pCom->recipe[ID_BOOM_H].direction != ID_REV)	pStep->status = STAT_END;//引込でない時は待機無し
			else if (d < AGENT_CHECK_BH_POS_CLEAR_SLW_RANGE)	pStep->status = STAT_END;
			else pStep->status = STAT_ACTIVE;
		}
		else if (motion == ID_HOIST){
			if(d< AGENT_CHECK_BH_POS_CLEAR_HST_DOWN_RANGE) pStep->status = STAT_END;
			else pStep->status = STAT_ACTIVE;
		}
		else {
			pStep->status = STAT_END;
		}
		v_out = 0.0;
	}break;
	//#	旋回位置到達待ち
	case CTR_TYPE_WAIT_POS_SLW: {
		double d = pEnv->cal_dist4target(ID_SLEW, true);
		if (motion == ID_BOOM_H) {
			if(pCom->recipe[ID_BOOM_H].direction == ID_REV)			pStep->status = STAT_END;//引込時は待機無し
			else if (d < AGENT_CHECK_SLW_POS_CLEAR_BH_RANGE_rad)	pStep->status = STAT_END;//旋回が目標位置付近到達で待機完了
			else pStep->status = STAT_ACTIVE;
		}
		else if (motion == ID_HOIST) {
			if (d < AGENT_CHECK_SLW_POS_CLEAR_HST_DOWN_RANGE_rad) pStep->status = STAT_END;
			else pStep->status = STAT_ACTIVE;
		}
		else {
			pStep->status = STAT_END;
		}
		v_out = 0.0;
	}break;
	//#	着床待ち
	case CTR_TYPE_WAIT_LAND: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	指定速度到達まで速度出力
	case CTR_TYPE_VOUT_V: {//速度FB到達
		pStep->status = STAT_ACTIVE;
		v_out = pStep->_v;

		double v1percent = pCraneStat->spec.notch_spd_f[motion][5] * 0.01; //1%速度
		double dv = v_out - pPLC_IO->status.v_fb[motion],dv_abs;
		if (dv < 0.0)dv_abs = -dv;
		else dv_abs = dv;

		if ((v_out > 0.0) && (dv < 0.0)) {
			pStep->status = STAT_END;
		}
		else if ((v_out < 0.0) && (dv > 0.0)) {
			pStep->status = STAT_END;

			//巻き下げ時は位置もチェック
			if (pPLC_IO->status.pos[ID_HOIST] < pStep->_p)
				pStep->status = STAT_END;
		}
		else if (dv_abs < v1percent) {
			pStep->status = STAT_END;
		}
		else;

		if (pStep->act_count >= pStep->time_count) {
			pStep->status = STAT_END;
		}
	}break;
	//#	指定位置到達まで指定速度出力
	case CTR_TYPE_VOUT_POS: {
		pStep->status = STAT_ACTIVE;
		v_out = pStep->_v;

		if (precipe->direction >= ID_STOP) {					//移動方向　＋
			if(pPLC_IO->status.pos[motion] >= pStep->_p)		//目標位置を越えた
				pStep->status = STAT_END;
		}
		else {													//移動方向　-
			if (pPLC_IO->status.pos[motion] < pStep->_p)		//目標位置を越えた
				pStep->status = STAT_END;
		}

		if (pStep->act_count >= pStep->time_count) {			//タイムオーバー
			pStep->status = STAT_END;
		}
	}break;
	//#	指定速度出力（目標位置または振れ抑制位相到達まで指定速度出力）
	case CTR_TYPE_VOUT_PHASE: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	着床まで指定速度出力
	case CTR_TYPE_VOUT_LAND: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	指定時間加速度出力	
	case CTR_TYPE_AOUT_TIME: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	指定速度到達まで加速度出力
	case CTR_TYPE_AOUT_V: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	指定位置到達まで加速度出力
	case CTR_TYPE_AOUT_POS: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	指定位相到達まで加速度出力
	case CTR_TYPE_AOUT_PHASE: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	着床まで指定加速度出力
	case CTR_TYPE_AOUT_LAND: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;
	//#	微小位置合わせ
	case CTR_TYPE_FINE_POS: {
		pStep->status = STAT_ACTIVE;
		double dx = pStep->_p - pPLC_IO->status.pos[motion];
		if (motion == ID_SLEW) {
			if (dx > PI180)dx -= PI360;
			else if(dx < -PI180) dx += PI360;
			else;
		}

		if ((dx < pCraneStat->spec.as_pos_level[motion][ID_LV_COMPLE]) && (dx > -pCraneStat->spec.as_pos_level[motion][ID_LV_COMPLE])) {
			pStep->status = STAT_END;
			v_out = 0.0;
		}
		else if (pStep->act_count >= pStep->time_count) {
			pStep->status = STAT_END;
			v_out = 0.0;
		}
		else if (dx > 0.0) {
			v_out = pStep->_v;
		}
		else if (dx < 0.0) {
			v_out = -pStep->_v;
		}
		else {
			v_out = 0.0;
		}
	}break;
	//#	FB振れ止め
	case CTR_TYPE_FB_SWAY: {
		pStep->status = STAT_ACTIVE;
		v_out = 0.0;
	}break;

	default:
		pStep->status = STAT_LOGICAL_ERROR;
		v_out = 0.0;
		break;
	}

	if (pCom->motion_status[motion] & STAT_END) {		 //レシピ出力完了済
		return 0.0;
	}
		
	if (pStep->status & STAT_END) {
		precipe->i_hot_step++;
		if (precipe->i_hot_step >= precipe->n_step) pCom->motion_status[motion] = STAT_END;
	}

	//モニタ用バッファセット
	if ((AgentInf_workbuf.auto_on_going & AUTO_TYPE_JOB_MASK)&&(pCom->active_motion[motion])) {
		AgentInf_workbuf.st_active_com_inf.motion_status[motion] = pCom->motion_status[motion];
		AgentInf_workbuf.st_active_com_inf.com_code = pCom->com_code;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step = pCom->recipe[motion].i_hot_step;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].n_step = pCom->recipe[motion].n_step;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].type = pCom->recipe[motion].steps[pCom->recipe[motion].i_hot_step].type;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].act_count = pCom->recipe[motion].steps[pCom->recipe[motion].i_hot_step].act_count;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].status = pCom->recipe[motion].steps[pCom->recipe[motion].i_hot_step].status;
	}
	else {
		AgentInf_workbuf.st_active_com_inf.motion_status[motion] = STAT_NA;
		AgentInf_workbuf.st_active_com_inf.com_code = pCom->com_code;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step = 0;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].n_step = 0;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].type = CTR_TYPE_WAIT_TIME;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].act_count = 0;
		AgentInf_workbuf.st_active_com_inf.recipe[motion].steps[AgentInf_workbuf.st_active_com_inf.recipe[motion].i_hot_step].status = STAT_NA;
	}
	return v_out;
}
#if 0

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

