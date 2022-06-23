#include "CMonWin.h"
#include "CHelper.h"
#include "CSharedMem.h"	    //共有メモリクラス

extern LPST_CRANE_STATUS pCraneStat;
extern LPST_PLC_IO pPLC_IO;
extern LPST_SWAY_IO pSway_IO;
extern LPST_REMOTE_IO pRemoteIO;
extern LPST_CS_INFO pCSinf;
extern LPST_POLICY_INFO pPOLICYinf;
extern LPST_AGENT_INFO pAGENTinf;

int CMonWin::init_main_window() {
	
	RECT rc;
	GetClientRect(hWnd_parent, &rc);
	LONG client_w = rc.right - rc.left;
	LONG client_h = rc.bottom - rc.top;
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(0);
	
	//ボタン作成
	stComCtrl.hwnd_map2d_startPB = CreateWindow(
		L"BUTTON", L"Start",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		client_w - 100, client_h - 70, 45, 25, hWnd_parent, (HMENU)IDC_MON_START_PB, hInst, NULL);
	stComCtrl.hwnd_map2d_startPB = CreateWindow(
		L"BUTTON", L"Stop",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		client_w - 50, client_h - 70, 45, 25, hWnd_parent, (HMENU)IDC_MON_STOP_PB, hInst, NULL);

	stComCtrl.hwnd_map2d_opt1_radio = CreateWindow(
		L"BUTTON", L"Disp0",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
		client_w - 340, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP0, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"Disp1",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 290, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP1, hInst, NULL);
	stComCtrl.hwnd_map2d_opt1_radio = CreateWindow(
		L"BUTTON", L"Disp2",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 240, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP2, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"Disp3",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		client_w - 190, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP3, hInst, NULL);
	stComCtrl.hwnd_map2d_opt1_radio = CreateWindow(
		L"BUTTON", L"Disp4",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 140, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP4, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"Disp5",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		client_w - 90, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP5, hInst, NULL);

 
	//表示フォント設定
	stGraphic.hfont_inftext = CreateFont(
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

	//Pen,Brush設定
	stGraphic.hpen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
	stGraphic.hbrush = CreateSolidBrush(RGB(128, 128, 128));
	
	//デバイスコンテキスト設定
	HDC hdc = GetDC(hWnd_parent);
	stGraphic.hBmap_mem0 = CreateCompatibleBitmap(hdc, INF_AREA_W, INF_AREA_H);
	stGraphic.hdc_mem0 = CreateCompatibleDC(hdc);
	SelectObject(stGraphic.hdc_mem0, stGraphic.hBmap_mem0);

	stGraphic.hBmap_bg = CreateCompatibleBitmap(hdc, INF_AREA_W, INF_AREA_H);
	stGraphic.hdc_mem_bg = CreateCompatibleDC(hdc);
	SelectObject(stGraphic.hdc_mem_bg, stGraphic.hBmap_bg);

	stGraphic.hBmap_gr = CreateCompatibleBitmap(hdc, GRAPHIC_AREA_W, GRAPHIC_AREA_H);
	stGraphic.hdc_mem_gr = CreateCompatibleDC(hdc);
	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hBmap_gr);

	stGraphic.hBmap_inf = CreateCompatibleBitmap(hdc, INF_AREA_W, INF_AREA_H);
	stGraphic.hdc_mem_inf = CreateCompatibleDC(hdc);
	SelectObject(stGraphic.hdc_mem_inf, stGraphic.hBmap_inf);

	PatBlt(stGraphic.hdc_mem0, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);
	stGraphic.hbrush = CreateSolidBrush(RGB(240, 240, 240));
	SelectObject(stGraphic.hdc_mem_bg, stGraphic.hbrush);//PATCOPY用に塗りつぶしパターンを指定
	PatBlt(stGraphic.hdc_mem_bg, 0, 0, INF_AREA_W, INF_AREA_H, PATCOPY);

	PatBlt(stGraphic.hdc_mem_gr, 0, 0, GRAPHIC_AREA_W, GRAPHIC_AREA_H, WHITENESS);
	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);

	ReleaseDC(hWnd_parent, hdc);

	draw_bg();
	InvalidateRect(hWnd_parent, NULL, TRUE);

	return 0;
}

int CMonWin::disp_update() {
	draw_inf();
	draw_graphic();
	InvalidateRect(hWnd_parent, NULL, TRUE);
    return 0;
}

VOID CMonWin::draw_bg() {

	PatBlt(stGraphic.hdc_mem_bg, 0, 0, INF_AREA_W, INF_AREA_H, PATCOPY);
	
	//マップ背景ライン描画
	stGraphic.hpen = CreatePen(PS_DOT, 2, RGB(200, 200, 200));
	SelectObject(stGraphic.hdc_mem_bg, stGraphic.hpen);

	//座標軸描画
	//クレーン部
	MoveToEx(stGraphic.hdc_mem_bg, CRANE_GRAPHIC_CENTER_X - CRANE_GRAPHIC_W/2, CRANE_GRAPHIC_CENTER_Y, NULL);	//横軸
	LineTo(stGraphic.hdc_mem_bg, CRANE_GRAPHIC_CENTER_X + CRANE_GRAPHIC_W / 2, CRANE_GRAPHIC_CENTER_Y);
	
	MoveToEx(stGraphic.hdc_mem_bg, CRANE_GRAPHIC_CENTER_X, CRANE_GRAPHIC_CENTER_Y - CRANE_GRAPHIC_H/2, NULL);	//縦軸
	LineTo(stGraphic.hdc_mem_bg, CRANE_GRAPHIC_CENTER_X, CRANE_GRAPHIC_CENTER_Y + CRANE_GRAPHIC_H/2);

	//吊荷部
	MoveToEx(stGraphic.hdc_mem_bg, LOAD_GRAPHIC_CENTER_X - LOAD_GRAPHIC_W/2 , LOAD_GRAPHIC_CENTER_Y, NULL);
	LineTo(stGraphic.hdc_mem_bg, LOAD_GRAPHIC_CENTER_X + LOAD_GRAPHIC_W/2, LOAD_GRAPHIC_CENTER_Y);

	MoveToEx(stGraphic.hdc_mem_bg, LOAD_GRAPHIC_CENTER_X, LOAD_GRAPHIC_CENTER_Y - LOAD_GRAPHIC_H / 2, NULL);
	LineTo(stGraphic.hdc_mem_bg, LOAD_GRAPHIC_CENTER_X, LOAD_GRAPHIC_CENTER_Y + LOAD_GRAPHIC_H/2);

	//走行位置表示部
	Rectangle(stGraphic.hdc_mem_bg, GNT_GRAPHIC_AREA_X, GNT_GRAPHIC_AREA_Y, GNT_GRAPHIC_AREA_X + GNT_GRAPHIC_AREA_W, GNT_GRAPHIC_AREA_Y + GNT_GRAPHIC_AREA_H);

	//巻位置表示部
	Rectangle(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X, MH_GRAPHIC_UPPER_LIM, MH_GRAPHIC_AREA_X + MH_GRAPHIC_AREA_W, MH_GRAPHIC_LOWER_LIM);
	MoveToEx(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X, MH_GRAPHIC_Y0, NULL);
	LineTo(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X + MH_GRAPHIC_AREA_W-1, MH_GRAPHIC_Y0);

	wstring ws;
	
	//テキストラベル表示
	switch (stGraphic.disp_item) {
	case IDC_MON_RADIO_DISP0:{
		ws = L"0";
		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		break;

	}
	case IDC_MON_RADIO_DISP1: {
		ws = L"1";
		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		break;

	}
	case IDC_MON_RADIO_DISP2: {
		ws = L"2";
		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		break;

	}
	case IDC_MON_RADIO_DISP3: {
		ws = L"3";
		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		break;

	}
	case IDC_MON_RADIO_DISP4: {
		ws = L"4";
		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		break;

	}
	case IDC_MON_RADIO_DISP5: {
		ws = L"5";
		TextOutW(stGraphic.hdc_mem_bg, 40, 25, ws.c_str(), (int)ws.length());
		break;

	}
	default:break;
	}

	ws = L"Spd Ref";
	TextOutW(stGraphic.hdc_mem_bg, 720, 55, ws.c_str(), (int)ws.length());

	ws = L"Spd FB";
	TextOutW(stGraphic.hdc_mem_bg, 795, 55, ws.c_str(), (int)ws.length());

	ws = L"Pos";
	TextOutW(stGraphic.hdc_mem_bg, 865, 55, ws.c_str(), (int)ws.length());


	
		
	InvalidateRect(hWnd_parent, NULL, TRUE);

	return;
}

VOID CMonWin::draw_inf() {
	
	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);


	TCHAR tbuf[32];

	wstring ws;
	//注意 wsprintfは小数点の書式が無い！！
	ws = L"HST";
	TextOutW(stGraphic.hdc_mem_inf, 670, 50, ws.c_str(), (int)ws.length());
	ws = L"GNT";
	TextOutW(stGraphic.hdc_mem_inf, 670, 65, ws.c_str(), (int)ws.length());
	ws = L"SLW";
	TextOutW(stGraphic.hdc_mem_inf, 670, 80, ws.c_str(), (int)ws.length());
	ws = L"BH ";
	TextOutW(stGraphic.hdc_mem_inf, 670, 95, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAGENTinf->v_ref[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 705, 50, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAGENTinf->v_ref[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 705, 65, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAGENTinf->v_ref[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 705, 80, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAGENTinf->v_ref[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 705, 95, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 785, 50, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 785, 65, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 785, 80, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 785, 95, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 855, 50, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 855, 65, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 855, 80, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 855, 95, ws.c_str(), (int)ws.length());

	return;
}

VOID CMonWin::draw_graphic() {
	PatBlt(stGraphic.hdc_mem_gr, 0, 0, GRAPHIC_AREA_W, GRAPHIC_AREA_H, WHITENESS);

	COLORREF color = RGB(255, 0, 0);
	POINT rc_xy;

	rc_xy.x = 200;
	rc_xy.y = 200;

	SetPixel(stGraphic.hdc_mem_gr, rc_xy.x, rc_xy.y, color);

	HBRUSH hOldBrush = (HBRUSH)SelectObject(stGraphic.hdc_mem_gr, GetStockObject(NULL_BRUSH));
	HPEN _hpen = (HPEN)CreatePen(PS_SOLID, 1, color);
	SelectObject(stGraphic.hdc_mem_gr, _hpen);
	Ellipse(stGraphic.hdc_mem_gr, 100,100,110,110);
	DeleteObject(SelectObject(stGraphic.hdc_mem_gr, _hpen));

	return;
}

int CMonWin::combine_map() {
	
	PatBlt(stGraphic.hdc_mem0, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);
	//背景を重ね合わせ
	TransparentBlt(stGraphic.hdc_mem0, 0, 0, INF_AREA_W, INF_AREA_H,
					stGraphic.hdc_mem_bg, 0, 0, INF_AREA_W, INF_AREA_H,
					RGB(255, 255, 255));

	//グラフィックを重ね合わせ
	TransparentBlt(stGraphic.hdc_mem0, GRAPHIC_AREA_X, GRAPHIC_AREA_Y, GRAPHIC_AREA_W, GRAPHIC_AREA_H,
					stGraphic.hdc_mem_gr, 0, 0, GRAPHIC_AREA_W, GRAPHIC_AREA_H,
					RGB(255, 255, 255));
	//テキストを重ね合わせ
	TransparentBlt(stGraphic.hdc_mem0, INF_AREA_X, INF_AREA_Y, INF_AREA_W, INF_AREA_H, 
					stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H,
					RGB(255, 255, 255));

	return 0;
}

int CMonWin::close_mon() {

	DeleteObject(stGraphic.hpen);
	DeleteObject(stGraphic.hbrush);

	//描画用デバイスコンテキスト解放

	DeleteObject(stGraphic.hBmap_mem0);	//ベース画面
	DeleteDC(stGraphic.hdc_mem0);

	DeleteObject(stGraphic.hBmap_bg);		//背景画面
	DeleteDC(stGraphic.hdc_mem_bg);

	DeleteObject(stGraphic.hBmap_gr);	//プロット画面
	DeleteDC(stGraphic.hdc_mem_gr);

	DeleteObject(stGraphic.hBmap_inf);	//TEXT画面
	DeleteDC(stGraphic.hdc_mem_inf);

	return 0;
}