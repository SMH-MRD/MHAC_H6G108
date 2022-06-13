#pragma once

#include "framework.h"

#define INF_AREA_X		    10		    //�e�L�X�g�����C���E�B���h�E��\���ʒuX
#define INF_AREA_Y		    30		    //�e�L�X�g�����C���E�B���h�E��\���ʒuY
#define INF_AREA_W		    1000        //�e�L�X�g����
#define INF_AREA_H		    450		    //�e�L�X�g������

#define GRAPHIC_AREA_X		10		    //�O���t�B�b�N�����C���E�B���h�E��\���ʒuX
#define GRAPHIC_AREA_Y		30		    //�O���t�B�b�N�����C���E�B���h�E��\���ʒuY
#define GRAPHIC_AREA_W		600	        //�O���t�B�b�N����
#define GRAPHIC_AREA_H		450		    //�O���t�B�b�N������

//Monitor��ʃO���t�B�b�N���Ǘ��\����
typedef struct _stMonGraphic {  
    DWORD disp_item;                    //�\������
    
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

    HPEN hpen;
    HBRUSH hbrush;

}ST_MON_GRAPHIC, *LPST_MON_GRAPHIC;


//����{�^��ID
#define IDC_MON_START_PB				10601
#define IDC_MON_STOP_PB					10602
#define IDC_MON_RADIO_DISP1				10605   //�\���ؑւP
#define IDC_MON_RADIO_DISP2				10606   //�\���ؑւQ
#define IDC_MON_RADIO_DISP3				10607   //�\���ؑւR
#define IDC_MON_RADIO_DISP4				10608   //�\���ؑւS
#define IDC_MON_RADIO_DISP5				10609   //�\���ؑւT
#define IDC_MON_RADIO_DISP6				10610   //�\���ؑւU

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
    CMonWin(HWND hWnd) { hWnd_parent = hWnd; }
    ~CMonWin() {}
    int init_main_window();
    int disp_update();
    int close_mon();

private:
    HWND hWnd_parent;

    ST_MON_GRAPHIC stGraphic;
    ST_MON_COM_OBJ stComCtrl;

    VOID draw_bg();



 
};

