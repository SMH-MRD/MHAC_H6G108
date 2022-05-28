#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

#define PLC_IO_HELTHY_NG_COUNT     8
#define SIM_HELTHY_NG_COUNT     8

typedef struct stEnvSubproc {
    bool is_plcio_join = false;
    bool is_sim_join = false;
} ST_ENV_SUBPROC, LPST_ENV_SUBPROC;


class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();

    ST_SPEC spec;       //仕様情報 Environmentが共有メモリにセットする。

    LPST_CRANE_STATUS pCraneStat;
    LPST_PLC_IO pPLC_IO;
    ST_ENV_SUBPROC st_subproc;

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    
    void routine_work(void* param);

    void input();               //外部データ取り込み
    void main_proc();           //処理内容
    void output();              //出力データ更新

    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();
    
};

