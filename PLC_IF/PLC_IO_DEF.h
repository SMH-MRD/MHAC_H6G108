#pragma once

#define ALL_BITS    0x0000 //WORDデータ
#define BIT0        0x0001
#define BIT1        0x0002
#define BIT2        0x0004
#define BIT3        0x0008
#define BIT4        0x0010
#define BIT5        0x0020
#define BIT6        0x0040
#define BIT7        0x0080
#define BIT8        0x0100
#define BIT9        0x0200
#define BIT10       0x0400
#define BIT11       0x0800
#define BIT12       0x1000
#define BIT13       0x2000
#define BIT14       0x4000
#define BIT15       0x8000

//ノッチビットパターン
#define NOTCH_PTN0_ALL   0x007F
#define NOTCH_PTN0_CLR   0xFF80
#define NOTCH_PTN0_0     0x0001
#define NOTCH_PTN0_F1    0x0002
#define NOTCH_PTN0_R1    0x0004
#define NOTCH_PTN0_F2    0x000A
#define NOTCH_PTN0_R2    0x000C
#define NOTCH_PTN0_F3    0x001A
#define NOTCH_PTN0_R3    0x001C
#define NOTCH_PTN0_F4    0x003A
#define NOTCH_PTN0_R4    0x003C
#define NOTCH_PTN0_F5    0x007A
#define NOTCH_PTN0_R5    0x007C

#define NOTCH_PTN1_ALL   0x3F80
#define NOTCH_PTN1_CLR   0xC07F
#define NOTCH_PTN1_0     0x0080
#define NOTCH_PTN1_F1    0x0100
#define NOTCH_PTN1_R1    0x0200
#define NOTCH_PTN1_F2    0x0500
#define NOTCH_PTN1_R2    0x0600
#define NOTCH_PTN1_F3    0x0D00
#define NOTCH_PTN1_R3    0x0E00
#define NOTCH_PTN1_F4    0x1D00
#define NOTCH_PTN1_R4    0x1E00
#define NOTCH_PTN1_F5    0x3D00
#define NOTCH_PTN1_R5    0x3E00

#define NOTCH_PTN2_ALL   0x00F8
#define NOTCH_PTN2_CLR   0xFF07
#define NOTCH_PTN2_0     0x0008
#define NOTCH_PTN2_F1    0x0010
#define NOTCH_PTN2_R1    0x0040
#define NOTCH_PTN2_F2    0x0030
#define NOTCH_PTN2_R2    0x00C0
#define NOTCH_PTN2_F3    0x0030
#define NOTCH_PTN2_R3    0x00C0
#define NOTCH_PTN2_F4    0x0030
#define NOTCH_PTN2_R4    0x00C0
#define NOTCH_PTN2_F5    0x0030
#define NOTCH_PTN2_R5    0x00C0


//PLC LINKインターフェースのデータサイズ

//BIT入力部分の割付マップサイズ
#define PLC_IF_SPARE_B_BUFSIZE      4
#define PLC_IF_PB_B_BUFSIZE         2
#define PLC_IF_NA_B_BUFSIZE         10

//WORD入力部分の割付マップサイズ
#define PLC_IF_MAIN_X_BUFSIZE       14
#define PLC_IF_MAIN_Y_BUFSIZE       6
#define PLC_IF_GNT_X_BUFSIZE        8
#define PLC_IF_GNT_Y_BUFSIZE        2
#define PLC_IF_OPE_X_BUFSIZE        10
#define PLC_IF_OPE_Y_BUFSIZE        3
#define PLC_IF_CC_X_BUFSIZE         12
#define PLC_IF_CC_Y_BUFSIZE         12
#define PLC_IF_CC_W_BUFSIZE         43
#define PLC_IF_ABS_DW_BUFSIZE       8   //WORD数　DWORD 4x2
#define PLC_IF_SENS_W_BUFSIZE       12

//BIT出力部分の割付マップサイズ
#define PLC_IF_PC_B_WRITE_COMSIZE   14  //PCコマンド出力部サイズ
#define PLC_IF_PC_B_WRITE_SIMSIZE   2   //PCシミュレーション出力部サイズ

//WORD出力部分の割付マップサイズ
#define PLC_IF_PC_W_WRITE_COMSIZE   32  //PCコマンド出力部サイズ
#define PLC_IF_PC_W_WRITE_SIMSIZE   48  //PCシミュレーション出力部サイズ


#define ID_WPOS     0
#define ID_BPOS     1
//PLC出力　Bレジスタ割付マップ
typedef struct StPLCOutBMap {
    UINT16 PB_as_off[2]          = { 5, BIT0 };
    UINT16 PB_as_on[2]           = { 5, BIT1 };
    UINT16 PB_auto_start[2]      = { 5, BIT2 };
    
    UINT16 PB_auto_target1[2]    = { 5, BIT4 };
    UINT16 PB_auto_target2[2]    = { 5, BIT5 };
    UINT16 PB_auto_target3[2]    = { 5, BIT6 };
    UINT16 PB_auto_target4[2]    = { 5, BIT7 };

    UINT16 PB_mode_crane[2]      = { 5, BIT13 };
    UINT16 PB_mode_remote[2]     = { 5, BIT14 };
}ST_PLC_OUT_BMAP, * LPST_PLC_OUT_BMAP;
//PLC出力　Wレジスタ割付マップ
typedef struct StPLCOutWMap {
    UINT16 com_hst_notch_0[2] = { 30, NOTCH_PTN0_0 };
    UINT16 com_hst_notch_f1[2] = { 30, NOTCH_PTN0_F1 };
    UINT16 com_hst_notch_f2[2] = { 30, NOTCH_PTN0_F2 };
    UINT16 com_hst_notch_f3[2] = { 30, NOTCH_PTN0_F3 };
    UINT16 com_hst_notch_f4[2] = { 30, NOTCH_PTN0_F4 };
    UINT16 com_hst_notch_f5[2] = { 30, NOTCH_PTN0_F5 };
    UINT16 com_hst_notch_r1[2] = { 30, NOTCH_PTN0_R1 };
    UINT16 com_hst_notch_r2[2] = { 30, NOTCH_PTN0_R2 };
    UINT16 com_hst_notch_r3[2] = { 30, NOTCH_PTN0_R3 };
    UINT16 com_hst_notch_r4[2] = { 30, NOTCH_PTN0_R4 };
    UINT16 com_hst_notch_r5[2] = { 30, NOTCH_PTN0_R5 };

    UINT16 com_gnt_notch_0[2] = { 30, NOTCH_PTN1_0 };
    UINT16 com_gnt_notch_f1[2] = { 30, NOTCH_PTN1_F1 };
    UINT16 com_gnt_notch_f2[2] = { 30, NOTCH_PTN1_F2 };
    UINT16 com_gnt_notch_f3[2] = { 30, NOTCH_PTN1_F3 };
    UINT16 com_gnt_notch_f4[2] = { 30, NOTCH_PTN1_F4 };
    UINT16 com_gnt_notch_f5[2] = { 30, NOTCH_PTN1_F5 };
    UINT16 com_gnt_notch_r1[2] = { 30, NOTCH_PTN1_R1 };
    UINT16 com_gnt_notch_r2[2] = { 30, NOTCH_PTN1_R2 };
    UINT16 com_gnt_notch_r3[2] = { 30, NOTCH_PTN1_R3 };
    UINT16 com_gnt_notch_r4[2] = { 30, NOTCH_PTN1_R4 };
    UINT16 com_gnt_notch_r5[2] = { 30, NOTCH_PTN1_R5 };

    UINT16 com_ctrl_source_on[2] = { 31, BIT0 };
    UINT16 com_ctrl_source_off[2] = { 31, BIT1 };
    UINT16 com_estop[2] = { 31, BIT2 };
    UINT16 com_op_rm_notch_0[2] = { 31, NOTCH_PTN2_0 };
    UINT16 com_op_rm_notch_f1[2] = { 31, NOTCH_PTN2_F1 };
    UINT16 com_op_rm_notch_f2[2] = { 31, NOTCH_PTN2_F2 };
    UINT16 com_op_rm_notch_r1[2] = { 31, NOTCH_PTN2_R1 };
    UINT16 com_op_rm_notch_r2[2] = { 31, NOTCH_PTN2_R2 };
    UINT16 com_hst_base_spd[2] = { 31, BIT8 };
    UINT16 com_hst_high_spd[2] = { 31, BIT9 };
    UINT16 com_sel_fook[2] = { 31, BIT10 };
    UINT16 com_sel_lifter[2] = { 31, BIT11 };
    UINT16 com_solo_clamp[2] = { 31, BIT12 };
    UINT16 com_link_clamp[2] = { 31, BIT13 };
    UINT16 com_hold_clamp[2] = { 31, BIT14 };
    UINT16 com_release_clamp[2] = { 31, BIT15 };

    UINT16 com_fault_reset[2] = { 32, BIT5 };
    UINT16 com_IL_bypass[2] = { 32, BIT6 };
    UINT16 com_hst_1m[2] = { 32, BIT8 };
    UINT16 com_temp_hold[2] = { 32, BIT9 };
    UINT16 com_slw_oil_sw[2] = { 32, BIT15 };
   
    UINT16 com_fook_left[2] = { 33, BIT0 };
    UINT16 com_fook_right[2] = { 33, BIT1 };
    UINT16 com_clutch_off[2] = { 33, BIT2 };
    UINT16 com_clutch_on[2] = { 33, BIT3 };

    UINT16 com_bh_notch_0[2] = { 34, NOTCH_PTN0_0 };
    UINT16 com_bh_notch_f1[2] = { 34, NOTCH_PTN0_F1 };
    UINT16 com_bh_notch_f2[2] = { 34, NOTCH_PTN0_F2 };
    UINT16 com_bh_notch_f3[2] = { 34, NOTCH_PTN0_F3 };
    UINT16 com_bh_notch_f4[2] = { 34, NOTCH_PTN0_F4 };
    UINT16 com_bh_notch_f5[2] = { 34, NOTCH_PTN0_F5 };
    UINT16 com_bh_notch_r1[2] = { 34, NOTCH_PTN0_R1 };
    UINT16 com_bh_notch_r2[2] = { 34, NOTCH_PTN0_R2 };
    UINT16 com_bh_notch_r3[2] = { 34, NOTCH_PTN0_R3 };
    UINT16 com_bh_notch_r4[2] = { 34, NOTCH_PTN0_R4 };
    UINT16 com_bh_notch_r5[2] = { 34, NOTCH_PTN0_R5 };

    UINT16 com_slw_notch_0[2] = { 34, NOTCH_PTN1_0 };
    UINT16 com_slw_notch_f1[2] = { 34, NOTCH_PTN1_F1 };
    UINT16 com_slw_notch_f2[2] = { 34, NOTCH_PTN1_F2 };
    UINT16 com_slw_notch_f3[2] = { 34, NOTCH_PTN1_F3 };
    UINT16 com_slw_notch_f4[2] = { 34, NOTCH_PTN1_F4 };
    UINT16 com_slw_notch_f5[2] = { 34, NOTCH_PTN1_F5 };
    UINT16 com_slw_notch_r1[2] = { 34, NOTCH_PTN1_R1 };
    UINT16 com_slw_notch_r2[2] = { 34, NOTCH_PTN1_R2 };
    UINT16 com_slw_notch_r3[2] = { 34, NOTCH_PTN1_R3 };
    UINT16 com_slw_notch_r4[2] = { 34, NOTCH_PTN1_R4 };
    UINT16 com_slw_notch_r5[2] = { 34, NOTCH_PTN1_R5 };

    UINT16 com_sel_ope_rm[2] = { 36, BIT10 };
    UINT16 com_sel_telecon[2] = { 36, BIT11 };

    UINT16 com_ctrl_source2_on[2] = { 37, BIT0 };
    UINT16 com_ctrl_source2_off[2] = { 37, BIT1 };

    UINT16 com_temp_hold_off[2] = { 37, BIT4 };
    UINT16 com_lift_bypass_on[2] = { 37, BIT6 };
    UINT16 com_lift_bypass_off[2] = { 37, BIT7 };

    UINT16 spd_hst_fb[2] = {77,ALL_BITS };
    UINT16 spd_gnt_fb[2] = {75 ,ALL_BITS };
    UINT16 spd_bh_fb[2] = { 83,ALL_BITS };
    UINT16 spd_slw_fb[2] = { 69,ALL_BITS };

    UINT16 load_fb[2] = { 120,ALL_BITS };

    UINT16 pos_hst_fb[2] = { 121,ALL_BITS };
    UINT16 pos_gnt_fb[2] = { 124 ,ALL_BITS };
    UINT16 pos_bh_fb[2] = { 123,ALL_BITS };
    UINT16 pos_slw_fb[2] = { 122,ALL_BITS };


}ST_PLC_OUT_WMAP, * LPST_PLC_OUT_WMAP;

//PC出力　Bレジスタ割付マップ
typedef struct StPCOutBMap {

    UINT16 healty[2]                 = { 0, BIT0 };
    UINT16 com_0stop[2]	            = { 0, BIT1 };
    UINT16 com_auto_device_on[2]	    = { 0, BIT2 };
    UINT16 com_auto_device_off[2]    = { 0, BIT3 };
    UINT16 com_sway_trget_on[2]	    = { 0, BIT5 };
    UINT16 com_sway_trget_off[2]     = { 0, BIT6 };
    UINT16 lamp_as_on[2]             = { 0, BIT8 };
    UINT16 lamp_as_off[2]            = { 0, BIT9 };
    UINT16 lamp_auto_start[2]        = { 0, BIT10 };
    UINT16 lamp_auto_tg1[2]          = { 0, BIT12 };
    UINT16 lamp_auto_tg2[2]          = { 0, BIT13 };
    UINT16 lamp_auto_tg3[2]          = { 0, BIT14 };
    UINT16 lamp_auto_tg4[2]          = { 0, BIT15 };

    UINT16 com_pc_ctr_act[2]         = { 1, BIT12 };
    UINT16 com_plc_emulate_act[2]    = { 1, BIT13 };
    UINT16 com_pc_analog_ref_mode[2] = { 1, BIT14 };
    UINT16 com_pc_fb[2]              = { 1, BIT15 };

    UINT16 com_hst_notch_0[2]        = { 2, NOTCH_PTN0_0 };
    UINT16 com_hst_notch_f1[2]       = { 2, NOTCH_PTN0_F1 };
    UINT16 com_hst_notch_f2[2]       = { 2, NOTCH_PTN0_F2 };
    UINT16 com_hst_notch_f3[2]       = { 2, NOTCH_PTN0_F3 };
    UINT16 com_hst_notch_f4[2]       = { 2, NOTCH_PTN0_F4 };
    UINT16 com_hst_notch_f5[2]       = { 2, NOTCH_PTN0_F5 };
    UINT16 com_hst_notch_r1[2]       = { 2, NOTCH_PTN0_R1 };
    UINT16 com_hst_notch_r2[2]       = { 2, NOTCH_PTN0_R2 };
    UINT16 com_hst_notch_r3[2]       = { 2, NOTCH_PTN0_R3 };
    UINT16 com_hst_notch_r4[2]       = { 2, NOTCH_PTN0_R4 };
    UINT16 com_hst_notch_r5[2]       = { 2, NOTCH_PTN0_R5 };

    UINT16 com_gnt_notch_0[2]        = { 2, NOTCH_PTN1_0 };
    UINT16 com_gnt_notch_f1[2]       = { 2, NOTCH_PTN1_F1 };
    UINT16 com_gnt_notch_f2[2]       = { 2, NOTCH_PTN1_F2 };
    UINT16 com_gnt_notch_f3[2]       = { 2, NOTCH_PTN1_F3 };
    UINT16 com_gnt_notch_f4[2]       = { 2, NOTCH_PTN1_F4 };
    UINT16 com_gnt_notch_f5[2]       = { 2, NOTCH_PTN1_F5 };
    UINT16 com_gnt_notch_r1[2]       = { 2, NOTCH_PTN1_R1 };
    UINT16 com_gnt_notch_r2[2]       = { 2, NOTCH_PTN1_R2 };
    UINT16 com_gnt_notch_r3[2]       = { 2, NOTCH_PTN1_R3 };
    UINT16 com_gnt_notch_r4[2]       = { 2, NOTCH_PTN1_R4 };
    UINT16 com_gnt_notch_r5[2]       = { 2, NOTCH_PTN1_R5 };

    UINT16 com_ctrl_source_on[2]     = { 3, BIT0 };
    UINT16 com_ctrl_source_off[2]    = { 3, BIT1 };
    UINT16 com_estop[2]              = { 3, BIT2 };
    UINT16 com_op_rm_notch_0[2]      = { 3, NOTCH_PTN2_0 };
    UINT16 com_op_rm_notch_f1[2]     = { 3, NOTCH_PTN2_F1 };
    UINT16 com_op_rm_notch_f2[2]     = { 3, NOTCH_PTN2_F2 };
    UINT16 com_op_rm_notch_r1[2]     = { 3, NOTCH_PTN2_R1 };
    UINT16 com_op_rm_notch_r2[2]     = { 3, NOTCH_PTN2_R2 };
    UINT16 com_hst_base_spd[2]       = { 3, BIT8 };
    UINT16 com_hst_high_spd[2]       = { 3, BIT9 };
    UINT16 com_sel_fook[2]           = { 3, BIT10 };
    UINT16 com_sel_lifter[2]         = { 3, BIT11 };
    UINT16 com_solo_clamp[2]         = { 3, BIT12 };
    UINT16 com_link_clamp[2]         = { 3, BIT13 };
    UINT16 com_hold_clamp[2]         = { 3, BIT14 };
    UINT16 com_release_clamp[2]      = { 3, BIT15 };
  
    UINT16 com_fault_reset[2]        = { 4, BIT5 };
    UINT16 com_IL_bypass[2]          = { 4, BIT6 };
    UINT16 com_hst_1m[2]             = { 4, BIT8 };
    UINT16 com_temp_hold[2]          = { 4, BIT9 };
    UINT16 com_slw_oil_sw[2]         = { 4, BIT15 };

    UINT16 com_fook_left[2]          = { 5, BIT0 };
    UINT16 com_fook_right[2]         = { 5, BIT1 };
    UINT16 com_clutch_off[2]         = { 5, BIT2 };
    UINT16 com_clutch_on[2]          = { 5, BIT3 };

    UINT16 com_bh_notch_0[2]         = { 6, NOTCH_PTN0_0 };
    UINT16 com_bh_notch_f1[2]        = { 6, NOTCH_PTN0_F1 };
    UINT16 com_bh_notch_f2[2]        = { 6, NOTCH_PTN0_F2 };
    UINT16 com_bh_notch_f3[2]        = { 6, NOTCH_PTN0_F3 };
    UINT16 com_bh_notch_f4[2]        = { 6, NOTCH_PTN0_F4 };
    UINT16 com_bh_notch_f5[2]        = { 6, NOTCH_PTN0_F5 };
    UINT16 com_bh_notch_r1[2]        = { 6, NOTCH_PTN0_R1 };
    UINT16 com_bh_notch_r2[2]        = { 6, NOTCH_PTN0_R2 };
    UINT16 com_bh_notch_r3[2]        = { 6, NOTCH_PTN0_R3 };
    UINT16 com_bh_notch_r4[2]        = { 6, NOTCH_PTN0_R4 };
    UINT16 com_bh_notch_r5[2]        = { 6, NOTCH_PTN0_R5 };

    UINT16 com_slw_notch_0[2]        = { 6, NOTCH_PTN1_0 };
    UINT16 com_slw_notch_f1[2]       = { 6, NOTCH_PTN1_F1 };
    UINT16 com_slw_notch_f2[2]       = { 6, NOTCH_PTN1_F2 };
    UINT16 com_slw_notch_f3[2]       = { 6, NOTCH_PTN1_F3 };
    UINT16 com_slw_notch_f4[2]       = { 6, NOTCH_PTN1_F4 };
    UINT16 com_slw_notch_f5[2]       = { 6, NOTCH_PTN1_F5 };
    UINT16 com_slw_notch_r1[2]       = { 6, NOTCH_PTN1_R1 };
    UINT16 com_slw_notch_r2[2]       = { 6, NOTCH_PTN1_R2 };
    UINT16 com_slw_notch_r3[2]       = { 6, NOTCH_PTN1_R3 };
    UINT16 com_slw_notch_r4[2]       = { 6, NOTCH_PTN1_R4 };
    UINT16 com_slw_notch_r5[2]       = { 6, NOTCH_PTN1_R5 };
  
    UINT16 com_sel_ope_rm[2]         = { 8, BIT10 };
    UINT16 com_sel_telecon[2]        = { 8, BIT11 };

    UINT16 com_ctrl_source2_on[2]    = { 9, BIT0 };
    UINT16 com_ctrl_source2_off[2]   = { 9, BIT1 };

    UINT16 com_temp_hold_off[2]      = { 9, BIT4 };
    UINT16 com_lift_bypass_on[2]     = { 9, BIT6 };
    UINT16 com_lift_bypass_off[2]    = { 9, BIT7 };

}ST_PC_OUT_BMAP, * LPST_PC_OUT_BMAP;
//PC出力　Wレジスタ割付マップ
typedef struct StPCOutWMap {
    UINT16 helthy[2]                 = { 0, ALL_BITS};
    UINT16 spd_ref_hst[2]            = { 1, ALL_BITS};
    UINT16 spd_ref_gnt[2]            = { 2, ALL_BITS };
    UINT16 spd_ref_oprm[2]           = { 3, ALL_BITS };
    UINT16 spd_ref_bh[2]             = { 4, ALL_BITS };
    UINT16 spd_ref_slw[2]            = { 5, ALL_BITS };
    
    UINT16 auto_tg1_slw_pos[2]       = { 8, ALL_BITS };
    UINT16 auto_tg1_bh_pos[2]        = { 9, ALL_BITS };
    UINT16 auto_tg2_slw_pos[2]       = { 10, ALL_BITS };
    UINT16 auto_tg2_bh_pos[2]        = { 11, ALL_BITS };
    UINT16 auto_tg3_slw_pos[2]       = { 12, ALL_BITS };
    UINT16 auto_tg3_bh_pos[2]        = { 13, ALL_BITS };
    UINT16 auto_tg4_slw_pos[2]       = { 14, ALL_BITS };
    UINT16 auto_tg4_bh_pos[2]        = { 15, ALL_BITS };

    UINT16 faults_auto1[2]           = { 16, ALL_BITS };
    UINT16 faults_auto2[2]           = { 17, ALL_BITS };
    UINT16 faults_auto3[2]           = { 18, ALL_BITS };
    UINT16 faults_auto4[2]           = { 19, ALL_BITS };

    UINT16 hst_abs_low[2]            = { 32, ALL_BITS };
    UINT16 hst_abs_high[2]           = { 33, ALL_BITS };
    UINT16 bh_abs_low[2]             = { 34, ALL_BITS };
    UINT16 bh_abs_high[2]            = { 35, ALL_BITS };
    UINT16 slw_abs_low[2]            = { 36, ALL_BITS };
    UINT16 slw_abs_high[2]           = { 37, ALL_BITS };
    UINT16 gnt_abs_low[2]            = { 38, ALL_BITS };
    UINT16 gnt_abs_high[2]           = { 39, ALL_BITS };

    UINT16 load_cell[2]              = { 40, ALL_BITS };
    UINT16 hst_pos[2]                = { 41, ALL_BITS };
    UINT16 slw_pos[2]                = { 42, ALL_BITS };
    UINT16 bh_pos[2]                 = { 43, ALL_BITS };
    UINT16 gnt_pos[2]                = { 44, ALL_BITS };

    UINT16 inv_slw1_moter_rpm[2]     = { 48, ALL_BITS };
    UINT16 inv_slw1_moter_trq[2]     = { 49, ALL_BITS };
    UINT16 inv_slw1_out_freq[2]      = { 50, ALL_BITS };
    UINT16 inv_slw2_moter_rpm[2]     = { 51, ALL_BITS };
    UINT16 inv_slw2_moter_trq[2]     = { 52, ALL_BITS };
    UINT16 inv_slw2_out_freq[2]      = { 53, ALL_BITS };
    UINT16 inv_gnt_moter_rpm[2]      = { 54, ALL_BITS };
    UINT16 inv_gnt_moter_trq[2]      = { 55, ALL_BITS };
    UINT16 inv_hst1_moter_rpm[2]     = { 56, ALL_BITS };
    UINT16 inv_hst1_moter_trq[2]     = { 57, ALL_BITS };
    UINT16 inv_hst1_out_freq[2]      = { 58, ALL_BITS };
    UINT16 inv_hst2_moter_rpm[2]     = { 59, ALL_BITS };
    UINT16 inv_hst2_moter_trq[2]     = { 60, ALL_BITS };
    UINT16 inv_hst2_out_freq[2]      = { 61, ALL_BITS };
    UINT16 inv_bh_moter_rpm[2]       = { 62, ALL_BITS };
    UINT16 inv_bh_moter_trq[2]       = { 63, ALL_BITS };
    UINT16 inv_bh_out_freq[2]        = { 64, ALL_BITS };
    UINT16 inv_reel1_moter_rpm[2]    = { 65, ALL_BITS };
    UINT16 inv_reel1_moter_trq[2]    = { 66, ALL_BITS };
    UINT16 inv_reel1_out_freq[2]     = { 67, ALL_BITS };
    UINT16 inv_reel2_moter_rpm[2]    = { 68, ALL_BITS };
    UINT16 inv_reel2_moter_trq[2]    = { 69, ALL_BITS };
    UINT16 inv_reel2_out_freq[2]     = { 70, ALL_BITS };
    UINT16 inv_oprm_moter_rpm[2]     = { 71, ALL_BITS };
    UINT16 inv_oprm_moter_trq[2]     = { 72, ALL_BITS };
    UINT16 inv_oprm_out_freq[2]      = { 73, ALL_BITS };
    UINT16 inv_fook_moter_rpm[2]     = { 74, ALL_BITS };
    UINT16 inv_fook_moter_trq[2]     = { 75, ALL_BITS };
    UINT16 inv_fook_out_freq[2]      = { 76, ALL_BITS };

}ST_PC_OUT_WMAP, * LPST_PC_OUT_WMAP;