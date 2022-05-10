#pragma once

//�z��Q�Ɨp����ID
#define MOTION_ID_MAX   8  //���䎲�ő吔

#define ID_HOIST        1   //�� �@       ID
#define ID_GANTRY       2   //���s        ID
#define ID_TROLLY       3   //���s        ID
#define ID_BOOM_H       3   //����        ID
#define ID_SLEW         4   //����        ID
#define ID_OP_ROOM      5   //�^�]���ړ��@ID
#define ID_H_ASSY       6   //�݋�        ID
#define ID_MOTION1      7   //�\��        ID
#define ID_MOTION2      8   //�\��        ID

//PLC IO�֘A
#define PLC_PB_MAX              64 //�^�]����{�^���o�^�ő吔
#define PLC_LAMP_MAX            64 //�^�]����{�^���o�^�ő吔
#ifndef PLC_PBL_ID

#define PID_E_STOP                   0
#define PID_CONTROL_SOURCE1_ON       1
#define PID_CONTROL_SOURCE1_OFF      2
#define PID_CONTROL_SOURCE2_ON       3
#define PID_CONTROL_SOURCE2_OFF      4
#define PID_FAULT_RESET              5
#define PID_IL_BYPASS                6
#define PID_FOOK_L                   7
#define PID_FOOK_R                   8
#define PID_TARGET1_MEM              9
#define PID_TARGET2_MEM              10
#define PID_TARGET3_MEM              11
#define PID_TARGET4_MEM              12
#define PID_MODE_OP_ROOM             13
#define PID_MODE_TELECON             14
#define PID_ANTSWAY_ON               15
#define PID_ANTSWAY_OFF              16
#define PID_AUTO_START               17
#define PID_CONTROL_SOURCE           18
#define PID_RAIL_CRAMP_ACTIVE        19
#define PID_RAIL_CRAMP_DEACTIVE      20
#define PID_TTB_ACTIVE               21
#define PID_SLEW_GRACE_ACTIVE        22
#define PID_GANTRY_GRACE_ACTIVE      23
#define PID_SLEW_PUMP_ACTIVE         24
#define PID_COIL_LIFTER              25
#define PID_HOIST_ASSY_RELEASE       26
#define PID_FAULT                    27
#define PID_ALARM                    28
#define PID_OP_TRY_SOURCE_ON         29
#define PID_OP_TRY_SOURCE_OFF        30
#define PID_LIFT_MAGNET_SELECT       31
#define PID_TEMP_HOLD_ON             32
#define PID_SAFE_SW_SAFE             33
#define PID_SAFE_SW_DANJER           34
#define PID_BORDING_CAUTION          35
#define PID_COIL_LIFT_MC3            36
#endif // !PLC_PBL_ID

//EXEC_STATUS
#ifndef AGENT_DO_ID

#define AGENT_DO_MAX            64 
#define AID_CONTROL_SOURCE1          1
#define AID_CONTROL_SOURCE2          2
#define AID_E_STOP                   3
#define AID_FAULT_RESET              4
#define AID_IL_BYPASS                5
#define AID_FOOK_L                   6
#define AID_FOOK_R                   7
#define AID_TARGET1_MEM              8
#define AID_TARGET2_MEM              9
#define AID_TARGET3_MEM3             10
#define AID_TARGET4_MEM4             11
#define AID_TARGET1_SELECT           12 
#define AID_TARGET2_SELECT           13
#define AID_TARGET3_SELECT           14
#define AID_TARGET4_SELECT           15
#define AID_ANTISWAY_ON              16
#define AID_AUTO_ACTIVE              17
#define AID_AUTO_SUSPEND             18
#define AID_AUTO_PROHIBIT            19
#endif // !AGENT_DO_ID

//SWAY_IO
#ifndef SWAY_IO_ID

#define SENSOR_TARGET_MAX            4 
#define DETECT_AXIS_MAX              4
#define LAMP_NUM                     3

#define SID_X                        0
#define SID_Y                        1
#define SID_TH                       2
#define SID_R                        3

#define SID_RED                      0
#define SID_GREEN                    1
#define SID_BLUE                     2
#endif // !SWAY_IO_ID
