#pragma once

#include "CVector3.h"
#include "COMMON_DEF.H"
#include "Spec.h"

//Moving Objectクラス
class CMob
{
public:
    CMob();
    CMob(double _dt);
    CMob(double _dt, Vector3& _r, Vector3& _v);
    ~CMob();

    double dt;      //計算時間間隔
    Vector3 r;      //位置ベクトル
    Vector3 v;      //速度ベクトル
    Vector3 fex;    //外力
    Vector3 dr;     //位置ベクトルの変化分
    Vector3 dv;     //速度ベクトルの変化分
    Vector3 R0;     //基準点
     
    //加速度ベクトルを与えるメソッド　　継承先で再定義する
    virtual Vector3 A(Vector3& r, Vector3& v); 
    virtual void set_fex(double,double,double);//外力

    //速度ベクトルを与えるメソッド
    virtual Vector3 V(Vector3& r, Vector3& v);
    //時間発展を計算するメソッド
    virtual void timeEvolution();
    virtual void init_mob(double _dt, Vector3& _r, Vector3& _v) {
        dt = _dt;
        r.copy(_r);
        v.copy(_v);
        return;
    }
 
private:

};

//クレーンクラス
//r,vは、吊点の位置と座標

class CCrane : public CMob
{
public:
    CCrane();
    ~CCrane();
    
    double M;                       //クレーン全体質量　Kg
    Vector3 rc;                     //クレーン中心点の位置ベクトル
    Vector3 vc;                     //クレーン中心点の速度ベクトル


    double r0[MOTION_ID_MAX];        //位置・角度
    double v0[MOTION_ID_MAX];        //速度・角速度
    double a0[MOTION_ID_MAX];        //加速度・角加速度

    double v_ref[MOTION_ID_MAX];     //速度・角速度指令
 
    double trq_fb[MOTION_ID_MAX];    //モータートルクFB
    bool motion_break[MOTION_ID_MAX];//ブレーキ開閉状態

    void set_v_ref(double hoist_ref,double gantry_ref,double slew_ref,double boomh_ref); //速度指令値入力
    void init_crane(double _dt, Vector3& _r, Vector3& _v,Vector3& r_offset, Vector3& v_offset); //入力：クレーン中心部　オフセット：クレーン中心と吊点との相対
    int set_spec(LPST_SPEC _pspec) { pspec = _pspec; return 0; }
    void update_ref_elapsed();     //各軸の指令出力経過時間セット
    void update_break_status();    //ブレーキ状態, ブレーキ開放経過時間セット
    
    //時間発展を計算するメソッド
    void timeEvolution();

private:
    double elaped_time[MOTION_ID_MAX];     //ブレーキ開放経過時間
    double Tf[MOTION_ID_MAX];              //加速度一時遅れ

    Vector3 A(Vector3& _r, Vector3& _v);    //吊点加速度計算
    void Ac();                              //クレーン加速度計算

    LPST_SPEC pspec;
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

    double M;                   //吊荷質量　Kg

private:

};




