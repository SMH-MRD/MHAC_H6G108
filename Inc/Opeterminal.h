#pragma once

#include <winsock.h>
#include <time.h>

/******* ����[��IF ���ʃ��b�Z�[�W�w�b�_��                 ***********/
typedef struct OteComHead { 
    INT32       myid;
    INT32       code;
    SOCKADDR_IN addr;
    INT32       status;
    INT32       oteid;
}ST_OTE_HEAD, * LPST_OTE_HEAD;

typedef struct MOteSndMsg {
    ST_OTE_HEAD         head;
}ST_MOTE_SND_MSG, * LPST_MOTE_SND_MSG;

/******* ����[��IF �}���`�L���X�g�ʐM��M���b�Z�[�W�\���� ***********/
typedef struct MOteRcvBody {
    UCHAR       ote_enable[32];	//�ڑ��\�[���t���O
    INT32	    n_remote_wait;  //�ڑ��҂����u�����䐔
    INT32	    onbord_seqno;   //�@���ڑ��V�[�P���X�ԍ�
    INT32	    remote_seqno;   //���u��ڑ��V�[�P���X�ԍ�
    INT32	    my_seqno;       //���g�̐ڑ��V�[�P���X�ԍ�
}ST_MOTE_RCV_BODY, * LPST_MOTE_RCV_BODY;

typedef struct MOteRcvMsg {
    ST_OTE_HEAD         head;
    ST_MOTE_RCV_BODY    body;
}ST_MOTE_RCV_MSG, * LPST_MOTE_RCV_MSG;


/******* ����[��IF ���j�`�L���X�g�ʐM���M���b�Z�[�W�\���� ***********/

typedef struct UOteSndBody {
    char        pad_ao[4];            //�p�f�B���O
    double      pos[7];             //�ʒuFB
    double      v_fb[6];            //���xFB
    double      v_ref[4];           //���x�w��
    double      tg_pos1[3];         //�ڕW�ʒu���W1
    double      tg_dist1[3];        //�ڕW�܂ł̋���1
    double      tg_pos2[3];         //�ڕW�ʒu���W2
    double      tg_dist2[3];        //�ڕW�܂ł̋���2
    double      tg_pos_semi[6][3];  //�������ڕW�ʒu���WS1-L3
    char        pad_lamp[4];            //�p�f�B���O
    UCHAR       lamp[64];           //�����v�\��
    INT16       notch_pos[4];       //�m�b�`�����v�\��
    char        pad_plc[4];         //�p�f�B���O
    INT16	    plc_data[99];       //�ڑ��҂����u�����䐔
}ST_UOTE_SND_BODY, * LPST_OTE_SND_BODY;

typedef struct UOteSndMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_SND_BODY    body;
}ST_UOTE_SND_MSG, * LPST_UOTE_SND_MSG;

/******* ����[��IF ���j�`�L���X�g�ʐM��M���b�Z�[�W�\���� ***********/
typedef struct UOteRcvBody {
    char        pad_ao[4];          //�p�f�B���O
    double      tg_pos1[3];         //�ڕW�ʒu���W1
    double      tg_dist1[3];        //�ڕW�܂ł̋���1
    double      tg_pos2[3];         //�ڕW�ʒu���W2
    double      tg_dist2[3];        //�ڕW�܂ł̋���2
    char        pad_pb[4];          //�p�f�B���O
    UCHAR       pb[64];             //�����v�\��
    INT16       notch_pos[4];       //�m�b�`�����v�\��
}ST_UOTE_RCV_BODY, * LPST_UOTE_RCV_BODY;

typedef struct UOteRcvdMsg {
    ST_OTE_HEAD         head;
    ST_UOTE_RCV_BODY    body;
}ST_UOTE_RCV_MSG, * LPST_UOTE_RCV_MSG;

