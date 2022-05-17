#pragma once

#include "CTaskObj.h"
#include "Spec.h"
#include "CSharedMem.h"

class CEnvironment :public CTaskObj
{
public:
    CEnvironment();
    ~CEnvironment();

    ST_SPEC spec;       //仕様情報 Environmentが共有メモリにセットする。

    LPST_CRANE_STATUS pCraneStat;

    LRESULT CALLBACK PanelProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp);

    void init_task(void* pobj);
    
    void routine_work(void* param);

    void get_external_data();   //外部データ取り込み
    void main_proc();           //処理内容
    void update_shared_data();   //出力データ更新

    //タブパネルのStaticテキストを設定
    void set_panel_tip_txt();
    //タブパネルのFunctionボタンのStaticテキストを設定
    void set_panel_pb_txt();
    
};

