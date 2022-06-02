#include "CWorkWindow_PLC.h"
#include "resource.h"
#include "PLC_IO_DEF.h"

CWorkWindow_PLC::CWorkWindow_PLC() {}
CWorkWindow_PLC::~CWorkWindow_PLC() {}
HWND CWorkWindow_PLC::hWorkWnd;
ST_PLC_DEBUG_PANEL CWorkWindow_PLC::stOpePaneStat;

//# #######################################################################
HWND CWorkWindow_PLC::open_WorkWnd(HWND hwnd) {

	InitCommonControls();//コモンコントロール初期化
	HINSTANCE hInst = GetModuleHandle(0);

#ifdef DIALOG_WND_TYPE

	//Workウィンドウの生成
	hWorkWnd = CreateDialog(hInst,L"IDD_OPERATION_PANEL", hwnd, (DLGPROC)WorkWndProc);
	MoveWindow(hWorkWnd, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H, TRUE);

#else
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WorkWndProc;// !CALLBACKでreturnを返していないとWindowClassの登録に失敗する
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("WorkWnd");
	wc.hIconSm = NULL;
	ATOM fb = RegisterClassExW(&wc);

	hWorkWnd = CreateWindow(TEXT("WorkWnd"),
		TEXT("WorkWnd"),
		WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H,
		hwnd,
		0,
		hInst,
		NULL);
#endif
	ShowWindow(hWorkWnd, SW_SHOW);
	UpdateWindow(hWorkWnd);

	return hWorkWnd;
};

//# Window 終了処理 ###################################################################################
int CWorkWindow_PLC::close_WorkWnd() {

	DestroyWindow(hWorkWnd);  //ウィンドウ破棄

	return 0;
}
//# コールバック関数 ########################################################################	

#ifdef DIALOG_WND_TYPE

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

		stOpePaneStat.slider_mh = stOpePaneStat.slider_bh = stOpePaneStat.slider_slew = stOpePaneStat.slider_gt = SLW_SLIDAR_0_NOTCH;


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
			SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, BH_SLIDAR_MAX - stOpePaneStat.slider_bh);
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

	case WM_NOTIFY:
		
		WPARAM ui_udn_deltapos = 4294966574;//(WPARAM)UDN_DELTAPOS;コンパイル時のC26454警告を出さない為一旦変数にコードを直接入れる

		if (wp == (WPARAM)IDC_SPIN_SLEW) {
			lpnmud = (LPNMUPDOWN)lp;
	
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_slew = lpnmud->iPos;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew );
				wsprintf(stOpePaneStat.static_slew_label, L"旋回 %02d", stOpePaneStat.slider_slew - SLW_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_SLEW_LABEL), stOpePaneStat.static_slew_label);
			}
		}
		else if(wp == (WPARAM)IDC_SPIN_BH) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_bh = lpnmud->iPos;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, BH_SLIDAR_MAX - stOpePaneStat.slider_bh);
				wsprintf(stOpePaneStat.static_bh_label, L"引込 %02d", stOpePaneStat.slider_bh - BH_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_BH_LABEL), stOpePaneStat.static_bh_label);
			}
		}
		else if (wp == (WPARAM)IDC_SPIN_MH) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_mh = lpnmud->iPos;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, MH_SLIDAR_MAX - stOpePaneStat.slider_mh );
				wsprintf(stOpePaneStat.static_mh_label, L"巻 %02d", stOpePaneStat.slider_mh - MH_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_MH_LABEL), stOpePaneStat.static_mh_label);
			}
		}
		else if (wp == (WPARAM)IDC_SPIN_GT) {
			lpnmud = (LPNMUPDOWN)lp;
			if (lpnmud->hdr.code == ui_udn_deltapos) {
				stOpePaneStat.slider_gt = lpnmud->iPos;
				SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, TRUE, GT_SLIDAR_MAX - stOpePaneStat.slider_gt);
				wsprintf(stOpePaneStat.static_gt_label, L"走行 %02d", stOpePaneStat.slider_gt - GT_SLIDAR_0_NOTCH);
				SetWindowText(GetDlgItem(hDlg, IDC_STATIC_GT_LABEL), stOpePaneStat.static_gt_label);
			}

		}

		break;
	}
	return FALSE;
}
#else
LRESULT CALLBACK CWorkWindow::WorkWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

	HDC hdc;
	switch (msg) {
	case WM_DESTROY: {
	}return 0;
	case WM_CREATE: {
	}break;
	case WM_TIMER: {
	}break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		hdc = BeginPaint(hwnd, &ps);
		EndPaint(hwnd, &ps);
	}break;
	case WM_COMMAND: {
		switch (LOWORD(wp)) {
		case ID_WORK_WND_CLOSE_PB: {
		}break;
		}
	}break;
	default:
		return DefWindowProc(hwnd, msg, wp, lp);
	}
	return 0;
}

#endif

int CWorkWindow_PLC::update_all_controls(HWND hDlg) {
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SLEW), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew + SLW_SLIDAR_0_NOTCH);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_BH), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew + BH_SLIDAR_0_NOTCH);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_MH), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew + MH_SLIDAR_0_NOTCH);
	SendMessage(GetDlgItem(hDlg, IDC_SLIDER_GT), TBM_SETPOS, TRUE, stOpePaneStat.slider_slew + GT_SLIDAR_0_NOTCH);
	return 0;
}