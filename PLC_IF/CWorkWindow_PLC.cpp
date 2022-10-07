#include "CWorkWindow_PLC.h"
#include "resource.h"
#include "PLC_IO_DEF.h"
#include "CPLC_IF.h"

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


		return TRUE;
	}break;
	case WM_COMMAND: {
		switch (LOWORD(wp)) {
		case IDC_BUTTON_SOURCE1_ON:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_SOURCE1_ON), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_source1_on = TRUE;
			}
			else {
				stOpePaneStat.button_source1_on = FALSE;
			}
		}break;
		case IDC_BUTTON_SOURCE1_OFF:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_SOURCE1_OFF), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_source1_off = TRUE;
			}
			else {
				stOpePaneStat.button_source1_off = FALSE;
			}
		}break;
		case IDC_BUTTON_SOURCE2_ON:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_SOURCE2_ON), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_source2_on = TRUE;
			}
			else {
				stOpePaneStat.button_source2_on = FALSE;
			}
		}break;
		case IDC_BUTTON_SOURCE2_OFF:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_SOURCE2_OFF), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_source2_off = TRUE;
			}
			else {
				stOpePaneStat.button_source2_off = FALSE;
			}
		}break;

		case IDC_BUTTON_AUTO_START:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_AUTO_START), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_auto_start = TRUE;
			}
			else {
				stOpePaneStat.button_auto_start = FALSE;
			}
		}break;
		case IDC_BUTTON_AUTO_RESET:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_AUTO_RESET), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_auto_reset = TRUE;
			}
			else {
				stOpePaneStat.button_auto_reset = FALSE;
			}
		}break;
		case IDC_BUTTON_FROM1:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_FROM1), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_from1 = TRUE;
			}
			else {
				stOpePaneStat.button_from1 = FALSE;
			}
		}break;
		case IDC_BUTTON_FROM2:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_FROM2), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_from2 = TRUE;
			}
			else {
				stOpePaneStat.button_from2 = FALSE;
			}
		}break;
		case IDC_BUTTON_FROM3:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_FROM3), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_from3 = TRUE;
			}
			else {
				stOpePaneStat.button_from3 = FALSE;
			}
		}break;
		case IDC_BUTTON_FROM4:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_FROM4), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_from4 = TRUE;
			}
			else {
				stOpePaneStat.button_from4 = FALSE;
			}
		}break;
		case IDC_BUTTON_TO1:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_TO1), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_to1 = TRUE;
			}
			else {
				stOpePaneStat.button_to1 = FALSE;
			}
		}break;
		case IDC_BUTTON_TO2:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_TO2), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_to2 = TRUE;
			}
			else {
				stOpePaneStat.button_to2 = FALSE;
			}
		}break;
		case IDC_BUTTON_TO3:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_TO3), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_to3 = TRUE;
			}
			else {
				stOpePaneStat.button_to3 = FALSE;
			}
		}break;
		case IDC_BUTTON_TO4:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_BUTTON_TO4), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_to4 = TRUE;
			}
			else {
				stOpePaneStat.button_to4 = FALSE;
			}
		}break;

		case IDC_CHECK_ESTOP:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_ESTOP), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_estop = TRUE;
			}
			else {
				stOpePaneStat.check_estop = FALSE;
			}
		}break;
		case IDC_CHECK_ANTISWAY:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_ANTISWAY), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.check_antisway = TRUE;
			}
			else {
				stOpePaneStat.check_antisway = FALSE;
			}
		}break;
		case IDC_CHECK_RMOTE:
		{
			if (BST_CHECKED == SendMessage(GetDlgItem(hDlg, IDC_CHECK_RMOTE), BM_GETCHECK, 0, 0)) {
				stOpePaneStat.button_remote = TRUE;
			}
			else {
				stOpePaneStat.button_remote = FALSE;
			}
		}break;
		case IDC_BUTTON_SLEW_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_SLEW), UDM_SETPOS, 0, SLW_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_slew = SLW_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew);
			wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);
		}break;
		case IDC_BUTTON_BH_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_BH), UDM_SETPOS, 0, BH_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_bh = BH_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, BH_SLIDAR_MAX - (INT64)stOpePaneStat.slider_bh);
			wsprintf(stOpePaneStat.static_bh_label, L"引込 %02d", stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_BH_LABEL), stOpePaneStat.static_bh_label);
		}break;
		case IDC_BUTTON_MH_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_MH), UDM_SETPOS, 0, MH_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_mh = MH_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, stOpePaneStat.slider_mh);
			wsprintf(stOpePaneStat.static_mh_label, L"巻 %02d", stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MH_LABEL), stOpePaneStat.static_mh_label);
		}break;
		case IDC_BUTTON_GT_0:
		{
			SendMessage(GetDlgItem(hDlg, IDC_SPIN_GT), UDM_SETPOS, 0, GT_SLIDAR_0_NOTCH);
			stOpePaneStat.slider_gt = GT_SLIDAR_0_NOTCH;
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, TRUE, stOpePaneStat.slider_gt);
			wsprintf(stOpePaneStat.static_gt_label, L"走行 %02d", stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH);
			SetWindowText(GetDlgItem(hDlg, IDC_STATIC_GT_LABEL), stOpePaneStat.static_gt_label);
		}break;
		}

	}break;

	case WM_NOTIFY://SPINコントロール用
		
		WPARAM ui_udn_deltapos = 4294966574;//(WPARAM)UDN_DELTAPOS;コンパイル時のC26454警告を出さない為一旦変数にコードを直接入れる

		if (wp == (WPARAM)IDC_SPIN_SLEW) {
			lpnmud = (LPNMUPDOWN)lp;
	
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_slew = lpnmud->iPos + lpnmud->iDelta;
				if (stOpePaneStat.slider_slew < 0) stOpePaneStat.slider_slew = 0;
				else if (stOpePaneStat.slider_slew > SLW_SLIDAR_MAX) stOpePaneStat.slider_slew = SLW_SLIDAR_MAX;
				else;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew );
				wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);
			}
		}
		else if(wp == (WPARAM)IDC_SPIN_BH) {
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
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, MH_SLIDAR_MAX - (INT64)stOpePaneStat.slider_mh );
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
			20, 250, 80, 20, hwnd, (HMENU)ID_PLCIO_CHK_PAUSE, hInst, NULL);


		//出力値強制セット
		stIOCheckObj.hwnd_chk_forceset = CreateWindow(L"BUTTON", L"強制(16進）", WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
			170, 250, 110, 20, hwnd, (HMENU)ID_PLCIO_CHK_FORCE, hInst, NULL);

		stIOCheckObj.hwnd_edit_forceset = CreateWindowEx(0, L"EDIT", L"0000", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_RIGHT,
			120, 250, 40, 20, hwnd, (HMENU)ID_PLCIO_EDIT_VALUE, hInst, NULL);
		SendMessage(stIOCheckObj.hwnd_edit_forceset, EM_SETLIMITTEXT, (WPARAM)4, 0);//入力可能文字数設定4文字

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

		stIOCheckObj.is_bi_hex = true;
		stIOCheckObj.is_wi_hex = true;
		stIOCheckObj.is_bo_hex = true;
		stIOCheckObj.is_wo_hex = true;

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
				if (id >= N_PC_B_OUT_WORD - PLCIO_IO_DISP_NUM) stIOCheckObj.bi_addr = DEVICE_TOP_B_OUT + N_PC_B_OUT_WORD - PLCIO_IO_DISP_NUM;
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
				if (id < 0) stIOCheckObj.bi_addr = DEVICE_TOP_B_OUT;
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
					pProcObj->mel_set_force(MEL_FORCE_PLC_B, false, stIOCheckObj.bi_addr - DEVICE_TOP_B_IN, value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PLC_B, true, stIOCheckObj.bi_addr - DEVICE_TOP_B_IN, value);
				break;
			}
			case ID_PLCIO_RADIO_BO: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PC_B, false, stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT, value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PC_B, true, stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT, value);
				break;
			}
			case ID_PLCIO_RADIO_WI: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PLC_W, false, stIOCheckObj.wi_addr - DEVICE_TOP_W_IN, value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PLC_W, true, stIOCheckObj.wi_addr - DEVICE_TOP_W_IN, value);
				break;
			}
			case ID_PLCIO_RADIO_WO: {
				if (BST_CHECKED == SendMessage(stIOCheckObj.hwnd_chk_forceset, BM_GETCHECK, 0, 0))
					pProcObj->mel_set_force(MEL_FORCE_PC_W, false, stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT, value);
				else
					pProcObj->mel_set_force(MEL_FORCE_PC_W, true, stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT, value);
				break;
			}
			}

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

int CWorkWindow_PLC::update_IOChk(HWND hwnd) {

	WCHAR wc[16];
	WORD source_w;
	LPST_MELSEC_NET pmel = pProcObj->get_melnet();

	//デバイスアドレス表示	
	wsprintf(wc, L"%04x", stIOCheckObj.bi_addr);
	SetWindowText(stIOCheckObj.hwnd_bi_addr_static, wc);

	wsprintf(wc, L"%04x", stIOCheckObj.wi_addr);
	SetWindowText(stIOCheckObj.hwnd_wi_addr_static, wc);

	wsprintf(wc, L"%04x", stIOCheckObj.bo_addr);
	SetWindowText(stIOCheckObj.hwnd_bo_addr_static, wc);

	wsprintf(wc, L"%04x", stIOCheckObj.wo_addr);
	SetWindowText(stIOCheckObj.hwnd_wo_addr_static, wc);


	if (stIOCheckObj.is_pause_update == false) {
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			source_w = pmel->plc_b_out[stIOCheckObj.bi_addr - DEVICE_TOP_B_IN + i];
			if (stIOCheckObj.is_bi_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_bi_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			source_w = pmel->plc_w_out[stIOCheckObj.wi_addr - DEVICE_TOP_W_IN + i];
			if (stIOCheckObj.is_wi_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_wi_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			source_w = pmel->pc_b_out[stIOCheckObj.bo_addr - DEVICE_TOP_B_OUT + i];
			if (stIOCheckObj.is_bo_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_bo_dat_static[i], wc);
		}
		for (int i = 0; i < PLCIO_IO_DISP_NUM; i++) {
			source_w = pmel->pc_w_out[stIOCheckObj.wo_addr - DEVICE_TOP_W_OUT + i];
			if (stIOCheckObj.is_wo_hex) {
				wsprintf(wc, L"%04XH", source_w);
			}
			else {
				wsprintf(wc, L"%05dD", source_w);
			}
			SetWindowTextW(stIOCheckObj.hwnd_wo_dat_static[i], wc);
		}

	}

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


	return 0;
}


