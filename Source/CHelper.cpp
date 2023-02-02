#include "CHelper.h"
#include <locale.h>

CHelper::CHelper() {}

CHelper::~CHelper() {}

/*****************************************************************************
string型をwstring型に変換します
- 引数　const std::string &src：変換前入力　, std::wstring &dest：変換後出力　
- 戻り値　NA
*****************************************************************************/
void CHelper::Str2Wstr(const std::string& src, std::wstring& dest) {

	setlocale(LC_ALL, "");

	wchar_t* wcs = new wchar_t[src.length() + 1];

	size_t ret_val;
	mbstowcs_s(&ret_val, wcs, src.length() + 1, src.c_str(), _TRUNCATE);

	dest = wcs;delete[] wcs;
	return;
}

/*****************************************************************************************
指定されたDC、位置に指定されたビットマップを書き込む（等倍）
- 引数	HDC hdc：書込先DC, HBITMAP hbmp：素材ビットマップ,
		POINT dst_pt：書込先の位置, POINT src_pt：書込元の位置
- 戻り値　NA
******************************************************************************************/
void CHelper::put_bmp_built(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt) {

	if ((hdc == NULL) || (hbmp == NULL)) return;

	BITMAP bmp;
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	int BMP_W = (int)bmp.bmWidth;
	int BMP_H = (int)bmp.bmHeight;

	HDC hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hbmp);

	BitBlt(hdc, dst_pt.x, dst_pt.y, BMP_W, BMP_H, hmdc, src_pt.x, src_pt.y, SRCCOPY);
	DeleteDC(hmdc);

	return;
};

/******************************************************************************************************
指定されたDC、位置に指定されたビットマップを書き込む（変倍）
- 引数	HDC hdc：書込先DC, HBITMAP hbmp：素材ビットマップ,
		POINT dst_pt：書込先の位置, POINT src_pt：書込元の位置,int retio_percent:書込先倍率
- 戻り値	NA
******************************************************************************************************/
void CHelper::put_bmp_stretch(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt, int retio_percent) {

	if ((hdc == NULL) || (hbmp == NULL)) return;

	BITMAP bmp;
	GetObject(hbmp, sizeof(BITMAP), &bmp);
	int BMP_W = (int)bmp.bmWidth;
	int BMP_H = (int)bmp.bmHeight;

	HDC hmdc = CreateCompatibleDC(hdc);
	SelectObject(hmdc, hbmp);

	StretchBlt(hdc, dst_pt.x, dst_pt.y, BMP_W * retio_percent / 100, BMP_H * retio_percent / 100, hmdc, src_pt.x, src_pt.y, BMP_W, BMP_H, SRCCOPY);
	DeleteDC(hmdc);

	return;
};

/****************************************************************************************************************************************************
整数のbit数をカウントする
- 引数　カウントする整数
- 戻り値　bit数
****************************************************************************************************************************************************/
UINT	CHelper::bits_count(UINT16 x) { int ret = 0; while (x) { x &= x - 1; ret++; } return ret; }
UINT	CHelper::bits_count(UINT32 x) { int ret = 0; while (x) { x &= x - 1; ret++; } return ret; }
UINT	CHelper::bits_count(UINT64 x) { int ret = 0; while (x) { x &= x - 1; ret++; } return ret; }

/*****************************************************************************
角度データを旋回の角度範囲(-180度～＋180度に校正する
- 引数　double* 　
- 戻り値　NA
*****************************************************************************/
void CHelper::fit_slew_axis(double* pd) {

	int n;
	double pi2;
	if (*pd > 0.0) {
		n = (int)(*pd / 6.2832);
		*pd -= (double)n * 6.2832;
		if(*pd > 3.1416) *pd -= 6.2832;
	}
	else {
		n = (int)(*pd / -6.2832);
		*pd += (double)n * 6.2832;
		if (*pd < -3.1416) *pd += 6.2832;
	}

	return;
}
