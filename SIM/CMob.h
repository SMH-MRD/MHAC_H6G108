#pragma once

#include "CVector3.h"

//Moving Objectクラス
class CMob
{
public:
    CMob();
    CMob(double _dt);
    CMob(double _dt, Vector3& _r, Vector3& _v);
    ~CMob();

    double dt;      //時間間隔
    Vector3 r;      //位置ベクトル
    Vector3 v;      //速度ベクトル
    Vector3 dr;     //位置ベクトルの変化分
    Vector3 dv;     //速度ベクトルの変化分

    double M;       //質量　Kg
    double I;       //慣性モーメント　Kg
    Vector3 R0;     //基準点
    double trq;     // 出力トルクNm
     
    //加速度ベクトルを与えるメソッド　　継承先で再定義する
    virtual Vector3 A(double t, Vector3& r, Vector3& v);
    virtual Vector3 A(double t, Vector3& r, Vector3& v, Vector3& trqref, Vector3& f); //外力あり

    //速度ベクトルを与えるメソッド
    virtual Vector3 V(double t, Vector3& r, Vector3& v);
    //時間発展を計算するメソッド
    virtual void timeEvolution(double t);
    virtual void init_mob(double _dt, Vector3& _r, Vector3& _v) {
        dt = _dt;
        r.copy(_r);
        v.copy(_v);
        return;
    }
    
    void set_R0(double x, double y, double z) { R0 = { x,y,z }; return; }
    Vector3 get_R0(double m) { return R0; }


private:

};

//クレーンクラス
class CCrane : public CMob
{
public:
    CCrane() {};
    ~CCrane() {};
private:

    Vector3 A(double t, Vector3& r, Vector3& v, Vector3& trqref, Vector3& f); //加速度計計算(外力あり）

};

//計算誤差吸収処理　紐長さ補正力＝補正ばね弾性力＋補正粘性抵抗力
#define compensationK 0.5       //紐長さ補正弾性係数
#define compensationGamma 0.5   //紐長さ粘性係数

//吊荷クラス
class CLoad : public CMob
{
public:
    CLoad() {};
    ~CLoad() {};
private:

};




