#pragma once

#include "framework.h"
#include "CSharedMem.h"

class CMonWin
{
public:
	CMonWin();
	~CMonWin();

	HINSTANCE hInst;

	HWND open_mon(HWND hwnd_parent);
	static LRESULT CALLBACK MonProc(HWND, UINT, WPARAM, LPARAM);

private:
	LPST_CRANE_STATUS		pCraneStat;
	LPST_PLC_IO				pPLC_IO;
	LPST_SWAY_IO			pSway_IO;
	LPST_REMOTE_IO			pRemoteIO;
	LPST_SIMULATION_STATUS	pSimStat;

};

