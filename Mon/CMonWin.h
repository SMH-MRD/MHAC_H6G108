#pragma once

#include "framework.h"

#define INF_AREA_X		            10		//テキスト部メインウィンドウ上表示位置X
#define INF_AREA_Y		            30		//テキスト部メインウィンドウ上表示位置Y
#define INF_AREA_W		            1020    //テキスト部幅
#define INF_AREA_H		            450		//テキスト部高さ

#define GRAPHIC_AREA_X		        0		//グラフィック部左上表示位置X
#define GRAPHIC_AREA_Y		        0		//グラフィック部左上表示位置Y
#define GRAPHIC_AREA_W		        620	    //グラフィック部幅
#define GRAPHIC_AREA_H		        450		//グラフィック部高さ

#define CMON_PIX_PER_M_CRANE        5       //1m 5pixel クレーン部
#define CMON_PIX_PER_M_HOIST        5       //1m 5pixel 巻位置部
#define CMON_PIX_R_BOOM_END         3       //ブーム端点表示円半径
#define CMON_PIX_PER_RAD_LOAD       1000    //1rad 1000pixel 吊荷部
#define CMON_PIX_PER_M_GNT          2       //1m 2pixel   走行位置
#define CMON_PIX_GNT_MARK_W         2       //走行位置表示マーク幅
#define CMON_PIX_HST_MARK_W         2       //巻位置表示マーク幅

#define CRANE_GRAPHIC_CENTER_X      160	    //クレーングラフィック中心位置X
#define CRANE_GRAPHIC_CENTER_Y	    230		//クレーングラフィック中心位置Y
#define CRANE_GRAPHIC_W             290	    //クレーングラフィック中心位置X
#define CRANE_GRAPHIC_H	            290		//クレーングラフィック中心位置Y

#define LOAD_GRAPHIC_CENTER_X       480		//吊荷グラフィック中心位置X
#define LOAD_GRAPHIC_CENTER_Y	    230		//吊荷グラフィック中心位置Y
#define LOAD_GRAPHIC_W              290		//吊荷グラフィック幅W
#define LOAD_GRAPHIC_H	            290		//吊荷グラフィック高さH

#define LOAD_GRAPHIC_MARKLINE_MM    200     //吊荷グラフィックの振れ幅チェックライン位置
#define LOAD_GRAPHIC_PIX_1RAD       1146.0     //15°≒　300pix→ 1rad　≒　1146pix

#define GNT_GRAPHIC_AREA_X          10		//走行位置表示グラフィック左上位置X
#define GNT_GRAPHIC_AREA_Y	        10		//走行位置表示グラフィック左上位置Y
#define GNT_GRAPHIC_AREA_W          620		//走行位置表示グラフィック幅W
#define GNT_GRAPHIC_AREA_H	        20		//走行位置表示グラフィック高さH
#define MH_GRAPHIC_AREA_X           315		//巻位置表示グラフィック左上位置X
#define MH_GRAPHIC_AREA_Y	        80		//巻位置表示グラフィック左上位置Y
#define MH_GRAPHIC_AREA_W           10		//巻位置表示グラフィック幅W
#define MH_GRAPHIC_AREA_H	        128		//巻位置表示グラフィック高さH
#define MH_GRAPHIC_Y0	            230		//巻位置表示グラフィック0m位置Y
#define MH_GRAPHIC_UPPER_LIM	    84		//巻上限位置表示Y0位置から14.5m分(146PIX)上
#define MH_GRAPHIC_LOWER_LIM	    340		//巻上限位置表示Y0位置から11m分(110PIX)下


#define N_CREATE_PEN                8
#define N_CREATE_BRUSH              8
#define CMON_RED_PEN                0
#define CMON_BLUE_PEN               1
#define CMON_GREEN_PEN              2
#define CMON_GLAY_PEN               3
#define CMON_YELLOW_PEN             4
#define CMON_MAZENDA_PEN            5 
#define CMON_MAZENDA_PEN2           6

#define CMON_RED_BRUSH              0
#define CMON_BLUE_BRUSH             1
#define CMON_GREEN_BRUSH            2
#define CMON_BG_BRUSH               3



//Monitor画面グラフィック部管理構造体
typedef struct _stMonGraphic {  
    int disp_item = 0;                    //表示項目
    
    int area_x = 0, area_y = 0, area_w = 0, area_h = 0; //メインウィンドウ上の表示エリア
    int bmp_w=0, bmp_h = 0;                   //グラフィックビットマップサイズ

    HBITMAP hBmap_mem0=NULL;
    HBITMAP hBmap_bg = NULL;
    HBITMAP hBmap_gr = NULL;
    HBITMAP hBmap_inf = NULL;
    HDC hdc_mem0=NULL;						//合成画面メモリデバイスコンテキスト
    HDC hdc_mem_bg = NULL;					    //グラフィック背景画面メモリデバイスコンテキスト
    HDC hdc_mem_gr = NULL;					    //グラフィック部メモリデバイスコンテキスト
    HDC hdc_mem_inf = NULL;					//文字画面メモリデバイスコンテキスト

    HFONT hfont_inftext=NULL;				//テキスト用フォント
    BLENDFUNCTION bf = { 0,0,0,0 };					//半透過設定構造体

    HPEN hpen[N_CREATE_PEN] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL };
    HBRUSH hbrush[N_CREATE_BRUSH] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

}ST_MON_GRAPHIC, *LPST_MON_GRAPHIC;


//操作ボタンID
#define IDC_MON_START_PB				10601
#define IDC_MON_STOP_PB					10602
#define IDC_MON_RADIO_DISP0				10605   //表示切替0
#define IDC_MON_RADIO_DISP1				10606   //表示切替1
#define IDC_MON_RADIO_DISP2				10607   //表示切替2
#define IDC_MON_RADIO_DISP3				10608   //表示切替3
#define IDC_MON_RADIO_DISP4				10609   //表示切替4
#define IDC_MON_RADIO_DISP5				10610   //表示切替5

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
    CMonWin(HWND hWnd);
    ~CMonWin();
    int init_main_window();
    int disp_update();
    int close_mon();
    int combine_map();

    ST_MON_GRAPHIC stGraphic;
    ST_MON_COM_OBJ stComCtrl;

    VOID draw_bg();
    VOID draw_inf();
    VOID draw_inf_main();
    VOID draw_inf_sway();
    VOID draw_inf_ui();

    VOID draw_graphic();

 private:
    HWND hWnd_parent;

};

