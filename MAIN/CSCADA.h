#pragma once

#include "CTaskObj.h"

#include "Spec.h"
#include "CSharedMem.h"
#include "MKChart.h"

#define SCAD_CHART_WND1         0   
#define SCAD_CHART_WND2         1 
#define SCAD_CHART_GRAPH1       0   
#define SCAD_CHART_GRAPH2       1 
#define SCAD_CHART_GRAPH3       2   
#define SCAD_CHART_GRAPH4       3 

#define SCAD_N_CHART_WND        2   //Chart表示Window数
#define SCAD_N_CHART_PER_WND    2   //Window内のChart表示数
#define SCAD_N_GRAPH_PAR_CHART  4   //Chart内のグラフ表示数
#define SCAD_N_BOOL_PAR_GRAPH	4   //グラフ毎のデジタル表示点数
#define SCAD_N_LEGEND_CH    	16  //凡例文字数
#define SCAD_N_CHART_AXIS        2   //Chart表示Window数
#define SCAD_X_AXIS              0   //X軸
#define SCAD_Y_AXIS              1   //Y軸

typedef struct _stCHART_PLOT
{
	double d[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART];                       //doubleデータ生値
    double d100[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART];                    //doubleデータ100%値
	int i[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART];                          //intデータ生値
    int i100[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART];                       //intデータ100%値
	bool b[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART][SCAD_N_BOOL_PAR_GRAPH];  //boolデータ生値
    bool b100[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART][SCAD_N_BOOL_PAR_GRAPH];  //boolデータ100%値
    char legend[SCAD_N_CHART_WND][SCAD_N_CHART_PER_WND][SCAD_N_GRAPH_PAR_CHART][SCAD_N_BOOL_PAR_GRAPH][SCAD_N_LEGEND_CH];//凡例文字列
}ST_CHART_PLOT, * LPST_CHART_PLOT;

class CSCADA :public CTaskObj
{
public:
    CSCADA();
    ~CSCADA();

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);

    void routine_work(void* param);

private:
    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void output();              //出力データ更新

    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();

    //CHART関連
    bool is_chart_test_active = false;
    ST_CHART_PLOT chart_plot_buf[SCAD_N_CHART_AXIS];
    void chart_test(int isample);
    int set_graph_item(int iwnd, int ichart, int igraph, int ibool,bool is_x, int type);
};



