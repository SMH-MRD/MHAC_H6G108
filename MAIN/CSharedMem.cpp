#include "CSharedMem.h"
/****************************************************************************************************************************************************
コンストラクタ　デストラクタ
****************************************************************************************************************************************************/
CSharedMem::CSharedMem()
{
	smem_available = L_OFF;			//共有メモリ無効
	data_size = set_data_size();		//データサイズ
	hMapFile = NULL;
	pMapTop = smem_dummy_buf;
	dwExist = 0;
	buf0Ptr = smem_dummy_buf;
	buf1Ptr = smem_dummy_buf;
	ibuf_write = 0;
	use_double_buff = false;		//ダブルバッファ利用の選択
}

CSharedMem::~CSharedMem()
{
	delete_smem(); 
}

/****************************************************************************************************************************************************
共有メモリを確保する
- 引数
	-入力　LPCTSTR szName：共有メモリ名, DWORD dwSize：確保するサイズ,
	-出力　HANDLE* hMapFile：ファイルマップオブジェクトハンドル, LPVOID* pMapTop:共有メモリ先頭アドレス, DWORD* dwExist:　GetLastError()エラー有り

- 戻り値　0:正常完了　-1,-2:異常完了
****************************************************************************************************************************************************/
int CSharedMem::create_smem(LPCTSTR szName, DWORD dwSize, LPCTSTR szMuName)
{
	DWORD	highByte = 0;			// 共有メモリは32bitサイズ以内を想定
	DWORD	lowByte;

	lowByte = dwSize;		// 32bitサイズ以上は定義不可
	data_size = dwSize;

	// 初期化
	hMapFile = NULL;
	pMapTop = NULL;
	dwExist = L_OFF;
	wstr_smem_name = szName;

	smem_available = L_OFF;

	// ファイル・マッピング・オブジェクトを作成(ページファイルを使用 ,セキュリティ属性なし ,読み込み/書き込みアクセス ,共有メモリのサイズ ,共有メモリ名)
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, highByte, lowByte, szName);
	if (hMapFile == NULL) {
		return(ERR_SHMEM_CREATE);
	}
	else {
		if (GetLastError() == ERROR_ALREADY_EXISTS)	dwExist = L_ON;
		// ファイル・ビューを作成(共有メモリのハンドル, アクセスの種類, ファイル オフセット(上位32ビット), ファイル オフセット(下位32ビット), マップするバイト数→0はファイルﾙマッピングオブジェクト全体)
		pMapTop = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
		if (pMapTop == NULL) {
			CloseHandle(hMapFile);	// 共有メモリを閉じる
			hMapFile = NULL; pMapTop = NULL;
			return(ERR_SHMEM_VIEW);
		}

		smem_available = L_ON;
		buf0Ptr = pMapTop;
		if (use_double_buff) buf1Ptr = (char*)pMapTop + data_size; else buf1Ptr = pMapTop;
	}

	clear_smem();

	hMutex= CreateMutex(NULL, FALSE, szMuName);
	if (hMutex == NULL) return ERR_SHMEM_MUTEX;

	return(OK_SHMEM);

}

/****************************************************************************************************************************************************
共有メモリを開放する
- 引数　HANDLE* hMapFile,：解放先ファイルマップオブジェクト LPVOID* pMapTop：解放先メモリ先頭アドレス
- 戻り値　0:正常完了,-2:異常完了
****************************************************************************************************************************************************/
int CSharedMem::delete_smem() {
	int		ret = OK_SHMEM;		// 関数ステータス

	CloseHandle(hMutex);

	if (pMapTop != NULL) {
		if (!UnmapViewOfFile(pMapTop)) {	// ファイル・ビューの削除
			ret = ERR_SHMEM_VIEW;
		}
	}

	// ファイル・マッピング・オブジェクトを閉じる
	if (hMapFile != NULL) {
		CloseHandle(hMapFile);
	}

	// ポインタ初期化
	hMapFile = NULL;
	pMapTop = NULL;

	return(ret);

}
/****************************************************************************************************************************************************
共有メモリを0クリアする
- 引数　なし
- 戻り値　L_ON:正常完了,　L_OFF:異常完了
****************************************************************************************************************************************************/
int CSharedMem::clear_smem() {

	if (!smem_available)
		return ERR_SHMEM_NOT_AVAILABLE;
	
	memset(pMapTop, 0, data_size);
	
	return(OK_SHMEM);
}

/****************************************************************************************************************************************************
共有メモリを更新する
- 引数　なし
- 戻り値　0,1:正常完了,　負号:異常完了
****************************************************************************************************************************************************/
int CSharedMem::update() {
	
	//共有メモリ未完
	if (!smem_available) 
		return ERR_SHMEM_NOT_AVAILABLE;
	
	if (ibuf_write)	ibuf_write = 0;
	else				ibuf_write = 1;
	
	return ibuf_write;
}

/****************************************************************************************************************************************************
共有メモリを更新する
- 引数　なし
- 戻り値　0,1:正常完了,　負号:異常完了
****************************************************************************************************************************************************/
int CSharedMem::set_data_size() {
	return SMEM_DATA_SIZE_MAX;
}

