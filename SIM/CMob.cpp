#include "CMob.h"
#include "SIMparaDEF.h"

extern ST_SPEC def_spec;

/********************************************************************************/
/*       Moving Object                                                          */
/********************************************************************************/
CMob::CMob() {
	dt = 0.01;
 	r = { 0.0,0.0,0.0 };
	v = { 0.0,0.0,0.0 };
	R0 = { 0.0,0.0,0.0};
}
CMob::CMob(double _dt) {
	dt = _dt;
	r = { 0.0,0.0,0.0 };
	v = { 0.0,0.0,0.0 };
	R0 = { 0.0,0.0,0.0 };
}
CMob::CMob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	R0 = { 0.0,0.0,0.0 };
}

CMob::~CMob() {}

//加速度ベクトル　継承先で再定義する

Vector3 CMob::A(Vector3& r, Vector3& v) {
	return r.clone();
};

void CMob::set_fex(double fx,double fy,double fz) {
	fex.x = fx;
	fex.y = fy;
	fex.z = fz;
	return;
};

void CMob::set_dt(double _dt) {
	dt = _dt;
	return;
};

//速度ベクトル
Vector3 CMob::V(Vector3& r, Vector3& v) {
	return v.clone();
}
//オイラー方による時間発展
void CMob::timeEvolution() {
/*
	Vector3 v1 = V(r, v);
	Vector3 a1 = A(r, v);

	Vector3 _v1 = Vector3(r.x + v1.x * dt / 2.0, r.y + v1.y * dt / 2.0, r.z + v1.z * dt / 2.0);
	Vector3 _a1 = Vector3(v.x + a1.x * dt / 2.0, v.y + a1.y * dt / 2.0, v.z + a1.z * dt / 2.0);
	Vector3 v2 = V(_v1, _a1);
	Vector3 a2 = A(_v1, _a1);

	Vector3 _v2 = Vector3(r.x + v2.x * dt / 2.0, r.y + v2.y * dt / 2.0, r.z + v2.z * dt / 2.0);
	Vector3 _a2 = Vector3(v.x + a2.x * dt / 2.0, v.y + a2.y * dt / 2.0, v.z + a2.z * dt / 2.0);
	Vector3 v3 = V(_v2, _a2);
	Vector3 a3 = A(_v2, _a2);

	Vector3 _v3 = Vector3(r.x + v3.x * dt, r.y + v3.y * dt, r.z + v3.z * dt);
	Vector3 _a3 = Vector3(v.x + a3.x * dt, v.y + a3.y * dt, v.z + a3.z * dt);
	Vector3 v4 = V(_v3, _a3);
	Vector3 a4 = A(_v3, _a3);

	dr.x = dt / 6.0 * (v1.x + 2.0 * v2.x + 2.0 * v3.x + v4.x);
	dr.y = dt / 6.0 * (v1.y + 2.0 * v2.y + 2.0 * v3.y + v4.y);
	dr.z = dt / 6.0 * (v1.z + 2.0 * v2.z + 2.0 * v3.z + v4.z);
	dv.x = dt / 6.0 * (a1.x + 2.0 * a2.x + 2.0 * a3.x + a4.x);
	dv.y = dt / 6.0 * (a1.y + 2.0 * a2.y + 2.0 * a3.y + a4.y);
	dv.z = dt / 6.0 * (a1.z + 2.0 * a2.z + 2.0 * a3.z + a4.z);
	*/

	Vector3 v1 = V(r, v);
	Vector3 a1 = A(r, v);

	Vector3 _v1 = Vector3(r.x + v1.x * dt / 2.0, r.y + v1.y * dt / 2.0, r.z + v1.z * dt / 2.0);
	Vector3 _a1 = Vector3(v.x + a1.x * dt / 2.0, v.y + a1.y * dt / 2.0, v.z + a1.z * dt / 2.0);
	Vector3 v2 = V(_v1, _a1);
	Vector3 a2 = A(_v1, _a1);

	Vector3 _v2 = Vector3(r.x + v2.x * dt / 2.0, r.y + v2.y * dt / 2.0, r.z + v2.z * dt / 2.0);
	Vector3 _a2 = Vector3(v.x + a2.x * dt / 2.0, v.y + a2.y * dt / 2.0, v.z + a2.z * dt / 2.0);
	Vector3 v3 = V(_v2, _a2);
	Vector3 a3 = A(_v2, _a2);

	Vector3 _v3 = Vector3(r.x + v3.x * dt, r.y + v3.y * dt, r.z + v3.z * dt);
	Vector3 _a3 = Vector3(v.x + a3.x * dt, v.y + a3.y * dt, v.z + a3.z * dt);
	Vector3 v4 = V(_v3, _a3);
	Vector3 a4 = A(_v3, _a3);

	dr.x = dt * v.x;
	dr.y = dt * v.y;
	dr.z = dt * v.z;
	dv.x = dt / 6.0 * (a1.x + 2.0 * a2.x + 2.0 * a3.x + a4.x);
	dv.y = dt / 6.0 * (a1.y + 2.0 * a2.y + 2.0 * a3.y + a4.y);
	dv.z = dt / 6.0 * (a1.z + 2.0 * a2.z + 2.0 * a3.z + a4.z);
}

/********************************************************************************/
/*       Crane Object                                                          */
/********************************************************************************/
CCrane::CCrane() { 
	pspec = &def_spec;
	accdec_cut_spd_range[ID_HOIST] = 0.01;
	accdec_cut_spd_range[ID_BOOM_H] = 0.01;
	accdec_cut_spd_range[ID_SLEW] = 0.001;
	accdec_cut_spd_range[ID_GANTRY] = 0.01;
}
CCrane::~CCrane() {}

void CCrane::set_v_ref(double hoist_ref, double gantry_ref, double slew_ref, double boomh_ref) {
	v_ref[ID_HOIST] = hoist_ref;
	v_ref[ID_BOOM_H] = boomh_ref;
	v_ref[ID_SLEW] = slew_ref;
	v_ref[ID_GANTRY] = gantry_ref;
	return;
}

// ﾄﾙｸT(N・m）= F x R　= J x dω/dt  仕事率P=Tω=Mav　a=Tω/Mv=MT/r
Vector3 CCrane::A(Vector3& _r, Vector3& _v) {

	Vector3 a;
	double r_bm = r0[ID_BOOM_H];//旋回半径
	//等角加速度運動　a = r x dω/dt(-sinθ,cosθ) -rxω^2(cosθ,sinθ)
	a.x = v0[ID_BOOM_H] * sin(r0[ID_SLEW]) - r_bm * a0[ID_SLEW] * sin(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * v0[ID_SLEW] * cos(r0[ID_SLEW]) + a0[ID_GANTRY];
	a.y = v0[ID_BOOM_H] * cos(r0[ID_SLEW]) + r_bm * a0[ID_SLEW] * cos(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * v0[ID_SLEW] * sin(r0[ID_SLEW]);
	a.z = 0.0;

	return a;
}


#define REF_CUT_BREAK_CLOSE_RETIO 0.5	//ブレーキを閉じる判定係数　１ノッチ速度との比率

void CCrane::Ac() {

	//加速指令計算
	if(!motion_break[ID_HOIST]) a_ref[ID_HOIST] = 0.0;
	else if ((v_ref[ID_HOIST] - v0[ID_HOIST])>accdec_cut_spd_range[ID_HOIST]) {
		if (v_ref[ID_HOIST] > 0.0) a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][FWD][ACC];//正転加速
		else a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][REV][DEC];//逆転減速
	}
	else if ((v_ref[ID_HOIST] - v0[ID_HOIST]) < -accdec_cut_spd_range[ID_HOIST]) {
		if (v_ref[ID_HOIST] > 0.0) a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][FWD][DEC];//正転減速
		else a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][REV][ACC];//逆転加速
	}
	else{
		a_ref[ID_HOIST] = 0.0;
	}

	if (!motion_break[ID_GANTRY]) a_ref[ID_GANTRY] = 0.0;
	else if ((v_ref[ID_GANTRY] - v0[ID_GANTRY]) > accdec_cut_spd_range[ID_GANTRY]) {
		if (v_ref[ID_GANTRY] > 0.0) a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][FWD][ACC];//正転加速
		else a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][REV][DEC];//逆転減速
	}
	else if ((v_ref[ID_GANTRY] - v0[ID_GANTRY]) < -accdec_cut_spd_range[ID_GANTRY]) {
		if (v_ref[ID_GANTRY] > 0.0) a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][FWD][DEC];//正転減速
		else a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][REV][ACC];//逆転加速
	}
	else {
		a_ref[ID_GANTRY] = 0.0;
	}

	if (!motion_break[ID_BOOM_H]) a_ref[ID_BOOM_H] = 0.0;
	else if ((v_ref[ID_BOOM_H] - v0[ID_BOOM_H]) > accdec_cut_spd_range[ID_BOOM_H]) {
		if (v_ref[ID_BOOM_H] > 0.0) a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][FWD][ACC];//正転加速
		else a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][REV][DEC];//逆転減速
	}
	else if ((v_ref[ID_BOOM_H] - v0[ID_BOOM_H]) < -accdec_cut_spd_range[ID_BOOM_H]) {
		if (v_ref[ID_BOOM_H] > 0.0) a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][FWD][DEC];//正転減速
		else a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][REV][ACC];//逆転加速
	}
	else {
		a_ref[ID_BOOM_H] = 0.0;
	}
	
	if (!motion_break[ID_SLEW]) a_ref[ID_SLEW] = 0.0;
	else if ((v_ref[ID_SLEW] - v0[ID_SLEW]) > accdec_cut_spd_range[ID_SLEW]) {
		if (v_ref[ID_SLEW] > 0.0) a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][FWD][ACC];//正転加速
		else a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][REV][DEC];//逆転減速
	}
	else if ((v_ref[ID_SLEW] - v0[ID_SLEW]) < -accdec_cut_spd_range[ID_SLEW]) {
		if (v_ref[ID_SLEW] > 0.0) a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][FWD][DEC];//正転減速
		else a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][REV][ACC];//逆転加速
	}
	else {
		a_ref[ID_SLEW] = 0.0;
	}
	
	//加速度計算　当面指令に対して一次遅れフィルタを入れる形で計算（将来的にトルク指令からの導出検討）
	if (motion_break[ID_HOIST]) {
		a0[ID_HOIST] = (dt * a_ref[ID_HOIST] + Tf[ID_HOIST] * a0[ID_HOIST]) / (dt + Tf[ID_HOIST]);
	}
	else {
		a0[ID_HOIST] = 0.0;
	}

	if (motion_break[ID_BOOM_H]) {
		a0[ID_BOOM_H] = (dt * a_ref[ID_BOOM_H] + Tf[ID_BOOM_H] * a0[ID_BOOM_H]) / (dt + Tf[ID_BOOM_H]);
	}
	else {
		a0[ID_BOOM_H] = 0.0;
	}

	if (motion_break[ID_SLEW]) {
		a0[ID_SLEW] = (dt * a_ref[ID_SLEW] + Tf[ID_SLEW] * a0[ID_SLEW]) / (dt + Tf[ID_SLEW]);
	}
	else {
		a0[ID_SLEW] = 0.0;
	}

	if (motion_break[ID_GANTRY]) {
		a0[ID_GANTRY] = (dt * a_ref[ID_GANTRY] + Tf[ID_GANTRY] * a0[ID_GANTRY]) / (dt + Tf[ID_GANTRY]);
	}
	else {
		a0[ID_GANTRY] = 0.0;
	}

	return;
}

void CCrane::timeEvolution() {
	//クレーン部
	//加速度計算
	Ac();	
	//速度計算(オイラー法）
	v0[ID_HOIST]	+= a0[ID_HOIST] * dt;	if (!motion_break[ID_HOIST]) v0[ID_HOIST] = 0.0;
	v0[ID_GANTRY]	+= a0[ID_GANTRY] * dt;	if (!motion_break[ID_GANTRY]) v0[ID_GANTRY] = 0.0;
	v0[ID_SLEW]		+= a0[ID_SLEW] * dt;	if (!motion_break[ID_SLEW]) v0[ID_SLEW] = 0.0;
	v0[ID_BOOM_H]	+= a0[ID_BOOM_H] * dt;	if (!motion_break[ID_BOOM_H]) v0[ID_BOOM_H] = 0.0;

	//位置計算(オイラー法）
	r0[ID_HOIST]	+= v0[ID_HOIST] * dt;	
	r0[ID_GANTRY]	+= v0[ID_GANTRY] * dt;
	r0[ID_BOOM_H]	+= v0[ID_BOOM_H] * dt;
	r0[ID_SLEW] += v0[ID_SLEW] * dt; if (r0[ID_SLEW] >= PI360)r0[ID_SLEW] -= PI360; if (r0[ID_SLEW] <= -PI360)r0[ID_SLEW] += PI360;

	vc.x = v0[ID_GANTRY]; vc.y = 0.0; vc.z = 0.0;
	rc.x = v0[ID_GANTRY]; vc.y = R0.y; vc.z = R0.z;

	//吊点部
	double r_bm = r0[ID_BOOM_H];//旋回半径
	v.x = v0[ID_BOOM_H] * cos(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * sin(r0[ID_SLEW]) + v0[ID_GANTRY];
	v.y = v0[ID_BOOM_H] * sin(r0[ID_SLEW]) + r_bm * v0[ID_SLEW] * cos(r0[ID_SLEW]);
	v.z = 0.0;

	r.x = r_bm * cos(r0[ID_SLEW]) + r0[ID_GANTRY];
	r.y = r_bm * sin(r0[ID_SLEW]);
	r.z = pspec->boom_high;
	return;
}

void CCrane::init_crane(double _dt, Vector3& _r, Vector3& _v, Vector3& r_offset, Vector3& v_offset) {

	rc = _r + r_offset;
	vc = _v + v_offset;
	init_mob(_dt, _r, _v);

	set_v_ref(0.0, 0.0, 0.0, 0.0);
	set_fex(0.0, 0.0, 0.0);

	
	//加速度一次遅れフィルタ時定数
	Tf[ID_HOIST] = SIM_TF_HOIST;
	Tf[ID_BOOM_H] = SIM_TF_BOOM_H;
	Tf[ID_SLEW] = SIM_TF_SLEW;
	Tf[ID_GANTRY] = SIM_TF_GANTRY;
}

#define BREAK_CLOSE_RETIO 0.5	//ブレーキを閉じる判定係数　１ノッチ速度との比率

// 各モーションのブレーキ状態をセット
void CCrane::update_break_status() {

	if (v_ref[ID_HOIST] != 0.0) {
		motion_break[ID_HOIST] = true;
	}
	else if ((v0[ID_HOIST] > pspec->notch_spd_r[ID_HOIST][1] * BREAK_CLOSE_RETIO)
		&& (v0[ID_HOIST] < pspec->notch_spd_f[ID_HOIST][1] * BREAK_CLOSE_RETIO)) {
		motion_break[ID_HOIST] = false;
	}
	else;

	if (v_ref[ID_GANTRY] != 0.0) {
		motion_break[ID_GANTRY] = true;
	}
	else if ((v0[ID_GANTRY] > pspec->notch_spd_r[ID_GANTRY][1] * BREAK_CLOSE_RETIO)
		&& (v0[ID_GANTRY] < pspec->notch_spd_f[ID_GANTRY][1] * BREAK_CLOSE_RETIO)) {
		motion_break[ID_GANTRY] = false;
	}
	else;

	if (v_ref[ID_SLEW] != 0.0) {
		motion_break[ID_SLEW] = true;
	}
	else if ((v0[ID_SLEW] > pspec->notch_spd_r[ID_SLEW][1] * BREAK_CLOSE_RETIO)
		&& (v0[ID_SLEW] < pspec->notch_spd_f[ID_SLEW][1] * BREAK_CLOSE_RETIO)) {
		motion_break[ID_SLEW] = false;
	}
	else;

	if (v_ref[ID_BOOM_H] != 0.0) {
		motion_break[ID_BOOM_H] = true;
	}
	else if ((v0[ID_BOOM_H] > pspec->notch_spd_r[ID_BOOM_H][1] * BREAK_CLOSE_RETIO)
		&& (v0[ID_BOOM_H] < pspec->notch_spd_f[ID_BOOM_H][1] * BREAK_CLOSE_RETIO)) {
		motion_break[ID_BOOM_H] = false;
	}
	else;


	if (motion_break[ID_HOIST]) {
		brk_elaped_time[ID_HOIST] += dt;
	}
	else {
		brk_elaped_time[ID_HOIST] = 0.0;
	}
	if (motion_break[ID_BOOM_H]) {
		brk_elaped_time[ID_BOOM_H] += dt;
	}
	else {
		brk_elaped_time[ID_BOOM_H] = 0.0;
	}
	if (motion_break[ID_SLEW]) {
		brk_elaped_time[ID_SLEW] += dt;
	}
	else {
		brk_elaped_time[ID_SLEW] = 0.0;
	}
	if (motion_break[ID_GANTRY]) {
		brk_elaped_time[ID_GANTRY] += dt;
	}
	else {
		brk_elaped_time[ID_GANTRY] = 0.0;
	}


	return;
}

