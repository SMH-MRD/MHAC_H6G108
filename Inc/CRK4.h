#pragma once
#ifndef __RK4_H__INCLUDED
#define __RK4_H__INCLUDED

#include "CVector3.h"

class RK4 {
private:
public:
	//プロパティ
	double dt = 0.01; //時間間隔
	Vector3 r;   //位置ベクトル
	Vector3 v;   //速度ベクトル
	Vector3 dr;  //位置ベクトルの変化分
	Vector3 dv;  //速度ベクトルの変化分
				 //コンストラクタ
	RK4() {}
	RK4(double _dt) {
		dt = _dt;
	}
	RK4(double _dt, Vector3& _r, Vector3& _v) {
		dt = _dt;
		r.copy(_r);
		v.copy(_v);
	}
	//ディストラクタ
	~RK4() {}
	//加速度ベクトルを与えるメソッド　　継承先で再定義する
	virtual Vector3 A(double t, Vector3& r, Vector3& v);
	//速度ベクトルを与えるメソッド
	virtual Vector3 V(double t, Vector3& r, Vector3& v);
	//時間発展を計算するメソッド
	virtual void timeEvolution(double t);
};

#endif // __RK4_H__INCLUDED

#ifndef __RK4_Nbody_H__INCLUDED
#define __RK4_Nbody_H__INCLUDED


class RK4_Nbody {
private:
public:
	//プロパティ
	int N;                              //物体数
	double dt;                          //時間刻み幅
	Vector3* rs, * vs;                   //位置ベクトルと速度ベクトル
	Vector3* drs, * dvs;                 //位置ベクトルと速度ベクトルの変化分
										//以下、ルンゲ・クッタ法で利用する配列
	Vector3* v1s, * a1s, * _v1s, * _a1s;   //１段目用
	Vector3* v2s, * a2s, * _v2s, * _a2s;   //２段目用
	Vector3* v3s, * a3s, * _v3s, * _a3s;   //３段目用
	Vector3* v4s, * a4s;                 //４段目用
										//コンストラクタ
	RK4_Nbody(int _N, double _dt) { //両引数とも必須
		N = _N;
		dt = _dt;
		rs = new Vector3[N]; vs = new Vector3[N]; drs = new Vector3[N]; dvs = new Vector3[N];
		v1s = new Vector3[N]; a1s = new Vector3[N]; _v1s = new Vector3[N]; _a1s = new Vector3[N];
		v2s = new Vector3[N]; a2s = new Vector3[N]; _v2s = new Vector3[N]; _a2s = new Vector3[N];
		v3s = new Vector3[N]; a3s = new Vector3[N]; _v3s = new Vector3[N]; _a3s = new Vector3[N];
		v4s = new Vector3[N];
		a4s = new Vector3[N];
	}
	//ディストラクタ
	~RK4_Nbody() {
		//動的確保したメモリの開放
		delete[] rs;  delete[] vs;  delete[] drs;  delete[] dvs;
		delete[] v1s; delete[] a1s; delete[] _v1s; delete[] _a1s;
		delete[] v2s; delete[] a2s; delete[] _v2s; delete[] _a2s;
		delete[] v3s; delete[] a3s; delete[] _v3s; delete[] _a3s;
		delete[] v4s; delete[] a4s;
	}
	//加速度ベクトルを与えるメソッド
	virtual void A(double t, Vector3* rs, Vector3* vs, Vector3* out_as);
	//速度ベクトルを与えるメソッド
	virtual void V(double t, Vector3* rs, Vector3* vs, Vector3* out_vs);
	//時間発展を計算するメソッド
	void timeEvolution(double t);
};

#endif // __RK4_Nbody_H__INCLUDED