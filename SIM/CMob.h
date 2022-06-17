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
    Vector3 r;      //�ʒu�x�N�g��
    Vector3 v;      //���x�x�N�g��
    Vector3 fex;    //�O��
    Vector3 dr;     //�ʒu�x�N�g���̕ω���
    Vector3 dv;     //���x�x�N�g���̕ω���
    Vector3 R0;     //��_
     
    //�����x�x�N�g����^���郁�\�b�h�@�@�p����ōĒ�`����
    virtual Vector3 A(Vector3& r, Vector3& v); 
    virtual void set_fex(double,double,double);//�O��

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

class CCrane : public CMob
{
public:
    CCrane();
    ~CCrane();
    
    double M;                       //�N���[���S�̎��ʁ@Kg
    Vector3 rc;                     //�N���[�����S�_�̈ʒu�x�N�g��
    Vector3 vc;                     //�N���[�����S�_�̑��x�x�N�g��


    double r0[MOTION_ID_MAX];        //�ʒu�E�p�x
    double v0[MOTION_ID_MAX];        //���x�E�p���x
    double a0[MOTION_ID_MAX];        //�����x�E�p�����x

    double v_ref[MOTION_ID_MAX];     //���x�E�p���x�w��
 
    double trq_fb[MOTION_ID_MAX];    //���[�^�[�g���NFB
    bool motion_break[MOTION_ID_MAX];//�u���[�L�J���

    void set_v_ref(double hoist_ref,double gantry_ref,double slew_ref,double boomh_ref); //���x�w�ߒl����
    void init_crane(double _dt, Vector3& _r, Vector3& _v,Vector3& r_offset, Vector3& v_offset); //���́F�N���[�����S���@�I�t�Z�b�g�F�N���[�����S�ƒݓ_�Ƃ̑���
    int set_spec(LPST_SPEC _pspec) { pspec = _pspec; return 0; }
    void update_ref_elapsed();     //�e���̎w�ߏo�͌o�ߎ��ԃZ�b�g
    void update_break_status();    //�u���[�L���, �u���[�L�J���o�ߎ��ԃZ�b�g
    
    //���Ԕ��W���v�Z���郁�\�b�h
    void timeEvolution();

private:
    double elaped_time[MOTION_ID_MAX];     //�u���[�L�J���o�ߎ���
    double Tf[MOTION_ID_MAX];              //�����x�ꎞ�x��

    Vector3 A(Vector3& _r, Vector3& _v);    //�ݓ_�����x�v�Z
    void Ac();                              //�N���[�������x�v�Z

    LPST_SPEC pspec;
};

//�v�Z�덷�z�������@�R�����␳�́��␳�΂˒e���́{�␳�S����R��
#define compensationK 0.5       //�R�����␳�e���W��
#define compensationGamma 0.5   //�R�����S���W��

//�݉׃N���X
class CLoad : public CMob
{
public:
    CLoad() {};
    ~CLoad() {};

    double M;                   //�݉׎��ʁ@Kg

private:

};




