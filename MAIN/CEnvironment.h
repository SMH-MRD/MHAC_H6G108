#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PLC_IO_HELTHY_NG_COUNT      8
#define SIM_HELTHY_NG_COUNT         8
#define SWAY_HELTHY_NG_COUNT        8



class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();



    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    void routine_work(void* param);
    bool check_tasks_init();            //制御タスクの初期化が完了しているかチェック
  
    double cal_hp_acc(int motion, int dir);                 //吊点の加速度計算(旋回はm/s2）旋回半径現在位置
    double cal_hp_acc(int motion, int dir, double R);       //吊点の加速度計算(旋回はm/s2）旋回半径指定R
    double cal_hp_dec(int motion, int dir);                 //吊点の減速度計算(旋回はm/s2）旋回半径現在位置
    double cal_hp_dec(int motion, int dir, double R);       //吊点の減速度計算(旋回はm/s2）旋回半径指定R
    double cal_arad_acc(int motion, int dir);               //加減速振れ振角計算rad 旋回半径現在位置
    double cal_arad_acc(int motion, int dir, double R);     //加減速振れ振角計算rad旋回半径指定R
    double cal_arad_dec(int motion, int dir);               //加減速振れ振角計算rad 旋回半径現在位置
    double cal_arad_dec(int motion, int dir, double R);     //加減速振れ振角計算rad 旋回半径指定R
    double cal_arad2(int motion, int dir);                  //加減速振れ振角の2乗計算rad
    double cal_arad2(int motion, int dir, double R);        //加減速振れ振角の2乗計算rad 旋回半径指定R
    bool is_sway_larger_than_accsway(int motion);           //振れ角が加速振れよりも大きいか判定
    double cal_dist4stop(int motion, bool is_abs_answer);   //停止距離計算
    double cal_dist4target(int motion, bool is_abs_answer); //目標位置までの距離
    int    set_auto_target(int motion, double target);      //目標位置までの距離
    double get_vmax(int motion);                            //最大速度
 
private:
    ST_SPEC spec;       //仕様情報 Environmentが共有メモリにセットする。
    ST_CRANE_STATUS stWorkCraneStat;

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    LPST_SWAY_IO pSway_IO;
    LPST_REMOTE_IO pRemoteIO;
    LPST_SIMULATION_STATUS pSimStat;
    LPST_CS_INFO pCSInf;
    LPST_POLICY_INFO pPolicyInf;
    LPST_AGENT_INFO pAgentInf;

 
    void input();                   //外部データ取り込み
    void main_proc();               //処理内容
    void output();                  //出力データ更新

    int parse_notch_com();          //ノッチ信号を速度指令に変換セット
    int mode_set();                 //モード状態セット
    int parse_for_auto_ctrl();      //振れ周期,振れ止め目標,ノッチ状態計算
    int pos_set();                  //位置情報セット

    void chk_subproc();             //サブプロセス状態チェック

    //メインパネルのTweetテキストを設定
    void tweet_update();
                                
    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();
    
};

