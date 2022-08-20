#pragma once


#include <Windows.h>
#include <commctrl.h>
#include <time.h>

#define NAME_OF_MK_INIFILE		L"mkchart"	//iniファイル名
#define EXT_OF_MK_INIFILE		L"ini"		//iniファイル拡張子

#define SET1_SECT_OF_INIFILE					L"SET1"			//CHART1 機能パラメータセクション
#define SET2_SECT_OF_INIFILE					L"SET2"			//CHART2 機能パラメータセクション
#define SET3_SECT_OF_INIFILE					L"SET3"			//CHART3 機能パラメータセクション
#define SET4_SECT_OF_INIFILE					L"SET4"			//CHART4 機能パラメータセクション

#define SET_CHART_TYPE_KEY_OF_INIFILE			L"CHART_TYPE"
#define SET_CHART_X_KEY_OF_INIFILE				L"CHART_WND_X"
#define SET_CHART_Y_KEY_OF_INIFILE				L"CHART_WND_Y"
#define SET_CHART_W_KEY_OF_INIFILE				L"CHART_WND_W"
#define SET_CHART_H_KEY_OF_INIFILE				L"CHART_WND_H"
#define SET_CHART_PLOT_MS_KEY_OF_INIFILE		L"PLOT_MS"

#define CHART_WND_X_DEF				20		//CHART WINDOW位置DEFAULT
#define CHART_WND_Y_DEF				600		//CHART WINDOW位置DEFAULT
#define CHART_WND_W_DEF				600		//CHART WINDOW幅DEFAULT
#define CHART_WND_H_DEF				480		//CHART WINDOW高さDEFAULT

#define CHART_WND_X_SCAT			620		//CHART WINDOW位置SCATTER
#define CHART_WND_Y_SCAT			600		//CHART WINDOW位置SCATTER
#define CHART_WND_W_SCAT			400		//CHART WINDOW幅SCATTER
#define CHART_WND_H_SCAT			480		//CHART WINDOW高さSCATTER

#define GRAPH_CHART_100				80		//100%ドット数
#define GRAPH_CHART_DOT_H			200		//１つのチャートの高さ
#define GRAPH_CHART_DOT_W			500		//１つのチャートの幅
#define CHART_MARGIN_X				50		//チャート書き出しポイント
#define CHART_MARGIN_Y				15		//チャート書き出しポイント
#define GRAPH_CHART_DISP_TIME_DEF	45000	//時間チャート表示時間範囲のデフォルトmsec
#define GRAPH_CHART_BOOL_H			4		//BOOLデータON時の高さ

#define SCAT_CHART_100				80		//100%ドット数
#define SCAT_CHART_DOT_H			200		//１つのチャートの高さ
#define SCAT_CHART_DOT_W			200		//１つのチャートの幅

#define SCAT_NUM					2		//
#define SCAT_DURATION_DEF			10000	//散布図表示更新時間msec
#define SCAT_INTERVAL				100		//散布図表示間隔
#define SCAT_MARGIN_X				25		//散布図チャート書き出しポイント
#define SCAT_MARGIN_Y				10		//散布図チャート書き出しポイント

#define PHASE_MAX_DEGREE			20		//散布図チャート表示最大角度



#define MK_CHART_WND_MAX			2		//表示可能チャートウィンドウ数
#define MK_CHART_MAX_PER_WND		2		//1ウィンドウ当たり表示可能チャート数
#define MK_MAX_GRAPH_PER_CHART		4		//1つのチャートに重ねて表示可能なグラフ数
#define MK_MAX_BOOL_PER_CHART		4		//１つのグラフに重ねて表示可能なON/OFFデータ数

#define MK_CHART_TYPE_TIME_GRAPH	0		//X:時間　Y:DATA
#define MK_CHART_TYPE_SCATTER		1		//散布図

#define MK_CHART1					0
#define MK_CHART2					1
#define MK_CHART3					2
#define MK_CHART4					3

#define MK_DATA_TYPE_NULL			0
#define MK_DATA_TYPE_TIME			1
#define MK_DATA_TYPE_DOUBLE			2
#define MK_DATA_TYPE_INT			3
#define MK_DATA_TYPE_BOOL			4

#define MK_CHART_BUF_MAX			600
#define MK_RANGE_100PERCENT			100000	//1%->1000 = 0.001%単位

#define MK_CHART_REFRESH_MS			10000

#define MK_CHART_PLOT_AXIS			2
#define MK_DATA_CODE_X				0
#define MK_DATA_CODE_Y				1
#define MK_ADDRESS_INT				0
#define MK_ADDRESS_DOUBLE			0
#define MK_ADDRESS_BOOL_MAX			4
#define MK_ADDRESS_BOOL1			0
#define MK_ADDRESS_BOOL2			1
#define MK_ADDRESS_BOOL3			2
#define MK_ADDRESS_BOOL4			3

//操作ボタンID
#define IDC_CHART_START_PB			10601
#define IDC_CHART_PAUSE_PB			10602
#define IDC_CHART_RADIO_OPT1		10603
#define IDC_CHART_RADIO_OPT2		10604

//起動タイマーID
#define ID_CHART_TIMER				100		//100msec

//メッセージID
#define WM_MKCHART_INIT				0x0400



	union MK_plot {					//時間チャート用Y軸プロット値の要素（アナログまたはBIT）チャートにプロットするデータ　ON/OFFは4点
		INT32 i_data;
		bool b_data[4];
	};

	typedef struct _stMK_plot {		//時間チャート用X,Y軸プロット値の要素
		INT32 x;
		MK_plot y;		//アナログ100%(0.001%単位）で正規化　デジタルはbool　
	}ST_PLOT_ELEMENT, * LPST_PLOT_ELEMENT;

	typedef struct _stPLOT_BUF {	//各グラフの時間チャート用X,Y軸プロット値の元データのポインタ
		double* pd[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		int* pi[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		bool* pb[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_MAX_BOOL_PER_CHART];
	}ST_PLOT_BUF, * LPST_PLOT_BUF;


	typedef struct _stRANGE_SET		//各グラフの時間チャート用X,Y軸プロット値の元データ100%値
	{
		double d100[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		int    i100[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];
		bool   b100[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_MAX_BOOL_PER_CHART];
	}ST_RANGE_SET, * LPST_RANGE_SET;

//chart_status定義
#define MK_CHART_OPEN				0x0001	//表示済
#define MK_CHART_ACTIVE				0x0002	//描画更新中
#define MK_CHART_PAUSE				0x0004	//描画更新保留中
#define MK_CHART_NO_LINE			0x0100	//線無
#define MK_CHART_NO_MARK			0x0200	//マーカ無

//chart_status定義デフォルト値
#define	MK_CHART_STATUS_DEF_BASIC	0x0202	//マーカ無　描画更新中
#define	MK_CHART_STATUS_DEF_SCATTER 0x0102	//線無　　　描画更新中

//chart_type定義
#define MK_CHART_TIME_GRAPH			0x0001
#define MK_CHART_SCATTER			0x0002

//プロット位置格納配列インデックス
#define MK_CHART_AO						0
#define MK_CHART_DO_0					0
#define MK_CHART_DO_1					1
#define MK_CHART_DO_2					2
#define MK_CHART_DO_3					3

	typedef struct _stCHART_SET
	{
		WORD chart_status;						//HBYTE 表示設定	LBYTE 実行状態
		WORD chart_type;						//グラフタイプ
		int num_of_plot;						//プロット数
		int chart_x, chart_y, chart_w, chart_h;	//チャート画面の位置　幅、高さ
		int graph_field_w, graph_field_h;		//Time GRAPHチャート表示領域のビットマップサイズ
		int plot_interval_ms;					//表示間隔msec（タイマーイベントの設定値）
		int refresh_interval;					//グラフ表示クリア間隔(プロット回数	散布図用）
		DWORD	start_time_ms;					//チャート開始時間（PCを起動してからの起動時間）
		DWORD	latast_plot_time_ms;			//最後にプロットした時間（PCを起動してからの起動時間）

	//Window
		HWND hwnd_chart;						//チャートウィンドウのハンドル
		HWND hwnd_chart_startPB;				//チャートスタートPBのハンドル
		HWND hwnd_chart_pausePB;				//チャートストップPBのハンドル
		HWND hwnd_chart_opt1_radio;				//チャートOption1PBのハンドル
		HWND hwnd_chart_opt2_radio;				//チャートOption2PBのハンドル
		
		HBITMAP hBmap_mem0;						//グラフ描画ビットマップメモリ（全体）
		HBITMAP hBmap_bg;						//グラフ描画ビットマップメモリ（背景）
		HBITMAP hBmap_graph;					//グラフ描画ビットマップメモリ（グラフ描画部）
		HBITMAP hBmap_inf;						//グラフ描画ビットマップメモリ（テキスト表示部）

		HDC hdc_mem0;							//合成画面メモリデバイスコンテキスト
		HDC hdc_mem_bg;							//背景画面メモリデバイスコンテキスト
		HDC hdc_mem_graph;						//グラフ画面メモリデバイスコンテキスト
		HDC hdc_mem_inf;						//文字画面メモリデバイスコンテキスト

		HFONT hfont_inftext;					//テキスト用フォント
		BLENDFUNCTION bf;						//半透過設定構造体

		int g_ms_per_dot;						//1dotあたりの時間(msec)
		int dot100percent_x, dot100percent_y;	//100%に割り当てるdot数
		POINT g_origin[MK_MAX_GRAPH_PER_CHART];	//グラフの原点

		bool bPrimaryDisp;						//時間チャートは2画像切り替え

		//元データ関連		
		ST_PLOT_BUF pdata[MK_CHART_PLOT_AXIS];					//各グラフの時間チャート用プロット値の元データのポインタをセットした構造体
		ST_RANGE_SET value100[MK_CHART_PLOT_AXIS];				//プロットデータ100%の値　時間は、100秒=100%とする 0セットでプロットしない
		int	data_type[MK_CHART_PLOT_AXIS][MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];	//プロットデータの型　[x軸y軸][WNDあたりのCHART数][CHARTあたりのグラフ数]

		UINT_PTR timerID;//プロットタイマーID

		int plot_buf_index;				//plot_dataの書き込みindex
		int plot_x;						//plotするx座標
		int spd_dot_per_sec;			//plot速度
		int graph_count[MK_CHART_MAX_PER_WND];
	
		//プロット値関連
		ST_PLOT_ELEMENT	plot_data[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_CHART_BUF_MAX];	//前処理プロット値(正規化値）
		POINT plot_p[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART][MK_MAX_BOOL_PER_CHART];			//ビットマップ上のプロット点

		HPEN hpen[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];		//グラフ描画用のPENハンドル
		HBRUSH hbrush[MK_CHART_MAX_PER_WND][MK_MAX_GRAPH_PER_CHART];	//グラフ描画用のBRUSHハンドル

	}ST_CHART_SET, * LPST_CHART_SET;


	class CMKChart
	{
	private:
		CMKChart();		//staticクラスにするのでprivateにする
		~CMKChart();	//staticクラスにするのでprivateにする
	public:

		static WCHAR szInipath[_MAX_PATH], szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szFName[_MAX_FNAME], szExt[_MAX_EXT];//iniファイルパス取得用
		static WCHAR szLogfolder1[_MAX_PATH];				//チャートログ用フォルダパス取得用
		static std::wstring log_path;						//チャートログ用フォルダパス取得用
		
		static ST_CHART_SET mkchartset[MK_CHART_WND_MAX];	//チャート管理構造体
		static HINSTANCE hInst;

		static int init_chartfunc();
		static int init_chart(int chartID);
		static HWND open_chart(int chartID, HWND hwnd_parent);
		static int clear_chart(int chartID);

		static int set_double_data(double* pd, int chart_WND_ID, int i_chart, int i_item, double* d_100, bool is_x);
		static int set_int_data(int* pi, int chart_WND_ID, int i_chart, int i_item, int* i_100, bool is_x);
		static int set_bool_data(bool* pb, int chart_WND_ID, int i_chart, int i_item, int i_bool, bool* b100);
		
		static int set_chart_spd(int chart_WND_ID, int disp_time_ms);//チャートのスピード　チャート幅記録にかかる時間msec
		static int set_reflesh_time(int chart_WND_ID, int period_ms);

		static int chart_start(int chartID, HWND hwnd_parent);
		static int get_chartID(HWND hWnd) { for (int i = 0; i < MK_CHART_WND_MAX; i++) { if (hWnd == mkchartset[i].hwnd_chart) return i; } return 0; }
		static int set_graph(int chart_WND_ID);
		static void draw_bg(int chartID);

		//グローバル関数
		static LRESULT CALLBACK ChartWndProc_A(HWND, UINT, WPARAM, LPARAM);//Time-Data Graph
		static LRESULT CALLBACK ChartWndProc_B(HWND, UINT, WPARAM, LPARAM);//Data-Data Graph
	};