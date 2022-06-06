#pragma once

#include "CVector3.h"

//Moving Object�N���X
class CMob
{
public:
    CMob();
    CMob(double _dt);
    CMob(double _dt, Vector3& _r, Vector3& _v);
    ~CMob();

    double dt;   //���ԊԊu
    Vector3 r;          //�ʒu�x�N�g��
    Vector3 v;          //���x�x�N�g��
    Vector3 dr;         //�ʒu�x�N�g���̕ω���
    Vector3 dv;         //���x�x�N�g���̕ω���

    double M;       //���ʁ@Kg
    double I;       //�������[�����g�@Kg
    Vector3 R0;     //��_
    double trq;     // �o�̓g���NNm
     
    
    //�����x�x�N�g����^���郁�\�b�h�@�@�p����ōĒ�`����
    virtual Vector3 A(double t, Vector3& r, Vector3& v);
    virtual Vector3 A(double t, Vector3& r, Vector3& v, Vector3& trqref, Vector3& f); //�O�͂���

    //���x�x�N�g����^���郁�\�b�h
    virtual Vector3 V(double t, Vector3& r, Vector3& v);
    //���Ԕ��W���v�Z���郁�\�b�h
    virtual void timeEvolution(double t);
    
    void set_R0(double x, double y, double z) { R0 = { x,y,z }; return; }
    Vector3 get_R0(double m) { return R0; }

private:

};

//�N���[���N���X
class CCrane : public CMob
{
public:
    CCrane() {};
    ~CCrane() {};
private:

};

//�݉׃N���X
class CLoad : public CMob
{
public:
    CLoad() {};
    ~CLoad() {};
private:

};




