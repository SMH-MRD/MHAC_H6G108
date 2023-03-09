#include "CWorkWindow_PLC.h"
#include "resource.h"
#include "PLC_IO_DEF.h"
#include "CPLC_IF.h"

#include <iostream>
#include <iomanip>
#include <sstream>

extern CPLC_IF* pProcObj;

CWorkWindow_PLC::CWorkWindow_PLC() {}
CWorkWindow_PLC::~CWorkWindow_PLC() {}
HWND CWorkWindow_PLC::hWorkWnd;
HWND CWorkWindow_PLC::hIOWnd;
ST_PLC_DEBUG_PANEL CWorkWindow_PLC::stOpePaneStat;
ST_IOCHECK_COM_OBJ CWorkWindow_PLC::stIOCheckObj;

//# #######################################################################
HWND CWorkWindow_PLC::open_WorkWnd(HWND hwnd) {

	InitCommonControls();//コモンコントロール初期化
	HINSTANCE hInst = GetModuleHandle(0);

	//Workウィンドウの生成
	hWorkWnd = CreateDialog(hInst,L"IDD_OPERATION_PANEL", hwnd, (DLGPROC)WorkWndProc);
	MoveWindow(hWorkWnd, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H, TRUE);

	ShowWindow(hWorkWnd, SW_SHOW);
	UpdateWindow(hWorkWnd);

	return hWorkWnd;
};

HWND CWorkWindow_PLC::open_IO_Wnd(HWND hwnd) {

	InitCommonControls();//コモンコントロール初期化
	HINSTANCE hInst = GetModuleHandle(0);

	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = IOWndProc;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("IO_CHK_Wnd");
	wc.hIconSm = NULL;
	ATOM fb = RegisterClassExW(&wc);

	hIOWnd = CreateWindow(TEXT("IO_CHK_Wnd"),
		TEXT("ChkWnd"),
		WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, IO_WND_X, IO_WND_Y, IO_WND_W, IO_WND_H,
		hwnd,
		0,
		hInst,
		NULL);

	ShowWindow(hWorkWnd, SW_SHOW);
	UpdateWindow(hWorkWnd);

	return hIOWnd;
};

//# Window 終了処理 ###################################################################################
int CWorkWindow_PLC::close_WorkWnd() {

	DestroyWindow(hWorkWnd);  //ウィンドウ破棄

	return 0;
}

//# コールバック関数 ########################################################################	

//ボタン入力OFF Delay用
static int off_delay_antisway = 0, off_delay_auto_mode = 0;
#define PB_OFF_DELAY_COUNT 2

LRESULT CALLBACK CWorkWindow_PLC::WorkWndProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	LPNMUPDOWN lpnmud;
	
	switch (msg) {
	case WM_INITDIALOG: {
		InitCommonControls();
		//トラックバーコントロールのレンジ設定
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETRANGE, TRUE, MAKELONG(SLW_SLIDAR_MIN, SLW_SLIDAR_MAX));
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETRANGE, TRUE, MAKELONG(BH_SLIDAR_MIN, BH_SLIDAR_MAX));
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETRANGE, TRUE, MAKELONG(MH_SLIDAR_MIN, MH_SLIDAR_MAX));
		SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETRANGE, TRUE, MAKELONG(GT_SLIDAR_MIN, GT_SLIDAR_MAX));
		//スピンコントロールのレンジ,初期値設定
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_SLEW), UDM_SETRANGE, 0, MAKELONG(SLW_SLIDAR_MAX, SLW_SLIDAR_MIN));
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_SLEW), UDM_SETPOS, 0, SLW_SLIDAR_0_NOTCH);
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_BH), UDM_SETRANGE, 0, MAKELONG(BH_SLIDAR_MAX, BH_SLIDAR_MIN));
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_BH), UDM_SETPOS, 0, BH_SLIDAR_0_NOTCH);
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_MH), UDM_SETRANGE, 0, MAKELONG(MH_SLIDAR_MAX, MH_SLIDAR_MIN));
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_MH), UDM_SETPOS, 0, MH_SLIDAR_0_NOTCH);
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_GT), UDM_SETRANGE, 0, MAKELONG(GT_SLIDAR_MAX, GT_SLIDAR_MIN));
		SendMessage(GetDlgItem(hDlg, IDC_SPIN_GT), UDM_SETPOS, 0, GT_SLIDAR_0_NOTCH);

		//コントロール初期状態設定
		update_all_controls(hDlg);

		SetTimer(hDlg, ID_WORK_UPDATE_TIMER, IO_CHK_TIMER_PRIOD, NULL);
	
		return TRUE;
	}break;		//表示更新タイマ起動
	case WM_CREATE: {
			break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wp)) {
		case IDC_CHECK_SOURCE1_ON:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SOURCE1_ON), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_source1_on = SET_ON;
			}
			else {
				stOpePaneStat.check_source1_on = SET_OFF;
			}
		}break;
		case IDC_CHECK_SOURCE1_OFF:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SOURCE1_OFF), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_source1_off = SET_ON;
			}
			else {
				stOpePaneStat.check_source1_off = SET_OFF;
			}
		}break;
		case IDC_CHECK_SOURCE2_ON:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SOURCE2_ON), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_source2_on = SET_ON;
			}
			else {
				stOpePaneStat.check_source2_on = SET_OFF;
			}
		}break;
		case IDC_CHECK_SOURCE2_OFF:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SOURCE2_OFF), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_source2_off = SET_ON;
			}
			else {
				stOpePaneStat.check_source2_off = SET_OFF;
			}
		}break;
		case IDC_BUTTON_AUTO_START:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_AUTO_START), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_auto_start = SET_ON;
			}
			else {
				stOpePaneStat.check_auto_start = SET_OFF;
			}
		}break;
		case IDC_BUTTON_AUTO_RESET:
		{
			stOpePaneStat.button_auto_reset = SET_ON;
		}break;
		case IDC_CHECK_S1:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_S1), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_s1 = SET_ON;
			}
			else {
				stOpePaneStat.check_s1 = SET_OFF;
			}
		}break;
		case IDC_CHECK_S2:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_S2), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_s2 = SET_ON;
			}
			else {
				stOpePaneStat.check_s2 = SET_OFF;
			}
		}break;
		case IDC_CHECK_S3:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_S3), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_s3 = SET_ON;
			}
			else {
				stOpePaneStat.check_s3 = SET_OFF;
			}
		}break;
		case IDC_BUTTON_SET_Z:
		{
			stOpePaneStat.button_set_z = SET_ON;
		}break;
		case IDC_CHECK_L1:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_L1), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_l1 = SET_ON;
			}
			else {
				stOpePaneStat.check_l1 = SET_OFF;
			}
		}break;
		case IDC_CHECK_L2:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_L2), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_l2 = SET_ON;
			}
			else {
				stOpePaneStat.check_l2 = SET_OFF;
			}
		}break;
		case IDC_CHECK_L3:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_L3), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_l3 = SET_ON;
			}
			else {
				stOpePaneStat.check_l3 = SET_OFF;
			}
		}break;
		case IDC_BUTTON_SET_XY:
		{
			stOpePaneStat.button_set_xy = SET_ON;
		}break;
		case IDC_CHECK_ESTOP:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_ESTOP), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_estop = SET_ON;
			}
			else {
				stOpePaneStat.check_estop = SET_OFF;
			}
		}break;
		case IDC_BUTTON_ANTISWAY:
		{
			stOpePaneStat.button_antisway = SET_ON;
		}break;
		case IDC_BUTTON_AUTO_MODE:
		{
			stOpePaneStat.button_auto_mode = SET_ON;
		}break;
		case IDC_BUTTON_MH_P1:
		{
			stOpePaneStat.button_mh_p1 = SET_ON;
		}break;
		case IDC_BUTTON_MH_P2:
		{
			stOpePaneStat.button_mh_p2 = SET_ON;
		}break;
		case IDC_BUTTON_MH_M1:
		{
			stOpePaneStat.button_mh_m1 = SET_ON;
		}break;
		case IDC_BUTTON_MH_M2:
		{
			stOpePaneStat.button_mh_m2 = SET_ON;
		}break;
		case IDC_BUTTON_SL_P1:
		{
			stOpePaneStat.button_sl_p1 = SET_ON;
		}break;
		case IDC_BUTTON_SL_P2:
		{
			stOpePaneStat.button_sl_p2 = SET_ON;
		}break;
		case IDC_BUTTON_SL_M1:
		{
			stOpePaneStat.button_sl_m1 = SET_ON;
		}break;
		case IDC_BUTTON_SL_M2:
		{
			stOpePaneStat.button_sl_m2 = SET_ON;
		}break;
		case IDC_BUTTON_BH_P1:
		{
			stOpePaneStat.button_bh_p1 = SET_ON;
		}break;
		case IDC_BUTTON_BH_P2:
		{
			stOpePaneStat.button_bh_p2 = SET_ON;
		}break;
		case IDC_BUTTON_BH_M1:
		{
			stOpePaneStat.button_bh_m1 = SET_ON;
		}break;
		case IDC_BUTTON_BH_M2:
		{
			stOpePaneStat.button_bh_m2 = SET_ON;
		}break;
		case IDC_BUTTON_PARK:
		{
			stOpePaneStat.button_park = SET_ON;
		}break;
		case IDC_BUTTON_GRND:
		{
			stOpePaneStat.button_grnd = SET_ON;
		}break;
		case IDC_BUTTON_PICK:
		{
			stOpePaneStat.button_pick = SET_ON;
		}break;
		case IDC_CHECK_RMOTE:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_RMOTE), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_remote = SET_ON;
			}
			else {
				stOpePaneStat.button_remote = SET_OFF;
			}
		}break;
		case IDC_BUTTON_SLEW_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_SLEW), UDM_SETPOS, 0, SLW_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_slew = SLW_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, SET_ON, stOpePaneStat.slider_slew);
			wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);
		}break;
		case IDC_BUTTON_BH_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_BH), UDM_SETPOS, 0, BH_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_bh = BH_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, SET_ON, BH_SLIDAR_MAX - (INT64)stOpePaneStat.slider_bh);
			wsprintf(stOpePaneStat.static_bh_label, L"引込 %02d", stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_BH_LABEL), stOpePaneStat.static_bh_label);
		}break;
		case IDC_BUTTON_MH_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_MH), UDM_SETPOS, 0, MH_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_mh = MH_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, SET_ON, stOpePaneStat.slider_mh);
			wsprintf(stOpePaneStat.static_mh_label, L"巻 %02d", stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MH_LABEL), stOpePaneStat.static_mh_label);
		}break;
		case IDC_BUTTON_GT_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_GT), UDM_SETPOS, 0, GT_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_gt = GT_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, SET_ON, stOpePaneStat.slider_gt);
			wsprintf(stOpePaneStat.static_gt_label, L"走行 %02d", stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_GT_LABEL), stOpePaneStat.static_gt_label);
		}break;
		case IDC_BUTTON_FAULT_RESET:
		{
			if (stOpePaneStat.button_fault_reset) stOpePaneStat.button_fault_reset = SET_OFF;
			else stOpePaneStat.button_fault_reset = SET_ON;
		}break;
		case IDC_CHECK_SPD_MODE:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SPD_MODE), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.chk_PC_ref_spd = SET_ON;
			}
			else {
				stOpePaneStat.chk_PC_ref_spd = SET_OFF;
			}
		}break;
		case IDC_CHECK_SIM_FB:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_SIM_FB), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.chk_sim_fb = SET_ON;
			}
			else {
				stOpePaneStat.chk_sim_fb = SET_OFF;
			}
		}break;
		case IDC_DISABLE_PANEL_INPUT:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_DISABLE_PANEL_INPUT), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.chk_input_disable = SET_ON;
			}
			else {
				stOpePaneStat.chk_input_disable = SET_OFF;
			}
		}break;
		}break;
	}
	case WM_PAINT:{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hDlg, &ps);
		// TODO: HDC を使用する描画コードをここに追加してください...
		BitBlt(hdc, 0, 0, WORK_WND_W, WORK_WND_H, stOpePaneStat.hdc_mem_gr, 0, 0, SRCCOPY);


		EndPaint(hDlg, &ps);
	}break;
	case WM_NOTIFY://SPINコントロール用
	{
		WPARAM ui_udn_deltapos = 4294966574;//(WPARAM)UDN_DELTAPOS;コンパイル時のC26454警告を出さない為一旦変数にコードを直接入れる

		if (wp == (WPARAM)IDC_SPIN_SLEW) {
			lpnmud = (LPNMUPDOWN)lp;

			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_slew = lpnmud->iPos + lpnmud->iDelta;
				if (stOpePaneStat.slider_slew < 0) stOpePaneStat.slider_slew = 0;
				else if (stOpePaneStat.slider_slew > SLW_SLIDAR_MAX) stOpePaneStat.slider_slew = SLW_SLIDAR_MAX;
				else;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew);
				wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);
			}
		}
		else if (wp == (WPARAM)IDC_SPIN_BH) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_bh = lpnmud->iPos + lpnmud->iDelta;
				if (stOpePaneStat.slider_bh < 0) stOpePaneStat.slider_bh = 0;
				else if (stOpePaneStat.slider_bh > BH_SLIDAR_MAX) stOpePaneStat.slider_bh = BH_SLIDAR_MAX;
				else;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, (INT64)BH_SLIDAR_MAX - (short)stOpePaneStat.slider_bh);
				wsprintf(stOpePaneStat.static_bh_label, L"引込 %02d", stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_BH_LABEL), stOpePaneStat.static_bh_label);
			}
		}
		else if (wp == (WPARAM)IDC_SPIN_MH) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_mh = lpnmud->iPos + lpnmud->iDelta;
				if (stOpePaneStat.slider_mh < 0) stOpePaneStat.slider_mh = 0;
				else if (stOpePaneStat.slider_mh > MH_SLIDAR_MAX) stOpePaneStat.slider_mh = MH_SLIDAR_MAX;
				else;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, MH_SLIDAR_MAX - (INT64)stOpePaneStat.slider_mh);
				wsprintf(stOpePaneStat.static_mh_label, L"巻 %02d", stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MH_LABEL), stOpePaneStat.static_mh_label);
			}
		}
		else if (wp == (WPARAM)IDC_SPIN_GT) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_gt = lpnmud->iPos + lpnmud->iDelta;
				if (stOpePaneStat.slider_gt < 0) stOpePaneStat.slider_gt = 0;
				else if (stOpePaneStat.slider_gt > GT_SLIDAR_MAX) stOpePaneStat.slider_gt = GT_SLIDAR_MAX;
				else;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, TRUE, GT_SLIDAR_MAX - (INT64)stOpePaneStat.slider_gt);
				wsprintf(stOpePaneStat.static_gt_label, L"走行 %02d", stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_GT_LABEL), stOpePaneStat.static_gt_label);
			}
		}

		break;
	}

	case WM_TIMER: {

		//PB入力オフディレイ
		if (stOpePaneStat.button_antisway > 0) stOpePaneStat.button_antisway--;
		if (stOpePaneStat.button_auto_mode > 0) stOpePaneStat.button_auto_mode--;
		if (stOpePaneStat.button_auto_reset > 0) stOpePaneStat.button_auto_reset--;
		if (stOpePaneStat.button_set_z > 0) stOpePaneStat.button_set_z--;
		if (stOpePaneStat.button_set_xy > 0) stOpePaneStat.button_set_xy--;
		if (stOpePaneStat.button_mh_p1 > 0) stOpePaneStat.button_mh_p1--;
		if (stOpePaneStat.button_mh_p2 > 0) stOpePaneStat.button_mh_p2--;
		if (stOpePaneStat.button_mh_m1 > 0) stOpePaneStat.button_mh_m1--;
		if (stOpePaneStat.button_mh_m2 > 0) stOpePaneStat.button_mh_m2--;
		if (stOpePaneStat.button_sl_p1 > 0) stOpePaneStat.button_sl_p1--;
		if (stOpePaneStat.button_sl_p2 > 0) stOpePaneStat.button_sl_p2--;
		if (stOpePaneStat.button_sl_m1 > 0) stOpePaneStat.button_sl_m1--;
		if (stOpePaneStat.button_sl_m2 > 0) stOpePaneStat.button_sl_m2--;
		if (stOpePaneStat.button_bh_p1 > 0) stOpePaneStat.button_bh_p1--;
		if (stOpePaneStat.button_bh_p2 > 0) stOpePaneStat.button_bh_p2--;
		if (stOpePaneStat.button_bh_m1 > 0) stOpePaneStat.button_bh_m1--;
		if (stOpePaneStat.button_bh_m2 > 0) stOpePaneStat.button_bh_m2--;
		if (stOpePaneStat.button_park > 0) stOpePaneStat.button_park--;
		if (stOpePaneStat.button_grnd > 0) stOpePaneStat.button_grnd--;
		if (stOpePaneStat.button_pick > 0) stOpePaneStat.button_pick--;

		update_Work(hDlg);
	}break;
	case WM_CTLCOLORSTATIC:break;
	case WM_DESTROY: {
		KillTimer(hDlg, ID_WORK_UPDATE_TIMER);
	}return 0;

	}

	return FALSE;
}

LRESULT CALLBACK CWorkWindow_PLC::IOWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	HDC hdc;
	WCHAR wc[16];

	switch (msg) {
	case WM_DESTROY: {
		KillTimer(hwnd, ID_IO_CHK_UPDATE_TIMER);
	}return 0;
	case WM_CREATE: {
		InitCommonControls();//コモンコントロール初期化
		HINSTANCE hInst = GetModuleHandle(0);

		stIOCheckObj.is_bi_hex = true;
		stIOCheckObj.is_wi_hex = true;
		stIOCheckObj.is_bo_hex = true;
		stIOCheckObj.is_wo_hex = true;

		stIOCheckObj.bi_addr = DEVICE_TOP_B_IN;
		stIOCheckObj.bo_addr = DEVICE_TOP_B_OUT;
		stIOCheckObj.wi_addr = DEVICE_TOP_W_IN;
		stIOCheckObj.wo_addr = DEVICE_TOP_W_OUT;

		//メインウィンドウにコントロール追加

		//LABEL
		//デバイスアドレス
		stIOCheckObj.hwnd_label_addr = CreateWindowW(TEXT("STATIC"), L"Device", WS_CHILD | WS_VISIBLE | SS_LEFT,
			20, 30, 50, 20, hwnd, (HMENU)ID_PLCIO_STATIC_LABEL_ADDR, hInst, NULL);
		//デバイス	No	
		for (LONG i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			wsprintf(wc, L"%01d", i);
			stIOCheckObj.hwnd_label_no[i] = CreateWindowW(TEXT("STATIC"), wc, WS_CHILD | WS_VISIBLE | SS_CENTER,
				60, 55 + 15 * i, 20, 20, hwnd, (HMENU)(ID_PLCIO_STATIC_LABEL_0 + (INT64)i), hInst, NULL);
		}

		//デバイスアドレス表示	
		wsprintf(wc, L"%04x", stIOCheckObj.bi_addr);
		stIOCheckObj.hwnd_bi_addr_static = CreateWindowW(TEXT("STATIC"), wc, WS_CHILD | WS_VISIBLE | SS_CENTER,
			80, 30, 50, 20, hwnd, (HMENU)ID_PLCIO_STATIC_DI_ADDR, hInst, NULL);

		wsprintf(wc, L"%04x", stIOCheckObj.wi_addr);
		stIOCheckObj.hwnd_wi_addr_static = CreateWindowW(TEXT("STATIC"), wc, WS_CHILD | WS_VISIBLE | SS_CENTER,
			130, 30, 50, 20, hwnd, (HMENU)ID_PLCIO_STATIC_AI_ADDR, hInst, NULL);

		wsprintf(wc, L"%04x", stIOCheckObj.bo_addr);
		stIOCheckObj.hwnd_bo_addr_static = CreateWindowW(TEXT("STATIC"), wc, WS_CHILD | WS_VISIBLE | SS_CENTER,
			180, 30, 50, 20, hwnd, (HMENU)ID_PLCIO_STATIC_DO_ADDR, hInst, NULL);

		wsprintf(wc, L"%04x", stIOCheckObj.wo_addr);
		stIOCheckObj.hwnd_wo_addr_static = CreateWindowW(TEXT("STATIC"), wc, WS_CHILD | WS_VISIBLE | SS_CENTER,
			230, 30, 50, 20, hwnd, (HMENU)ID_PLCIO_STATIC_AO_ADDR, hInst, NULL);

		//デバイス値表示
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			stIOCheckObj.hwnd_bi_dat_static[i] = CreateWindowW(TEXT("STATIC"), L"0000", WS_CHILD | WS_VISIBLE | SS_CENTER,
				80, 55 + 15 * i, 50, 20, hwnd, (HMENU)(ID_PLCIO_STATIC_DI0 + INT64(i)), hInst, NULL);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			stIOCheckObj.hwnd_wi_dat_static[i] = CreateWindowW(TEXT("STATIC"), L"0000", WS_CHILD | WS_VISIBLE | SS_CENTER,
				130, 55 + 15 * i, 50, 20, hwnd, (HMENU)(ID_PLCIO_STATIC_AI0 + INT64(i)), hInst, NULL);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			stIOCheckObj.hwnd_bo_dat_static[i] = CreateWindowW(TEXT("STATIC"), L"0000", WS_CHILD | WS_VISIBLE | SS_CENTER,
				180, 55 + 15 * i, 50, 20, hwnd, (HMENU)(ID_PLCIO_STATIC_DO0 + INT64(i)), hInst, NULL);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			stIOCheckObj.hwnd_wo_dat_static[i] = CreateWindowW(TEXT("STATIC"), L"0000", WS_CHILD | WS_VISIBLE | SS_CENTER,
				230, 55 + 15 * i, 50, 20, hwnd, (HMENU)(ID_PLCIO_STATIC_AO0 + INT64(i)), hInst, NULL);
		}


		//ノッチ出力値表示

		stIOCheckObj.hwnd_notch_out_lavel = CreateWindowW(TEXT("STATIC"), L"           0+-2345", WS_CHILD | WS_VISIBLE | SS_LEFT,
			290, 55, 150, 20, hwnd, (HMENU)ID_PLCIO_STATIC_LABEL_NOCH, hInst, NULL);
		stIOCheckObj.hwnd_mh_notch_out_static = CreateWindowW(TEXT("STATIC"), L"HST OUT:", WS_CHILD | WS_VISIBLE | SS_LEFT,
			290, 75 , 150, 20, hwnd, (HMENU)ID_PLCIO_STATIC_MH_NOTCH, hInst, NULL);
		stIOCheckObj.hwnd_slw_notch_out_static = CreateWindowW(TEXT("STATIC"), L"SLW OUT:", WS_CHILD | WS_VISIBLE | SS_LEFT,
			290, 95, 150, 20, hwnd, (HMENU)ID_PLCIO_STATIC_SLW_NOTCH, hInst, NULL);
		stIOCheckObj.hwnd_bh_notch_out_static = CreateWindowW(TEXT("STATIC"), L"BH  OUT:", WS_CHILD | WS_VISIBLE | SS_LEFT,
			290, 115, 150, 20, hwnd, (HMENU)ID_PLCIO_STATIC_SLW_NOTCH, hInst, NULL);


		//ビット指令強制出力
		stIOCheckObj.hwnd_chk_pc_ctrl = CreateWindow(L"BUTTON", L"PC CTRL", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			290, 30, 80, 20, hwnd, (HMENU)ID_PLCIO_CHK_PC_CTRL, hInst, NULL);
		stIOCheckObj.hwnd_chk_plc_emulate = CreateWindow(L"BUTTON", L"PLC EMU", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			375, 30, 80, 20, hwnd, (HMENU)ID_PLCIO_CHK_EMULATE, hInst, NULL);

		//種別選択ラジオボタン
		stIOCheckObj.hwnd_radio_bi = CreateWindow(L"BUTTON", L"BI", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
			80, 5, 50, 20, hwnd, (HMENU)ID_PLCIO_RADIO_BI, hInst, NULL);
		stIOCheckObj.hwnd_radio_wi = CreateWindow(L"BUTTON", L"WI", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			130, 5, 50, 20, hwnd, (HMENU)ID_PLCIO_RADIO_WI, hInst, NULL);
		stIOCheckObj.hwnd_radio_bo = CreateWindow(L"BUTTON", L"BO", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			180, 5, 50, 20, hwnd, (HMENU)ID_PLCIO_RADIO_BO, hInst, NULL);
		stIOCheckObj.hwnd_radio_wo = CreateWindow(L"BUTTON", L"WO", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			230, 5, 50, 20, hwnd, (HMENU)ID_PLCIO_RADIO_WO, hInst, NULL);

		//出力値強制セット
		stIOCheckObj.hwnd_chk_pause = CreateWindow(L"BUTTON", L"PAUSE", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			20, 225, 80, 20, hwnd, (HMENU)ID_PLCIO_CHK_PAUSE, hInst, NULL);

		stIOCheckObj.hwnd_chk_forceset = CreateWindow(L"BUTTON", L"強制(16進）", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
			170, 225, 110, 20, hwnd, (HMENU)ID_PLCIO_CHK_FORCE, hInst, NULL);

		stIOCheckObj.hwnd_edit_forceset = CreateWindowEx(0, L"EDIT", L"0000", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_RIGHT,
			120, 225, 40, 20, hwnd, (HMENU)ID_PLCIO_EDIT_VALUE, hInst, NULL);
		SendMessage(stIOCheckObj.hwnd_edit_forceset, EM_SETLIMITTEXT, (WPARAM)4, 0);//入力可能文字数設定4文字


		//MELNET通信ボードステータス表示
		 CreateWindowW(TEXT("STATIC"), L"mel stat", WS_CHILD | WS_VISIBLE | SS_CENTER,
			20, 250, 60, 20, hwnd, (HMENU)ID_PLCIO_STATIC_LABEL_MEL_STAT, hInst, NULL);
		stIOCheckObj.hwnd_mel_status_static = CreateWindowW(TEXT("STATIC"), L"0000", WS_CHILD | WS_VISIBLE | SS_CENTER,
			85, 250, 70, 20, hwnd, (HMENU)ID_PLCIO_STATIC_MEL_STAT, hInst, NULL);

		CreateWindowW(TEXT("STATIC"), L"mel err", WS_CHILD | WS_VISIBLE | SS_CENTER,
			160, 250, 60, 20, hwnd, (HMENU)ID_PLCIO_STATIC_LABEL_MEL_ERR, hInst, NULL);
		stIOCheckObj.hwnd_mel_err_static = CreateWindowW(TEXT("STATIC"), L"00000000", WS_CHILD | WS_VISIBLE | SS_CENTER,
			225, 250, 70, 20, hwnd, (HMENU)ID_PLCIO_STATIC_MEL_ERR, hInst, NULL);

		//オフセット変更
		stIOCheckObj.hwnd_iochk_plusPB = CreateWindow(L"BUTTON", L"+", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20, 200, 20, 20, hwnd, (HMENU)ID_PLCIO_PB_PLUS, hInst, NULL);
		stIOCheckObj.hwnd_iochk_minusPB = CreateWindow(L"BUTTON", L"-", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			45, 200, 20, 20, hwnd, (HMENU)ID_PLCIO_PB_MINUS, hInst, NULL);
		stIOCheckObj.hwnd_iochk_minusPB = CreateWindow(L"BUTTON", L"RESET", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			70, 200, 40, 20, hwnd, (HMENU)ID_PLCIO_PB_RESET, hInst, NULL);

		stIOCheckObj.hwnd_edit_offset = CreateWindowEx(0, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_RIGHT,
			115, 200, 50, 20, hwnd, (HMENU)ID_PLCIO_EDIT_OFFSET, hInst, NULL);
		SendMessage(stIOCheckObj.hwnd_edit_offset, EM_SETLIMITTEXT, (WPARAM)4, 0);//入力可能文字数設定4文字

		stIOCheckObj.hwnd_label_offset = CreateWindowW(TEXT("STATIC"), L"OFFSET(10進）", WS_CHILD | WS_VISIBLE | SS_LEFT,
			170, 200, 110, 20, hwnd, (HMENU)ID_PLCIO_STATIC_LABEL_OFFSET, hInst, NULL);

		//10/16進表示
		stIOCheckObj.hwnd_iochk_decPB = CreateWindow(L"BUTTON", L"DEC", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE | WS_GROUP,
			15, 60, 35, 20, hwnd, (HMENU)ID_PLCIO_PB_DEC, hInst, NULL);
		stIOCheckObj.hwnd_iochk_hexPB = CreateWindow(L"BUTTON", L"HEX", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_PUSHLIKE,
			15, 85, 35, 20, hwnd, (HMENU)ID_PLCIO_PB_HEX, hInst, NULL);

		stIOCheckObj.bi_addr = DEVICE_TOP_B_IN;
		stIOCheckObj.bo_addr = DEVICE_TOP_B_OUT;
		stIOCheckObj.wi_addr = DEVICE_TOP_W_IN;
		stIOCheckObj.wo_addr = DEVICE_TOP_W_OUT;

		//表示更新タイマ起動
		SetTimer(hwnd, ID_IO_CHK_UPDATE_TIMER, IO_CHK_TIMER_PRIOD, NULL);

	}break;
	case WM_TIMER: {
		update_IOChk(hwnd);
	}break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}break;
	case WM_COMMAND: {
		switch (LOWORD(wp)) {
		case ID_PLCIO_PB_PLUS: {
			int n = GetDlgItemText(hwnd, ID_PLCIO_EDIT_OFFSET, (LPTSTR)wc, 5);
			int offset = _wtoi(wc);
			int id;
			switch (stIOCheckObj.IO_selected) {
			case ID_PLCIO_RADIO_BI: {
				id = stIOCheckObj.bi_addr - DEVICE_TOP_B_IN + offset;
				if (id >= N_PLC_B_OUT_WORD - PLCIO_IO_DISP_NUM) stIOCheckObj.bi_addr = DEVICE_TOP_B_IN + N_PLC_B_OUT_WORD - PLCIO_IO_DISP_NUM;
				else stIOCheckObj.bi_addr += offset;
				break;
			}
			case ID_PLCIO_RADIO_BO: {
				id = stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT + offset;
				if (id >= N_PC_B_OUT_WORD - PLCIO_IO_DISP_NUM) stIOCheckObj.bo_addr = DEVICE_TOP_B_OUT + N_PC_B_OUT_WORD - PLCIO_IO_DISP_NUM;
				else stIOCheckObj.bo_addr += offset;
				break;
			}
			case ID_PLCIO_RADIO_WI: {
				id = stIOCheckObj.wi_addr - DEVICE_TOP_W_IN + offset;
				if (id >= N_PLC_W_OUT_WORD - PLCIO_IO_DISP_NUM) stIOCheckObj.wi_addr = DEVICE_TOP_W_IN + N_PLC_W_OUT_WORD - PLCIO_IO_DISP_NUM;
				else stIOCheckObj.wi_addr += offset;
				break;
			}
			case ID_PLCIO_RADIO_WO: {
				id = stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT + offset;
				if (id >= N_PC_W_OUT_WORD - PLCIO_IO_DISP_NUM) stIOCheckObj.wo_addr = DEVICE_TOP_W_OUT + N_PC_W_OUT_WORD - PLCIO_IO_DISP_NUM;
				else stIOCheckObj.wo_addr += offset;
				break;
			}
			}
		}break;
		case ID_PLCIO_PB_MINUS: {
			int n = GetDlgItemText(hwnd, ID_PLCIO_EDIT_OFFSET, (LPTSTR)wc, 5);
			int offset = _wtoi(wc);
			int id;
			switch (stIOCheckObj.IO_selected) {
			case ID_PLCIO_RADIO_BI: {
				id = stIOCheckObj.bi_addr - DEVICE_TOP_B_IN - offset;
				if (id < 0) stIOCheckObj.bi_addr = DEVICE_TOP_B_IN;
				else stIOCheckObj.bi_addr -= offset;
				break;
			}
			case ID_PLCIO_RADIO_BO: {
				id = stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT - offset;
				if (id < 0) stIOCheckObj.bo_addr = DEVICE_TOP_B_OUT;
				else stIOCheckObj.bo_addr -= offset;
				break;
			}
			case ID_PLCIO_RADIO_WI: {
				id = stIOCheckObj.wi_addr - DEVICE_TOP_W_IN - offset;
				if (id < 0) stIOCheckObj.wi_addr = DEVICE_TOP_W_IN;
				else stIOCheckObj.wi_addr -= offset;
				break;
			}
			case ID_PLCIO_RADIO_WO: {
				id = stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT - offset;
				if (id < 0) stIOCheckObj.wo_addr = DEVICE_TOP_W_OUT;
				else stIOCheckObj.wo_addr -= offset;
				break;
			}
			}
		}break;
		case ID_PLCIO_PB_RESET: {
			stIOCheckObj.bi_addr = DEVICE_TOP_B_IN;
			stIOCheckObj.bo_addr = DEVICE_TOP_B_OUT;
			stIOCheckObj.wi_addr = DEVICE_TOP_W_IN;
			stIOCheckObj.wo_addr = DEVICE_TOP_W_OUT;
			pProcObj->mel_set_force(MEL_FORCE_RESET, false, 0, 0);
		}break;
		case ID_PLCIO_PB_DEC:
		case ID_PLCIO_PB_HEX:
		{
			switch (stIOCheckObj.IO_selected) {
			case ID_PLCIO_RADIO_BI: {
				if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_PB_DEC), BM_GETCHECK, 0, 0)) stIOCheckObj.is_bi_hex = false;
				else stIOCheckObj.is_bi_hex = true;
				break;
			}
			case ID_PLCIO_RADIO_BO: {
				if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_PB_DEC), BM_GETCHECK, 0, 0)) stIOCheckObj.is_bo_hex = false;
				else stIOCheckObj.is_bo_hex = true;
				break;
			}
			case ID_PLCIO_RADIO_WI: {
				if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_PB_DEC), BM_GETCHECK, 0, 0)) stIOCheckObj.is_wi_hex = false;
				else stIOCheckObj.is_wi_hex = true;
				break;
			}
			case ID_PLCIO_RADIO_WO: {
				if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_PB_DEC), BM_GETCHECK, 0, 0)) stIOCheckObj.is_wo_hex = false;
				else stIOCheckObj.is_wo_hex = true;
				break;
			}
			}

		}break;
		case ID_PLCIO_RADIO_BI:
		case ID_PLCIO_RADIO_BO:
		case ID_PLCIO_RADIO_WI:
		case ID_PLCIO_RADIO_WO:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_RADIO_BI), BM_GETCHECK, 0, 0)) stIOCheckObj.IO_selected = ID_PLCIO_RADIO_BI;
			else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_RADIO_BO), BM_GETCHECK, 0, 0))  stIOCheckObj.IO_selected = ID_PLCIO_RADIO_BO;
			else if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_RADIO_WO), BM_GETCHECK, 0, 0))  stIOCheckObj.IO_selected = ID_PLCIO_RADIO_WO;
			else stIOCheckObj.IO_selected = ID_PLCIO_RADIO_WI;

		}break;
		case ID_PLCIO_CHK_FORCE: {
			GetDlgItemText(hwnd, ID_PLCIO_EDIT_VALUE, (LPTSTR)wc, 5);
			unsigned __int64 value = _wcstoui64(wc, NULL, 16);
			switch (stIOCheckObj.IO_selected) {
			case ID_PLCIO_RADIO_BI: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PLC_B, false, stIOCheckObj.bi_addr - DEVICE_TOP_B_IN, (WORD)value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PLC_B, true, stIOCheckObj.bi_addr - DEVICE_TOP_B_IN, (WORD)value);
				break;
			}
			case ID_PLCIO_RADIO_BO: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PC_B, false, stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT, (WORD)value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PC_B, true, stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT, (WORD)value);
				break;
			}
			case ID_PLCIO_RADIO_WI: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PLC_W, false, stIOCheckObj.wi_addr - DEVICE_TOP_W_IN, (WORD)value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PLC_W, true, stIOCheckObj.wi_addr - DEVICE_TOP_W_IN, (WORD)value);
				break;
			}
			case ID_PLCIO_RADIO_WO: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PC_W, false, stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT, (WORD)value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PC_W, true, stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT, (WORD)value);
				break;
			}
			}

		}break;
		case ID_PLCIO_CHK_PC_CTRL: {
			if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_pc_ctrl, BM_GETCHECK, 0, 0))pProcObj->set_pc_ctrl_forced(true);
			else pProcObj->set_pc_ctrl_forced(false);
		}break;
		case ID_PLCIO_CHK_EMULATE: {
			if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_plc_emulate, BM_GETCHECK, 0, 0))pProcObj->set_plc_emu_forced(true);
			else pProcObj->set_plc_emu_forced(false);
		}break;
		case ID_PLCIO_CHK_PAUSE: {
			if (BST_CHECKED == SendMessage(GetDlgItem(hwnd, ID_PLCIO_CHK_PAUSE), BM_GETCHECK, 0, 0)) stIOCheckObj.is_pause_update = true;
			else stIOCheckObj.is_pause_update = false;
		}break;
		}
	}break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

int CWorkWindow_PLC::update_all_controls(HWND hDlg) {
	stOpePaneStat.slider_slew = SLW_SLIDAR_0_NOTCH;
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew);
	wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);

	stOpePaneStat.slider_mh = MH_SLIDAR_0_NOTCH;
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, stOpePaneStat.slider_mh);
	wsprintf(stOpePaneStat.static_mh_label, L"巻 %02d", stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MH_LABEL), stOpePaneStat.static_mh_label);

	stOpePaneStat.slider_bh = BH_SLIDAR_0_NOTCH;
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, BH_SLIDAR_MAX - (INT64)stOpePaneStat.slider_bh);
	wsprintf(stOpePaneStat.static_bh_label, L"引込 %02d", stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_BH_LABEL), stOpePaneStat.static_bh_label);

	stOpePaneStat.slider_gt = GT_SLIDAR_0_NOTCH;
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, TRUE, stOpePaneStat.slider_gt);
	wsprintf(stOpePaneStat.static_gt_label, L"走行 %02d", stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH);
	SetWindowText(GetDlgItem(hDlg, IDC_STATIC_GT_LABEL), stOpePaneStat.static_gt_label);
	return 0;
}

/******************************************************************************************************/
/*　　                 パネル上のランプ表示更新（TIMERイベントで呼び出し）　　　　　　　　　　　　　　*/
/******************************************************************************************************/
int CWorkWindow_PLC::update_Work(HWND hwnd) {

	//振れ止め
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_as_on[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_as_on[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_ANTISWAY_LAMP), L"●");
	}else SetWindowText(GetDlgItem(hwnd, IDC_ANTISWAY_LAMP), L"○");
	//起動
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_start[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_start[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_AUTOSTART_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_AUTOSTART_LAMP), L"○");

	//自動モード
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_mode[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_mode[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_AUTO_MODE_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_AUTO_MODE_LAMP), L"○");

	//半自動リセット
	if (stOpePaneStat.button_auto_reset == TRUE) {
		SetWindowText(GetDlgItem(hwnd, IDC_SEMI_RESET_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_SEMI_RESET_LAMP), L"○");

	//目標位置セット確定
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_set_xy[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_set_xy[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_XY_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_XY_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_set_xy[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_set_xy[ID_BPOS]) {
			SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_XY_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_XY_LAMP), L"○");

	//自動コマンド設定
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_park[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_park[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_PARK_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_PARK_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_pick[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_pick[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_PICK_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_PICK_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_grnd[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_grnd[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_GRND_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_GRND_LAMP), L"○");


	//半自動設定
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_s1[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_s1[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S1_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S1_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_s2[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_s2[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S2_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S2_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_s3[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_s3[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S3_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_S3_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_set_z[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_set_z[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_Z_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_SET_Z_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_l1[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_l1[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L1_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L1_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_l2[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_l2[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L2_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L2_LAMP), L"○");

	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.lamp_auto_tg_l3[ID_WPOS]] & pProcObj->melnet.pc_b_map.lamp_auto_tg_l3[ID_BPOS]) {
		SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L3_LAMP), L"●");
	}
	else SetWindowText(GetDlgItem(hwnd, IDC_STATIC_L3_LAMP), L"○");


	return 0;
}
int CWorkWindow_PLC::update_IOChk(HWND hwnd) {

	WCHAR wc[16];
	WORD source_w;
	LPST_MELSEC_NET pmel = pProcObj->get_melnet();
	WORD buf_id;
	std::wostringstream wos;

	//デバイスアドレス表示	
	buf_id = stIOCheckObj.bi_addr + (stIOCheckObj.bi_addr - DEVICE_TOP_B_IN) * 15;//BレジスタはBit単位のアドレスを表示する
	wsprintf(wc, L"%04x", buf_id);
	SetWindowText(stIOCheckObj.hwnd_bi_addr_static, wc);

	wsprintf(wc, L"%04x", stIOCheckObj.wi_addr);
	SetWindowText(stIOCheckObj.hwnd_wi_addr_static, wc);

	buf_id = stIOCheckObj.bo_addr + (stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT) * 15;//BレジスタはBit単位のアドレスを表示する
	wsprintf(wc, L"%04x", buf_id);
	SetWindowText(stIOCheckObj.hwnd_bo_addr_static, wc);

	wsprintf(wc, L"%04x", stIOCheckObj.wo_addr);
	SetWindowText(stIOCheckObj.hwnd_wo_addr_static, wc);

	if (stIOCheckObj.is_pause_update == false) { //表示アップデートポーズフラグOFF
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			buf_id = stIOCheckObj.bi_addr - DEVICE_TOP_B_IN;
			source_w = pmel->plc_b_out[buf_id + i];
			if (stIOCheckObj.is_bi_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_bi_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			buf_id = stIOCheckObj.wi_addr - DEVICE_TOP_W_IN;
			source_w = pmel->plc_w_out[buf_id + i];
			if (stIOCheckObj.is_wi_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_wi_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			buf_id = stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT;
			source_w = pmel->pc_b_out[buf_id + i];
			if (stIOCheckObj.is_bo_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_bo_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			buf_id = stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT;
			source_w = pmel->pc_w_out[buf_id + i];

			if (stIOCheckObj.is_wo_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_wo_dat_static[i], wc);
		}

	}

	wsprintf(wc, L"%08d", pmel->read_size_w);
	SetWindowTextW(stIOCheckObj.hwnd_mel_status_static, wc);
	wsprintf(wc, L"%08d", pmel->err);
	SetWindowTextW(stIOCheckObj.hwnd_mel_err_static, wc);



	//デバイス選択ラジオボタン設定
	switch (stIOCheckObj.IO_selected) {
	case ID_PLCIO_RADIO_BI: {
		if(pProcObj->melnet.is_force_set_active[MEL_FORCE_PLC_B]) 
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_CHECKED, 0);
		else
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_UNCHECKED, 0);
		break;
	}
	case ID_PLCIO_RADIO_BO: {
		if (pProcObj->melnet.is_force_set_active[MEL_FORCE_PC_B])
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_CHECKED, 0);
		else
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_UNCHECKED, 0);
		break;
	}
	case ID_PLCIO_RADIO_WI: {
		if (pProcObj->melnet.is_force_set_active[MEL_FORCE_PLC_W])
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_CHECKED, 0);
		else
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_UNCHECKED, 0);
		break;
	}
	case ID_PLCIO_RADIO_WO: {
		if (pProcObj->melnet.is_force_set_active[MEL_FORCE_PC_W])
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_CHECKED, 0);
		else
			SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_SETCHECK, BST_UNCHECKED, 0);
		break;
	}

	}

	//制御PC　ノッチ指令値表示
	wostringstream wo;

	WORD wsource = pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.com_hst_notch_0[ID_WPOS]];
	wo << L"HST OUT:";
	int mask = 0x0001;
	for (int i = 0;i < 7;i++) {
		if (wsource & mask) wo << L"1"; else wo << L"0";
		mask = mask << 1;
	}
	SetWindowTextW(stIOCheckObj.hwnd_mh_notch_out_static, (wo.str()).c_str()); wo.str(L""); wo.clear();

	wsource = pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.com_bh_notch_0[ID_WPOS]];
	wo << L"BH  OUT:";
	mask = 0x0001;
	for (int i = 0;i < 7;i++) {
		if (wsource & mask) wo << L"1"; else wo << L"0";
		mask = mask << 1;
	}
	SetWindowTextW(stIOCheckObj.hwnd_bh_notch_out_static, (wo.str()).c_str()); wo.str(L""); wo.clear();

	wsource = pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.com_slw_notch_0[ID_WPOS]];
	wo << L"SLW OUT:";
	mask = 0x0080;
	for (int i = 0;i < 7;i++) {
		if (wsource & mask) wo << L"1"; else wo << L"0";
		mask = mask << 1;
	}
	SetWindowTextW(stIOCheckObj.hwnd_slw_notch_out_static, (wo.str()).c_str()); wo.str(L""); wo.clear();


	//PC CONTROL チェックボックスの表示更新
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.com_pc_ctr_act[ID_WPOS]] & 0x1000)
		SendMessage(stIOCheckObj.hwnd_chk_pc_ctrl, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendMessage(stIOCheckObj.hwnd_chk_pc_ctrl, BM_SETCHECK, BST_UNCHECKED, 0);


	//PLC エミュレータモードチェックボックスの表示更新
	if (pProcObj->melnet.pc_b_out[pProcObj->melnet.pc_b_map.com_plc_emulate_act[ID_WPOS]] & 0x2000)
		SendMessage(stIOCheckObj.hwnd_chk_plc_emulate, BM_SETCHECK, BST_CHECKED, 0);
	else
		SendMessage(stIOCheckObj.hwnd_chk_plc_emulate, BM_SETCHECK, BST_UNCHECKED, 0);


	return 0;
}

