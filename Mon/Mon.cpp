// Mon.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "Mon.h"
#include "CMonWin.h"

#include <windowsx.h>       //コモンコントロール用
#include <commctrl.h>       //コモンコントロール用

#define MAX_LOADSTRING 100

// グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

//-共有メモリオブジェクトポインタ:
CSharedMem* pCraneStatusObj = NULL;
CSharedMem* pSwayStatusObj = NULL;
CSharedMem* pPLCioObj = NULL;
CSharedMem* pSwayIO_Obj = NULL;
CSharedMem* pRemoteIO_Obj = NULL;
CSharedMem* pCSInfObj = NULL;
CSharedMem* pPolicyInfObj = NULL;
CSharedMem* pAgentInfObj = NULL;

LPST_CRANE_STATUS pCraneStat = NULL;
LPST_PLC_IO pPLC_IO = NULL;
LPST_SWAY_IO pSway_IO = NULL;
LPST_REMOTE_IO pRemoteIO = NULL;
LPST_CS_INFO pCSinf = NULL;
LPST_POLICY_INFO pPOLICYinf = NULL;
LPST_AGENT_INFO pAGENTinf = NULL;

CMonWin* pMonWin;

static HWND                 hWnd_status_bar;    //ステータスバーのウィンドウのハンドル

// このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//# ウィンドウにステータスバーを追加
HWND CreateStatusbarMain(HWND hWnd);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: ここにコードを挿入してください。

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MON, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 共有メモリオブジェクトのインスタンス化
    pCraneStatusObj = new CSharedMem;
    pPLCioObj = new CSharedMem;
    pSwayIO_Obj = new CSharedMem;
    pRemoteIO_Obj = new CSharedMem;
    pCSInfObj = new CSharedMem;
    pPolicyInfObj = new CSharedMem;
    pAgentInfObj = new CSharedMem;
    

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MON));

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

    // 共有メモリオブジェクト削除
    delete pCraneStatusObj;
    delete pPLCioObj;
    delete pSwayIO_Obj;
    delete pRemoteIO_Obj;
    delete  pCSInfObj;
    delete pPolicyInfObj;
    delete pAgentInfObj;

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MON));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MON);
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

   ///-共有メモリ取得 ##################
   if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME, sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) return(FALSE);
   else if ((pCraneStat = (LPST_CRANE_STATUS)pCraneStatusObj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME, sizeof(ST_PLC_IO), MUTEX_PLC_IO_NAME)) return(FALSE);
   else if ((pPLC_IO = (LPST_PLC_IO)pPLCioObj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pSwayIO_Obj->create_smem(SMEM_SWAY_IO_NAME, sizeof(ST_SWAY_IO), MUTEX_SWAY_IO_NAME)) return(FALSE);
   else if ((pSway_IO = (LPST_SWAY_IO)pSwayIO_Obj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pRemoteIO_Obj->create_smem(SMEM_REMOTE_IO_NAME, sizeof(ST_REMOTE_IO), MUTEX_REMOTE_IO_NAME)) return(FALSE);
   else if ((pRemoteIO = (LPST_REMOTE_IO)pRemoteIO_Obj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pCSInfObj->create_smem(SMEM_CS_INFO_NAME, sizeof(ST_CS_INFO), MUTEX_CS_INFO_NAME)) return(FALSE);
   else if ((pCSinf = (LPST_CS_INFO)pCSInfObj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pPolicyInfObj->create_smem(SMEM_POLICY_INFO_NAME, sizeof(ST_POLICY_INFO), MUTEX_POLICY_INFO_NAME)) return(FALSE);
   else if ((pPOLICYinf = (LPST_POLICY_INFO)pPolicyInfObj->get_pMap()) == NULL) return(FALSE);

   if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME, sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)) return(FALSE);
   else if ((pAGENTinf = (LPST_AGENT_INFO)pAgentInfObj->get_pMap()) == NULL) return(FALSE);

   
   //メインウィンドウクリエイト
   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
       MAIN_WND_INIT_POS_X, MAIN_WND_INIT_POS_Y,
       MAIN_WND_INIT_SIZE_W, MAIN_WND_INIT_SIZE_H,
       nullptr, nullptr, hInstance, nullptr);
   if (!hWnd){
      return FALSE;
   }
   else {
       pMonWin = new CMonWin(hWnd);
       //メインウィンドウにコントロール配置
       pMonWin->init_main_window();
   }

   
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

static UINT16 helthy_count=0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    
    case WM_CREATE: {

        //メインウィンドウにステータスバー付加
        hWnd_status_bar = CreateStatusbarMain(hWnd);

        //表示更新タイマ起動
        SetTimer(hWnd, ID_UPDATE_TIMER, TIMER_PRIOD, NULL);

    }break;

    case WM_TIMER: {
        switch (wParam)
        {
        case ID_UPDATE_TIMER:
            pMonWin->disp_update();

            TCHAR tbuf[32];
            wsprintf(tbuf, L"　Helty:%06d", helthy_count);
            SendMessage(hWnd_status_bar, SB_SETTEXT, 5, (LPARAM)tbuf);
            helthy_count++;
            return 0;
        default:
            return 0;
        }
    }break;
    
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

    case WM_SIZE:
    {
        //ステータスバーにサイズ変更を通知付加
        SendMessage(hWnd_status_bar, WM_SIZE, wParam, lParam);
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
        //表示更新タイマ破棄
        KillTimer(hWnd, ID_UPDATE_TIMER);
        pMonWin->close_mon();

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
    int sb_size[] = { 180,360,540,720,900,1080 };//ステータス区切り位置

    InitCommonControls();
    hSBWnd = CreateWindowEx(
        0, //拡張スタイル
        STATUSCLASSNAME, //ウィンドウクラス
        NULL, //タイトル
        WS_CHILD | SBS_SIZEGRIP, //ウィンドウスタイル
        0, 0, //位置
        0, 0, //幅、高さ
        hWnd, //親ウィンドウ
        (HMENU)ID_STATUS, //ウィンドウのＩＤ
        hInst, //インスタンスハンドル
        NULL);
    SendMessage(hSBWnd, SB_SETPARTS, (WPARAM)6, (LPARAM)(LPINT)sb_size);//6枠で各枠の仕切り位置をパラーメータ指定
    ShowWindow(hSBWnd, SW_SHOW);
    return hSBWnd;
}
