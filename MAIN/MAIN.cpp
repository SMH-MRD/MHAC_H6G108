// MAIN.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "framework.h"
#include "MAIN.h"

#include "CTaskObj.h"		//タスクスレッドのクラス
#include "CHelper.h"		//補助関数集合クラス
#include "CEnvironment.h"	//環境クラス
#include "CPolicy.h"		//戦略クラス
#include "CAgent.h"			//エージェントクラス
#include "CSCADA.h"			//SCADAクラス
#include "CClientService.h"	//SCADAクラス
#include "CSharedMem.h"	    //共有メモリクラス

#include <windowsx.h>       //コモンコントロール用
#include <commctrl.h>       //コモンコントロール用

using namespace std;

#define MAX_LOADSTRING 100

///# 各種宣言　*****************************************************************************************************************
//-グローバル変数:
HINSTANCE hInst;                                // 現在のインターフェイス
WCHAR szTitle[MAX_LOADSTRING];                  // タイトル バーのテキスト
WCHAR szWindowClass[MAX_LOADSTRING];            // メイン ウィンドウ クラス名

SYSTEMTIME      gAppStartTime;					//アプリケーション開始時間
LPWSTR          pszInifile;					    // iniファイルのパス
wstring         wstrPathExeFolder;				// 実行ファイル格納フォルダのパス
wstring         wstrPathLogFolder;				// ログファイル格納フォルダのパス
wstring         wstrPathImgFolder;				// 画像ファイル格納フォルダのパス

vector<void*>	VectpCTaskObj;	                //タスクオブジェクトのポインタ
ST_iTask        g_itask;	                    //タスクオブジェクトのインデックス

//-共有メモリオブジェクトポインタ:
CSharedMem*  pCraneStatusObj;
CSharedMem*  pSwayStatusObj;
CSharedMem*  pSimulationStatusObj;
CSharedMem*  pPLCioObj;
CSharedMem*  pSwayIO_Obj;
CSharedMem*  pRemoteIO_Obj;
CSharedMem*  pCSInfObj;
CSharedMem*  pPolicyInfObj;
CSharedMem*  pAgentInfObj;
                                                
 //-スタティック変数:
static HWND                 hWnd_status_bar;    //ステータスバーのウィンドウのハンドル
static HWND                 hTabWnd;            //操作パネル用タブコントロールウィンドウのハンドル
static ST_KNL_MANAGE_SET    knl_manage_set;     //マルチスレッド管理用構造体
static vector<HWND>	        VectTweetHandle;	//メインウィンドウのスレッドツイートメッセージ表示Staticハンドル
static vector<HANDLE>	    VectHevent;		    //マルチスレッド用イベントのハンドル

static HIMAGELIST	        hImgListTaskIcon;	//タスクアイコン用イメージリスト

//-このコード モジュールに含まれる関数の宣言を転送します:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

//-アプリケーション専用の関数

// スレッド実行のためのゲート関数
// 引数　pObj タスククラスインスタンスのポインタ
static unsigned __stdcall thread_gate_func(void* pObj)
{
    CTaskObj* pthread_obj = (CTaskObj*)pObj;
    return pthread_obj->run(pObj);
}

HWND	            CreateStatusbarMain(HWND); //メインウィンドウステータスバー作成関数

int		            Init_tasks(HWND hWnd); //アプリケーション毎のタスクオブジェクトの初期設定
HWND	            CreateTaskSettingWnd(HWND hWnd); //個別タスク設定タブウィンドウ生成用関数
LRESULT CALLBACK    TaskTabDlgProc(HWND, UINT, WPARAM, LPARAM); //個別タスク設定タブウィンドウ用 メッセージハンドリング関数

DWORD	            knlTaskStartUp(); //実行させるタスクスレッドの起動処理
VOID	CALLBACK    alarmHandlar(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2); //マルチメディアタイマイベント処理関数　スレッド処理起動用イベントオブジェクト活性化処理


///# 関数　*****************************************************************************************************************
//
//  関数:wWinMain()
//
//  目的: エントリーポイント
//

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

     
    // 共有メモリオブジェクトのインスタンス化
    pCraneStatusObj         = new CSharedMem;
    pSwayStatusObj          = new CSharedMem;
    pSimulationStatusObj    = new CSharedMem;
    pPLCioObj               = new CSharedMem;
    pSwayIO_Obj             = new CSharedMem;
    pRemoteIO_Obj           = new CSharedMem;
    pCSInfObj               = new CSharedMem;
    pPolicyInfObj           = new CSharedMem;
    pAgentInfObj            = new CSharedMem;

    // グローバル文字列を初期化する
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MAIN, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // アプリケーション初期化の実行:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAIN));

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


///#　*****************************************************************************************************************
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MAIN);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

///#　*****************************************************************************************************************
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
       TAB_DIALOG_W + 40, (MSG_WND_H + MSG_WND_Y_SPACE) * TASK_NUM + TAB_DIALOG_H + 110,
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

/// アプリケーション固有の初期化処理を追加します。

  ///-ini file path設定##################
    static WCHAR dstpath[_MAX_PATH], szDrive[_MAX_DRIVE], szPath[_MAX_PATH], szFName[_MAX_FNAME], szExt[_MAX_EXT];
   //--exe failのpathを取得
   GetModuleFileName(NULL, dstpath, (sizeof(WCHAR) * _MAX_PATH / 2));
   //--取得したpathを分割
   _wsplitpath_s(dstpath, szDrive, sizeof(szDrive) / 2, szPath, sizeof(szPath) / 2, szFName, sizeof(szFName) / 2, szExt, sizeof(szExt) / 2);
   //--フォルダのパスとiniファイルのパスに合成
   _wmakepath_s(dstpath, sizeof(dstpath) / 2, szDrive, szPath, NAME_OF_INIFILE, EXT_OF_INIFILE);
   pszInifile = dstpath;

  ///-共有メモリ割付&設定##################
   if (OK_SHMEM != pCraneStatusObj->create_smem(SMEM_CRANE_STATUS_NAME,  sizeof(ST_CRANE_STATUS), MUTEX_CRANE_STATUS_NAME)) return(FALSE);
   if (OK_SHMEM != pSwayStatusObj->create_smem(SMEM_SWAY_STATUS_NAME,  sizeof(ST_SWAY_STATUS), MUTEX_SWAY_STATUS_NAME)) return(FALSE);
   if (OK_SHMEM != pSimulationStatusObj->create_smem(SMEM_SIMULATION_STATUS_NAME,  sizeof(ST_SIMULATION_STATUS), MUTEX_SIMULATION_STATUS_NAME)) return(FALSE);
   if (OK_SHMEM != pPLCioObj->create_smem(SMEM_PLC_IO_NAME,  sizeof(ST_PLC_IO), MUTEX_PLC_IO_NAME)) return(FALSE);
   if (OK_SHMEM != pSwayIO_Obj->create_smem(SMEM_SWAY_IO_NAME,  sizeof(ST_SWAY_IO), MUTEX_SWAY_IO_NAME)) return(FALSE);
   if (OK_SHMEM != pRemoteIO_Obj->create_smem(SMEM_REMOTE_IO_NAME,  sizeof(ST_REMOTE_IO), MUTEX_REMOTE_IO_NAME)) return(FALSE);
   if (OK_SHMEM !=  pCSInfObj->create_smem(SMEM_CS_INFO_NAME,  sizeof(ST_CS_INFO),MUTEX_CS_INFO_NAME)) return(FALSE);
   if (OK_SHMEM != pPolicyInfObj->create_smem(SMEM_POLICY_INFO_NAME,  sizeof(ST_POLICY_INFO),MUTEX_POLICY_INFO_NAME)) return(FALSE);
   if (OK_SHMEM != pAgentInfObj->create_smem(SMEM_AGENT_INFO_NAME,  sizeof(ST_AGENT_INFO), MUTEX_AGENT_INFO_NAME)) return(FALSE);
   
  /// -タスク設定##################
   //タスクオブジェクト個別設定
   Init_tasks(hWnd);
   
   InvalidateRect(hWnd, NULL, FALSE); //WM_PAINTを発生させてアイコンを描画させる
   UpdateWindow(hWnd);

   // タスクオブジェクトスレッド起動	
   knlTaskStartUp();		

   // タスクループ処理起動マルチメディアタイマ起動
   {
       // --マルチメディアタイマ精度設定
       TIMECAPS wTc;//マルチメディアタイマ精度構造体
       if (timeGetDevCaps(&wTc, sizeof(TIMECAPS)) != TIMERR_NOERROR) 	return((DWORD)FALSE);
       knl_manage_set.mmt_resolution = MIN(MAX(wTc.wPeriodMin, TARGET_RESOLUTION), wTc.wPeriodMax);
       if (timeBeginPeriod(knl_manage_set.mmt_resolution) != TIMERR_NOERROR) return((DWORD)FALSE);

       _RPT1(_CRT_WARN, "MMTimer Period = %d\n", knl_manage_set.mmt_resolution);

       // --マルチメディアタイマセット
       knl_manage_set.KnlTick_TimerID = timeSetEvent(knl_manage_set.cycle_base, knl_manage_set.mmt_resolution, (LPTIMECALLBACK)alarmHandlar, 0, TIME_PERIODIC);

       // --マルチメディアタイマー起動失敗判定　メッセージBOX出してFALSE　returen
       if (knl_manage_set.KnlTick_TimerID == 0) {	 //失敗確認表示
           LPVOID lpMsgBuf;
           FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL,
               0, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), /* Default language*/(LPTSTR)&lpMsgBuf, 0, NULL);
           MessageBox(NULL, (LPCWSTR)lpMsgBuf, L"MMT Failed!!", MB_OK | MB_ICONINFORMATION);// Display the string.
           LocalFree(lpMsgBuf);// Free the buffer.
           return((DWORD)FALSE);
       }
   }


   return TRUE;
}

///#　*****************************************************************************************************************
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
    case WM_CREATE:
    {
        //メインウィンドウにステータスバー付加
        hWnd_status_bar = CreateStatusbarMain(hWnd);
    }
    break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            //タスクツィートメッセージアイコン描画
            for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) ImageList_Draw(hImgListTaskIcon, i, hdc, 0, i * (MSG_WND_H + MSG_WND_Y_SPACE), ILD_NORMAL);
            EndPaint(hWnd, &ps);
        }
        break;

    case WM_SIZE:
    {
        //ステータスバーにサイズ変更を通知付加
        SendMessage(hWnd_status_bar, WM_SIZE, wParam, lParam);

        //タスク設定ウィンドウの位置調整
        RECT rc;
        GetClientRect(hWnd, &rc);
        TabCtrl_AdjustRect(hTabWnd, FALSE, &rc);
        MoveWindow(hTabWnd, TAB_POS_X, TAB_POS_Y, TAB_DIALOG_W, TAB_DIALOG_H, TRUE);

        for (unsigned i = 0; i < VectpCTaskObj.size(); i++) 
        {
            CTaskObj* pObj = (CTaskObj*)VectpCTaskObj[i];
            MoveWindow(pObj->inf.hWnd_opepane, TAB_POS_X, TAB_POS_Y + TAB_SIZE_H, TAB_DIALOG_W, TAB_DIALOG_H - TAB_SIZE_H, TRUE);
        }
    }
    break;
    case WM_NOTIFY://コモンコントロールでイベントが起こった場合、およびコモンコントロールが情報を親ウィンドウに要求する場合に、コモンコントロールの親ウィンドウに送信されます。
    {
        int tab_index = TabCtrl_GetCurSel(((NMHDR*)lParam)->hwndFrom);//タブコントロールの選択タブID取得

        
        for (unsigned i = 0; i < VectpCTaskObj.size(); i++)//全タスクオブジェクト確認
        {
            CTaskObj* pObj = (CTaskObj*)VectpCTaskObj[i];
            //タスク設定パネルウィンドウ位置調整
            MoveWindow(pObj->inf.hWnd_opepane, TAB_POS_X, TAB_POS_Y + TAB_SIZE_H, TAB_DIALOG_W, TAB_DIALOG_H - TAB_SIZE_H, TRUE);
            if ((VectpCTaskObj.size() - 1 - pObj->inf.index) == tab_index) //タブ選択されているタスクのみ処理
            {
                ShowWindow(pObj->inf.hWnd_opepane, SW_SHOW);
                //パネルのタスク名更新
                HWND hname_static = GetDlgItem(pObj->inf.hWnd_opepane, IDC_TAB_TASKNAME);
                SetWindowText(hname_static, pObj->inf.name);
                //パネルの設定ボタンテキスト更新
                pObj->set_panel_pb_txt();

                //実行関数の設定状況に応じてOption Checkボタンセット
                if (pObj->inf.work_select == THREAD_WORK_OPTION1) 
                {
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK1), BM_SETCHECK, BST_CHECKED, 0L);
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK2), BM_SETCHECK, BST_UNCHECKED, 0L);
                }
                else if (pObj->inf.work_select == THREAD_WORK_OPTION2) 
                {
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK1), BM_SETCHECK, BST_UNCHECKED, 0L);
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK2), BM_SETCHECK, BST_CHECKED, 0L);
                }
                else 
                {
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK1), BM_SETCHECK, BST_UNCHECKED, 0L);
                    SendMessage(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TASK_OPTION_CHECK2), BM_SETCHECK, BST_UNCHECKED, 0L);
                }

                // ウィンドウをフォアグラウンドに持ってくる 
                SetForegroundWindow(pObj->inf.hWnd_opepane);
            }
            else 
            {
                ShowWindow(pObj->inf.hWnd_opepane, SW_HIDE);
            }
        }
    }
    break;
    case WM_DESTROY:
        {
           
        timeKillEvent(knl_manage_set.KnlTick_TimerID);//マルチメディアタイマ停止
        Sleep(100);//100msec待機      

         ///-共有メモリ解放##################
         pCraneStatusObj->delete_smem();
         pSwayStatusObj->delete_smem();
         pSimulationStatusObj->delete_smem();
         pPLCioObj->delete_smem();
         pSwayIO_Obj->delete_smem();
         pRemoteIO_Obj->delete_smem();
         pCSInfObj->delete_smem();
         pPolicyInfObj->delete_smem();
         pAgentInfObj->delete_smem();
  
         PostQuitMessage(0);
         }
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

///#　*****************************************************************************************************************
//  関数: Init_tasks(HWND hWnd)
//
//  目的: アプリケーションタスクの初期設定をします。
//  
int Init_tasks(HWND hWnd)
{

    HBITMAP hBmp;
    CTaskObj* ptempobj;
    int task_index = 0;

    InitCommonControls();//コモンコントロール初期化
    hImgListTaskIcon = ImageList_Create(32, 32, ILC_COLOR | ILC_MASK, 2, 0);//タスクアイコン表示用イメージリスト設定

    //###Task1 設定 ENVIRONMENT
    {
        /// -タスクインスタンス作成->リスト登録
        ptempobj = new CEnvironment;
        VectpCTaskObj.push_back((void*)ptempobj);
        g_itask.environment = task_index;

        /// -タスクインデクスセット
        ptempobj->inf.index = task_index++;

        /// -イベントオブジェクトクリエイト->リスト登録	
        VectHevent.push_back(ptempobj->inf.hevents[ID_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

        /// -スレッド起動周期セット
        ptempobj->inf.cycle_ms = ENV_SCAN_MS;//タスクスキャンタイムmsec

        /// -ツイートメッセージ用iconセット
        hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_ENV");//ビットマップ割り当て
        ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
        DeleteObject(hBmp);

        ///オブジェクト名セット
        DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, ENVIRONMENT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name) / 2, PATH_OF_INIFILE);
        str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, ENVIRONMENT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname) / 2, PATH_OF_INIFILE);

        ///実行関数選択
        ptempobj->inf.work_select = THREAD_WORK_ROUTINE;
        ///スレッド起動に使うイベント数（定周期タイマーのみの場合１）
        ptempobj->inf.n_active_events = 1;

    }
    //###Task2 設定 SCADA
    {
        /// -タスクインスタンス作成->リスト登録
        ptempobj = new CSCADA;
        VectpCTaskObj.push_back((void*)ptempobj);
        g_itask.scada = task_index;

        /// -タスクインデクスセット
        ptempobj->inf.index = task_index++;

        /// -イベントオブジェクトクリエイト->リスト登録	
        VectHevent.push_back(ptempobj->inf.hevents[ID_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

        /// -スレッド起動周期セット
        ptempobj->inf.cycle_ms = SCADA_SCAN_MS;

        /// -ツイートメッセージ用iconセット
        hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_SCADA");//ビットマップ割り当て
        ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
        DeleteObject(hBmp);

        ///オブジェクト名セット
        DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, SCADA_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name) / 2, PATH_OF_INIFILE);
        str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, SCADA_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname) / 2, PATH_OF_INIFILE);

        ///実行関数選択
        ptempobj->inf.work_select = THREAD_WORK_ROUTINE;

        ///スレッド起動に使うイベント数（定周期タイマーのみの場合１）
        ptempobj->inf.n_active_events = 1;

    }

    //###Task3 設定 AGENT
    {
        /// -タスクインスタンス作成->リスト登録
        ptempobj = new CAgent;
        VectpCTaskObj.push_back((void*)ptempobj);
        g_itask.agent = task_index;

        /// -タスクインデクスセット
        ptempobj->inf.index = task_index++;

        /// -イベントオブジェクトクリエイト->リスト登録	
        VectHevent.push_back(ptempobj->inf.hevents[ID_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

        /// -スレッド起動周期セット
        ptempobj->inf.cycle_ms = AGENT_SCAN_MS;


        /// -ツイートメッセージ用iconセット
        hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_AGENT");//ビットマップ割り当て
        ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
        DeleteObject(hBmp);

        ///オブジェクト名セット
        DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, AGENT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name) / 2, PATH_OF_INIFILE);
        str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, AGENT_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname) / 2, PATH_OF_INIFILE);

        ///実行関数選択
        ptempobj->inf.work_select = THREAD_WORK_ROUTINE;

        ///スレッド起動に使うイベント数（定周期タイマーのみの場合１）
        ptempobj->inf.n_active_events = 1;

    }
    //###Task4 設定 POLICY
    {
        /// -タスクインスタンス作成->リスト登録
        ptempobj = new CPolicy;
        VectpCTaskObj.push_back((void*)ptempobj);
        g_itask.policy = task_index;

        /// -タスクインデクスセット
        ptempobj->inf.index = task_index++;

        /// -イベントオブジェクトクリエイト->リスト登録	
        VectHevent.push_back(ptempobj->inf.hevents[ID_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

        /// -スレッド起動周期セット
        ptempobj->inf.cycle_ms = POLICY_SCAN_MS;

        /// -ツイートメッセージ用iconセット
        hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_POLICY");//ビットマップ割り当て
        ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
        DeleteObject(hBmp);

        ///オブジェクト名セット
        DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, POLICY_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name) / 2, PATH_OF_INIFILE);
        str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, POLICY_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname) / 2, PATH_OF_INIFILE);

        ///実行関数選択
        ptempobj->inf.work_select = THREAD_WORK_ROUTINE;

        ///スレッド起動に使うイベント数（定周期タイマーのみの場合１）
        ptempobj->inf.n_active_events = 1;

    }

    //###Task5 設定 Client Service
    {
        /// -タスクインスタンス作成->リスト登録
        ptempobj = new CClientService;
        VectpCTaskObj.push_back((void*)ptempobj);
        g_itask.client = task_index;

        /// -タスクインデクスセット
        ptempobj->inf.index = task_index++;

        /// -イベントオブジェクトクリエイト->リスト登録	
        VectHevent.push_back(ptempobj->inf.hevents[ID_TIMER_EVENT] = CreateEvent(NULL, FALSE, FALSE, NULL));//自動リセット,初期値非シグナル

        /// -スレッド起動周期セット
        ptempobj->inf.cycle_ms = CS_SCAN_MS;

        /// -ツイートメッセージ用iconセット
        hBmp = (HBITMAP)LoadBitmap(hInst, L"IDB_CS");//ビットマップ割り当て
        ImageList_AddMasked(hImgListTaskIcon, hBmp, RGB(255, 255, 255));
        DeleteObject(hBmp);

        ///オブジェクト名セット
        DWORD	str_num = GetPrivateProfileString(OBJ_NAME_SECT_OF_INIFILE, CLIENT_SERVICE_KEY_OF_INIFILE, L"No Name", ptempobj->inf.name, sizeof(ptempobj->inf.name) / 2, PATH_OF_INIFILE);
        str_num = GetPrivateProfileString(OBJ_SNAME_SECT_OF_INIFILE, CLIENT_SERVICE_KEY_OF_INIFILE, L"No Name", ptempobj->inf.sname, sizeof(ptempobj->inf.sname) / 2, PATH_OF_INIFILE);

        ///実行関数選択
        ptempobj->inf.work_select = THREAD_WORK_ROUTINE;

        ///スレッド起動に使うイベント数（定周期タイマーのみの場合１）
        ptempobj->inf.n_active_events = 1;

    }


    ///各タスク用設定ウィンドウ作成
    InitCommonControls();	//コモンコントロール初期化
    hTabWnd = CreateTaskSettingWnd(hWnd);

    //各タスク残パラメータセット
    knl_manage_set.num_of_task = (unsigned int)VectpCTaskObj.size();//タスク数登録
    for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) {
        CTaskObj* pobj = (CTaskObj*)VectpCTaskObj[i];

        pobj->inf.index = i;	//タスクインデックスセット

        pobj->inf.hWnd_parent = hWnd;//親ウィンドウのハンドルセット
        pobj->inf.hInstance = hInst;//親ウィンドウのハンドルセット

        // -ツイートメッセージ用Static window作成->リスト登録	
        pobj->inf.hWnd_msgStatics = CreateWindow(L"STATIC", L"...", WS_CHILD | WS_VISIBLE, MSG_WND_ORG_X, MSG_WND_ORG_Y + MSG_WND_H * i + i * MSG_WND_Y_SPACE, MSG_WND_W, MSG_WND_H, hWnd, (HMENU)ID_STATIC_MAIN, hInst, NULL);
        VectTweetHandle.push_back(pobj->inf.hWnd_msgStatics);

        //その他設定
        pobj->inf.psys_counter = &knl_manage_set.sys_counter;
        pobj->inf.act_count = 0;//起動チェック用カウンタリセット
         //起動周期カウント値
        if (pobj->inf.cycle_ms >= SYSTEM_TICK_ms)	pobj->inf.cycle_count = pobj->inf.cycle_ms / SYSTEM_TICK_ms;
        else pobj->inf.cycle_count = 1;

        //最後に初期化関数呼び出し
        pobj->init_task(pobj);
    }

    return L_ON;
}
///#　*****************************************************************************************************************
//  関数: CreateTaskSettingWnd(HWND hWnd)
//
//  目的: アプリケーションタスク調整用パネルウィンドウを生成します。
//  
HWND CreateTaskSettingWnd(HWND hWnd)
{
    RECT rc;
    TC_ITEM tc[MAX_APP_TASK];//タブコントロール設定構造体

    //タブコントロールウィンドウの生成
    GetClientRect(hWnd, &rc);
    HWND hTab = CreateWindowEx(0, WC_TABCONTROL, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE,
        rc.left + TAB_POS_X, rc.top + TAB_POS_Y, TAB_DIALOG_W, TAB_DIALOG_H, hWnd, (HMENU)ID_TASK_SET_TAB, hInst, NULL);

    //Task Setting用パネルウィンドウタブ作成
    for (unsigned i = 0; i < VectpCTaskObj.size(); i++) {
        CTaskObj* pObj = (CTaskObj*)VectpCTaskObj[i];

        //タブコントロールにタブの追加
        tc[i].mask = (TCIF_TEXT | TCIF_IMAGE);//テキストとイメージ配置
        tc[i].pszText = pObj->inf.sname;
        tc[i].iImage = pObj->inf.index;       //イメージのID
        SendMessage(hTab, TCM_INSERTITEM, (WPARAM)0, (LPARAM)&tc[i]);

        //パネルウィンドウの生成
        pObj->inf.hWnd_opepane = CreateDialog(hInst, L"IDD_DIALOG_TASKSET1", hWnd, (DLGPROC)TaskTabDlgProc);
        pObj->set_panel_pb_txt();
        MoveWindow(pObj->inf.hWnd_opepane, TAB_POS_X, TAB_POS_Y + TAB_SIZE_H, TAB_DIALOG_W, TAB_DIALOG_H - TAB_SIZE_H, TRUE);

        //初期値はindex 0のウィンドウを表示
        if (i == 0) {
            ShowWindow(pObj->inf.hWnd_opepane, SW_SHOW);
            SetWindowText(GetDlgItem(pObj->inf.hWnd_opepane, IDC_TAB_TASKNAME), pObj->inf.name);//タスク名をスタティックテキストに表示
        }
        else ShowWindow(pObj->inf.hWnd_opepane, SW_HIDE);
    }

    //タブコントロールにイメージリストセット
    SendMessage(hTab, TCM_SETIMAGELIST, (WPARAM)0, (LPARAM)hImgListTaskIcon);

    return hTab;
}

///#　*****************************************************************************************************************
//  関数:TaskTabDlgProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的: アプリケーションタスク調整用タブウィンドウのメッセージハンドリング関数。
//  
LRESULT CALLBACK TaskTabDlgProc(HWND hDlg, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_INITDIALOG: {

        InitCommonControls();

        //メッセージ用リストコントロールの設定
        LVCOLUMN lvcol;

        //列ラベル設定
        LPTSTR strItem0[] = { (LPTSTR)(L"time"), (LPTSTR)(L"message") };//列ラベル
        int CX[] = { 60, 600 };//列幅
        HWND hList = GetDlgItem(hDlg, IDC_LIST1);
        lvcol.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
        lvcol.fmt = LVCFMT_LEFT;
        for (int i = 0; i < 2; i++) {
            lvcol.cx = CX[i];             // 表示位置
            lvcol.pszText = strItem0[i];  // 見出し
            lvcol.iSubItem = i;           // サブアイテムの番号
            ListView_InsertColumn(hList, i, &lvcol);
        }


        //リスト行追加
        LVITEM item;
        item.mask = LVIF_TEXT;
        for (int i = 0; i < MSG_LIST_MAX; i++) {
            item.pszText = (LPWSTR)L".";   // テキスト
            item.iItem = i;               // 番号
            item.iSubItem = 0;            // サブアイテムの番号
            ListView_InsertItem(hList, &item);
        }
        return TRUE;
    }break;
    case WM_COMMAND: {
        CTaskObj* pObj = (CTaskObj*)VectpCTaskObj[VectpCTaskObj.size() - TabCtrl_GetCurSel(hTabWnd) - 1];
       
        //タスクオブジェクト固有の処理
        pObj->PanelProc(hDlg, msg, wp, lp);
    }break;
    }
    return FALSE;
}
///#　*****************************************************************************************************************
//  関数: knlTaskStartUp()
//
//  目的    :登録されているタスクオブジェクトのスレッドを起動します。
//  機能	:[KNL]システム/ユーザタスクスタートアップ関数
//  処理	:自プロセスのプライオリティ設定，カーネルの初期設定,タスク生成，基本周期設定
//  戻り値  :Win32APIエラーコード
//  
DWORD knlTaskStartUp()
{
    HANDLE	myPrcsHndl;	// 本プログラムのプロセスハンドル
 
    //-プロセスハンドル取得
    if ((myPrcsHndl = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_SET_INFORMATION, FALSE, _getpid())) == NULL)	return(GetLastError());
    _RPT1(_CRT_WARN, "KNL Priority For Windows(before) = %d \n", GetPriorityClass(myPrcsHndl));

    //-自プロセスのプライオリティを最優先ランクに設定
    if (SetPriorityClass(myPrcsHndl, REALTIME_PRIORITY_CLASS) == 0)	return(GetLastError());
    _RPT1(_CRT_WARN, "KNL Priority For NT(after) = %d \n", GetPriorityClass(myPrcsHndl));

    //-アプリケーションタスク数が最大数を超えた場合は終了
    if (VectpCTaskObj.size() >= MAX_APP_TASK)	return((DWORD)ERROR_BAD_ENVIRONMENT);

    //- アプリケーションスレッド生成処理	
    for (unsigned i = 0; i < VectpCTaskObj.size(); i++) {

        CTaskObj* pobj = (CTaskObj*)VectpCTaskObj[i];

        // タスク生成(スレッド生成)
        // 他ﾌﾟﾛｾｽとの共有なし,スタック初期サイズ　デフォルト, スレッド実行関数　引数で渡すオブジェクトで対象切り替え,スレッド関数の引数（対象のオブジェクトのポインタ）, 即実行Createflags, スレッドID取り込み
        pobj->inf.hndl = (HANDLE)_beginthreadex(    //戻り値：スレッドハンドル
            (void*)NULL,					        //スレッドの属性
            0,								        //Stack初期Size
            thread_gate_func,				        //スレッドプロシジャー
            VectpCTaskObj[i],				        //スレッドのパラメータ
            (unsigned)0,					        //スレッド作成フラグ
            (unsigned*)&(pobj->inf.ID)		        //スレッドIDを受け取る変数のアドレス
        );

        // タスクプライオリティ設定
        if (pobj->inf.hndl != 0) {
            HANDLE hTh = pobj->inf.hndl;
            if (SetThreadPriority(hTh, pobj->inf.priority) == 0)
                return(GetLastError());
        }
        else {
            return(GetLastError());
        }
        _RPT2(_CRT_WARN, "Task[%d]_priority = %d\n", i, GetThreadPriority(pobj->inf.hndl));

        pobj->inf.act_count = 0;		// 基本ティックのカウンタ変数クリア
        pobj->inf.time_over_count = 0;	// 予定周期オーバーカウントクリア
    }

    return L_ON;
}

///#　*****************************************************************************************************************
//  関数: alarmHandlar(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
//
//  目的: マルチメディアタイマーイベントコールバック関数
//  
VOID	CALLBACK    alarmHandlar(UINT uID, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
    LONG64 tmttl;
    knl_manage_set.sys_counter++;

    //スレッドループ再開イベントセット処理
    for (unsigned i = 0; i < knl_manage_set.num_of_task; i++) {
        CTaskObj* pobj = (CTaskObj*)VectpCTaskObj[i];
        pobj->inf.act_count++;
        if (pobj->inf.act_count >= pobj->inf.cycle_count) {
            PulseEvent(VectHevent[i]);
            pobj->inf.act_count = 0;
            pobj->inf.total_act++;
        }
    }

    //Statusバーに経過時間表示
    if (knl_manage_set.sys_counter % 40 == 0) {// 1sec毎

        //起動後経過時間計算
        tmttl = (long long)knl_manage_set.sys_counter * knl_manage_set.cycle_base;//アプリケーション起動後の経過時間msec
        knl_manage_set.Knl_Time.wMilliseconds = (WORD)(tmttl % 1000); tmttl /= 1000;
        knl_manage_set.Knl_Time.wSecond = (WORD)(tmttl % 60); tmttl /= 60;
        knl_manage_set.Knl_Time.wMinute = (WORD)(tmttl % 60); tmttl /= 60;
        knl_manage_set.Knl_Time.wHour = (WORD)(tmttl % 60); tmttl /= 24;
        knl_manage_set.Knl_Time.wDay = (WORD)(tmttl % 24);

        TCHAR tbuf[32];
        wsprintf(tbuf, L"%3dD %02d:%02d:%02d", knl_manage_set.Knl_Time.wDay, knl_manage_set.Knl_Time.wHour, knl_manage_set.Knl_Time.wMinute, knl_manage_set.Knl_Time.wSecond);
        SendMessage(hWnd_status_bar, SB_SETTEXT, 5, (LPARAM)tbuf);
    }
    return;
}
