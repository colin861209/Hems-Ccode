#ifndef HEMS_H
#define HEMS_H

//typedef struct
//{
//	int *tse;
//	float *Pa;
//}changeable;

extern float step1_bill;			//�Ω�B�J�@�p��q�O
extern float step1_sell;			//�Ω�B�J�@�p��q�O
extern float step1_PESS;			//�Ω�B�J�@�p��PESS-��


extern int sample_time;				//��___�Ө��ˮɶ�
extern int time_block;				//�`���ˮɶ�
extern int app_count;				//�`�t���ƶq(�i���_�t�� + ���i���_�t��)
extern int variable;				//�ܼƼƶq
extern int divide;					//�C�@�p�ɦ�___�Ө��ˮɶ�
extern int sa_counter;				//���sa�����Ұʤ��i���_�t��+�ܰʭt���Ӽ�
extern int rasa_counter;		    //���ra()>=sa()�����Ұʤ��i���_�t��+�ܰʭt���Ӽ�


extern float delta_T;

extern int i, j, m, n, h, k;
extern int RT_enable;               //�O�_�ҰʤήɱƵ{(�p�G�@���Ƶ{�����h���Ұ�)
extern double z;

extern int interrupt_num;			//�i���_�t���ƶq
extern int *interrupt_start;		//�i���_�t���Ƶ{�ҩl�ɨ�
extern int *interrupt_end;			//�i���_�t���Ƶ{����ɨ�
extern int *interrupt_ot;			//�i���_�t���w�p����ɶ�
extern int *interrupt_reot;			//�i���_�t���Ѿl����ɶ�
extern float *interrupt_p;			//�i���_�t���T�w�ӯ�


extern int uninterrupt_num;			//���i���_�t���ƶq
extern int *uninterrupt_start;		//���i���_�t���Ƶ{�ҩl�ɨ�
extern int *uninterrupt_end;		//���i���_�t���Ƶ{����ɨ�
extern int *uninterrupt_ot;			//���i���_�t���w�p����ɶ�
extern int *uninterrupt_reot;		//���i���_�t���Ѿl����ɶ�
extern float *uninterrupt_p;		//���i���_�t���T�w�ӯ�


extern int *uninterrupt_flag;		//���i���_�t���O�_�w�}�ҺX��

extern int varying_num;			//�ܰʭt���ƶq
extern int *varying_start;		//�ܰʭt���Ƶ{�ҩl�ɨ�
extern int *varying_end;			//�ܰʭt���Ƶ{����ɨ�
extern int *varying_ot;		//�ܰʭt���w�p����ɶ�
extern int *varying_reot;		//�ܰʭt���Ѿl����ɶ�

extern int ponit_num;


extern int *total_block;
extern int **block;
extern float **INT_power;
extern float **UNINT_power;
extern float **power;

extern int *varying_flag;		//�ܰʭt�����A�X��(�O�_�w�}��)

extern float *price;
extern float solar[24];

extern float Cbat;
extern float Vsys;
extern float SOC_ini;
extern float SOC_min;
extern float SOC_max;
extern float SOC_thres;
extern float Pbat_min;
extern float Pbat_max;
extern float Pgrid_max;
extern float Psell_max;
extern float Pfc_max;
extern float Delta_battery;

extern char column[400] ;



void GLPK(int *, int *, int *, int *, float *, int *, int *, int *, int *, float *, int *, int *, int *, int *, int *, int *, int **, float **, int, float *, int *);
//void GLPK_1(int *, int *, int *, int *, float *, int *, int *, int *, int *, float *, int *, int *, int *, int *, int *, int *, int **, float **, int, float *);
void *new2d(int, int, int);
#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))

#endif 