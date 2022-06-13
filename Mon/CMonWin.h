#pragma once

#include "framework.h"

#define INF_AREA_X		    10		    //テキスト部メインウィンドウ上表示位置X
#define INF_AREA_Y		    30		    //テキスト部メインウィンドウ上表示位置Y
#define INF_AREA_W		    1000        //テキスト部幅
#define INF_AREA_H		    450		    //テキスト部高さ

#define GRAPHIC_AREA_X		10		    //グラフィック部メインウィンドウ上表示位置X
#define GRAPHIC_AREA_Y		30		    //グラフィック部メインウィンドウ上表示位置Y
#define GRAPHIC_AREA_W		600	        //グラフィック部幅
#define GRAPHIC_AREA_H		450		    //グラフィック部高さ

//Monitor画面グラフィック部管理構造体
typedef struct _stMonGraphic {  
    DWORD disp_item;                    //表示項目
    
    int area_x, area_y, area_w, area_h; //メインウィンドウ上の表示エリア
    int bmp_w, bmp_h;                   //グラフィックビットマップサイズ

    HBITMAP hBmap_mem0;
    HBITMAP hBmap_bg;
    HBITMAP hBmap_gr;
    HBITMAP hBmap_inf;
    HDC hdc_mem0;						//合成画面メモリデバイスコンテキスト
    HDC hdc_mem_bg;					    //グラフィック背景画面メモリデバイスコンテキスト
    HDC hdc_mem_gr;					    //グラフィック部メモリデバイスコンテキスト
    HDC hdc_mem_inf;					//文字画面メモリデバイスコンテキスト

    HFONT hfont_inftext;				//テキスト用フォント
    BLENDFUNCTION bf;					//半透過設定構造体

    HPEN hpen;
    HBRUSH hbrush;

}ST_MON_GRAPHIC, *LPST_MON_GRAPHIC;


//操作ボタンID
#define IDC_MON_START_PB				10601
#define IDC_MON_STOP_PB					10602
#define IDC_MON_RADIO_DISP1				10605   //表示切替１
#define IDC_MON_RADIO_DISP2				10606   //表示切替２
#define IDC_MON_RADIO_DISP3				10607   //表示切替３
#define IDC_MON_RADIO_DISP4				10608   //表示切替４
#define IDC_MON_RADIO_DISP5				10609   //表示切替５
#define IDC_MON_RADIO_DISP6				10610   //表示切替６

//STATIC TEXT ID
#define IDC_MON_STATIC0					10611		
#define IDC_MON_STATIC1					10612		
#define IDC_MON_STATIC2					10613		
#define IDC_MON_STATIC3					10614		
#define IDC_MON_STATIC4					10615		
#define IDC_MON_STATIC5					10616		

//Monitor画面コモンコントロール管理構造体
typedef struct _stMonComObj {

    HWND hwnd_map2d_startPB;					//スタートPBのハンドル
    HWND hwnd_map2d_stopPB;					    //ストップPBのハンドル
    HWND hwnd_map2d_opt1_radio;					//チャートOption1PBのハンドル
    HWND hwnd_map2d_opt2_radio;					//チャートOption2PBのハンドル
    HWND hwnd_map2d_opt3_radio;					//チャートOption3PBのハンドル
    HWND hwnd_map2d_opt4_radio;					//チャートOption4PBのハンドル
    HWND hwnd_map2d_opt5_radio;					//チャートOption5PBのハンドル
    HWND hwnd_map2d_opt6_radio;					//チャートOption6PBのハンドル

    HWND hwnd_map2d_static0;					//スタティックテキストのハンドル
    HWND hwnd_map2d_static1;					//スタティックテキストのハンドル
    HWND hwnd_map2d_static2;					//スタティックテキストのハンドル
    HWND hwnd_map2d_static3;					//スタティックテキストのハンドル
    HWND hwnd_map2d_static4;					//スタティックテキストのハンドル
    HWND hwnd_map2d_static5;					//スタティックテキストのハンドル

} ST_MON_COM_OBJ, *LPST_MON_COM_OBJ;


class CMonWin
{
public:
    CMonWin(HWND hWnd) { hWnd_parent = hWnd; }
    ~CMonWin() {}
    int init_main_window();
    int disp_update();
    int close_mon();

private:
    HWND hWnd_parent;

    ST_MON_GRAPHIC stGraphic;
    ST_MON_COM_OBJ stComCtrl;

    VOID draw_bg();



 
};

