#pragma once

#ifndef __Vector3_H__INCLUDED
#define __Vector3_H__INCLUDED

#include <math.h>

class Vector3 {

private:
public:
	//プロパティ
	double x, y, z;
	//コンストラクタ
	Vector3() {
		x = y = z = 0;
	};
	Vector3(double _x, double _y, double _z) {
		x = _x;	y = _y;	z = _z;
	}
	//ディストラクタ
	~Vector3() {};

	//オーバーロード「*=」
	friend void operator *= (Vector3& v1, const Vector3& v2) {
		v1.x *= v2.x;	v1.y *= v2.y;	v1.z *= v2.z;
	}
	//オーバーロード「+=」
	friend void operator += (Vector3& v1, const Vector3& v2) {
		v1.x += v2.x;	v1.y += v2.y;	v1.z += v2.z;
	}
	//オーバーロード「*」
	friend Vector3 operator * (const Vector3& v1, const Vector3& v2) {
		Vector3 v;
		v.x = v1.x * v2.x;	v.y = v1.y * v2.y;	v.z = v1.z * v2.z;
		return v;
	}
	//オーバーロード「*」
	friend Vector3 operator * (const Vector3& vec, double s) {
		Vector3 v;
		v.x = vec.x * s;	v.y = vec.y * s;	v.z = vec.z * s;
		return v;
	}
	//オーバーロード「*」
	friend Vector3 operator * (double s, const Vector3 vec) {
		Vector3 v;
		v.x = vec.x * s;	v.y = vec.y * s;	v.z = vec.z * s;
		return v;
	}
	//オーバーロード「+」
	friend Vector3 operator + (const Vector3& v1, const Vector3& v2) {
		Vector3 v;
		v.x = v1.x + v2.x;	v.y = v1.y + v2.y;	v.z = v1.z + v2.z;
		return v;
	}
	//オーバーロード「+」
	friend Vector3 operator + (const Vector3& v1, double s) {
		Vector3 v;
		v.x = v1.x + s;	v.y = v1.y + s;	v.z = v1.z + s;
		return v;
	}
	//オーバーロード「+」
	friend Vector3 operator + (double s, const Vector3& v1) {
		Vector3 v;
		v.x = v1.x + s;	v.y = v1.y + s;	v.z = v1.z + s;
		return v;
	}
	//オーバーロード「-」
	friend Vector3 operator - (const Vector3& v1, const Vector3& v2) {
		Vector3 v;
		v.x = v1.x - v2.x;	v.y = v1.y - v2.y;	v.z = v1.z - v2.z;
		return v;
	}
	//オーバーロード「-」
	friend Vector3 operator - (const Vector3& v1, double s) {
		Vector3 v;
		v.x = v1.x - s;	v.y = v1.y - s;	v.z = v1.z - s;
		return v;
	}
	//オーバーロード「-」
	friend Vector3 operator - (double s, const Vector3& v1) {
		Vector3 v;
		v.x = s - v1.x;	v.y = s - v1.y;	v.z = s - v1.z;
		return v;
	}
	//オーバーロード「/」
	friend Vector3 operator / (const Vector3& v1, const Vector3& v2) {
		Vector3 v;
		v.x = v1.x / v2.x;
		v.y = v1.y / v2.y;
		v.z = v1.z / v2.z;
		return v;
	}
	//オーバーロード「/」
	friend Vector3 operator / (const Vector3& vec, double s) {
		Vector3 v;
		v.x = vec.x / s;	v.y = vec.y / s;	v.z = vec.z / s;
		return v;
	}
	//オーバーロード「/」
	friend Vector3 operator / (double s, const Vector3 vec) {
		Vector3 v;
		v.x = s / vec.x;	v.y = s / vec.y;	v.z = s / vec.z;
		return v;
	}

	//静的メンバ関数
	static double dot(Vector3&, Vector3&);           //内積
	static double distanceSq(Vector3&, Vector3&);    //距離の２乗
	static double distance(Vector3&, Vector3&);      //距離
	static double lengthSq(Vector3&);                //長さの２乗
	static double length(Vector3&);                  //長さ
	static Vector3 addVectors(Vector3&, Vector3&);   //和
	static Vector3 subVectors(Vector3&, Vector3&);   //差
	static Vector3 crossVectors(Vector3&, Vector3&); //外積
													 //メンバ関数
	Vector3 clone();                                 //クローン
	Vector3& set(double, double, double);            //ベクトル成分の設定
	Vector3& copy(Vector3&);                         //コピー
	Vector3& add(Vector3&);                          //和
	Vector3& sub(Vector3&);                          //差
	Vector3& addScalor(double);                      //スカラー和
	Vector3& subScalor(double);                      //スカラー差
	Vector3& multiply(double);                       //スカラー積
	Vector3& multiply(Vector3&);                     //成分ごとの積
	Vector3& multiplyScalor(double);                 //スカラー積
	Vector3& divide(double);                         //スカラー商
	Vector3& divide(Vector3&);                       //成分ごとの商
	Vector3& divideScalor(double);                   //スカラー商
	Vector3& normalize();                            //規格化
	Vector3& cross(Vector3&);                        //外積
	Vector3& cyl2rec(Vector3&, double th);			//円筒座標→直行座標
	Vector3& rec2cyl(Vector3&, double th);			//直行座標→円筒座標
	double dot(Vector3&);                            //内積
	double length();                                 //長さ
	double lengthSq();                               //長さの２乗
	double angleTo(Vector3&);                        //なす角
	double distanceTo(Vector3&);                     //距離
	double distanceToSquared(Vector3&);              //距離の２乗
	bool equals(Vector3&);                           //同値判定

};

#endif // __Vector3_H__INCLUDED