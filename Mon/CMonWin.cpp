#include "CMonWin.h"
#include "CHelper.h"
#include "CSharedMem.h"	    //共有メモリクラス

#include <iostream>
#include <iomanip>
#include <sstream>

extern LPST_CRANE_STATUS pCraneStat;
extern LPST_PLC_IO pPLC_IO;
extern LPST_SWAY_IO pSway_IO;
extern LPST_OTE_IO pOTE_IO;
extern LPST_CS_INFO pCSinf;
extern LPST_POLICY_INFO pPolicyInf;
extern LPST_AGENT_INFO pAgentInf;
extern LPST_SIMULATION_STATUS pSimStat;

std::wostringstream woMSG;
std::wstring wsMSG;

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
		L"BUTTON", L"MAIN",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
		client_w - 360, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP0, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"SWAY",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 310, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP1, hInst, NULL);
	stComCtrl.hwnd_map2d_opt1_radio = CreateWindow(
		L"BUTTON", L"UI",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 260, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP2, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"JOB",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		client_w - 210, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP3, hInst, NULL);
	stComCtrl.hwnd_map2d_opt1_radio = CreateWindow(
		L"BUTTON", L"Disp4",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE ,
		client_w - 160, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP4, hInst, NULL);
	stComCtrl.hwnd_map2d_opt2_radio = CreateWindow(
		L"BUTTON", L"Disp5",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
		client_w - 110, 5, 45, 25, hWnd_parent, (HMENU)IDC_MON_RADIO_DISP5, hInst, NULL);

 
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
	switch (stGraphic.disp_item) {
	case IDC_MON_RADIO_DISP0:
		draw_inf_main();break;
	case IDC_MON_RADIO_DISP1:
		draw_inf_sway();break;
	case IDC_MON_RADIO_DISP2:
		draw_inf_ui();break;
	default:
		draw_inf_main();break;
	}

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

	return;
}

VOID CMonWin::draw_inf() {
	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);
	return;
}

static wostringstream wos;

VOID CMonWin::draw_inf_ui(){
	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);

	TCHAR tbuf[32];
	wstring ws;
	
	ws = L"PLC UI";
	TextOutW(stGraphic.hdc_mem_bg, 680, 35, ws.c_str(), (int)ws.length());

	wos.str(L"");
	wos << L"振止 自動 起動  SET_Z  SET_XY :";
	wos << pPLC_IO->ui.PB[ID_PB_ANTISWAY_ON] << pPLC_IO->ui.PB[ID_PB_AUTO_MODE] << pPLC_IO->ui.PB[ID_PB_AUTO_START] << L" ";
	wos << pPLC_IO->ui.PB[ID_PB_AUTO_SET_Z] << pPLC_IO->ui.PB[ID_PB_AUTO_SET_XY];
	TextOutW(stGraphic.hdc_mem_bg, 680, 60, wos.str().c_str(), (int)wos.str().length());

	wos.str(L"");
	wos << L"S1 S2 S3 :";
	wos << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_S1] << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_S2] << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_S3];
	wos << L"    L1 L2 L3 :";
	wos << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_L1] << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_L2] << pPLC_IO->ui.PBsemiauto[SEMI_AUTO_L3];
	TextOutW(stGraphic.hdc_mem_bg, 680, 80, wos.str().c_str(), (int)wos.str().length());

	wos.str(L"");
	wos << L"PARK PICK GRND 解除:";
	wos << pPLC_IO->ui.PB[ID_PB_PARK] << pPLC_IO->ui.PB[ID_PB_PICK] << pPLC_IO->ui.PB[ID_PB_GRND] << pPLC_IO->ui.PB[ID_PB_AUTO_RESET];
	TextOutW(stGraphic.hdc_mem_bg, 680, 100, wos.str().c_str(), (int)wos.str().length());

	wos.str(L"");
	wos << L"MH:";
	wos << pPLC_IO->ui.PB[ID_PB_MH_P1] << pPLC_IO->ui.PB[ID_PB_MH_P2] << pPLC_IO->ui.PB[ID_PB_MH_M1] << pPLC_IO->ui.PB[ID_PB_MH_M2];
	wos << L" SL:";
	wos << pPLC_IO->ui.PB[ID_PB_SL_P1] << pPLC_IO->ui.PB[ID_PB_SL_P2] << pPLC_IO->ui.PB[ID_PB_SL_M1] << pPLC_IO->ui.PB[ID_PB_SL_M2];
	wos << L" BH:";
	wos << pPLC_IO->ui.PB[ID_PB_BH_P1] << pPLC_IO->ui.PB[ID_PB_BH_P2] << pPLC_IO->ui.PB[ID_PB_BH_M1] << pPLC_IO->ui.PB[ID_PB_BH_M2];
	wos << L"(+ ++ - --)";
	TextOutW(stGraphic.hdc_mem_bg, 680, 120, wos.str().c_str(), (int)wos.str().length());

	wos.str(L"                                                                 ");
	TextOutW(stGraphic.hdc_mem_bg, 20, 40, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"");
	wos << L"Auto Target MH:" << pCSinf->semi_auto_selected_target.pos[ID_HOIST];
	wos << L"  SL:" << pCSinf->semi_auto_selected_target.pos[ID_SLEW];
	wos << L"  BH:" << pCSinf->semi_auto_selected_target.pos[ID_BOOM_H];
	TextOutW(stGraphic.hdc_mem_bg, 20, 40, wos.str().c_str(), (int)wos.str().length());

	return;
}

VOID CMonWin::draw_inf_sway() {

	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);

	TCHAR tbuf[32];
	wstring ws;


	ws = L"θx(deg)： ";
	_stprintf_s(tbuf, L"%.4f", pSway_IO->th[ID_SLEW] * RAD2DEG);	ws += tbuf;
	ws += L"   θy(deg)： ";
	_stprintf_s(tbuf, L"%.4f", pSway_IO->th[ID_BOOM_H] * RAD2DEG);	ws += tbuf;ws += L"        ";

	TextOutW(stGraphic.hdc_mem_bg, 680, 55, ws.c_str(), (int)ws.length());

	ws = L"Til(deg)： ";
	_stprintf_s(tbuf, L"%.4f", pSway_IO->tilt_rad[ID_SLEW] * RAD2DEG);	ws += tbuf;
	ws += L"   Til(deg)： ";
	_stprintf_s(tbuf, L"%.4f", pSway_IO->tilt_rad[ID_BOOM_H] * RAD2DEG);	ws += tbuf;

	TextOutW(stGraphic.hdc_mem_bg, 680, 80, ws.c_str(), (int)ws.length());

	return;
}

VOID CMonWin::draw_inf_main() {

	PatBlt(stGraphic.hdc_mem_inf, 0, 0, INF_AREA_W, INF_AREA_H, WHITENESS);

	TCHAR tbuf[32];
	wstring ws;

	ws = L"MAIN";TextOutW(stGraphic.hdc_mem_bg, 960, 10, ws.c_str(), (int)ws.length());

	ws = L"BH ";TextOutW(stGraphic.hdc_mem_bg, 745, 35, ws.c_str(), (int)ws.length());
	ws = L"SLW";TextOutW(stGraphic.hdc_mem_bg, 805, 35, ws.c_str(), (int)ws.length());
	ws = L"HST";TextOutW(stGraphic.hdc_mem_bg, 865, 35, ws.c_str(), (int)ws.length());

	ws = L"Spd Ref"	;TextOutW(stGraphic.hdc_mem_bg, 670, 55, ws.c_str(), (int)ws.length());
	ws = L"Spd FB"	;TextOutW(stGraphic.hdc_mem_bg, 670, 70, ws.c_str(), (int)ws.length());
	ws = L"Pos"		;TextOutW(stGraphic.hdc_mem_bg, 670, 85, ws.c_str(), (int)ws.length());

	//注意 wsprintfは小数点の書式が無いので_stprintf_sを使う！！
//クレーン速度指令
	_stprintf_s(tbuf, L":%.4f", pAgentInf->v_ref[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pAgentInf->v_ref[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pAgentInf->v_ref[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pAgentInf->v_ref[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 910, 25, ws.c_str(), (int)ws.length());

	//クレーン速度FB
	_stprintf_s(tbuf, L":%.4f", pPLC_IO->status.v_fb[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 40, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 40, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850,40, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.v_fb[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 910, 40, ws.c_str(), (int)ws.length());
	
	//クレーン位置FB
	_stprintf_s(tbuf, L":%.4f", pPLC_IO->status.pos[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 720, 55, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 55, ws.c_str(), (int)ws.length());//旋回rad
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 55, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pPLC_IO->status.pos[ID_GANTRY]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 910, 55, ws.c_str(), (int)ws.length());

	ws = L"COMMAND:";TextOutW(stGraphic.hdc_mem_bg, 660, 110, ws.c_str(), (int)ws.length());

	if (!(pAgentInf->auto_on_going & AUTO_TYPE_JOB_MASK)) wos.str(L"NULL");
	else if (pAgentInf->st_active_com_inf.com_code.i_list == ID_JOBTYPE_JOB) wos.str(L"JOB");
	else if(pAgentInf->st_active_com_inf.com_code.i_list == ID_JOBTYPE_SEMI) wos.str(L"SEMI");
	else if (pAgentInf-> st_active_com_inf.com_code.i_list == ID_JOBTYPE_ANTISWAY) wos.str(L"FBAS");
	else  wos.str(L"ERR");
	TextOutW(stGraphic.hdc_mem_inf, 740, 80, wos.str().c_str(), (int)wos.str().length());

	ws = L"CNT:";TextOutW(stGraphic.hdc_mem_bg, 800, 110, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04d", pAgentInf->command_count); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 830, 80, ws.c_str(), (int)ws.length());


	ws = L"Rcnt";TextOutW(stGraphic.hdc_mem_bg, 670, 130, ws.c_str(), (int)ws.length());;
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe_counter[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 100, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe_counter[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 100, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe_counter[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 100, ws.c_str(), (int)ws.length());

	ws = L"mStat";TextOutW(stGraphic.hdc_mem_bg, 670, 150, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.motion_status[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 120, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.motion_status[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 120, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.motion_status[ID_HOIST]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 120, ws.c_str(), (int)ws.length());

	ws = L"sStat";TextOutW(stGraphic.hdc_mem_bg, 670, 170, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].steps[pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].i_hot_step].status);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 140, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_SLEW].steps[pAgentInf->st_active_com_inf.recipe[ID_SLEW].i_hot_step].status);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 140, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_HOIST].steps[pAgentInf->st_active_com_inf.recipe[ID_HOIST].i_hot_step].status);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 140, ws.c_str(), (int)ws.length());

	ws = L"iStep";TextOutW(stGraphic.hdc_mem_bg, 670, 190, ws.c_str(), (int)ws.length());
	wos.str(L"NA");wos << pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].i_hot_step << L"/" << pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].n_step;
	TextOutW(stGraphic.hdc_mem_inf, 725, 160, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"NA");wos << pAgentInf->st_active_com_inf.recipe[ID_SLEW].i_hot_step << L"/" << pAgentInf->st_active_com_inf.recipe[ID_SLEW].n_step;
	TextOutW(stGraphic.hdc_mem_inf, 790, 160, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"NA");wos << pAgentInf->st_active_com_inf.recipe[ID_HOIST].i_hot_step << L"/" << pAgentInf->st_active_com_inf.recipe[ID_HOIST].n_step;
	TextOutW(stGraphic.hdc_mem_inf, 850, 160, wos.str().c_str(), (int)wos.str().length());

	ws = L"sType";TextOutW(stGraphic.hdc_mem_bg, 670, 210, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].steps[pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].i_hot_step].type);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 180, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_SLEW].steps[pAgentInf->st_active_com_inf.recipe[ID_SLEW].i_hot_step].type);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 180, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_active_com_inf.recipe[ID_HOIST].steps[pAgentInf->st_active_com_inf.recipe[ID_HOIST].i_hot_step].type);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 180, ws.c_str(), (int)ws.length());

	ws = L"sAcnt";TextOutW(stGraphic.hdc_mem_bg, 670, 230, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].steps[pAgentInf->st_active_com_inf.recipe[ID_BOOM_H].i_hot_step].act_count);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 200, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe[ID_SLEW].steps[pAgentInf->st_active_com_inf.recipe[ID_SLEW].i_hot_step].act_count);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 200, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_active_com_inf.recipe[ID_HOIST].steps[pAgentInf->st_active_com_inf.recipe[ID_HOIST].i_hot_step].act_count);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 850, 200, ws.c_str(), (int)ws.length());


	ws = L"振止";TextOutW(stGraphic.hdc_mem_bg, 660, 270, ws.c_str(), (int)ws.length());
	if (pAgentInf->antisway_on_going == ANTISWAY_ALL_MANUAL) wos.str(L"NA");			//振れ止めコマンドのレシピ停止中
	else if (pAgentInf->antisway_on_going & ANTISWAY_BH_COMPLETE) wos.str(L"FIN");		//振れ止めコマンドのレシピ停止中
	else if (pAgentInf->antisway_on_going & ANTISWAY_BH_ACTIVE) wos.str(L"ACT");
	else  wos.str(L"REQ");
	TextOutW(stGraphic.hdc_mem_inf, 725, 240, wos.str().c_str(), (int)wos.str().length());

	if (pAgentInf->antisway_on_going == ANTISWAY_ALL_MANUAL) wos.str(L"NA");			//振れ止めコマンドのレシピ停止中
	else if (pAgentInf->antisway_on_going & ANTISWAY_SLEW_COMPLETE) wos.str(L"FIN");		//振れ止めコマンドのレシピ停止中
	else if (pAgentInf->antisway_on_going & ANTISWAY_SLEW_ACTIVE) wos.str(L"ACT");
	else  wos.str(L"REQ");
	TextOutW(stGraphic.hdc_mem_inf, 790, 240, wos.str().c_str(), (int)wos.str().length());

	
	ws = L"Cnt";TextOutW(stGraphic.hdc_mem_bg, 670, 290, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->as_count[ID_BOOM_H]);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 260, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->as_count[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 260, ws.c_str(), (int)ws.length());

	ws = L"mStat";TextOutW(stGraphic.hdc_mem_bg, 670, 310, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_as_comset.motion_status[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 280, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_as_comset.motion_status[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 280, ws.c_str(), (int)ws.length());

	ws = L"sType";TextOutW(stGraphic.hdc_mem_bg, 670, 330, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_as_comset.recipe[ID_BOOM_H].steps[pAgentInf->st_as_comset.recipe[ID_BOOM_H].i_hot_step].type);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 300, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%04x", pAgentInf->st_as_comset.recipe[ID_SLEW].steps[pAgentInf->st_as_comset.recipe[ID_SLEW].i_hot_step].type);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 300, ws.c_str(), (int)ws.length());

	ws = L"iStep";TextOutW(stGraphic.hdc_mem_bg, 670, 350, ws.c_str(), (int)ws.length());

	wos.str(L"NA");wos << pAgentInf->st_as_comset.recipe[ID_BOOM_H].i_hot_step << L"/" << pAgentInf->st_as_comset.recipe[ID_BOOM_H].n_step;
	TextOutW(stGraphic.hdc_mem_inf, 725, 320, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"NA");wos << pAgentInf->st_as_comset.recipe[ID_SLEW].i_hot_step << L"/" << pAgentInf->st_as_comset.recipe[ID_SLEW].n_step;
	TextOutW(stGraphic.hdc_mem_inf, 790, 320, wos.str().c_str(), (int)wos.str().length());

	ws = L"sAcnt";TextOutW(stGraphic.hdc_mem_bg, 670, 370, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf->st_as_comset.recipe[ID_BOOM_H].steps[pAgentInf->st_as_comset.recipe[ID_BOOM_H].i_hot_step].act_count);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 725, 340, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%06d", pAgentInf-> st_as_comset.recipe[ID_SLEW].steps[pAgentInf->st_as_comset.recipe[ID_SLEW].i_hot_step].act_count);ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 790, 340, ws.c_str(), (int)ws.length());

	//振れ
	ws = L"TH_RAD";
	TextOutW(stGraphic.hdc_mem_bg, 400, 35, ws.c_str(), (int)ws.length());
	ws = L"R_RAD";
	TextOutW(stGraphic.hdc_mem_bg, 465, 35, ws.c_str(), (int)ws.length());
	ws = L"TH_W";
	TextOutW(stGraphic.hdc_mem_bg, 530, 35, ws.c_str(), (int)ws.length());
	ws = L"R_W";
	TextOutW(stGraphic.hdc_mem_bg, 595, 35, ws.c_str(), (int)ws.length());
	ws = L"SIM_SW";
	TextOutW(stGraphic.hdc_mem_bg, 330, 55, ws.c_str(), (int)ws.length());
	ws = L"CTR_SW";
	TextOutW(stGraphic.hdc_mem_bg, 330, 75, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L":%.4f", pSimStat->sway_io.th[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 385, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat->sway_io.th[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 455, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat->sway_io.dth[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 515, 25, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSimStat->sway_io.dth[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 580, 25, ws.c_str(), (int)ws.length());

	_stprintf_s(tbuf, L":%.4f", pSway_IO->th[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 385, 45, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->th[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 455, 45, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->dth[ID_SLEW]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 515, 45, ws.c_str(), (int)ws.length());
	_stprintf_s(tbuf, L"%.4f", pSway_IO->dth[ID_BOOM_H]); ws = tbuf;
	TextOutW(stGraphic.hdc_mem_inf, 580, 45, ws.c_str(), (int)ws.length());

	wos.str(L"                                                     ");
	TextOutW(stGraphic.hdc_mem_bg, 10, 35, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"");
	wos << L"SEMI TG MH:" << fixed << setprecision(3) << pCSinf->semi_auto_selected_target.pos[ID_HOIST];
	wos << L"  SL:" << pCSinf->semi_auto_selected_target.pos[ID_SLEW];
	wos << L"  BH:" << pCSinf->semi_auto_selected_target.pos[ID_BOOM_H];
	TextOutW(stGraphic.hdc_mem_bg, 10, 35, wos.str().c_str(), (int)wos.str().length());

	wos.str(L"                                                     ");
	TextOutW(stGraphic.hdc_mem_bg, 10, 60, wos.str().c_str(), (int)wos.str().length());
	wos.str(L"");
	wos << L"CTR TG MH:" << fixed << setprecision(3) << pAgentInf->auto_pos_target.pos[ID_HOIST];
	wos << L"  SL:" << pAgentInf->auto_pos_target.pos[ID_SLEW];
	wos << L"  BH:" << pAgentInf->auto_pos_target.pos[ID_BOOM_H];
	TextOutW(stGraphic.hdc_mem_bg, 10, 60, wos.str().c_str(), (int)wos.str().length());
	

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
	double sin_slew = sin(pAgentInf->auto_pos_target.pos[ID_SLEW]);
	double cos_slew = cos(pAgentInf->auto_pos_target.pos[ID_SLEW]);
	double x = pAgentInf->auto_pos_target.pos[ID_BOOM_H] * cos_slew;
	double y = pAgentInf->auto_pos_target.pos[ID_BOOM_H] * sin_slew;

	tg_xy.x = CRANE_GRAPHIC_CENTER_X + (int)(x * CMON_PIX_PER_M_CRANE);
	tg_xy.y = CRANE_GRAPHIC_CENTER_Y - (int)(y * CMON_PIX_PER_M_CRANE);

	SelectObject(stGraphic.hdc_mem_gr, stGraphic.hpen[CMON_RED_PEN]);
	SelectObject(stGraphic.hdc_mem_gr, GetStockObject(NULL_BRUSH));
	MoveToEx(stGraphic.hdc_mem_gr, tg_xy.x-2, tg_xy.y -2, NULL);
	LineTo(stGraphic.hdc_mem_gr, tg_xy.x + 2, tg_xy.y + 2);
	MoveToEx(stGraphic.hdc_mem_gr, tg_xy.x + 2, tg_xy.y - 2, NULL);
	LineTo(stGraphic.hdc_mem_gr, tg_xy.x - 2, tg_xy.y + 2);

	//半自動ターゲット描画
	if (pCSinf->semi_auto_selected != SEMI_AUTO_TG_CLR) {
		sin_slew = sin(pCSinf->semi_auto_setting_target[pCSinf->semi_auto_selected].pos[ID_SLEW]);
		cos_slew = cos(pCSinf->semi_auto_setting_target[pCSinf->semi_auto_selected].pos[ID_SLEW]);
		x = pCSinf->semi_auto_setting_target[pCSinf->semi_auto_selected].pos[ID_BOOM_H] * cos_slew;
		y = pCSinf->semi_auto_setting_target[pCSinf->semi_auto_selected].pos[ID_BOOM_H] * sin_slew;

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
	int gnt_pix = GNT_GRAPHIC_AREA_X + (int)(pPLC_IO->status.pos[ID_GANTRY] * CMON_PIX_PER_M_GNT);
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