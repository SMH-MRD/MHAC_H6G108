// SIM.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "SIM.h"
#include "CSharedMem.h"	    //共有メモリクラス

#include <windowsx.h>       //コモンコントロール用
#include <commctrl.h>       //コモンコントロール用

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

//-共有メモリオブジェクトポインタ:
CCraneStatus* pCraneStatusObj;
CSwayStatus* pSwayStatusObj;
CSimulationStatus* pSimulationStatusObj;
CPLCIO* pPLCIO_Obj;
CSwayIO* pSwayIO_Obj;
CRemoteIO* pRemoteIO_Obj;
CJobStatus* pJobStatusObj;
CCommandStatus* pCommandStatusObj;
CExecStatus* pExecStatusObj;

//-共有メモリポインタ:
LPST_SIMULATION_STATUS pSimStat;




static HWND                 hWnd_status_bar;    //ステータスバーのウィンドウのハンドル


// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

HWND CreateStatusbarMain(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SIM, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SIM));

    MSG msg;

    // メイン メッセージ ループ:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  関数: MyRegisterClass()
//
//  目的: ウィンドウ クラスを登録します。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SIM));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SIM);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   関数: InitInstance(HINSTANCE, int)
//
//   目的: インスタンス ハンドルを保存して、メイン ウィンドウを作成します
//
//   コメント:
//
//        この関数で、グローバル変数でインスタンス ハンドルを保存し、
//        メイン プログラム ウィンドウを作成および表示します。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // グローバル変数にインスタンス ハンドルを格納する

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
       MAIN_WND_INIT_POS_X, MAIN_WND_INIT_POS_Y,
       MAIN_WND_INIT_SIZE_W, MAIN_WND_INIT_SIZE_H,
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   // 共有メモリオブジェクトのインスタンス化
   pCraneStatusObj = new CCraneStatus;
   pSwayStatusObj = new CSwayStatus;
   pSimulationStatusObj = new CSimulationStatus;
   pPLCIO_Obj = new CPLCIO;
   pSwayIO_Obj = new CSwayIO;
   pRemoteIO_Obj = new CRemoteIO;
   pJobStatusObj = new CJobStatus;
   pCommandStatusObj = new CCommandStatus;
   pExecStatusObj = new CExecStatus;

   if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME,  sizeof(ST_SIMULATION_STATUS))) return(FALSE);

   LPST_SIMULATION_STATUS pSimStat = (LPST_SIMULATION_STATUS)pSimulationStatusObj->get_writePtr();

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  関数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND  - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウを描画する
//  WM_DESTROY  - 中止メッセージを表示して戻る
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
    {
        //メインウィンドウにステータスバー付加
        hWnd_status_bar = CreateStatusbarMain(hWnd);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // 選択されたメニューの解析:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: HDC を使用する描画コードをここに追加してください...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// バージョン情報ボックスのメッセージ ハンドラーです。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

///#　*****************************************************************************************************************
//  関数: CreateStatusbarMain(HWND)
//
//  目的: メイン ウィンドウ下部にアプリケーションの状態を表示用のステータスバーを配置します。
//  
HWND CreateStatusbarMain(HWND hWnd)
{
    HWND hSBWnd;
    int sb_size[] = { 100,200,300,400,525,615 };//ステータス区切り位置

    InitCommonControls();
    hSBWnd = CreateWindowEx(
        0,                          //拡張スタイル
        STATUSCLASSNAME,            //ウィンドウクラス
        NULL,                       //タイトル
        WS_CHILD | SBS_SIZEGRIP,    //ウィンドウスタイル
        0, 0, //位置
        0, 0, //幅、高さ
        hWnd, //親ウィンドウ
        (HMENU)ID_STATUS,           //ウィンドウのＩＤ
        hInst,                      //インスタンスハンドル
        NULL);
     SendMessage(hSBWnd, SB_SETPARTS, (WPARAM)6, (LPARAM)(LPINT)sb_size);//6枠で各枠の仕切り位置をパラーメータ指定
    ShowWindow(hSBWnd, SW_SHOW);
    return hSBWnd;
}
