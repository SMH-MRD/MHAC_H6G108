#pragma once

#include <Windows.h>
#include <commctrl.h>
#include <time.h>
#include <string>

#define DIALOG_WND_TYPE

#define WORK_WND_X							600			//MAP�\���ʒuX
#define WORK_WND_Y							425			//MAP�\���ʒuY
#define WORK_WND_W							480		    //MAP WINDOW��
#define WORK_WND_H							320			//MAP WINDOW����

#define WORK_SCAN_TIME						200			// �\���X�V����msec


//�R���g���[��ID
#define ID_WORK_WND_BASE                    10600
#define ID_WORK_WND_CLOSE_PB				ID_WORK_WND_BASE+1


//�N���^�C�}�[ID
#define ID_WORK_WND_TIMER					100

class CWorkWindow
{
public:
	CWorkWindow();
	~CWorkWindow();

	std::wstring wstr;
	static HWND hWorkWnd;

	virtual HWND open_WorkWnd(HWND hwnd_parent);
	static LRESULT CALLBACK WorkWndProc(HWND, UINT, WPARAM, LPARAM);
	int close_WorkWnd();
};

