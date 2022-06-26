#pragma once

#include "framework.h"

#define INF_AREA_X		        10		//�e�L�X�g�����C���E�B���h�E��\���ʒuX
#define INF_AREA_Y		        30		//�e�L�X�g�����C���E�B���h�E��\���ʒuY
#define INF_AREA_W		        1020    //�e�L�X�g����
#define INF_AREA_H		        450		//�e�L�X�g������

#define GRAPHIC_AREA_X		    0		//�O���t�B�b�N������\���ʒuX
#define GRAPHIC_AREA_Y		    0		//�O���t�B�b�N������\���ʒuY
#define GRAPHIC_AREA_W		    620	    //�O���t�B�b�N����
#define GRAPHIC_AREA_H		    350		//�O���t�B�b�N������

#define CMON_PIX_PER_M_CRANE    5      //   1m 5pixel �N���[����
#define CMON_PIX_PER_M_HOIST    5      //   1m 5pixel ���ʒu��
#define CMON_PIX_R_BOOM_END     2       //  �u�[���[�_�\���~���a
#define CMON_PIX_PER_RAD_LOAD   1000    //  1rad 1000pixel �݉ו�
#define CMON_PIX_PER_M_GNT      2       //  1m 2pixel   ���s�ʒu
#define CMON_PIX_GNT_MARK_W     2       //  ���s�ʒu�\���}�[�N��
#define CMON_PIX_HST_MARK_W     2       //  ���ʒu�\���}�[�N��

#define CRANE_GRAPHIC_CENTER_X  160	    //�N���[���O���t�B�b�N���S�ʒuX
#define CRANE_GRAPHIC_CENTER_Y	230		//�N���[���O���t�B�b�N���S�ʒuY
#define CRANE_GRAPHIC_W         290	    //�N���[���O���t�B�b�N���S�ʒuX
#define CRANE_GRAPHIC_H	        290		//�N���[���O���t�B�b�N���S�ʒuY
#define LOAD_GRAPHIC_CENTER_X   480		//�݉׃O���t�B�b�N���S�ʒuX
#define LOAD_GRAPHIC_CENTER_Y	230		//�݉׃O���t�B�b�N���S�ʒuY
#define LOAD_GRAPHIC_W          290		//�݉׃O���t�B�b�N��W
#define LOAD_GRAPHIC_H	        290		//�݉׃O���t�B�b�N����H
#define GNT_GRAPHIC_AREA_X      10		//���s�ʒu�\���O���t�B�b�N����ʒuX
#define GNT_GRAPHIC_AREA_Y	    10		//���s�ʒu�\���O���t�B�b�N����ʒuY
#define GNT_GRAPHIC_AREA_W      620		//���s�ʒu�\���O���t�B�b�N��W
#define GNT_GRAPHIC_AREA_H	    20		//���s�ʒu�\���O���t�B�b�N����H
#define MH_GRAPHIC_AREA_X       315		//���ʒu�\���O���t�B�b�N����ʒuX
#define MH_GRAPHIC_AREA_Y	    80		//���ʒu�\���O���t�B�b�N����ʒuY
#define MH_GRAPHIC_AREA_W       10		//���ʒu�\���O���t�B�b�N��W
#define MH_GRAPHIC_AREA_H	    128		//���ʒu�\���O���t�B�b�N����H
#define MH_GRAPHIC_Y0	        230		//���ʒu�\���O���t�B�b�N0m�ʒuY
#define MH_GRAPHIC_UPPER_LIM	84		//������ʒu�\��Y0�ʒu����14.5m��(146PIX)��
#define MH_GRAPHIC_LOWER_LIM	340		//������ʒu�\��Y0�ʒu����11m��(110PIX)��


#define N_CREATE_PEN            8
#define N_CREATE_BRUSH          8
#define CMON_RED_PEN            0
#define CMON_BLUE_PEN           1
#define CMON_GREEN_PEN          2
#define CMON_GLAY_PEN           3
#define CMON_RED_BRUSH          0
#define CMON_BLUE_BRUSH         1
#define CMON_GREEN_BRUSH        2
#define CMON_BG_BRUSH           3



//Monitor��ʃO���t�B�b�N���Ǘ��\����
typedef struct _stMonGraphic {  
    int disp_item = 0;                    //�\������
    
    int area_x, area_y, area_w, area_h; //���C���E�B���h�E��̕\���G���A
    int bmp_w, bmp_h;                   //�O���t�B�b�N�r�b�g�}�b�v�T�C�Y

    HBITMAP hBmap_mem0;
    HBITMAP hBmap_bg;
    HBITMAP hBmap_gr;
    HBITMAP hBmap_inf;
    HDC hdc_mem0;						//������ʃ������f�o�C�X�R���e�L�X�g
    HDC hdc_mem_bg;					    //�O���t�B�b�N�w�i��ʃ������f�o�C�X�R���e�L�X�g
    HDC hdc_mem_gr;					    //�O���t�B�b�N���������f�o�C�X�R���e�L�X�g
    HDC hdc_mem_inf;					//������ʃ������f�o�C�X�R���e�L�X�g

    HFONT hfont_inftext;				//�e�L�X�g�p�t�H���g
    BLENDFUNCTION bf;					//�����ߐݒ�\����

    HPEN hpen[N_CREATE_PEN];
    HBRUSH hbrush[N_CREATE_BRUSH];

}ST_MON_GRAPHIC, *LPST_MON_GRAPHIC;


//����{�^��ID
#define IDC_MON_START_PB				10601
#define IDC_MON_STOP_PB					10602
#define IDC_MON_RADIO_DISP0				10605   //�\���ؑ�0
#define IDC_MON_RADIO_DISP1				10606   //�\���ؑ�1
#define IDC_MON_RADIO_DISP2				10607   //�\���ؑ�2
#define IDC_MON_RADIO_DISP3				10608   //�\���ؑ�3
#define IDC_MON_RADIO_DISP4				10609   //�\���ؑ�4
#define IDC_MON_RADIO_DISP5				10610   //�\���ؑ�5

//STATIC TEXT ID
#define IDC_MON_STATIC0					10611		
#define IDC_MON_STATIC1					10612		
#define IDC_MON_STATIC2					10613		
#define IDC_MON_STATIC3					10614		
#define IDC_MON_STATIC4					10615		
#define IDC_MON_STATIC5					10616		

//Monitor��ʃR�����R���g���[���Ǘ��\����
typedef struct _stMonComObj {

    HWND hwnd_map2d_startPB;					//�X�^�[�gPB�̃n���h��
    HWND hwnd_map2d_stopPB;					    //�X�g�b�vPB�̃n���h��
    HWND hwnd_map2d_opt1_radio;					//�`���[�gOption1PB�̃n���h��
    HWND hwnd_map2d_opt2_radio;					//�`���[�gOption2PB�̃n���h��
    HWND hwnd_map2d_opt3_radio;					//�`���[�gOption3PB�̃n���h��
    HWND hwnd_map2d_opt4_radio;					//�`���[�gOption4PB�̃n���h��
    HWND hwnd_map2d_opt5_radio;					//�`���[�gOption5PB�̃n���h��
    HWND hwnd_map2d_opt6_radio;					//�`���[�gOption6PB�̃n���h��

    HWND hwnd_map2d_static0;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��
    HWND hwnd_map2d_static1;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��
    HWND hwnd_map2d_static2;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��
    HWND hwnd_map2d_static3;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��
    HWND hwnd_map2d_static4;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��
    HWND hwnd_map2d_static5;					//�X�^�e�B�b�N�e�L�X�g�̃n���h��

} ST_MON_COM_OBJ, *LPST_MON_COM_OBJ;


class CMonWin
{
public:
    CMonWin(HWND hWnd);
    ~CMonWin();
    int init_main_window();
    int disp_update();
    int close_mon();
    int combine_map();

    ST_MON_GRAPHIC stGraphic;
    ST_MON_COM_OBJ stComCtrl;

    VOID draw_bg();
    VOID draw_inf();
    VOID draw_graphic();

private:
    HWND hWnd_parent;

};

