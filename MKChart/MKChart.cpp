// MKChart.cpp : スタティック ライブラリ用の関数を定義します。
//
#include <stdexcept>

#include "pch.h"
#include "framework.h"
#include "MKChart.h"


//初期値セット用ダミー
static double	dummy_d = 1.0;
static int		dummy_i = 1;
static bool		dummy_b = true;
static double*	dummy_pd = &dummy_d;
static int*		dummy_pi = &dummy_i;
static bool*	dummy_pb = &dummy_b;

WCHAR CMKChart::szInipath[_MAX_PATH], CMKChart::szDrive[_MAX_DRIVE], CMKChart::szPath[_MAX_PATH], CMKChart::szFName[_MAX_FNAME], CMKChart::szExt[_MAX_EXT];//iniファイルパス取得用
WCHAR CMKChart::szLogfolder1[_MAX_PATH];				//チャートログ用フォルダパス取得用
std::wstring CMKChart::log_path;						//チャートログ用フォルダパス取得用
ST_CHART_SET CMKChart::mkchartset[MK_CHART_WND_MAX];	//チャート管理構造体
HINSTANCE CMKChart::hInst;

//コンストラクタ　デストラクタ
CMKChart::CMKChart() {}
CMKChart::~CMKChart() {}

/*##########################################################################*/
/*	関数：init_chartfunc()													*/
/*	ini fileからパラメータ値を取得します。									*/
/*##########################################################################*/
int CMKChart::init_chartfunc() {

	//チャート画面　最大MK_CHART_WND_MAXまで表示
	for (int i = 0; i < MK_CHART_WND_MAX; i++) {
		for (int j = 0; j < MK_CHART_MAX_PER_WND; j++) {
			for (int k = 0; k < MK_MAX_GRAPH_PER_CHART; k++) {
				mkchartset[i].data_type[j][k][MK_DATA_CODE_X] = MK_DATA_TYPE_NULL;
				mkchartset[i].data_type[j][k][MK_DATA_CODE_Y] = MK_DATA_TYPE_NULL;

				mkchartset[i].pdata[MK_DATA_CODE_X].ppd[j][k] = &dummy_pd;
				mkchartset[i].pdata[MK_DATA_CODE_Y].ppd[j][k] = &dummy_pd;
				mkchartset[i].pdata[MK_DATA_CODE_X].ppi[j][k] = &dummy_pi;
				mkchartset[i].pdata[MK_DATA_CODE_Y].ppi[j][k] = &dummy_pi;
				mkchartset[i].pvalue100[MK_DATA_CODE_X].ppd100[j][k] = &dummy_pd;
				mkchartset[i].pvalue100[MK_DATA_CODE_Y].ppd100[j][k] = &dummy_pd;
				mkchartset[i].pvalue100[MK_DATA_CODE_X].ppi100[j][k] = &dummy_pi;
				mkchartset[i].pvalue100[MK_DATA_CODE_Y].ppi100[j][k] = &dummy_pi;
				for (int l = 0; l < MK_MAX_BOOL_PER_CHART; l++) {
					mkchartset[i].pdata[MK_DATA_CODE_X].ppb[j][k][l] = &dummy_pb;
					mkchartset[i].pdata[MK_DATA_CODE_Y].ppb[j][k][l] = &dummy_pb;
					mkchartset[i].pvalue100[MK_DATA_CODE_X].ppb100[j][k][l] = &dummy_pb;
					mkchartset[i].pvalue100[MK_DATA_CODE_Y].ppb100[j][k][l] = &dummy_pb;
				}
			}
		}
	}

	hInst = GetModuleHandle(0);

	//iniファイルPATH取得
	GetModuleFileName(NULL, szInipath, sizeof(szInipath));// exe fileのパス取り込み
	_wsplitpath_s(szInipath, szDrive, sizeof(szDrive), szPath, sizeof(szPath), szFName, sizeof(szFName), szExt, sizeof(szExt)); // exe fileのパス分割
	log_path.clear(); log_path += szPath;
	_wmakepath_s(szInipath, sizeof(szInipath), szDrive, szPath, NAME_OF_MK_INIFILE, EXT_OF_MK_INIFILE); // ini fileのパスセット

	//iniファイル設定値読み込み
	TCHAR buf_t[128];


	//CHART 1
	DWORD	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_X_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].chart_x = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_Y_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].chart_y = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_W_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].chart_w = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_H_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].chart_h = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_PLOT_MS_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].plot_interval_ms = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET1_SECT_OF_INIFILE, SET_CHART_TYPE_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART1].chart_type = _wtoi(buf_t);

	//CHART 2
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_X_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].chart_x = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_Y_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].chart_y = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_W_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].chart_w = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_H_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].chart_h = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_PLOT_MS_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].plot_interval_ms = _wtoi(buf_t);
	str_num = GetPrivateProfileString(SET2_SECT_OF_INIFILE, SET_CHART_TYPE_KEY_OF_INIFILE, L"FAILED", buf_t, sizeof(buf_t), szInipath); if (str_num < 4) mkchartset[MK_CHART2].chart_type = _wtoi(buf_t);

	return 0;
};

/*##########################################################################*/
/*	関数：open_chart()														*/
/*	Chart種別に応じたWINDOWクラスを登録して表示します。						*/
/*	すでにWINDOWが開いているときは、WINDOWを閉じます。						*/
/*##########################################################################*/
HWND CMKChart::open_chart(int chartID, HWND hwnd_parent) {

	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);

	if (mkchartset[chartID].hwnd_chart != NULL) {
		DestroyWindow(mkchartset[chartID].hwnd_chart);
		mkchartset[chartID].hwnd_chart = NULL;
		return NULL;
	}


	if (mkchartset[chartID].chart_type == MK_CHART_TYPE_SCATTER) {//chart timeは、iniファイルで設定
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ChartWndProc_A;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("MKChart_Scatter");
		wc.hIconSm = NULL;
		ATOM fb = RegisterClassExW(&wc);

		//Windowクリエイト
		mkchartset[chartID].hwnd_chart = CreateWindow(TEXT("MKChart_Scatter"),
			TEXT("MK CHART_SCATTER"),
			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION,
			mkchartset[chartID].chart_x, mkchartset[chartID].chart_y, mkchartset[chartID].chart_w, mkchartset[chartID].chart_h,
			hwnd_parent,
			0,
			hInst,
			NULL);
	}
	else {//デフォルトChart（時間軸グラフ）
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ChartWndProc_A;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hIcon = NULL;
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = TEXT("MKChart_Basic");
		wc.hIconSm = NULL;
		ATOM fb = RegisterClassExW(&wc);

		//Windowクリエイト
		mkchartset[chartID].hwnd_chart = CreateWindow(TEXT("MKChart_Basic"),
			TEXT("MK CHART_BASIC"),
			WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION,
			mkchartset[chartID].chart_x, mkchartset[chartID].chart_y, mkchartset[chartID].chart_w, mkchartset[chartID].chart_h,
			hwnd_parent,
			0,
			hInst,
			NULL);
	}

	//初期化処理呼び出し
	SendMessage(mkchartset[chartID].hwnd_chart, WM_MKCHART_INIT, chartID, 0L);

	//Windowを表示
	ShowWindow(mkchartset[chartID].hwnd_chart, SW_SHOW);
	UpdateWindow(mkchartset[chartID].hwnd_chart);

	mkchartset[chartID].chart_status |= MK_CHART_OPEN;

	return mkchartset[chartID].hwnd_chart;
};

//##########################################################################*/
/*	関数：init_chart()														*/
/*	チャート初期化															*/
/*	表示フォント設定,描画オブジェクト用意									*/
/*##########################################################################*/
int CMKChart::init_chart(int chartID) {
	/*
			int nFramX = GetSystemMetrics(SM_CXSIZEFRAME);		//ウィンドウ周囲の幅
			int nFramY = GetSystemMetrics(SM_CYSIZEFRAME);		//ウィンドウ周囲の高さ
			int nCaption = GetSystemMetrics(SM_CYCAPTION);		//タイトルバーの高さ
			int scrw = GetSystemMetrics(SM_CXSCREEN);			//プライマモニタの幅
			int scrh = GetSystemMetrics(SM_CYSCREEN);			//プライマモニタの高さ
	*/

	//表示フォント
	mkchartset[chartID].hfont_inftext = CreateFont(
		12,						//int cHeight
		0,						//int cWidth
		0,						//int cEscapement
		0,						//int cOrientation
		0,						//int cWeight
		FALSE,					//DWORD bItalic
		FALSE,					//DWORD bUnderline
		FALSE,					//DWORD bStrikeOut
		SHIFTJIS_CHARSET,		//DWORD iCharSet
		OUT_DEFAULT_PRECIS,		//DWORD iOutPrecision
		CLIP_DEFAULT_PRECIS,	//DWORD iClipPrecision
		PROOF_QUALITY,			//DWORD iQuality
		FIXED_PITCH | FF_MODERN,//DWORD iPitchAndFamily
		TEXT("Arial")			//LPCWSTR pszFaceName
	);

	//描画用ペンとブラシを保持
	mkchartset[chartID].hpen[MK_CHART1][MK_GRAPH1] = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	mkchartset[chartID].hpen[MK_CHART1][MK_GRAPH2] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	mkchartset[chartID].hpen[MK_CHART1][MK_GRAPH3] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	mkchartset[chartID].hpen[MK_CHART1][MK_GRAPH4] = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));
	mkchartset[chartID].hpen[MK_CHART2][MK_GRAPH1] = CreatePen(PS_SOLID, 1, RGB(0, 0, 255));
	mkchartset[chartID].hpen[MK_CHART2][MK_GRAPH2] = CreatePen(PS_SOLID, 1, RGB(0, 255, 0));
	mkchartset[chartID].hpen[MK_CHART2][MK_GRAPH3] = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	mkchartset[chartID].hpen[MK_CHART2][MK_GRAPH4] = CreatePen(PS_SOLID, 1, RGB(255, 255, 0));

	mkchartset[chartID].hbrush[MK_CHART1][MK_GRAPH1] = CreateSolidBrush(RGB(0, 0, 255));
	mkchartset[chartID].hbrush[MK_CHART1][MK_GRAPH2] = CreateSolidBrush(RGB(0, 255, 0));
	mkchartset[chartID].hbrush[MK_CHART1][MK_GRAPH3] = CreateSolidBrush(RGB(255, 0, 0));
	mkchartset[chartID].hbrush[MK_CHART1][MK_GRAPH4] = CreateSolidBrush(RGB(255, 255, 0));
	mkchartset[chartID].hbrush[MK_CHART2][MK_GRAPH1] = CreateSolidBrush(RGB(0, 0, 255));
	mkchartset[chartID].hbrush[MK_CHART2][MK_GRAPH2] = CreateSolidBrush(RGB(0, 255, 0));
	mkchartset[chartID].hbrush[MK_CHART2][MK_GRAPH3] = CreateSolidBrush(RGB(255, 0, 0));
	mkchartset[chartID].hbrush[MK_CHART2][MK_GRAPH4] = CreateSolidBrush(RGB(255, 255, 0));


	//Chart表示状態デフォルトセット プロットACTIVE　#Line MArkerの有無
	if (mkchartset[chartID].chart_type == MK_CHART_TYPE_TIME_GRAPH) {
		mkchartset[chartID].chart_status |= MK_CHART_STATUS_DEF_BASIC;
	}
	if (mkchartset[chartID].chart_type == MK_CHART_TYPE_SCATTER) {
		mkchartset[chartID].chart_status |= MK_CHART_STATUS_DEF_SCATTER;
	}

	//Radio button 表示セット
	if (mkchartset[chartID].chart_status & MK_CHART_NO_LINE)
		SendMessage(mkchartset[chartID].hwnd_chart_opt1_radio, BM_SETCHECK, BST_UNCHECKED, 0L);
	else
		SendMessage(mkchartset[chartID].hwnd_chart_opt1_radio, BM_SETCHECK, BST_CHECKED, 0L);

	if (mkchartset[chartID].chart_status & MK_CHART_NO_MARK)
		SendMessage(mkchartset[chartID].hwnd_chart_opt2_radio, BM_SETCHECK, BST_UNCHECKED, 0L);
	else
		SendMessage(mkchartset[chartID].hwnd_chart_opt2_radio, BM_SETCHECK, BST_CHECKED, 0L);

	if (mkchartset[chartID].chart_status & MK_CHART_ACTIVE) {
		SetWindowText(mkchartset[chartID].hwnd_chart_startPB, L"Stop");
	}
	else {
		SetWindowText(mkchartset[chartID].hwnd_chart_startPB, L"Start");
	}

	
	if (mkchartset[chartID].chart_type == MK_CHART_TYPE_SCATTER) {	//散布図
		//CHART原点セット
		mkchartset[chartID].g_origin[0].x = SCAT_MARGIN_X + SCAT_CHART_DOT_W / 2;
		mkchartset[chartID].g_origin[0].y = SCAT_CHART_DOT_H / 2;
		mkchartset[chartID].g_origin[1].x = SCAT_MARGIN_X + SCAT_CHART_DOT_W / 2;
		mkchartset[chartID].g_origin[1].y = SCAT_MARGIN_Y + SCAT_CHART_DOT_H + SCAT_CHART_DOT_H / 2;
		//↓↓↓↓↓↓　以下将来用
		mkchartset[chartID].g_origin[2].x = SCAT_MARGIN_X + SCAT_MARGIN_X + SCAT_CHART_DOT_W + SCAT_CHART_DOT_W / 2;
		mkchartset[chartID].g_origin[2].y = SCAT_CHART_DOT_H / 2;
		mkchartset[chartID].g_origin[3].x = SCAT_MARGIN_X + SCAT_MARGIN_X + SCAT_CHART_DOT_W + SCAT_CHART_DOT_W / 2;
		mkchartset[chartID].g_origin[3].y = SCAT_MARGIN_Y + SCAT_CHART_DOT_H + SCAT_CHART_DOT_H / 2;

		//散布図用　描画クリア間隔  refresh_interval: クリアする時間間隔をプロット回数に変換した値
		if (mkchartset[chartID].plot_interval_ms > 0) {//タイマーイベント間隔(ini fileで設定)
			if (mkchartset[chartID].refresh_interval < 5 )//表示更新間隔(回数）が5回以下
				mkchartset[chartID].refresh_interval = MK_CHART_REFRESH_MS / mkchartset[chartID].plot_interval_ms;
			else;//前回値保持
		}
		else {
			mkchartset[chartID].plot_interval_ms = 100;
			mkchartset[chartID].refresh_interval = MK_CHART_REFRESH_MS/ mkchartset[chartID].plot_interval_ms;
		}

		mkchartset[chartID].graph_field_w = CHART_WND_W_SCAT;	//グラフ表示エリアの幅
		mkchartset[chartID].graph_field_h = CHART_WND_H_SCAT;	//グラフ表示エリアの高さ

		mkchartset[chartID].dot100percent_x = SCAT_CHART_100;	//100%のドット数 X軸
		mkchartset[chartID].dot100percent_y = SCAT_CHART_100;	//100%のドット数 Y軸

		//プロット点を原点にセット
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			for (int j = 0; j < MK_CHART_MAX_PER_WND; j++) {
				for (int k = 0; k < MK_MAX_BOOL_PER_CHART; k++) {
					mkchartset[chartID].plot_p[i][j][k].x = mkchartset[chartID].g_origin[i].x;
					mkchartset[chartID].plot_p[i][j][k].y = mkchartset[chartID].g_origin[i].y;
				}
			}
		}
	}
	else {//時間軸グラフ図
		//CHART原点セット
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			mkchartset[chartID].g_origin[i].x = CHART_MARGIN_X;
			mkchartset[chartID].g_origin[i].y = GRAPH_CHART_DOT_H / 2 + GRAPH_CHART_DOT_H * i;

			for (int j = 0; j < MK_CHART_MAX_PER_WND; j++) {
				for (int k = 0; k < MK_MAX_BOOL_PER_CHART; k++) {	//DIOチャートの原点位置
					mkchartset[chartID].plot_p[i][j][k].x = mkchartset[chartID].g_origin[i].x;
					mkchartset[chartID].plot_p[i][j][k].y = mkchartset[chartID].g_origin[i].y + GRAPH_CHART_DOT_H/ MK_MAX_GRAPH_PER_CHART/2 * k;
				}
			}
		}

		//1dotあたりのms
		mkchartset[chartID].g_ms_per_dot = GRAPH_CHART_DISP_TIME_DEF / GRAPH_CHART_DOT_W;
		//チャートグラフの幅と高さ
		mkchartset[chartID].graph_field_w = GRAPH_CHART_DOT_W + CHART_MARGIN_X;
		mkchartset[chartID].graph_field_h = GRAPH_CHART_DOT_H;
		mkchartset[chartID].bPrimaryDisp = true;	//初期値は第１画面表示

		//100%のドット数　
		mkchartset[chartID].dot100percent_x = GRAPH_CHART_DOT_W;//時間軸はチャートの表示幅
		mkchartset[chartID].dot100percent_y = GRAPH_CHART_100;

	}

	memset(mkchartset[chartID].plot_data, 0, sizeof(mkchartset[chartID].plot_data));//チャートデータバッファの初期化
	mkchartset[chartID].plot_buf_index = 0;					//チャートデータバッファのindex
	mkchartset[chartID].plot_x = 0;							//データプロットBITMAP上のX位置
	mkchartset[chartID].start_time_ms = GetTickCount();		//GetTickCountはPCを起動してからの経過時間をミリ秒単位で返すための関数

	//chart速度セット　指定時間より1秒あたりのdot数をセットする
	set_chart_spd(chartID, GRAPH_CHART_DISP_TIME_DEF);

/*
	for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) mkchartset[chartID].graph_count[i] = 0;
	for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
		mkchartset[chartID].graph_count[i] = 0;
		for (int k = 0; k < MK_MAX_GRAPH_PER_CHART; k++) {
			if (mkchartset[chartID].data_type[i][k][MK_DATA_CODE_Y] > MK_DATA_TYPE_NULL) {
				mkchartset[chartID].graph_count[i]++;	//各チャートに表示するグラフ数カウント
				set_graph(chartID);						//開始点セット
			}
			else break;
		}
	}
*/
	return 0;
}

//##########################################################################*/
/*	関数：clear_chart()														*/
/*	チャートを閉じるときの処理（タイマ解放　デバイスコンテキスト解放）		*/
/*##########################################################################*/
int CMKChart::clear_chart(int chartID) {

	//タイマー解放
	KillTimer(NULL, mkchartset[chartID].timerID);

	for (int i = 0; i < MK_MAX_GRAPH_PER_CHART; i++) {
		DeleteObject(mkchartset[chartID].hpen[i]);
		DeleteObject(mkchartset[chartID].hbrush[i]);
	}

	//ビットマップメモリ解放
	DeleteObject(mkchartset[chartID].hBmap_mem0);
	DeleteObject(mkchartset[chartID].hBmap_bg);
	DeleteObject(mkchartset[chartID].hBmap_graph);

	//描画用デバイスコンテキスト解放
	DeleteDC(mkchartset[chartID].hdc_mem0);			//ベース画面
	DeleteDC(mkchartset[chartID].hdc_mem_bg);		//背景画面
	DeleteDC(mkchartset[chartID].hdc_mem_graph);	//プロット画面

	DestroyWindow(mkchartset[chartID].hwnd_chart);  //ウィンドウ破棄
	mkchartset[chartID].hwnd_chart = NULL;

	mkchartset[chartID].chart_status &= ~MK_CHART_OPEN;

	return 0;
}

//########################################################################	
LRESULT CALLBACK CMKChart::ChartWndProc_A(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	HDC hdc;

	switch (msg) {
	case WM_DESTROY: {

		clear_chart(get_chartID(hwnd));	//タイマ解放　ビットマップメモリ,デバイスコンテキスト解放

	}return 0;

	case WM_TIMER: {

		int chartID_work = get_chartID(hwnd);
		mkchartset[chartID_work].chart_status |= MK_CHART_ACTIVE;

		if (mkchartset[chartID_work].chart_status & MK_CHART_PAUSE) return 0;	//PAUSE時は処理無し

		set_graph(chartID_work);//バッファに100%(0.001％単位で正規化したデータをセット

		//SelectObject(mkchartset[chartID_work].hdc_mem_bg, GetStockObject(BLACK_PEN));
		SelectObject(mkchartset[chartID_work].hdc_mem_graph, GetStockObject(DC_PEN));
		SetDCPenColor(mkchartset[chartID_work].hdc_mem_graph, RGB(0, 0, 255));

		if (mkchartset[chartID_work].chart_type == MK_CHART_TYPE_SCATTER) {	// Scatter Chart
			bool b_refresh = true;

			//プロット回数がリフレッシュ設定値周期に達したときグラフ画面クリア　
			if (mkchartset[chartID_work].plot_buf_index % mkchartset[chartID_work].refresh_interval == 0) {
				PatBlt(mkchartset[chartID_work].hdc_mem_graph, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
				PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
			}

			int retio100 = MK_RANGE_100PERCENT / SCAT_CHART_100; //正規化されたプロットデータ値を描画エリアのDot数に変換する係数
			//100(0.001%単位）100%のドット数→チャートプロット係数（set_graph()でMK_RANGE_100PERCENTで正規化された値がバッファにセットされている。

			for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
				if (mkchartset[chartID_work].graph_count[i]) {//チャートに表示するグラフ有
					for (int j = 0; j < MK_MAX_GRAPH_PER_CHART; j++) {
						if (mkchartset[chartID_work].graph_count[i] > j) {//グラフ設定数分だけ処理
							//描画用のPENセット
							SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hpen[i][j]);

							LONG old_x = mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x;	//x軸前回値（アナログ値）
							LONG old_y = mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].y;	//y軸前回値（アナログ値）

							mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x = mkchartset[chartID_work].g_origin[i].x
								+ mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].x / retio100;		//x軸 +方向が+表示

							mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].y = mkchartset[chartID_work].g_origin[i].y
								- mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].y.i_data / retio100;//y軸 -方向が+表示

							MoveToEx(mkchartset[chartID_work].hdc_mem_graph,
								mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x,
								mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].y,
								NULL
							);

							if (!(mkchartset[chartID_work].chart_status & MK_CHART_NO_MARK)) {	//正方形のマーカ
								//	SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hbrush[j]);
								Rectangle(mkchartset[chartID_work].hdc_mem_graph,
									mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x - 2,
									mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].y - 2,
									mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x + 2,
									mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].y + 2);
							}
							if (!(mkchartset[chartID_work].chart_status & MK_CHART_NO_LINE)) {	//前回位置へ線を引く
								LineTo(mkchartset[chartID_work].hdc_mem_graph,old_x,old_y);
							}
						}
						else break;
					}
				}
				else continue;
			}
		}
		else {	// Time Chart
			int retio100 = MK_RANGE_100PERCENT / GRAPH_CHART_100;//正規化されたプロットデータ値を描画エリアのDot数に変換する係数
			bool b_refresh = true;
			for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
				if (mkchartset[chartID_work].graph_count[i]) {
					for (int j = 0; j < MK_MAX_GRAPH_PER_CHART; j++) {
						if (mkchartset[chartID_work].graph_count[i] > j) {
							// DIOデータ
							if (mkchartset[chartID_work].data_type[i][j][MK_DATA_CODE_Y] == MK_DATA_TYPE_BOOL) {
								LONG old_x = mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x;	//時間軸前回値（アナログ値）
								LONG old_y;	//DO前回値（デジタル値）
								//PENセット
								SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hpen[j]);

								for (int k = 0; k < MK_MAX_BOOL_PER_CHART; k++) {

									old_y = mkchartset[chartID_work].plot_p[i][j][k].y;//y軸前回値（デジタル値）
									MoveToEx(mkchartset[chartID_work].hdc_mem_graph, old_x, mkchartset[chartID_work].plot_p[i][j][k].y, NULL);//前回位置へ移動

									if (!k) {//x軸のポイントセットは、最初のデータのみ
										//x軸のプロットDOT位置　=　原点位置 + （開始から10msec単位の時間/100 * 1秒あたりのDOT数）%　Chart幅のDOT数
										mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x = mkchartset[chartID_work].g_origin[i].x
											+ (mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].x * mkchartset[chartID_work].spd_dot_per_sec / 100) % GRAPH_CHART_DOT_W;
									}

									//y軸のプロットDOT起点位置　=　原点位置 + 高さ方向をグラフ数分割した下からの位置　-　DIO項目数で分割した下からの位置：y軸は-が+方向
									//y軸のプロットDOT位置　=　y軸のプロットDOT起点位置 - bool値*GRAPH_CHART_BOOL_H：y軸は-が+方向
									mkchartset[chartID_work].plot_p[i][j][k].y = mkchartset[chartID_work].g_origin[i].y + (GRAPH_CHART_100 * (MK_MAX_GRAPH_PER_CHART - j) - GRAPH_CHART_100 * k /MK_MAX_BOOL_PER_CHART) / MK_MAX_GRAPH_PER_CHART
										- mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].y.b_data[k] * GRAPH_CHART_BOOL_H;

									if (old_x > mkchartset[chartID_work].plot_p[i][j][MK_CHART_AO].x) {
										if (b_refresh == true) {//最初のチャートの時だけグラフクリア
											PatBlt(mkchartset[chartID_work].hdc_mem_graph, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
											PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
											b_refresh = false;
										}
									}
									else {
										if (!(mkchartset[chartID_work].chart_status & MK_CHART_NO_LINE)) {
											//前回値分ライン描画
											LineTo(mkchartset[chartID_work].hdc_mem_graph,
												mkchartset[chartID_work].plot_p[i][j][0].x,
												old_y
											);
											//今回値分ライン描画
											LineTo(mkchartset[chartID_work].hdc_mem_graph,
												mkchartset[chartID_work].plot_p[i][j][0].x,
												mkchartset[chartID_work].plot_p[i][j][k].y
											);
										}
									}
								}
							}
							//アナログデータ
							else {//double int データ
								SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hpen[j]);
								MoveToEx(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].plot_p[i][j][0].x, mkchartset[chartID_work].plot_p[i][j][0].y, NULL);

								LONG old_x = mkchartset[chartID_work].plot_p[i][j][0].x;
								mkchartset[chartID_work].plot_p[i][j][0].x = mkchartset[chartID_work].g_origin[i].x
									+ (mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].x * mkchartset[chartID_work].spd_dot_per_sec / 100) % GRAPH_CHART_DOT_W;

								mkchartset[chartID_work].plot_p[i][j][0].y = mkchartset[chartID_work].g_origin[i].y
									- mkchartset[chartID_work].plot_data[i][j][mkchartset[chartID_work].plot_buf_index].y.i_data / retio100;

								if (old_x > mkchartset[chartID_work].plot_p[i][j][0].x) {
									if (b_refresh == true) {//最初のチャートの時だけグラフクリア
										PatBlt(mkchartset[chartID_work].hdc_mem_graph, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
										PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
										b_refresh = false;
									}
								}
								else {

									if (!(mkchartset[chartID_work].chart_status & MK_CHART_NO_MARK)) {
										//	SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hbrush[j]);
										Rectangle(mkchartset[chartID_work].hdc_mem_graph,
											mkchartset[chartID_work].plot_p[i][j][0].x - 2,
											mkchartset[chartID_work].plot_p[i][j][0].y - 2,
											mkchartset[chartID_work].plot_p[i][j][0].x + 2,
											mkchartset[chartID_work].plot_p[i][j][0].y + 2);
									}

									if (!(mkchartset[chartID_work].chart_status & MK_CHART_NO_LINE)) {
										LineTo(mkchartset[chartID_work].hdc_mem_graph,
											mkchartset[chartID_work].plot_p[i][j][0].x,
											mkchartset[chartID_work].plot_p[i][j][0].y
										);
									}
								}
							}
						}
						else break;
					}
				}
				else continue;
			}
		}

		InvalidateRect(hwnd, NULL, TRUE);

		//					hdc = GetDC(hwnd);
		//					TextOut(hdc, 0, test_count%100, L"Test\0", sizeof("Test\0"));
	}return 0;
	case WM_MKCHART_INIT: {
		int chartID_work = wp;
		//ボタン作成
		if (mkchartset[chartID_work].chart_type == MK_CHART_TYPE_SCATTER){
			mkchartset[chartID_work].hwnd_chart_startPB = CreateWindow(
				L"BUTTON", L"Start",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CHART_WND_W_SCAT - 105, CHART_WND_H_SCAT - 70, 40, 25, hwnd, (HMENU)IDC_CHART_START_PB, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_pausePB = CreateWindow(
				L"BUTTON", L"Pause",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CHART_WND_W_SCAT - 60, CHART_WND_H_SCAT - 70, 44, 25, hwnd, (HMENU)IDC_CHART_PAUSE_PB, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_opt1_radio = CreateWindow(
				L"BUTTON", L"Line",
				WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
				CHART_WND_W_SCAT - 175, 5, 70, 25, hwnd, (HMENU)IDC_CHART_RADIO_OPT1, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_opt2_radio = CreateWindow(
				L"BUTTON", L"Mark",
				WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
				CHART_WND_W_SCAT - 95, 5, 70, 25, hwnd, (HMENU)IDC_CHART_RADIO_OPT2, hInst, NULL);
		}
		else {
			mkchartset[chartID_work].hwnd_chart_startPB = CreateWindow(
				L"BUTTON", L"Start",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CHART_WND_W_DEF - 105, CHART_WND_H_DEF - 70, 40, 25, hwnd, (HMENU)IDC_CHART_START_PB, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_pausePB = CreateWindow(
				L"BUTTON", L"Pause",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				CHART_WND_W_DEF - 60, CHART_WND_H_DEF - 70, 44, 25, hwnd, (HMENU)IDC_CHART_PAUSE_PB, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_opt1_radio = CreateWindow(
				L"BUTTON", L"Line",
				WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
				CHART_WND_W_DEF - 175, 5, 70, 25, hwnd, (HMENU)IDC_CHART_RADIO_OPT1, hInst, NULL);
			mkchartset[chartID_work].hwnd_chart_opt2_radio = CreateWindow(
				L"BUTTON", L"Mark",
				WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON,
				CHART_WND_W_DEF - 95, 5, 70, 25, hwnd, (HMENU)IDC_CHART_RADIO_OPT2, hInst, NULL);
		}


		//チャート初期化
		init_chart(chartID_work);

		//デバイスコンテキスト設定
		hdc = GetDC(hwnd);
		if (mkchartset[chartID_work].chart_type == MK_CHART_TYPE_SCATTER) {
			mkchartset[chartID_work].hBmap_mem0 = CreateCompatibleBitmap(hdc, CHART_WND_W_SCAT, CHART_WND_H_SCAT);
			mkchartset[chartID_work].hdc_mem0 = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem0, mkchartset[chartID_work].hBmap_mem0);

			mkchartset[chartID_work].hBmap_bg = CreateCompatibleBitmap(hdc, CHART_WND_W_SCAT, CHART_WND_H_SCAT);
			mkchartset[chartID_work].hdc_mem_bg = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].hBmap_bg);

			mkchartset[chartID_work].hBmap_graph = CreateCompatibleBitmap(hdc, CHART_WND_W_SCAT, CHART_WND_H_SCAT);
			mkchartset[chartID_work].hdc_mem_graph = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hBmap_graph);

			PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
			PatBlt(mkchartset[chartID_work].hdc_mem_bg, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
			PatBlt(mkchartset[chartID_work].hdc_mem_graph, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);

			ReleaseDC(hwnd, hdc);
		}
		else {
			mkchartset[chartID_work].hBmap_mem0 = CreateCompatibleBitmap(hdc, CHART_WND_W_DEF * 4, CHART_WND_H_DEF);
			mkchartset[chartID_work].hdc_mem0 = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem0, mkchartset[chartID_work].hBmap_mem0);

			mkchartset[chartID_work].hBmap_bg = CreateCompatibleBitmap(hdc, CHART_WND_W_DEF, CHART_WND_H_DEF);
			mkchartset[chartID_work].hdc_mem_bg = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].hBmap_bg);

			mkchartset[chartID_work].hBmap_graph = CreateCompatibleBitmap(hdc, mkchartset[chartID_work].graph_field_w, mkchartset[chartID_work].graph_field_h);
			mkchartset[chartID_work].hdc_mem_graph = CreateCompatibleDC(hdc);
			SelectObject(mkchartset[chartID_work].hdc_mem_graph, mkchartset[chartID_work].hBmap_graph);

			PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_DEF * 4, CHART_WND_H_DEF, WHITENESS);
			PatBlt(mkchartset[chartID_work].hdc_mem_bg, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
			PatBlt(mkchartset[chartID_work].hdc_mem_graph, 0, 0, CHART_WND_W_DEF * 4, CHART_WND_H_DEF, WHITENESS);

			ReleaseDC(hwnd, hdc);
		}

		draw_bg(chartID_work);


		//タイマー起動　plot_interval_msは、ini
		mkchartset[chartID_work].timerID = SetTimer(hwnd, ID_CHART_TIMER + chartID_work, mkchartset[chartID_work].plot_interval_ms, NULL);
		SetWindowText(mkchartset[chartID_work].hwnd_chart_startPB, L"Stop");
		return 0;
	}
	case WM_CREATE: break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);

		int chartID_work = get_chartID(hwnd);
		TransparentBlt(mkchartset[chartID_work].hdc_mem0, 0, 20, mkchartset[chartID_work].chart_w, mkchartset[chartID_work].chart_h, mkchartset[chartID_work].hdc_mem_bg, 0, 0, mkchartset[chartID_work].chart_w, mkchartset[chartID_work].chart_h, RGB(255, 255, 255));
		TransparentBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, mkchartset[chartID_work].chart_w, mkchartset[chartID_work].chart_h, mkchartset[chartID_work].hdc_mem_graph, 0, 0, mkchartset[chartID_work].chart_w, mkchartset[chartID_work].chart_h, RGB(255, 255, 255));

		if (mkchartset[chartID_work].chart_type == MK_CHART_TYPE_SCATTER) {
			BitBlt(hdc, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, mkchartset[chartID_work].hdc_mem0, 0, 0, SRCCOPY);
		}
		else {
			BitBlt(hdc, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, mkchartset[chartID_work].hdc_mem0, 0, 0, SRCCOPY);
		}

		EndPaint(hwnd, &ps);

		break;
	}
	case WM_COMMAND: {
		int chartID = get_chartID(hwnd);
		switch (LOWORD(wp)) {
		case IDC_CHART_START_PB:
		{
			if (mkchartset[chartID].chart_status & MK_CHART_ACTIVE) {
				mkchartset[chartID].chart_status &= ~MK_CHART_ACTIVE;
				SetWindowText(mkchartset[chartID].hwnd_chart_startPB, L"Start");
				KillTimer(hwnd, mkchartset[chartID].timerID);
			}
			else {
				init_chart(chartID);
				if (mkchartset[chartID].chart_type == MK_CHART_TYPE_SCATTER) {
					PatBlt(mkchartset[chartID].hdc_mem_graph, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
					PatBlt(mkchartset[chartID].hdc_mem0, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
				}
				else {
					PatBlt(mkchartset[chartID].hdc_mem_graph, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
					PatBlt(mkchartset[chartID].hdc_mem0, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
				}
				mkchartset[chartID].timerID = SetTimer(hwnd, ID_CHART_TIMER + chartID, mkchartset[chartID].plot_interval_ms, NULL);
				mkchartset[chartID].chart_status |= MK_CHART_ACTIVE;
				SetWindowText(mkchartset[chartID].hwnd_chart_startPB, L"Stop");

				InvalidateRect(hwnd, NULL, TRUE);
			}
			break;
		}
		case IDC_CHART_PAUSE_PB: {
			if (mkchartset[chartID].chart_status & MK_CHART_PAUSE) {
				mkchartset[chartID].chart_status &= ~MK_CHART_PAUSE;
				SetWindowText(mkchartset[chartID].hwnd_chart_pausePB, L"Pause");
			}
			else {
				mkchartset[chartID].chart_status |= MK_CHART_PAUSE;
				SetWindowText(mkchartset[chartID].hwnd_chart_pausePB, L"Act");
			}
			break;
		}
		case IDC_CHART_RADIO_OPT1: {
			if (mkchartset[chartID].chart_status & MK_CHART_NO_LINE) {
				mkchartset[chartID].chart_status &= ~MK_CHART_NO_LINE;
				SendMessage(mkchartset[chartID].hwnd_chart_opt1_radio, BM_SETCHECK, BST_CHECKED, 0L);
			}
			else {
				mkchartset[chartID].chart_status |= MK_CHART_NO_LINE;
				SendMessage(mkchartset[chartID].hwnd_chart_opt1_radio, BM_SETCHECK, BST_UNCHECKED, 0L);
			}
			break;
		}
		case IDC_CHART_RADIO_OPT2: {
			if (mkchartset[chartID].chart_status & MK_CHART_NO_MARK) {
				mkchartset[chartID].chart_status &= ~MK_CHART_NO_MARK;
				SendMessage(mkchartset[chartID].hwnd_chart_opt2_radio, BM_SETCHECK, BST_CHECKED, 0L);
			}
			else {
				mkchartset[chartID].chart_status |= MK_CHART_NO_MARK;
				SendMessage(mkchartset[chartID].hwnd_chart_opt2_radio, BM_SETCHECK, BST_UNCHECKED, 0L);
			}
			break;
		}
		}
	}
				   return 0;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}

//########################################################################	
LRESULT CALLBACK CMKChart::ChartWndProc_B(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	HDC hdc;

	switch (msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_TIMER:
		hdc = GetDC(hwnd);
		TextOut(hdc, 0, 0, L"Test\0", sizeof("Test\0"));
		return 0;
	case WM_CREATE: {
		break;
	}
	case WM_PAINT: {
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
		break;
	}
				 return 0;
	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

//######################################################################################
// set_data_type()
//######################################################################################
int CMKChart::set_data_type(int type, int chart_WND_ID, int i_chart, int i_item, bool is_x) {

	if (is_x)
		mkchartset[chart_WND_ID].data_type[i_chart][i_item][MK_DATA_CODE_X] = type;
	else
		mkchartset[chart_WND_ID].data_type[i_chart][i_item][MK_DATA_CODE_Y] = type;
	return 0;

}

//######################################################################################
// set_double_data()
// doubleデータと100%値が格納されるポインタをセット(ライブラリ利用側から利用）
// 散布図は、x,y軸の指定有
//######################################################################################
int CMKChart::set_double_data(double** ppd, int chart_WND_ID, int i_chart, int i_item, double** ppd_100, bool is_x) {
	if ((chart_WND_ID < 0) || (i_chart >= MK_CHART_WND_MAX))return -1;
	if ((i_item < 0) || (i_item >= MK_MAX_GRAPH_PER_CHART))return -1;


	if (is_x) {
		mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_X].ppd[i_chart][i_item] = ppd;
		mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_X].ppd100[i_chart][i_item] = ppd_100;
	}
	else {
		mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppd[i_chart][i_item] = ppd;
		mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_Y].ppd100[i_chart][i_item] = ppd_100;
	}
	return 0;
};

//######################################################################################
// set_int_data()
// intデータと100%値が格納されるポインタをセット(ライブラリ利用側から利用）
// 散布図は、x,y軸の指定有
//######################################################################################
int CMKChart::set_int_data(int** ppi, int chart_WND_ID, int i_chart, int i_item, int** ppi_100, bool is_x) {
	if ((chart_WND_ID < 0) || (i_chart >= MK_CHART_MAX_PER_WND))return -1;
	if ((i_item < 0) || (i_item >= MK_MAX_GRAPH_PER_CHART))return -1;

	if (is_x) {
		mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_X].ppi[i_chart][i_item] = ppi;
		mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_X].ppi100[i_chart][i_item] = ppi_100;
	}
	else {
		mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppi[i_chart][i_item] = ppi;
		mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_Y].ppi100[i_chart][i_item] = ppi_100;
	}

	return 0;
};


//######################################################################################
// set_bool_data()
// DIOデータの100%値と表示値が格納されるポインタをセット(ライブラリ利用側から利用）
//######################################################################################
int CMKChart::set_bool_data(bool** ppb, int chart_WND_ID, int i_chart, int i_item, int i_bool, bool** ppb100) {
	if ((chart_WND_ID < 0) || (i_chart >= MK_CHART_MAX_PER_WND))return -1;
	if ((i_item < 0) || (i_item >= MK_MAX_GRAPH_PER_CHART))return -1;
	if ((i_bool < 0) || (i_bool >= MK_MAX_BOOL_PER_CHART))return -1;

	//BOOLはX軸は時間のみ→x軸はy軸と同じ参照先をセット
	//x軸
	mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_X].ppb[i_chart][i_item][i_bool] = ppb;
	mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_X].ppb100[i_chart][i_item][i_bool] = ppb100;

	//y軸
	mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppb[i_chart][i_item][i_bool] = ppb;
	mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_Y].ppb100[i_chart][i_item][i_bool] = ppb100;

	return 0;
};

//######################################################################################
// chart_start()
// Chartの表示を開始(init_chart()を呼び出す
// Window Openしていない時は、Windowのopenする。open関数から開init_chartが起動される
//######################################################################################
int  CMKChart::chart_start(int chartID, HWND hwnd_parent) {
	if (mkchartset[chartID].hwnd_chart == NULL) {//初期化未完
		open_chart(chartID, hwnd_parent);
	}
	else {
		init_chart(chartID);
	}
	return 0;
};

//######################################################################################
// set_graph()
// 表示データを100％（0.001%）単位で正規化したものをプロット値としてバッファにセット
//######################################################################################
int CMKChart::set_graph(int chart_WND_ID) {

	//チャートバッファのindex更新
	if (mkchartset[chart_WND_ID].plot_buf_index >= MK_CHART_BUF_MAX-1) mkchartset[chart_WND_ID].plot_buf_index = 0;
	else mkchartset[chart_WND_ID].plot_buf_index++;

	if (mkchartset[chart_WND_ID].plot_buf_index >= MK_CHART_BUF_MAX) return -1;

	int time_10ms = int((GetTickCount() - mkchartset[chart_WND_ID].start_time_ms) / 10); 
	//GetTickCountはPCを起動してからの経過時間をミリ秒単位で返すための関数
	//Chart	初期化からの経過時間(10msec単位)　start_time_msは、チャート開始時にセット

	for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {		//Chart Window単位
		for (int j = 0; j < MK_MAX_GRAPH_PER_CHART; j++) {	//各グラフ単位

			//各グラフY軸データのプロット値をセット　
			if (mkchartset[chart_WND_ID].data_type[i][j][MK_DATA_CODE_Y] == MK_DATA_TYPE_DOUBLE) {		//double 元データを100%で正規化後、プロット値への変換係数を掛ける
				mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].y.i_data			//yは、INT32とbool[4]のunion
					= (int)(MK_RANGE_100PERCENT * (**mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppd[i][j]) / **mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_Y].ppd100[i][j]);
			}
			else if (mkchartset[chart_WND_ID].data_type[i][j][MK_DATA_CODE_Y] == MK_DATA_TYPE_INT) {	//int 元データを100%で正規化後、プロット値への変換係数を掛ける
				mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].y.i_data			//yは、INT32とbool[4]のunion
					= (int)(MK_RANGE_100PERCENT * (**mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppi[i][j]) / **mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_Y].ppi100[i][j]);
			}
			else if (mkchartset[chart_WND_ID].data_type[i][j][MK_DATA_CODE_Y] == MK_DATA_TYPE_BOOL) {	//bool DIOは、bool値でセット
				for (int k = 0; k < MK_MAX_BOOL_PER_CHART; k++) {
					mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].y.b_data[k]	//yは、INT32とbool[4]のunion
						= **mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_Y].ppb[i][j][k];		
				};
			}
			else break;	//データ無し

			//各グラフX軸データのプロット値をセット　
			if (mkchartset[chart_WND_ID].chart_type == MK_CHART_TYPE_SCATTER) {	//散布図のx軸の値
				if (mkchartset[chart_WND_ID].data_type[i][j][MK_DATA_CODE_X] == MK_DATA_TYPE_DOUBLE) {
					mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].x
						= (int)(MK_RANGE_100PERCENT * (**mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_X].ppd[i][j]) / **mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_X].ppd100[i][j]);
				}
				else if (mkchartset[chart_WND_ID].data_type[i][j][MK_DATA_CODE_X] == MK_DATA_TYPE_INT) {
					mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].x
						= (int)(MK_RANGE_100PERCENT * (**mkchartset[chart_WND_ID].pdata[MK_DATA_CODE_X].ppi[i][j]) / **mkchartset[chart_WND_ID].pvalue100[MK_DATA_CODE_X].ppi100[i][j]);
				}
				else break;
			}
			else {	//時間グラフのx軸の値　chart初期化から10msec単位の起動時間
				mkchartset[chart_WND_ID].plot_data[i][j][mkchartset[chart_WND_ID].plot_buf_index].x = time_10ms;
			}
		}
	}
	return 0;
}
//########################################################################
// set_chart_spd()
// 描画のDotとチャート表示時間の対応設定
//########################################################################
int CMKChart::set_chart_spd(int chart_WND_ID, int disp_time_ms) {
	int temp_s = disp_time_ms / 1000;//グラフの幅で表示する秒数
	if (temp_s < 1) {//チャート表示範囲が1秒未満
		mkchartset[chart_WND_ID].spd_dot_per_sec = GRAPH_CHART_DOT_W;			//1秒あたりのドット数　=　チャート幅
		mkchartset[chart_WND_ID].g_ms_per_dot = 1000 / GRAPH_CHART_DOT_W;		//msec/dot　=　チャート幅
	}
	else {
		mkchartset[chart_WND_ID].spd_dot_per_sec = GRAPH_CHART_DOT_W / temp_s;		//dot/sec
		mkchartset[chart_WND_ID].g_ms_per_dot = disp_time_ms / GRAPH_CHART_DOT_W;	//msec/dot
	}

	draw_bg(chart_WND_ID);

	return 0;
}
//########################################################################
// set_reflesh_time()
// 散布図の描画クリア時間のセット関数
// refresh_interval:指定時間をプロット回数に変換
//########################################################################
int CMKChart::set_reflesh_time(int chartID, int period_ms) {
	if (period_ms > 0)
		mkchartset[chartID].refresh_interval = period_ms / mkchartset[chartID].plot_interval_ms;
	return 0;
};
//########################################################################
// draw_bg()
// 背景の描画(座標軸、目盛線の描画
//########################################################################
void CMKChart::draw_bg(int chartID_work) {

	if (mkchartset[chartID_work].chart_type == MK_CHART_TYPE_SCATTER) {	//散布図

		//描画領域クリア
		PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);
		PatBlt(mkchartset[chartID_work].hdc_mem_bg, 0, 0, CHART_WND_W_SCAT, CHART_WND_H_SCAT, WHITENESS);

		//チャート背景ライン他描画
		HPEN _hpen = CreatePen(PS_DOT, 2, RGB(20, 20, 20));
		SelectObject(mkchartset[chartID_work].hdc_mem_bg, _hpen);
		//座標軸描画
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y);

			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_DOT_H / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_DOT_H / 2);

		}

		//目盛線描画
		_hpen = CreatePen(PS_DOT, 0, RGB(128, 128, 128));
		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem_bg, _hpen));	//置き換え前のオブジェクト削除
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			//y軸100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_100, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_100);
			//y軸50%のライン			
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_100 / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_100 / 2);
			//y軸-100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_100, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_100);
			//y軸-50%のライン	
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_100 / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_DOT_W / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_100 / 2);
			//x軸100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_100, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_DOT_H / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_100, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_DOT_H / 2);
			//x軸50%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_100 / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_DOT_H / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + SCAT_CHART_100 / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_DOT_H / 2);
			//x軸-100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_100, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_DOT_H / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_100, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_DOT_H / 2);
			//x軸-50%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_100 / 2, mkchartset[chartID_work].g_origin[i].y - SCAT_CHART_DOT_H / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x - SCAT_CHART_100 / 2, mkchartset[chartID_work].g_origin[i].y + SCAT_CHART_DOT_H / 2);
		}
		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem_bg, GetStockObject(WHITE_BRUSH)));
	}
	else {	//時間チャート

		//描画領域クリア
		PatBlt(mkchartset[chartID_work].hdc_mem0, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);
		PatBlt(mkchartset[chartID_work].hdc_mem_bg, 0, 0, CHART_WND_W_DEF, CHART_WND_H_DEF, WHITENESS);

		//チャート背景ライン他描画
		SelectObject(mkchartset[chartID_work].hdc_mem_bg, GetStockObject(DC_PEN));
		SetDCPenColor(mkchartset[chartID_work].hdc_mem_bg, RGB(20, 20, 20));
		//座標軸描画
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y , NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + GRAPH_CHART_DOT_W, mkchartset[chartID_work].g_origin[i].y);

			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_DOT_H / 2 - 5, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_DOT_H / 2 + 5);

		}

		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem0, GetStockObject(WHITE_BRUSH)));
		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem_bg, GetStockObject(WHITE_BRUSH)));
		//目盛線描画
		HPEN _hpen = CreatePen(PS_DOT, 0, RGB(128, 128, 128));
		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem_bg, _hpen));
		for (int i = 0; i < MK_CHART_MAX_PER_WND; i++) {
			//y軸100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_100, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + GRAPH_CHART_DOT_W, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_100);
			//y軸50%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_100 / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + GRAPH_CHART_DOT_W, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_100 / 2);
			//y軸-100%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_100, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + GRAPH_CHART_DOT_W, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_100);
			//y軸-50%のライン
			MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_100 / 2, NULL);
			LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + GRAPH_CHART_DOT_W, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_100 / 2);

			//x軸1秒区切りのライン
			int w_1sec;//1秒あたりのDot数
			if (mkchartset[chartID_work].g_ms_per_dot > 0)
				w_1sec = 1000 / mkchartset[chartID_work].g_ms_per_dot;
			else
				w_1sec = GRAPH_CHART_DOT_W;//1秒あたりのDot数

			for (int j = 0; j < GRAPH_CHART_DOT_W / w_1sec; j++) {
				MoveToEx(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + (j + 1) * w_1sec, mkchartset[chartID_work].g_origin[i].y + GRAPH_CHART_DOT_H / 2 - 5, NULL);
				LineTo(mkchartset[chartID_work].hdc_mem_bg, mkchartset[chartID_work].g_origin[i].x + (j + 1) * w_1sec, mkchartset[chartID_work].g_origin[i].y - GRAPH_CHART_DOT_H / 2 + 5);
			}
		}

		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem0, GetStockObject(WHITE_BRUSH)));
		DeleteObject(SelectObject(mkchartset[chartID_work].hdc_mem_bg, GetStockObject(WHITE_BRUSH)));
	}
	;
	return;
};

