#include "CVector3.h"
#include "CRK4.h"

//##### RK4
//加速度ベクトル　継承先で再定義する
Vector3 RK4::A(double, Vector3&, Vector3&) {
	return r.clone();
};
//速度ベクトル
Vector3 RK4::V(double t, Vector3& r, Vector3& v) {
	return v.clone();
}
//ルンゲ・クッタ法による時間発展
void RK4::timeEvolution(double t) {

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

//##### RK4_NBODY
//加速度ベクトル　継承先で再定義する
void RK4_Nbody::A(double t, Vector3* rs, Vector3* vs, Vector3* out_vs) {
	for (int i = 0; i < N; i++) {
		out_vs[i] = vs[i];
	}
}
//速度ベクトル
void RK4_Nbody::V(double t, Vector3* rs, Vector3* vs, Vector3* out_vs) {
	for (int i = 0; i < N; i++) {
		out_vs[i] = vs[i];
	}
}
//ルンゲ・クッタ法による時間発展
void RK4_Nbody::timeEvolution(double t) {
	//１段目
	V(t, rs, vs, v1s);
	A(t, rs, vs, a1s);
	for (int i = 0; i < N; i++) {
		_v1s[i].set(rs[i].x + v1s[i].x * dt / 2.0, rs[i].y + v1s[i].y * dt / 2.0, rs[i].z + v1s[i].z * dt / 2.0);
		_a1s[i].set(vs[i].x + a1s[i].x * dt / 2.0, vs[i].y + a1s[i].y * dt / 2.0, vs[i].z + a1s[i].z * dt / 2.0);
	}
	//２段目
	V(t + dt / 2.0, _v1s, _a1s, v2s);
	A(t + dt / 2.0, _v1s, _a1s, a2s);
	for (int i = 0; i < N; i++) {
		_v2s[i].set(rs[i].x + v2s[i].x * dt / 2.0, rs[i].y + v2s[i].y * dt / 2.0, rs[i].z + v2s[i].z * dt / 2.0);
		_a2s[i].set(vs[i].x + a2s[i].x * dt / 2.0, vs[i].y + a2s[i].y * dt / 2.0, vs[i].z + a2s[i].z * dt / 2.0);
	}
	//３段目
	V(t + dt / 2.0, _v2s, _a2s, v3s);
	A(t + dt / 2.0, _v2s, _a2s, a3s);
	for (int i = 0; i < N; i++) {
		_v3s[i].set(rs[i].x + v3s[i].x * dt, rs[i].y + v3s[i].y * dt, rs[i].z + v3s[i].z * dt);
		_a3s[i].set(vs[i].x + a3s[i].x * dt, vs[i].y + a3s[i].y * dt, vs[i].z + a3s[i].z * dt);
	}
	//４段目
	V(t + dt, _v3s, _a3s, v4s);
	A(t + dt, _v3s, _a3s, a4s);
	for (int i = 0; i < N; i++) {
		drs[i].x = dt / 6.0 * (v1s[i].x + 2.0 * v2s[i].x + 2.0 * v3s[i].x + v4s[i].x);
		drs[i].y = dt / 6.0 * (v1s[i].y + 2.0 * v2s[i].y + 2.0 * v3s[i].y + v4s[i].y);
		drs[i].z = dt / 6.0 * (v1s[i].z + 2.0 * v2s[i].z + 2.0 * v3s[i].z + v4s[i].z);
		dvs[i].x = dt / 6.0 * (a1s[i].x + 2.0 * a2s[i].x + 2.0 * a3s[i].x + a4s[i].x);
		dvs[i].y = dt / 6.0 * (a1s[i].y + 2.0 * a2s[i].y + 2.0 * a3s[i].y + a4s[i].y);
		dvs[i].z = dt / 6.0 * (a1s[i].z + 2.0 * a2s[i].z + 2.0 * a3s[i].z + a4s[i].z);
	}

}
