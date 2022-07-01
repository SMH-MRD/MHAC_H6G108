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

//â¡ë¨ìxÉxÉNÉgÉãÅ@åpè≥êÊÇ≈çƒíËã`Ç∑ÇÈ

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

//ë¨ìxÉxÉNÉgÉã
Vector3 CMob::V(Vector3& r, Vector3& v) {
	return v.clone();
}
//ÉIÉCÉâÅ[ï˚Ç…ÇÊÇÈéûä‘î≠ìW
void CMob::timeEvolution() {

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
	M = 1000.0;
	for (int i = 0; i < MOTION_ID_MAX;i++) {
		is_fwd_endstop[i] = false;
		is_rev_endstop[i] = false;
	}
	r0[ID_GANTRY] = pspec->gantry_pos_min;
	r0[ID_HOIST] = 0.0;
	r0[ID_BOOM_H] = pspec->boom_pos_min;
	r0[ID_SLEW] = 0.0;
}
CCrane::~CCrane() {}

void CCrane::set_v_ref(double hoist_ref, double gantry_ref, double slew_ref, double boomh_ref) {
	v_ref[ID_HOIST] = hoist_ref;
	v_ref[ID_BOOM_H] = boomh_ref;
	v_ref[ID_SLEW] = slew_ref;
	v_ref[ID_GANTRY] = gantry_ref;
	return;
}

// ƒŸ∏T(NÅEmÅj= F x RÅ@= J x dÉ÷/dt  édéñó¶P=TÉ÷=MavÅ@a=TÉ÷/Mv=MT/r
Vector3 CCrane::A(Vector3& _r, Vector3& _v) {

	Vector3 a;
	double r_bm = r0[ID_BOOM_H];//ê˘âÒîºåa
	//ìôäpâ¡ë¨ìxâ^ìÆÅ@a = r x dÉ÷/dt(-sinÉ∆,cosÉ∆) -rxÉ÷^2(cosÉ∆,sinÉ∆)
	a.x = v0[ID_BOOM_H] * sin(r0[ID_SLEW]) - r_bm * a0[ID_SLEW] * sin(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * v0[ID_SLEW] * cos(r0[ID_SLEW]) + a0[ID_GANTRY];
	a.y = v0[ID_BOOM_H] * cos(r0[ID_SLEW]) + r_bm * a0[ID_SLEW] * cos(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * v0[ID_SLEW] * sin(r0[ID_SLEW]);
	a.z = 0.0;

	return a;
}


#define REF_CUT_BREAK_CLOSE_RETIO 0.5	//ÉuÉåÅ[ÉLÇï¬Ç∂ÇÈîªíËåWêîÅ@ÇPÉmÉbÉ`ë¨ìxÇ∆ÇÃî‰ó¶

void CCrane::Ac() {

	//â¡ë¨éwóﬂåvéZ
	if(!motion_break[ID_HOIST]) a_ref[ID_HOIST] = 0.0;
	else if ((v_ref[ID_HOIST] - v0[ID_HOIST])>accdec_cut_spd_range[ID_HOIST]) {
		if (v_ref[ID_HOIST] > 0.0) a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][FWD][ACC];//ê≥ì]â¡ë¨
		else a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][REV][DEC];//ãtì]å∏ë¨
	}
	else if ((v_ref[ID_HOIST] - v0[ID_HOIST]) < -accdec_cut_spd_range[ID_HOIST]) {
		if (v_ref[ID_HOIST] > 0.0) a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][FWD][DEC];//ê≥ì]å∏ë¨
		else a_ref[ID_HOIST] = pspec->accdec[ID_HOIST][REV][ACC];//ãtì]â¡ë¨
	}
	else{
		a_ref[ID_HOIST] = 0.0;
	}
	//ã…å¿í‚é~
	if((a_ref[ID_HOIST] > 0.0) && (is_fwd_endstop[ID_HOIST])) a_ref[ID_HOIST] = 0.0;
	if ((a_ref[ID_HOIST] < 0.0) && (is_rev_endstop[ID_HOIST])) a_ref[ID_HOIST] = 0.0;


	if (!motion_break[ID_GANTRY]) a_ref[ID_GANTRY] = 0.0;
	else if ((v_ref[ID_GANTRY] - v0[ID_GANTRY]) > accdec_cut_spd_range[ID_GANTRY]) {
		if (v_ref[ID_GANTRY] > 0.0) a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][FWD][ACC];//ê≥ì]â¡ë¨
		else a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][REV][DEC];//ãtì]å∏ë¨
	}
	else if ((v_ref[ID_GANTRY] - v0[ID_GANTRY]) < -accdec_cut_spd_range[ID_GANTRY]) {
		if (v_ref[ID_GANTRY] > 0.0) a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][FWD][DEC];//ê≥ì]å∏ë¨
		else a_ref[ID_GANTRY] = pspec->accdec[ID_GANTRY][REV][ACC];//ãtì]â¡ë¨
	}
	else {
		a_ref[ID_GANTRY] = 0.0;
	}

	//ã…å¿í‚é~
	if ((a_ref[ID_GANTRY] > 0.0) && (is_fwd_endstop[ID_GANTRY])) a_ref[ID_GANTRY] = 0.0;
	if ((a_ref[ID_GANTRY] < 0.0) && (is_rev_endstop[ID_GANTRY])) a_ref[ID_GANTRY] = 0.0;



	if (!motion_break[ID_BOOM_H]) a_ref[ID_BOOM_H] = 0.0;
	else if ((v_ref[ID_BOOM_H] - v0[ID_BOOM_H]) > accdec_cut_spd_range[ID_BOOM_H]) {
		if (v_ref[ID_BOOM_H] > 0.0) a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][FWD][ACC];//ê≥ì]â¡ë¨
		else a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][REV][DEC];//ãtì]å∏ë¨
	}
	else if ((v_ref[ID_BOOM_H] - v0[ID_BOOM_H]) < -accdec_cut_spd_range[ID_BOOM_H]) {
		if (v_ref[ID_BOOM_H] > 0.0) a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][FWD][DEC];//ê≥ì]å∏ë¨
		else a_ref[ID_BOOM_H] = pspec->accdec[ID_BOOM_H][REV][ACC];//ãtì]â¡ë¨
	}
	else {
		a_ref[ID_BOOM_H] = 0.0;
	}

	//ã…å¿í‚é~
	if ((a_ref[ID_BOOM_H] > 0.0) && (is_fwd_endstop[ID_BOOM_H])) a_ref[ID_BOOM_H] = 0.0;
	if ((a_ref[ID_BOOM_H] < 0.0) && (is_rev_endstop[ID_BOOM_H])) a_ref[ID_BOOM_H] = 0.0;
	
	if (!motion_break[ID_SLEW]) a_ref[ID_SLEW] = 0.0;
	else if ((v_ref[ID_SLEW] - v0[ID_SLEW]) > accdec_cut_spd_range[ID_SLEW]) {
		if (v_ref[ID_SLEW] > 0.0) a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][FWD][ACC];//ê≥ì]â¡ë¨
		else a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][REV][DEC];//ãtì]å∏ë¨
	}
	else if ((v_ref[ID_SLEW] - v0[ID_SLEW]) < -accdec_cut_spd_range[ID_SLEW]) {
		if (v_ref[ID_SLEW] > 0.0) a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][FWD][DEC];//ê≥ì]å∏ë¨
		else a_ref[ID_SLEW] = pspec->accdec[ID_SLEW][REV][ACC];//ãtì]â¡ë¨
	}
	else {
		a_ref[ID_SLEW] = 0.0;
	}
	
	//â¡ë¨ìxåvéZÅ@ìññ éwóﬂÇ…ëŒÇµÇƒàÍéüíxÇÍÉtÉBÉãÉ^Çì¸ÇÍÇÈå`Ç≈åvéZÅiè´óàìIÇ…ÉgÉãÉNéwóﬂÇ©ÇÁÇÃì±èoåüì¢Åj
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


	//í›ì_â¡ë¨ìxÉxÉNÉgÉã
	double a_er = a0[ID_BOOM_H] - r0[ID_BOOM_H] * v0[ID_SLEW] * v0[ID_SLEW];		//à¯çûï˚å¸â¡ë¨ìxÅ@à¯çûâ¡ë¨ìxÅ{ê˘âÒï™
	double a_eth = r0[ID_BOOM_H] * a0[ID_SLEW] + 2.0 * v0[ID_BOOM_H] * v0[ID_SLEW];	//ê˘âÒï˚å¸â¡ë¨ìx
	double a_z = 0.0;


	a.x = a0[ID_GANTRY] + a_er * cos(r0[ID_SLEW]) - a_eth * sin(r0[ID_SLEW]);
	a.y = a_er * sin(r0[ID_SLEW]) + a_eth * cos(r0[ID_SLEW]);
	a.z = a_z;

	return;
}

void CCrane::timeEvolution() {
	//ÉNÉåÅ[Éìïî
	//â¡ë¨ìxåvéZ
	Ac();	
	//ë¨ìxåvéZ(ÉIÉCÉâÅ[ñ@Åj
	v0[ID_HOIST]	+= a0[ID_HOIST] * dt;	if (!motion_break[ID_HOIST]) v0[ID_HOIST] = 0.0;
	v0[ID_GANTRY]	+= a0[ID_GANTRY] * dt;	if (!motion_break[ID_GANTRY]) v0[ID_GANTRY] = 0.0;
	v0[ID_SLEW]		+= a0[ID_SLEW] * dt;	if (!motion_break[ID_SLEW]) v0[ID_SLEW] = 0.0;
	v0[ID_BOOM_H]	+= a0[ID_BOOM_H] * dt;	if (!motion_break[ID_BOOM_H]) v0[ID_BOOM_H] = 0.0;

	//ã…å¿í‚é~
	if (((v0[ID_HOIST] > 0.0) && (is_fwd_endstop[ID_HOIST])) || (v0[ID_HOIST] < 0.0) && (is_rev_endstop[ID_HOIST])) v0[ID_HOIST] = 0.0;
	if (((v0[ID_GANTRY] > 0.0) && (is_fwd_endstop[ID_GANTRY])) || (v0[ID_GANTRY] < 0.0) && (is_rev_endstop[ID_GANTRY])) v0[ID_GANTRY] = 0.0;
	if (((v0[ID_BOOM_H] > 0.0) && (is_fwd_endstop[ID_BOOM_H])) || (v0[ID_BOOM_H] < 0.0) && (is_rev_endstop[ID_BOOM_H])) v0[ID_BOOM_H] = 0.0;


	//à íuåvéZ(ÉIÉCÉâÅ[ñ@Åj
	r0[ID_HOIST]	+= v0[ID_HOIST] * dt;	
	r0[ID_GANTRY]	+= v0[ID_GANTRY] * dt;
	r0[ID_BOOM_H]	+= v0[ID_BOOM_H] * dt;
	r0[ID_SLEW] += v0[ID_SLEW] * dt; if (r0[ID_SLEW] >= PI360)r0[ID_SLEW] -= PI360; if (r0[ID_SLEW] <= -PI360)r0[ID_SLEW] += PI360;

	vc.x = v0[ID_GANTRY]; vc.y = 0.0; vc.z = 0.0;
	rc.x = r0[ID_GANTRY]; rc.y = R0.y; rc.z = R0.z;

	//í›ì_ïî
	double r_bm = r0[ID_BOOM_H];//ê˘âÒîºåa
	v.x = v0[ID_BOOM_H] * cos(r0[ID_SLEW]) - r_bm * v0[ID_SLEW] * sin(r0[ID_SLEW]) + v0[ID_GANTRY];
	v.y = v0[ID_BOOM_H] * sin(r0[ID_SLEW]) + r_bm * v0[ID_SLEW] * cos(r0[ID_SLEW]);
	v.z = 0.0;

	r.x = r_bm * cos(r0[ID_SLEW]) + r0[ID_GANTRY];
	r.y = r_bm * sin(r0[ID_SLEW]);
	r.z = pspec->boom_high;
	l_mh = pspec->boom_high - r0[ID_HOIST];	//ÉçÅ[Éví∑
	return;
}

void CCrane::init_crane(double _dt, Vector3& _r, Vector3& _v, Vector3& r_offset, Vector3& v_offset) {

	rc = _r + r_offset;
	vc = _v + v_offset;
	init_mob(_dt, _r, _v);

	set_v_ref(0.0, 0.0, 0.0, 0.0);
	set_fex(0.0, 0.0, 0.0);

	
	//â¡ë¨ìxàÍéüíxÇÍÉtÉBÉãÉ^éûíËêî
	Tf[ID_HOIST] = SIM_TF_HOIST;
	Tf[ID_BOOM_H] = SIM_TF_BOOM_H;
	Tf[ID_SLEW] = SIM_TF_SLEW;
	Tf[ID_GANTRY] = SIM_TF_GANTRY;
}

#define BREAK_CLOSE_RETIO 0.5	//ÉuÉåÅ[ÉLÇï¬Ç∂ÇÈîªíËåWêîÅ@ÇPÉmÉbÉ`ë¨ìxÇ∆ÇÃî‰ó¶

// äeÉÇÅ[ÉVÉáÉìÇÃÉuÉåÅ[ÉLèÛë‘ÇÉZÉbÉg
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

/********************************************************************************/
/*      Load Object(í›â◊Åj                                                      */
/********************************************************************************/

void CLoad ::init_mob(double _dt, Vector3& _r, Vector3& _v) {
	dt = _dt;
	r.copy(_r);
	v.copy(_v);
	m = 10000.0;//10 ton
	//m = 40000.0;//40 ton
	return;
}

Vector3 CLoad::A(double t, Vector3& r, Vector3& v) {
	Vector3 a;
	Vector3 L_;

	L_ = L_.subVectors(r, pCrane->r);

	double Sdivm = S() / m;

	a = L_.clone().multiplyScalor(Sdivm);
	a.z -= GA;

	//åvéZåÎç∑Ç…ÇÊÇÈÉçÅ[Éví∑Ç∏ÇÍï‚ê≥
	Vector3 hatL = L_.clone().normalize();
	// ï‚ê≥ÇŒÇÀíeê´óÕ
	Vector3 ak = hatL.clone().multiplyScalor(-compensationK * (pCrane->l_mh - L_.length()));
	Vector3 v_ = v_.subVectors(v, pCrane->v);
	// ï‚ê≥îSê´íÔçRóÕ
	Vector3 agamma = hatL.clone().multiplyScalor(-compensationGamma * v_.dot(hatL));
	// í£óÕÇ…Ç–Ç‡ÇÃí∑Ç≥ÇÃï‚ê≥óÕÇâ¡Ç¶ÇÈ

	a.add(ak).add(agamma);

	return a;
} //Model of acceleration

double  CLoad::S() { //AÇÃåvéZïîÇÃä÷åWÇ≈S/LÇ∆Ç»Ç¡ÇƒÇ¢ÇÈÅBä™Ç´ÇÃâ¡ë¨ìxï™Ç™í«â¡Ç≥ÇÍÇƒÇ¢ÇÈÅB
	Vector3 v_ = v.clone().sub(pCrane->v);
	double v_abs2 = v_.lengthSq();
	Vector3 vectmp;
	Vector3 vecL = vectmp.subVectors(r, pCrane->r);

	return -m * (v_abs2 - pCrane->a.dot(vecL) - GA * vecL.z - (pCrane->a0[ID_HOIST] * pCrane->l_mh + pCrane->v0[ID_HOIST] * pCrane->v0[ID_HOIST])) / (pCrane->l_mh * pCrane->l_mh);

}
