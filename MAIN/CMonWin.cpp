#include "CMonWin.h"

HWND CMonWin::open_mon(HWND hwnd_parent) {

	WNDCLASSEX wc;

	hInst = GetModuleHandle(0);

	ZeroMemory(&wc, sizeof(wc));
	wc.cbSize = sizeof(WNDCLASSEX);

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = MonProc;// !CALLBACK‚Åreturn‚ð•Ô‚µ‚Ä‚¢‚È‚¢‚ÆWindowClass‚Ì“o˜^‚ÉŽ¸”s‚·‚é
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(0, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = TEXT("SCADA_MON");
	wc.hIconSm = NULL;
	ATOM fb = RegisterClassExW(&wc);

	HWND hwnd = CreateWindow(TEXT("SCADA_MON"),
		TEXT("SCADA_MON"),
		WS_POPUPWINDOW | WS_VISIBLE | WS_CAPTION, 20, 600, 400, 400,
		hwnd_parent,
		0,
		hInst,
		NULL);

	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd);
	return hwnd;
};

CMonWin::CMonWin() {}
CMonWin::~CMonWin() {}



//########################################################################	

LRESULT CALLBACK CMonWin::MonProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {

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
		break;
	}
	case WM_COMMAND: {
		switch (LOWORD(wp)) {

		case 1: 
			break;

		}
	} return 0;

	}
	return DefWindowProc(hwnd, msg, wp, lp);
}

