#pragma once

#define BIT0    0x0001
#define BIT1    0x0002
#define BIT2    0x0004
#define BIT3    0x0008
#define BIT4    0x0010
#define BIT5    0x0020
#define BIT6    0x0040
#define BIT7    0x0080
#define BIT8    0x0100
#define BIT9    0x0200
#define BIT10   0x0400
#define BIT11   0x0800
#define BIT12   0x1000
#define BIT13   0x2000
#define BIT14   0x4000
#define BIT15   0x8000

#define B_HST_NOTCH_0 (plc_link.ope_x_buf[0]) & BIT0

//PLC UI PB
#define PLC_UI_PB_ESTOP         0
#define PLC_UI_PB_ANTISWAY      1
#define PLC_UI_PB_AUTO_START    2
#define PLC_UI_PB_AUTO_RESET    3
#define PLC_UI_PB_AUTO_FROM1    4
#define PLC_UI_PB_AUTO_FROM2    5
#define PLC_UI_PB_AUTO_FROM3    6
#define PLC_UI_PB_AUTO_FROM4    7
#define PLC_UI_PB_AUTO_TO1      8
#define PLC_UI_PB_AUTO_TO2      9
#define PLC_UI_PB_AUTO_TO3      10
#define PLC_UI_PB_AUTO_TO4      11
#define PLC_UI_CS_REMOTE        12

//PLC UI LAMP

//PLC STATUS CONTROL
#define PLC_STAT_CONTROL_SOURCE 0
#define PLC_STAT_REMOTE         1

