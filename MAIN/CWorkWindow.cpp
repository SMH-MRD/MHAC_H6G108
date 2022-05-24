#include "CWorkWindow.h"

CWorkWindow::CWorkWindow() {}
CWorkWindow::~CWorkWindow() {}
HWND CWorkWindow::hWorkWnd;

//# #######################################################################
HWND CWorkWindow::open_WorkWnd(HWND hwnd_parent) {

	WNDCLASSEX wc;

	hInst = GetModuleHandle(0);

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

	hWorkWnd = CreateWindow(TEXT("WorkWn"),
		TEXT("Lidar_Map01"),
		WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, WORK_WND_X, WORK_WND_Y, WORK_WND_W, WORK_WND_H,
		hwnd_parent,
		0,
		hInst,
		NULL);

	ShowWindow(hWorkWnd, SW_SHOW);
	UpdateWindow(hWorkWnd);
	
	return hWorkWnd;
};

//# Window 終了処理 ###################################################################################
int CWorkWindow::close_WorkWnd() {

	DestroyWindow(hWorkWnd);  //ウィンドウ破棄

	return 0;
}
//# コールバック関数 ########################################################################	

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
