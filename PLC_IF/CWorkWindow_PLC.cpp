#include "CWorkWindow_PLC.h"

CWorkWindow::CWorkWindow() {}
CWorkWindow::~CWorkWindow() {}
HWND CWorkWindow::hWorkWnd;

//# #######################################################################
HWND CWorkWindow::open_WorkWnd(HWND hwnd) {

	InitCommonControls();//�R�����R���g���[��������
	HINSTANCE hInst = GetModuleHandle(0);

#ifdef DIALOG_WND_TYPE

	//Work�E�B���h�E�̐���
	hWorkWnd = CreateDialog(hInst,L"IDD_OPERATION_PANEL", hwnd, (DLGPROC)WorkWndProc);
	MoveWindow(hWorkWnd, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H, TRUE);
#else
	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(wc));

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WorkWndProc;// !CALLBACK��return��Ԃ��Ă��Ȃ���WindowClass�̓o�^�Ɏ��s����
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

//# Window �I������ ###################################################################################
int CWorkWindow::close_WorkWnd() {

	DestroyWindow(hWorkWnd);  //�E�B���h�E�j��

	return 0;
}
//# �R�[���o�b�N�֐� ########################################################################	

#ifdef DIALOG_WND_TYPE

LRESULT CALLBACK CWorkWindow::WorkWndProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg) {
	case WM_INITDIALOG: {

		InitCommonControls();
		return TRUE;
	}break;
	case WM_COMMAND: {

	}break;
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