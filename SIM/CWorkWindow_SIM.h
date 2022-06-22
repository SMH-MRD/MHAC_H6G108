#pragma once

#include <Windows.h>
#include <commctrl.h>
#include <time.h>
#include <string>

#define WORK_WND_X							600			//MAP表示位置X
#define WORK_WND_Y							20			//MAP表示位置Y
#define WORK_WND_W							400		    //MAP WINDOW幅
#define WORK_WND_H							300			//MAP WINDOW高さ

#define WORK_SCAN_TIME						200			// 表示更新周期msec


//コントロールID
#define ID_WORK_WND_BASE                    10600
#define ID_WORK_WND_CLOSE_PB				ID_WORK_WND_BASE+1


//起動タイマーID
#define ID_WORK_WND_TIMER					100

#define NUM_OF_CTR_OBJECT					128

class CWorkWindow
{
public:
	CWorkWindow();
	~CWorkWindow();

	std::wstring wstr;
	HINSTANCE hInst;
	static HWND hWorkWnd;

	virtual HWND open_WorkWnd(HWND hwnd_parent);
	static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
	int close_WorkWnd();

private:
	
};


