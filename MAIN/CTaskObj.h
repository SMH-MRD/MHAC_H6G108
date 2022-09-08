#pragma once

#include "CHelper.h"
#include <process.h>
#include <typeinfo>
#include "resource.h"

#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

//タスク条件設定部
#define DEFAUT_TASK_CYCLE			25		//定周期起動デフォルト値 msec
#define TASK_EVENT_MAX				8		//タスク起動イベントオブジェクト最大数

//inf.thread_com用　スレッドループ制御用
#define REPEAT_INFINIT				0		//永久ループ
#define TERMINATE_THREAD			1		//スレッド中断
#define REPEAT_ONCE					-1		//逐次処理

//スレッド実行のrun()関数内の実行関数選択用
#define THREAD_WORK_IDLE			0		//アイドリング処理
#define THREAD_WORK_ROUTINE			1		//通常処理
#define THREAD_WORK_OPTION1			2		//オプション処理1
#define THREAD_WORK_OPTION2			3		//オプション処理2

#define MSG_LIST_MAX				14

#define ID_TIMER_EVENT				0	//タイマー用イベント配列の位置

//***********************************************
//  タスクオブジェクトインデックス構造体    
//***********************************************
typedef struct stiTask {
	int environment;
	int scada;
	int agent;
	int policy;
	int client;
	int spare1;
	int spare2;
	int spare3;
}ST_iTask, * PST_iTask;

/***********************************************************************
タスクオブジェクトの個別管理情報構造体
************************************************************************/
typedef struct st_thread_info {
	//-オブジェクト識別情報
	TCHAR			name[24];						//オブジェクト名
	TCHAR			sname[8];						//オブジェクト略称
	HBITMAP			hBmp;							//オブジェクト識別用ビットマップ

	//-スレッド設定内容
	int				index=0;						//スレッドIndex
	unsigned int	ID=0;							//スレッドID
	HANDLE			hndl=NULL;						//スレッドハンドル
	HANDLE			hevents[TASK_EVENT_MAX];		//イベントハンドル
	int				n_active_events = 1;			//有効なイベント数
	int				event_triggered=0;				//発生したイベントの番号
	unsigned int	cycle_ms=0;						//スレッド実行設定周期
	unsigned int	cycle_count=0;					//スレッド実行設定周期　Tick count（ms/system tick)
	int				trigger_type=0;					//スレッド起動条件　定周期orイベント
	int				priority=0;						//スレッドのプライオリティ
	int				thread_com=0;					//スレッド制御フラグ 　0:繰り返し　1:中断　それ以外：逐次処理:

	//-スレッドモニタ情報
	DWORD			start_time=0;					//現スキャンのスレッド開始時間
	DWORD			act_time=0;						//1スキャンのスレッド実処理時間
	DWORD			period=0;						//スレッドループ周期実績
	DWORD			act_count=0;					//スレッドループカウンタ
	DWORD			total_act=0;					//起動積算カウンタ
	unsigned int	time_over_count=0;				//予定周期をオーバーした回数

	//-関連ウィンドウハンドル
	HWND			hWnd_parent = NULL;				//親ウィンドウのハンドル
	HWND			hWnd_msgStatics = NULL;			//親ウィンドウメッセージ表示用ウィンドウへのハンドル
	HWND			hWnd_opepane = NULL;			//自メインウィンドウのハンドル（メインフレーム上に配置）
	HWND			hWnd_msgList = NULL;			//自メインウィンドウのメッセージ表示用リストコントロールへのハンドル
	HWND			hWnd_work = NULL;				//自専用作業用ウィンドウのハンドル

	HINSTANCE		hInstance;

	//-操作パネル関連
	int				cnt_PNLlist_msg = 0;			//パネルメッセージリストのカウント数
	int				panel_func_id = 1;				//パネルfunctionボタンの選択内容
	int				panel_type_id = 1;				//パネルtypeボタンの選択内容

	//-外部インターフェース
	unsigned long* psys_counter;					//メインシステムカウンターの参照先ポインタ
	unsigned		work_select = 0;				//スレッド実行の関数の種類を指定

	bool is_init_complete = false;

}ST_THREAD_INFO, * PST_THREAD_INFO;

/***********************************************************************
タスクスレッドオブジェクトベースクラス
************************************************************************/
class CTaskObj {
public:
	ST_THREAD_INFO inf;

	CTaskObj();
	virtual ~CTaskObj();//デストラクタ

	virtual void init_task(void* pobj);
	unsigned __stdcall run(void* param);//スレッド実行対象関数

	//スレッド実行対象関数内で呼ばれる処理 run()内でいずれかが選択実施される
	// set_work()でいずれを実施するか設定する 
	virtual void routine_work(void* param);						//通常処理
	virtual void optional_work1(void* param);					//オプション処理１
	virtual void optional_work2(void* param);					//オプション処理２
	virtual void default_work(void* param) { Sleep(1000); };	//アイドル処理

	//定周期処理の実行関数選択
	unsigned int set_work(int work_id) { inf.work_select = work_id; return(inf.work_select); };

	//自クラス型名の表示
	virtual void output_name() { const type_info& t = typeid(this); std::cout << "My type is " << t.name() << std::endl; }

	//自タスク固有ウィンドウ生成
	virtual HWND CreateOwnWindow(HWND h_parent_wnd);

	//自タスク設定タブパネルウィンドウ処理関数
	virtual LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);
	//タブパネルのStaticテキストを設定
	virtual void set_panel_tip_txt();
	//タブパネルのFunctionボタンのStaticテキストを設定
	virtual void set_panel_pb_txt() { return; }
	//パラメータ初期表示値設定
	virtual void set_PNLparam_value(float p1, float p2, float p3, float p4, float p5, float p6);
	//タブパネルのFunctionボタンのリセット
	virtual void reset_panel_func_pb(HWND hDlg);
	virtual void reset_panel_item_pb(HWND hDlg);

protected:
	CHelper tool;
	ostringstream ostrs;		//ostringstream  一時文字列バッファ
	wostringstream wostrs;		//wostringstream 一時文字列バッファ
	wstring wstr;				//wstring 一時文字列バッファ
	string  str;				//string 一時文字列バッファ

	//メインウィンドウへのショートメッセージ表示
	void tweet2owner(const std::string& src);
	//メインウィンドウへのショートメッセージ表示
	void tweet2owner(const std::wstring& srcw); 
	//リストコントロールへのショートメッセージ表示
	void txout2msg_listbox(const std::wstring str);
	//リストコントロールへのショートメッセージ表示
	void txout2msg_listbox(const std::string str);
};
