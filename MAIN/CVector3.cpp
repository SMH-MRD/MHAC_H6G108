#include "framework.h"
#include "CVector3.h"

////////////////////////////////////////////////////////////
// 静的メンバ関数
////////////////////////////////////////////////////////////
//内積
double Vector3::dot(Vector3& v1, Vector3& v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
//距離の２乗
double Vector3::distanceSq(Vector3& v1, Vector3& v2) {
	return (v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y) + (v1.z - v2.z) * (v1.z - v2.z);
}
//距離
double Vector3::distance(Vector3& v1, Vector3& v2) {
	return sqrt(distanceSq(v1, v2));
}
//長さの２乗
double Vector3::lengthSq(Vector3& v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}
//長さ
double Vector3::length(Vector3& v) {
	return sqrt(lengthSq(v));
}
//和
Vector3 Vector3::addVectors(Vector3& v1, Vector3& v2) {
	Vector3 v;
	v.x = v1.x + v2.x;
	v.y = v1.y + v2.y;
	v.z = v1.z + v2.z;
	return v;
}
//差
Vector3 Vector3::subVectors(Vector3& v1, Vector3& v2) {
	Vector3 v;
	v.x = v1.x - v2.x;
	v.y = v1.y - v2.y;
	v.z = v1.z - v2.z;
	return v;
}
//外積
Vector3 Vector3::crossVectors(Vector3& v1, Vector3& v2) {
	Vector3 v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}
////////////////////////////////////////////////////////////
// メンバ関数
////////////////////////////////////////////////////////////
//ベクトル成分の設定
Vector3& Vector3::set(double _x, double _y, double _z) {
	x = _x;
	y = _y;
	z = _z;
	return *this;
}
//コピー
Vector3& Vector3::copy(Vector3& vec) {
	x = vec.x;
	y = vec.y;
	z = vec.z;
	return *this;
}
//クローン
Vector3 Vector3::clone() {
	return Vector3(x, y, z);
}
//和
Vector3& Vector3::add(Vector3& vec) {
	x += vec.x;
	y += vec.y;
	z += vec.z;
	return *this;
}
//差
Vector3& Vector3::sub(Vector3& vec) {
	x -= vec.x;
	y -= vec.y;
	z -= vec.z;
	return *this;
}
//スカラー和
Vector3& Vector3::addScalor(double s) {
	x += s;
	y += s;
	z += s;
	return *this;
}
//スカラー差
Vector3& Vector3::subScalor(double s) {
	x -= s;
	y -= s;
	z -= s;
	return *this;
}
//スカラー積
Vector3& Vector3::multiply(double s) {
	x *= s;
	y *= s;
	z *= s;
	return *this;
}
//成分ごとの積
Vector3& Vector3::multiply(Vector3& vec) {
	x *= vec.x;
	y *= vec.y;
	z *= vec.z;
	return *this;
}
//スカラー積
Vector3& Vector3::multiplyScalor(double s) {
	multiply(s);
	return *this;
}
//スカラー商
Vector3& Vector3::divide(double s) {
	x /= s;
	y /= s;
	z /= s;
	return *this;
}
//成分ごとの商
Vector3& Vector3::divide(Vector3& vec) {
	x /= vec.x;
	y /= vec.y;
	z /= vec.z;
	return *this;
}
//スカラー商
Vector3& Vector3::divideScalor(double s) {
	divide(s);
	return *this;
}

//規格化
Vector3& Vector3::normalize() {
	double l = (*this).length();
	(*this).divideScalor(l);
	return *this;
}
//外積
Vector3& Vector3::cross(Vector3& vec) {
	x = y * vec.z - z * vec.y;
	y = z * vec.x - x * vec.z;
	z = x * vec.y - y * vec.x;
	return *this;
}
//内積
double Vector3::dot(Vector3& vec) {
	return Vector3::dot(*this, vec);
}
//長さ
double Vector3::length() {
	return Vector3::length(*this);
}
//長さの２乗
double Vector3::lengthSq() {
	return Vector3::lengthSq(*this);
}
//なす角
double Vector3::angleTo(Vector3& vec) {
	double d = (*this).dot(vec) / ((*this).length() * vec.length());
	return acos(d);
}
//距離
double Vector3::distanceTo(Vector3& vec) {
	return Vector3::distance(*this, vec);
}
//距離の２乗
double Vector3::distanceToSquared(Vector3& vec) {
	return Vector3::distanceSq(*this, vec);
}
//同値判定
bool Vector3::equals(Vector3& vec) {
	return (x == vec.x) && (y == vec.y) && (z == vec.z);
}
//円筒座標→直行座標
Vector3& Vector3::cyl2rec(Vector3& vec, double th) {
	x = vec.x * cos(th) - vec.y * sin(th);
	y = vec.x * sin(th) + vec.y * cos(th);
	z = vec.z;
	return *this;
};
//直行座標→円筒座標
Vector3& Vector3::rec2cyl(Vector3& vec, double th) {
	x = vec.x * cos(th) + vec.y * sin(th);
	y = -vec.x * sin(th) + vec.y * cos(th);
	z = vec.z;
	return *this;
};


