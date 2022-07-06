#pragma once

#include "CVector3.h"
#include "COMMON_DEF.H"
#include "Spec.h"

//Moving Object�N���X
class CMob
{
public:
    CMob();
    CMob(double _dt);
    CMob(double _dt, Vector3& _r, Vector3& _v);
    ~CMob();

    double dt;      //�v�Z���ԊԊu
    Vector3 a;      //�����x�x�N�g��
    Vector3 r;      //�ʒu�x�N�g��
    Vector3 v;      //���x�x�N�g��
    Vector3 r0;     //�ݓ_��ʒu�x�N�g��
    Vector3 v0;     //�ݓ_����x�x�N�g��
    Vector3 fex;    //�O��
    Vector3 dr;     //�ʒu�x�N�g���̕ω���
    Vector3 dv;     //���x�x�N�g���̕ω���
    Vector3 R0;     //��_
     
    //�����x�x�N�g����^���郁�\�b�h�@�@�p����ōĒ�`����
    virtual Vector3 A(Vector3& r, Vector3& v); 
    virtual void set_fex(double,double,double);//�O��
    virtual void set_dt(double);//�v�Z���ԊԊu�Z�b�g


    //���x�x�N�g����^���郁�\�b�h
    virtual Vector3 V(Vector3& r, Vector3& v);
    //���Ԕ��W���v�Z���郁�\�b�h
    virtual void timeEvolution();
    virtual void init_mob(double _dt, Vector3& _r, Vector3& _v) {
        dt = _dt;
        r.copy(_r);
        v.copy(_v);
        return;
    }
 
private:

};

//�N���[���N���X
//r,v�́A�ݓ_�̈ʒu�ƍ��W

#define SIM_INIT_SCAN               0.025       //�Эڰ��ݽ�����я����l�@25ms
#define SIM_INIT_R                  10.0        //���������l m
#define SIM_INIT_TH                 0.0         //���񏉊��l rad
#define SIM_INIT_L                  10.0        //���[�v�������l m
#define SIM_INIT_X                  10.0        //���s�����l m
#define SIM_INIT_M                  10000.0     //�׏d�����l kg

class CCrane : public CMob
{
public:
    CCrane();
    ~CCrane();
    
    double M;                       //�N���[���S�̎��ʁ@Kg
    double l_mh;                    //�����[�v�� m
    Vector3 rc;                     //�N���[�����S�_�̈ʒu�x�N�g��
    Vector3 vc;                     //�N���[�����S�_�̑��x�x�N�g��


    double r0[MOTION_ID_MAX];        //�ʒu�E�p�x
    double v0[MOTION_ID_MAX];        //���x�E�p���x
    double a0[MOTION_ID_MAX];        //�����x�E�p�����x

    double v_ref[MOTION_ID_MAX];     //���x�E�p���x�w��
    double a_ref[MOTION_ID_MAX];     //�����x�E�p�����x�w��

    bool is_fwd_endstop[MOTION_ID_MAX];         //���]�Ɍ�����
    bool is_rev_endstop[MOTION_ID_MAX];         //�t�]�Ɍ�����
 
    double trq_fb[MOTION_ID_MAX];               //���[�^�[�g���NFB
    bool motion_break[MOTION_ID_MAX];           //�u���[�L�J���

    void set_v_ref(double hoist_ref,double gantry_ref,double slew_ref,double boomh_ref);        //���x�w�ߒl����
    void init_crane(); 
    int set_spec(LPST_SPEC _pspec) { pspec = _pspec; return 0; }
    void update_break_status();                 //�u���[�L���, �u���[�L�J���o�ߎ��ԃZ�b�g
    
    void timeEvolution();                       //���Ԕ��W���v�Z���郁�\�b�h

 private:
    double brk_elaped_time[MOTION_ID_MAX];      //�u���[�L�J���o�ߎ���
    double Tf[MOTION_ID_MAX];                   //�����x�ꎞ�x��

    Vector3 A(Vector3& _r, Vector3& _v);        //�ݓ_�����x�v�Z
    void Ac();                                  //�N���[�������x�v�Z

    LPST_SPEC pspec;
    double accdec_cut_spd_range[MOTION_ID_MAX]; //�������w�߂�0�ɂ��鑬�x�w�߂�FB�̍��͈̔�
};

//�v�Z�덷�z�������@�R�����␳�́��␳�΂˒e���́{�␳�S����R��
#define compensationK 0.5       //�R�����␳�e���W��
#define compensationGamma 0.5   //�R�����S���W��

//�݉׃N���X
class CLoad : public CMob
{
public:
    CLoad() { m = 10000.0; pCrane = NULL; };
    ~CLoad() {};

    void init_mob(double t, Vector3& r, Vector3& v);
    void update_relative_vec();         //�ݓ_�Ƃ̑��΃x�N�g���X�V
    Vector3 A(Vector3& r, Vector3& v);  //Model of acceleration
    double S();	//Rope tension

    CCrane * pCrane;
    double m;                   //�݉׎��ʁ@Kg
 
    int set_m(double _m) { m = _m; return(0); }
    int set_crane(CCrane* _pCrane) { pCrane = _pCrane; return(0); }

private:

};




