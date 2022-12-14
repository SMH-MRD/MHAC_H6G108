#include "CMonWin.h"
#include "CHelper.h"
#include "CSharedMem.h"	    //共有メモリクラス

extern LPST_CRANE_STATUS pCraneStat;
extern LPST_PLC_IO pPLC_IO;
extern LPST_SWAY_IO pSway_IO;
extern LPST_REMOTE_IO pRemoteIO;
extern LPST_CS_INFO pCSinf;
extern LPST_POLICY_INFO pPolicyInf;
extern LPST_AGENT_INFO pAgentInf;
extern LPST_SIMULATION_STATUS pSimStat;

CMonWin::CMonWin(HWND hWnd){
	hWnd_parent = hWnd;
	memset(&stGraphic, 0, sizeof(ST_MON_GRAPHIC));
	memset(&stComCtrl, 0, sizeof(ST_MON_COM_OBJ));
	stGraphic.disp_item = IDC_MON_RADIO_DISP0;
	
	
	for (int i = 0;i < N_CREATE_PEN;i++) stGraphic.hpen[i] = NULL;
	for (int i = 0;i < N_CREATE_BRUSH;i++) stGraphic.hbrush[i] = NULL;
}
CMonWin::~CMonWin() {

}


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
		client_w - 100, client_h - 55, 45, 25, hWnd_parent, (HMENU)IDC_MON_START_PB, hInst, NULL);
	stComCtrl.hwnd_map2d_startPB = CreateWindow(
		L"BUTTON", L"Stop",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		client_w - 50, client_h - 55, 45, 25, hWnd_parent, (HMENU)IDC_MON_STOP_PB, hInst, NULL);

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
	stGraphic.hpen[CMON_RED_PEN] = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
	stGraphic.hpen[CMON_GREEN_PEN] = CreatePen(PS_SOLID, 2, RGB(0,255, 0));
	stGraphic.hpen[CMON_BLUE_PEN] = CreatePen(PS_SOLID, 2, RGB(0,0,255));
	stGraphic.hpen[CMON_GLAY_PEN] = CreatePen(PS_DOT, 2, RGB(200, 200, 200));
	stGraphic.hpen[CMON_YELLOW_PEN] = CreatePen(PS_DOT, 2, RGB(255, 255, 0));
	stGraphic.hpen[CMON_MAZENDA_PEN] = CreatePen(PS_DOT, 2, RGB(255, 0, 255));
	stGraphic.hbrush[CMON_BG_BRUSH] = CreateSolidBrush(RGB(240, 240, 240));
	stGraphic.hbrush[CMON_RED_BRUSH] = CreateSolidBrush(RGB(255, 0, 0));
	stGraphic.hbrush[CMON_GREEN_BRUSH] = CreateSolidBrush(RGB(0, 255, 0));
	stGraphic.hbrush[CMON_BLUE_BRUSH] = CreateSolidBrush(RGB(0,0,255));
	
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
	SelectObject(stGraphic.hdc_mem_bg, stGraphic.hbrush[CMON_BG_BRUSH]);//PATCOPY用に塗りつぶしパターンを指定
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
	
	SelectObject(stGraphic.hdc_mem_bg, stGraphic.hpen[CMON_GLAY_PEN]);

	//# 座標軸描画
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

	//# 走行位置表示部
	Rectangle(stGraphic.hdc_mem_bg, GNT_GRAPHIC_AREA_X, GNT_GRAPHIC_AREA_Y, GNT_GRAPHIC_AREA_X + GNT_GRAPHIC_AREA_W, GNT_GRAPHIC_AREA_Y + GNT_GRAPHIC_AREA_H);

	//# 巻位置表示部
	Rectangle(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X, MH_GRAPHIC_UPPER_LIM, MH_GRAPHIC_AREA_X + MH_GRAPHIC_AREA_W, MH_GRAPHIC_LOWER_LIM);
	MoveToEx(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X, MH_GRAPHIC_Y0, NULL);
	LineTo(stGraphic.hdc_mem_bg, MH_GRAPHIC_AREA_X + MH_GRAPHIC_AREA_W-1, MH_GRAPHIC_Y0);

	wstring ws;
	
	//テキストラベル表示
	switch (stGraphic.disp_item) {
	case IDC_MON_RADIO_DISP0:{
//		ws = L"0";
//		TextOutW(stGraphic.hdc_mem_bg, 30, 30, ws.c_str(), (int)ws.length());
		ws = L"Spd Ref";
		TextOutW(stGraphic.hdc_mem_bg, 725, 55, ws.c_str(), (int)ws.length());

		ws = L"Spd FB";
		TextOutW(stGraphic.hdc_mem_bg, 800, 55, ws.c_str(), (int)ws.length());

		ws = L"Pos";
		TextOutW(stGraphic.hdc_mem_bg, 870, 55, ws.c_str(), (int)ws.length());

		ws = L"HST";
		TextOutW(stGraphic.hdc_mem_bg, 670, 80, ws.c_str(), (int)ws.length());
		ws = L"GNT";
		TextOutW(stGraphic.hdc_mem_bg, 670, 95, ws.c_str(), (int)ws.length());
		ws = L"SLW";
		TextOutW(stGraphic.hdc_mem_bg, 670, 110, ws.c_str(), (int)ws.length());
		ws = L"BH ";
		TextOutW(stGraphic.hdc_mem_bg, 670, 125, ws.c_str(), (int)ws.length());


		ws = L"TH_RAD";
		TextOutW(stGraphic.hdc_mem_bg, 740, 150, ws.c_str(), (int)ws.length());

		ws = L"R_RAD";
		TextOutW(stGraphic.hdc_mem_bg, 810, 150, ws.c_str(), (int)ws.length());

		ws = L"TH_W";
		TextOutW(stGraphic.hdc_mem_bg, 875, 150, ws.c_str(), (int)ws.length());

		ws = L"R_W";
		TextOutW(stGraphic.hdc_mem_bg, 940, 150, ws.c_str(), (int)ws.length());

		ws = L"SIM_SW";
		TextOutW(stGraphic.hdc_mem_bg, 670, 170, ws.c_str(), (int)ws.length());

		ws = L"CTR_SW";
		TextOutW(stGraphic.hdc_mem_bg, 670, 185, ws.c_str(), (int)ws.length());

		ws = L"COM ID";
		TextOutW(stGraphic.hdc_mem_bg, 670, 215, ws.c_str(), (int)ws.length());

		ws = L"STATUS";
		TextOutW(stGraphic.hdc_mem_bg, 770, 215, ws.c_str(), (int)ws.length());

		ws = L"BH";
		TextOutW(stGraphic.hdc_mem_bg, 740, 240, ws.c_str(), (int)ws.length());

		ws = L"SLEW";
		TextOutW(stGraphic.hdc_mem_bg, 810, 240, ws.c_str(), (int)ws.length());

		ws = L"HOIST";
		TextOutW(stGraphic.hdc_mem_bg, 880, 240, ws.c_str(), (int)ws.length());

		ws = L"TYPE";
		TextOutW(stGraphic.hdc_mem_bg, 670, 260, ws.c_str(), (int)ws.length());
		ws = L"nStep";
		TextOutW(stGraphic.hdc_mem_bg, 670, 280, ws.c_str(), (int)ws.length());
		ws = L"iAct";
		TextOutW(stGraphic.hdc_mem_bg, 670, 300, ws.c_str(), (int)ws.length());
		ws = L"step";
		TextOutW(stGraphic.hdc_mem_bg, 670, 320, ws.c_str(), (int)ws.length());
		ws = L"acount";
		TextOutW(stGraphic.hdc_mem_bg, 670, 340, ws.c_str(), (int)ws.length());
		ws = L"mode";
		TextOutW(stGraphic.hdc_mem_bg, 670, 360, ws.c_str(), (int)ws.length());


		InvalidateRect(hWnd_parent, NULL, TRUE);
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



	return;
}

VOID CMonWin::draw_inf() {
	
	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);

	TCHAR tbuf[32];
	wstring ws;

	//注意 wsprintfは小数点の書式が無いので_stprintf_sを使う！！
	//クレーン速度指令
	_stprintf_s(tbuf, L":%.4f", pAgentInf->v_ref[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 710, 50, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAgentInf->v_ref[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 710, 65, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAgentInf->v_ref[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 710, 80, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%.4f", pAgentInf->v_ref[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 710, 95, ws.c_str(), (int)ws.length());
	//クレーン速度FB
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 50, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 65, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 80, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 95, ws.c_str(), (int)ws.length());
	//クレーン位置FB
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 860, 50, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 860, 65, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L"%.1f", pPLC_IO->status.pos[ID_SLEW]/ PI1DEG); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 930, 80, ws.c_str(), (int)ws.length());//旋回deg

	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 860, 80, ws.c_str(), (int)ws.length());//旋回rad

	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 860, 95, ws.c_str(), (int)ws.length());

	//振れ
	_stprintf_s(tbuf, L":%.4f", pSimStat->sway_io.th[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 140, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat->sway_io.th[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 800, 140, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat-> sway_io.dth[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 865, 140, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat->sway_io.dth[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 930, 140, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L":%.4f", pSway_IO ->th[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 155, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->th[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 800, 155, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->dth[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 865, 155, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->dth[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 930, 155, ws.c_str(), (int)ws.length());



	//COMMAND
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].id); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 185, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].com_status); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 810, 185, ws.c_str(), (int)ws.length());

	//BH
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_BOOM_H].motion_type); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 230, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_BOOM_H].n_step); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 250, ws.c_str(), (int)ws.length());

	int i_act = pPolicyInf->com[pPolicyInf->i_com].motion_stat[ID_BOOM_H].iAct;
	_stprintf_s(tbuf, L":%4d", i_act); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 270, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_BOOM_H].steps[i_act].type); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 290, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%4d", pPolicyInf->com[pPolicyInf->i_com].motion_stat[ID_BOOM_H].step_act_count); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 310, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L":%4d", pAgentInf->auto_active[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 330, ws.c_str(), (int)ws.length());


	//SLEW
	_stprintf_s(tbuf, L"%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_SLEW].motion_type); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 230, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_SLEW].n_step); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 250, ws.c_str(), (int)ws.length());

	i_act = pPolicyInf->com[pPolicyInf->i_com].motion_stat[ID_SLEW].iAct;
	_stprintf_s(tbuf, L"%4d", i_act); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 270, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%4d", pPolicyInf->com[pPolicyInf->i_com].recipe[ID_SLEW].steps[i_act].type); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 290, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%4d", pPolicyInf->com[pPolicyInf->i_com].motion_stat[ID_SLEW].step_act_count); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 310, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%4d", pAgentInf->auto_active[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 330, ws.c_str(), (int)ws.length());

	
	//テストアウト
	TextOutW(stGraphic.hdc_mem_inf, 350, 50,L"rad", 3);

	
	return;
}

VOID CMonWin::draw_graphic() {
	PatBlt(stGraphic.hdc_mem_gr, 0, 0, GRAPHIC_AREA_W, GRAPHIC_AREA_H, WHITENESS);


	COLORREF color_pt = RGB(255, 0, 0); //ポイント描画色

	POINT load_xy;
	//吊荷描画
	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hbrush[CMON_GREEN_BRUSH]);
	SelectObject(stGraphic.hdc_mem_gr, GetStockObject(NULL_PEN));

	load_xy.x = CRANE_GRAPHIC_CENTER_X + (int)(pCraneStat->rl.x * CMON_PIX_PER_M_CRANE);
	load_xy.y = CRANE_GRAPHIC_CENTER_Y - (int)(pCraneStat->rl.y * CMON_PIX_PER_M_CRANE);
	Ellipse(stGraphic.hdc_mem_gr,
		load_xy.x - 6,
		load_xy.y - 6,
		load_xy.x + 6,
		load_xy.y + 6);

	//# ブーム先端描画
	POINT boom_end_xy;					//ブーム先端位置
	boom_end_xy.x = CRANE_GRAPHIC_CENTER_X + (int)(pCraneStat->rc.x * CMON_PIX_PER_M_CRANE);
	boom_end_xy.y = CRANE_GRAPHIC_CENTER_Y - (int)(pCraneStat->rc.y * CMON_PIX_PER_M_CRANE);

	
	//#自動目標位置
	POINT tg_xy;
	double sin_slew = sin(pAgentInf->pos_target[ID_SLEW]);
	double cos_slew = cos(pAgentInf->pos_target[ID_SLEW]);
	double x = pAgentInf->pos_target[ID_BOOM_H] * cos_slew;
	double y = pAgentInf->pos_target[ID_BOOM_H] * sin_slew;

	tg_xy.x = CRANE_GRAPHIC_CENTER_X + (int)(x * CMON_PIX_PER_M_CRANE);
	tg_xy.y = CRANE_GRAPHIC_CENTER_Y - (int)(y * CMON_PIX_PER_M_CRANE);

	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hpen[CMON_RED_PEN]);
	SelectObject(stGraphic.hdc_mem_gr, GetStockObject(NULL_BRUSH));
	MoveToEx(stGraphic.hdc_mem_gr, tg_xy.x-2, tg_xy.y -2, NULL);
	LineTo(stGraphic.hdc_mem_gr, tg_xy.x + 2, tg_xy.y + 2);
	MoveToEx(stGraphic.hdc_mem_gr, tg_xy.x + 2, tg_xy.y - 2, NULL);
	LineTo(stGraphic.hdc_mem_gr, tg_xy.x - 2, tg_xy.y + 2);

	//半自動ターゲット描画
	if (pCraneStat->semi_auto_selected != SEMI_AUTO_TG_CLR) {
		sin_slew = sin(pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][ID_SLEW]);
		cos_slew = cos(pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][ID_SLEW]);
		x = pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][ID_BOOM_H] * cos_slew;
		y = pCraneStat->semi_auto_setting_target[pCraneStat->semi_auto_selected][ID_BOOM_H] * sin_slew;

		tg_xy.x = CRANE_GRAPHIC_CENTER_X + (int)(x * CMON_PIX_PER_M_CRANE);
		tg_xy.y = CRANE_GRAPHIC_CENTER_Y - (int)(y * CMON_PIX_PER_M_CRANE);

		SelectObject(stGraphic.hdc_mem_gr, stGraphic.hpen[CMON_MAZENDA_PEN]);
		Ellipse(stGraphic.hdc_mem_gr,tg_xy.x - 6,tg_xy.y - 6,tg_xy.x + 6,tg_xy.y + 6);
	}


	//ペン、ブラシセット
	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hbrush[CMON_BLUE_BRUSH]);
	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hpen[CMON_BLUE_PEN]);
	//ブームライン描画
	MoveToEx(stGraphic.hdc_mem_gr, CRANE_GRAPHIC_CENTER_X, CRANE_GRAPHIC_CENTER_Y, NULL);
	LineTo(stGraphic.hdc_mem_gr, boom_end_xy.x, boom_end_xy.y);

	//ブーム先端描画
	Ellipse(stGraphic.hdc_mem_gr, 
		boom_end_xy.x - CMON_PIX_R_BOOM_END,
		boom_end_xy.y - CMON_PIX_R_BOOM_END,
		boom_end_xy.x + CMON_PIX_R_BOOM_END,
		boom_end_xy.y + CMON_PIX_R_BOOM_END);



//# 走行位置描画
	//走行位置描画
	int gnt_pix = GNT_GRAPHIC_AREA_X + (int)(pCraneStat->rc0.x * CMON_PIX_PER_M_GNT);
	RECT rc = { gnt_pix - CMON_PIX_GNT_MARK_W,	GNT_GRAPHIC_AREA_Y,
				gnt_pix + CMON_PIX_GNT_MARK_W,	GNT_GRAPHIC_AREA_Y + GNT_GRAPHIC_AREA_H };

	FillRect(stGraphic.hdc_mem_gr, &rc, stGraphic.hbrush[CMON_GREEN_BRUSH]);


//# 巻位置描画
	//巻位置描画
	int hst_pix = MH_GRAPHIC_Y0 - (int)(pCraneStat->rl.z * CMON_PIX_PER_M_HOIST);
	rc = { MH_GRAPHIC_AREA_X, hst_pix - CMON_PIX_HST_MARK_W,
				MH_GRAPHIC_AREA_X + MH_GRAPHIC_AREA_W,hst_pix + CMON_PIX_HST_MARK_W };

	FillRect(stGraphic.hdc_mem_gr, &rc, stGraphic.hbrush[CMON_RED_BRUSH]);

	//SetPixel(stGraphic.hdc_mem_gr, rc_xy.x, rc_xy.y, color_pt);


//# 吊荷振れ描画

	LONG r=6;
	POINT pt;

	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hbrush[CMON_BLUE_BRUSH]);
	SelectObject(stGraphic.hdc_mem_gr, GetStockObject(NULL_PEN));
	pt.x = LONG(pSimStat->sway_io.th[ID_SLEW] * 573.0)+ LOAD_GRAPHIC_CENTER_X;	//30°≒　300pix→ 1rad　≒　572pix
	pt.y = -LONG(pSimStat->sway_io.th[ID_BOOM_H] * 573.0)+ LOAD_GRAPHIC_CENTER_Y;	//30°≒　300pix
	Ellipse(stGraphic.hdc_mem_gr, pt.x-r, pt.y-r, pt.x+r, pt.y+r);

	r = 4;
	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hbrush[CMON_GREEN_BRUSH]);
	pt.x = LONG(pSway_IO->th[ID_SLEW] * 573.0) + LOAD_GRAPHIC_CENTER_X;	//30°≒　300pix
	pt.y = -LONG(pSway_IO->th[ID_BOOM_H] * 573.0) + LOAD_GRAPHIC_CENTER_Y;	//30°≒　300pix
	Ellipse(stGraphic.hdc_mem_gr, pt.x - r, pt.y - r, pt.x + r, pt.y + r);

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

	//描画用ペン、ブラシ解放
	for (int i = 0;i < N_CREATE_PEN;i++) {
		if (stGraphic.hpen[i] != NULL) DeleteObject(stGraphic.hpen[i]);
	}

	for (int i = 0;i < N_CREATE_BRUSH;i++) {
		if (stGraphic.hbrush[i] != NULL)DeleteObject(stGraphic.hbrush[i]);
	}

	//描画用デバイスコンテキスト解放

	DeleteObject(stGraphic.hBmap_mem0);	//ベース画面
	DeleteDC(stGraphic.hdc_mem0);

	DeleteObject(stGraphic.hBmap_bg);	//背景画面
	DeleteDC(stGraphic.hdc_mem_bg);

	DeleteObject(stGraphic.hBmap_gr);	//プロット画面
	DeleteDC(stGraphic.hdc_mem_gr);

	DeleteObject(stGraphic.hBmap_inf);	//TEXT画面
	DeleteDC(stGraphic.hdc_mem_inf);

	return 0;
}