#pragma once
#include "framework.h"

#define Bitcheck(a,b)	(a >> b) & 1
#define Bitset(a,b)		a |= (1<<b)
#define Bitclear(a,b)		a &= ~(1<<b)

using namespace std;

class CHelper
{
public:
	CHelper();
	~CHelper();
	//文字列
	static void Str2Wstr(const string& src, wstring& dest);									//string→wstringに変換
	//Bitmap
	static void put_bmp_built(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt);						//ビットマップ表示
	static void put_bmp_stretch(HDC hdc, HBITMAP hbmp, POINT dst_pt, POINT src_pt, int retio_persent);  //ビットマップ拡大縮小表示

	//ビット数カウント
	static UINT	bits_count(UINT16 x);
	static UINT	bits_count(UINT32 x);
	static UINT	bits_count(UINT64 x);

};
