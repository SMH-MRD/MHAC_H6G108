#pragma once

#include "framework.h"

#define GRAPHIC_AREA_X		0		//グラフィック部メインウィンドウ上表示位置X
#define GRAPHIC_AREA_Y		0		//グラフィック部メインウィンドウ上表示位置Y
#define GRAPHIC_AREA_W		600	    //グラフィック部幅
#define GRAPHIC_AREA_H		480		//グラフィック部高さ

//操作ボタンID
#define IDC_MAP2D_START_PB					10601
#define IDC_MAP2D_PAUSE_PB					10602
#define IDC_MAP2D_DOWN_PB					10603
#define IDC_MAP2D_UP_PB						10604
#define IDC_MAP2D_RADIO_DISP1				10605   //表示切替１
#define IDC_MAP2D_RADIO_DISP2				10606   //表示切替２
#define IDC_MAP2D_RADIO_DISP3				10607   //表示切替３
#define IDC_MAP2D_RADIO_DISP4				10608   //表示切替４
#define IDC_MAP2D_RADIO_DISP5				10609   //表示切替５
#define IDC_MAP2D_RADIO_DISP6				10610   //表示切替６

//STATIC TEXT ID
#define IDC_MAP2D_STATIC0					10611		
#define IDC_MAP2D_STATIC1					10612		
#define IDC_MAP2D_STATIC2					10613		
#define IDC_MAP2D_STATIC3					10614		
#define IDC_MAP2D_STATIC4					10615		
#define IDC_MAP2D_STATIC5					10616		



class CMonWin
{
public:
    CMonWin(HWND hWnd) { hWnd_parent = hWnd; }
    ~CMonWin() {}
    int init_main_window();
    int disp_update();

private:
    HWND hWnd_parent;

};

