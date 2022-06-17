#pragma once

//int cnt_start_delay[MOTION_ID_MAX];  モーター起動遅れ時間sec
#define SIM_TLOSS_HOIST        1.0    //巻 　      
#define SIM_TLOSS_GANTRY       0.5   //走行       
#define SIM_TLOSS_TROLLY       0.3   //横行       
#define SIM_TLOSS_BOOM_H       0.3   //引込       
#define SIM_TLOSS_SLEW         0.3   //旋回       
 
//double Tf[MOTION_ID_MAX];               加速度一時遅れ
#define SIM_TF_HOIST           0.2   //巻 　      
#define SIM_TF_GANTRY          0.2   //走行       
#define SIM_TF_TROLLY          0.2   //横行       
#define SIM_TF_BOOM_H          0.2   //引込       
#define SIM_TF_SLEW            0.2   //旋回       