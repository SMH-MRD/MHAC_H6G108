#include "CMob.h"

/********************************************************************************/
/*       Moving Object                                                          */
/********************************************************************************/
CMob::CMob() {
	dt = 0.01;
    M = 1.0;
	I = 1.0;
	trq = 0.0;
	r = { 0.0,0.0,0.0 };
	v = { 0.0,0.0,0.0 };
	R0 = { 0.0,0.0,0.0};
}
CMob::CMob(double _dt) {
	dt = _dt;
	M = 1.0;
	I = 1.0;
	trq = 0.0;
	r = { 0.0,0.0,0.0 };
	v = { 0.0,0.0,0.0 };
	R0 = { 0.0,0.0,0.0 };
}
CMob::CMob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	M = 1.0;
	I = 1.0;
	trq = 0.0;
	R0 = { 0.0,0.0,0.0 };
}

CMob::~CMob() {}

//加速度ベクトル　継承先で再定義する
Vector3 CMob::A(double, Vector3& r, Vector3& v) {
	return r.clone();
};
Vector3 CMob::A(double, Vector3& r, Vector3& v, Vector3& trq, Vector3& f) {
	return r.clone();
};

//速度ベクトル
Vector3 CMob::V(double t, Vector3& r, Vector3& v) {
	return v.clone();
}
//ルンゲ・クッタ法による時間発展
void CMob::timeEvolution(double t) {

	Vector3 v1 = V(t, r, v);
	Vector3 a1 = A(t, r, v);

	Vector3 _v1 = Vector3(r.x + v1.x * dt / 2.0, r.y + v1.y * dt / 2.0, r.z + v1.z * dt / 2.0);
	Vector3 _a1 = Vector3(v.x + a1.x * dt / 2.0, v.y + a1.y * dt / 2.0, v.z + a1.z * dt / 2.0);
	Vector3 v2 = V(t + dt / 2.0, _v1, _a1);
	Vector3 a2 = A(t + dt / 2.0, _v1, _a1);

	Vector3 _v2 = Vector3(r.x + v2.x * dt / 2.0, r.y + v2.y * dt / 2.0, r.z + v2.z * dt / 2.0);
	Vector3 _a2 = Vector3(v.x + a2.x * dt / 2.0, v.y + a2.y * dt / 2.0, v.z + a2.z * dt / 2.0);
	Vector3 v3 = V(t + dt / 2.0, _v2, _a2);
	Vector3 a3 = A(t + dt / 2.0, _v2, _a2);

	Vector3 _v3 = Vector3(r.x + v3.x * dt, r.y + v3.y * dt, r.z + v3.z * dt);
	Vector3 _a3 = Vector3(v.x + a3.x * dt, v.y + a3.y * dt, v.z + a3.z * dt);
	Vector3 v4 = V(t + dt, _v3, _a3);
	Vector3 a4 = A(t + dt, _v3, _a3);

	dr.x = dt / 6.0 * (v1.x + 2.0 * v2.x + 2.0 * v3.x + v4.x);
	dr.y = dt / 6.0 * (v1.y + 2.0 * v2.y + 2.0 * v3.y + v4.y);
	dr.z = dt / 6.0 * (v1.z + 2.0 * v2.z + 2.0 * v3.z + v4.z);
	dv.x = dt / 6.0 * (a1.x + 2.0 * a2.x + 2.0 * a3.x + a4.x);
	dv.y = dt / 6.0 * (a1.y + 2.0 * a2.y + 2.0 * a3.y + a4.y);
	dv.z = dt / 6.0 * (a1.z + 2.0 * a2.z + 2.0 * a3.z + a4.z);

}

/********************************************************************************/
/*       Crane Object                                                          */
/********************************************************************************/
// ﾄﾙｸT(N・m）= F x R　= J x dω/dt  仕事率P=Tω=Mav　a=Tω/Mv=MT/r

Vector3 CCrane::A(double _dt, Vector3& _r, Vector3& _v, Vector3& trqf, Vector3& _f) {
	return _r.clone();
};