#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h> //162 165 76~86 579~581�Ȯɵ��ѱ�
#include <glpk.h> //435~438�h�gsame_day
#include <math.h>
#include <mysql.h>
#include <iostream> //504~513 sample_time ��b�iGLPK�e�A�o�˹L�@�Ѥ~���|�n���s��
#include "HEMS.hpp" //231 PARM_id���F�Oparameter_id~�ҥH��ƥ������S��s��
#include "SQLFunction.hpp"
//void GLPK(int *);
//common variable
int RT_enable = 0;
int h, i, j, k, m, n = 0;
double z = 0;

// common parameter
int time_block = 0, app_count = 0, variable = 0, divide = 0, sample_time = 0;
float delta_T = 0.0; //sa_counter = 0, rasa_counter
float Cbat = 0.0, Vsys = 0.0, SOC_ini = 0.0, SOC_min = 0.0, SOC_max = 0.0, SOC = 0.0, SOC_thres = 0.0, Pbat_min = 0.0, Pbat_max = 0.0, Pgrid_max = 0.0, Psell_max, Delta_battery = 0.0, Pfc_max = 0.0;

float step1_bill = 0.0, step1_sell = 0.0, step1_PESS = 0.0; //?�[?B?J?@?p??q?O

// app parameter
int interrupt_num = 0, uninterrupt_num = 0, varying_num = 0;
// power

int stop_enable = 0;

char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";

int main(void)
{
	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);

	RT_enable = 0;

	//one_day scheduling MYSQL read PARM
	/*============================== ???w?????? ============================== */

	int same_day = 0, real_time = 0;

	float *base_par = new float[16]; //LP_BASE_PARM:1~16
	char *s_time = new char[3];		 //LP_BASE_PARM:17(�t�ήɶ�)
	int *position = new int[16];	 //load�����A

	if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "wang", 6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		system("pause");
		return 0;
	}
	printf("Connect to Mysql sucess!!\n");
	mysql_set_character_set(mysql_con, "utf8");

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 "); //?i???_?t??
	interrupt_num = turn_value_to_int(0);
	printf("LINE %d: interruptable app num:%d\n", __LINE__, interrupt_num);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=2 "); //???i???_?t??
	uninterrupt_num = turn_value_to_int(0);
	printf("LINE %d: uninterruptable app num:%d\n", __LINE__, uninterrupt_num);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=3 "); //?????t??
	varying_num = turn_value_to_int(0);
	printf("LINE %d: varying app num:%d\n", __LINE__, varying_num);

	for (i = 1; i <= 17; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "select value from LP_BASE_PARM where 	parameter_id = '%d'", i);
		fetch_row_value();
		if (i < 17)
			base_par[i - 1] = turn_float(0);

		else
			s_time = mysql_row[0];
	}

	char *token = strtok(s_time, "-");
	int time_tmp[3], tt = 0;
	while (token != NULL)
	{
		time_tmp[tt] = atoi(token);
		token = strtok(NULL, "-");
		tt++;
	}
	if ((time_tmp[0] != (now_time.tm_year + 1900)) || (time_tmp[1] != (now_time.tm_mon + 1)) || (time_tmp[2] != (now_time.tm_mday)))
		real_time = 0;

	//send parameter to variale number
	time_block = base_par[0];
	Vsys = base_par[5];
	Cbat = base_par[6];
	SOC_min = base_par[7];
	SOC_max = base_par[8];
	SOC_thres = base_par[9];
	Pgrid_max = base_par[12];
	Psell_max = base_par[13];
	real_time = (int)base_par[14];

	divide = (time_block / 24);
	delta_T = 1.0 / (float)divide;

	app_count = interrupt_num + uninterrupt_num + varying_num;		//15=12+2+1
	variable = app_count + 1 + uninterrupt_num + (varying_num * 2); //app_count(12+2+1) + 6(Pgrid+Pess+Pcharge+Pdischarge+SOC+z) + uninterrupt_num(2) + varying_num(1)*2 +
																	//FC(1)+FC_T(1)+z_Pfc(5)+s_Pfc(5) +Pfc_on(1)+Pfc_off(1)+Pfc_choice(1)
	//Pbat_max = 0.1*Vsys*Cbat;
	//Pbat_min = 0.1*Vsys*Cbat;
	Pbat_max = 1.53;
	Pbat_min = 1.53;
	//Pfc_max = 5.13;
	Pfc_max = 0.0;

	base_par[1] = app_count;
	base_par[2] = interrupt_num;
	base_par[3] = uninterrupt_num;
	base_par[4] = varying_num;
	base_par[10] = Pbat_max;
	base_par[11] = Pbat_min;

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 28");
	sample_time = turn_value_to_int(0);
	memset(sql_buffer, 0, sizeof(sql_buffer));

	for (i = 1; i <= 13; i++)
	{
		if (base_par[i - 1] - (int)base_par[i - 1] != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%.3f' WHERE  parameter_id = '%d'", base_par[i - 1], i);
			mysql_query(mysql_con, sql_buffer);
		}
		else
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%.0f' WHERE  parameter_id = '%d'", base_par[i - 1], i);
			mysql_query(mysql_con, sql_buffer);
		}
	}
	printf("************ System State ************* \n");
	printf("LINE %d: time block:%d\n", __LINE__, time_block);
	printf("LINE %d: interrupt numbers:%d\n", __LINE__, interrupt_num);
	printf("LINE %d: uninterrupt numbers:%d\n", __LINE__, uninterrupt_num);
	printf("LINE %d: varying numbers:%d\n", __LINE__, varying_num);
	printf("LINE %d: variable numbers:%d\n", __LINE__, variable);
	printf("LINE %d: app counts:%d\n", __LINE__, app_count);
	printf("LINE %d: system voltage:%.1f\n", __LINE__, Vsys);
	printf("LINE %d: battery capacity:%.3f\n", __LINE__, Cbat);
	printf("LINE %d: SOC min:%.2f\n", __LINE__, SOC_min);
	printf("LINE %d: SOC max:%.2f\n", __LINE__, SOC_max);
	printf("LINE %d: SOC threads:%.2f\n", __LINE__, SOC_thres);
	printf("LINE %d: Pbat min:%.3f\n", __LINE__, Pbat_min);
	printf("LINE %d: Pbat max:%.3f\n", __LINE__, Pbat_max);
	printf("LINE %d: Pgrid max:%.2f\n", __LINE__, Pgrid_max);
	printf("LINE %d: Psell max:%.2f\n", __LINE__, Psell_max);
	printf("LINE %d: Last run date:%d-%02d-%02d\n", __LINE__, time_tmp[0], time_tmp[1], time_tmp[2]);
	printf("LINE %d: User set for realtime(0->no 1->yes):%d\n", __LINE__, real_time);
	printf("LINE %d: Last running were same day(0->no 1->yes):%d\n\n", __LINE__, same_day);

	float *price = new float[24];
	float **INT_power = NEW2D(interrupt_num, 4, float);
	float **UNINT_power = NEW2D(uninterrupt_num, 4, float);
	float **VAR_power = NEW2D(varying_num, 9, float);

	for (i = 0; i < app_count; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
		position[i] = turn_value_to_int(0);
	}

	for (i = 1; i < interrupt_num + 1; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 1 ORDER BY number ASC LIMIT %d,1", i - 1);
		for (int j = 0; j < 4; j++)
		{
			INT_power[i - 1][j] = turn_value_to_float(j);
		}
	}

	for (i = 1; i < uninterrupt_num + 1; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 2 ORDER BY number ASC LIMIT %d,1", i - 1);
		for (int j = 0; j < 4; j++)
		{
			UNINT_power[i - 1][j] = turn_value_to_float(j);
		}
	}

	for (i = 1; i < varying_num + 1; i++) //????
	{

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time,power1,power2,power3,block1,block2,block3 FROM load_list WHERE group_id = 3 ORDER BY number ASC LIMIT %d,1", i - 1);
		for (int j = 0; j < 9; j++)
		{
			VAR_power[i - 1][j] = turn_value_to_float(j);
		}
	}

	//??????e?q??
	for (i = 1; i < 25; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT price_value FROM price WHERE price_period = %d", i - 1);
		price[i - 1] = turn_value_to_float(0);
	}

	/*===========================???t??A?}?C?j?p=================================*/
	int *interrupt_start = new int[interrupt_num];
	int *interrupt_end = new int[interrupt_num];
	int *interrupt_ot = new int[interrupt_num];
	int *interrupt_reot = new int[interrupt_num];
	float *interrupt_p = new float[interrupt_num];

	int *uninterrupt_start = new int[uninterrupt_num];
	int *uninterrupt_end = new int[uninterrupt_num];
	int *uninterrupt_ot = new int[uninterrupt_num];
	int *uninterrupt_reot = new int[uninterrupt_num];
	float *uninterrupt_p = new float[uninterrupt_num];
	int *uninterrupt_flag = new int[uninterrupt_num]; //?X??

	int *varying_start = new int[varying_num];
	int *varying_end = new int[varying_num];
	int *varying_ot = new int[varying_num];
	int *varying_reot = new int[varying_num];
	int **varying_t_pow = NEW2D(varying_num, 3, int);
	float **varying_p_pow = NEW2D(varying_num, 3, float);
	int *varying_flag = new int[varying_num]; //?X??

	/*===========================?N?}?C??l??=================================*/
	for (i = 0; i < interrupt_num; i++)
	{
		interrupt_start[i] = 0;
		interrupt_end[i] = 0;
		interrupt_ot[i] = 0;
		interrupt_reot[i] = 0;
		interrupt_p[i] = 0.0;
	}
	for (j = 0; j < uninterrupt_num; j++)
	{
		uninterrupt_start[j] = 0;
		uninterrupt_end[j] = 0;
		uninterrupt_ot[j] = 0;
		uninterrupt_reot[j] = 0;
		uninterrupt_p[j] = 0.0;
		uninterrupt_flag[j] = 0;
	}
	for (k = 0; k < varying_num; k++)
	{
		varying_start[k] = 0;
		varying_end[k] = 0;
		varying_ot[k] = 0;
		varying_reot[k] = 0;
		varying_flag[k] = 0;
		for (j = 0; j < 3; j++)
		{
			varying_t_pow[k][j] = 0;
			varying_p_pow[k][j] = 0.0;
		}
	}

	/*===========================?]?w?]??_??????P?\?v=============================*/
	//???G?????????w????96block ??H??????Hdivide(All the time has been converted to 96block so do not multiply "divide")
	for (i = 0; i < interrupt_num; i++)
	{
		interrupt_start[i] = ((int)(INT_power[i][0] * divide));
		interrupt_end[i] = ((int)(INT_power[i][1] * divide)) - 1;
		interrupt_ot[i] = ((int)(INT_power[i][2] * divide));
		interrupt_p[i] = INT_power[i][3];
	}
	for (i = 0; i < uninterrupt_num; i++)
	{
		uninterrupt_start[i] = ((int)(UNINT_power[i][0] * divide));
		uninterrupt_end[i] = ((int)(UNINT_power[i][1] * divide)) - 1;
		uninterrupt_ot[i] = ((int)(UNINT_power[i][2] * divide));
		uninterrupt_p[i] = UNINT_power[i][3];
	}

	for (i = 0; i < varying_num; i++)
	{
		varying_start[i] = ((int)(VAR_power[i][0] * divide));
		varying_end[i] = ((int)(VAR_power[i][1] * divide)) - 1;
		varying_ot[i] = ((int)(VAR_power[i][2] * divide));
		for (j = 0; j < 3; j++)
		{
			varying_p_pow[i][j] = VAR_power[i][3 + j];
		}
		for (j = 0; j < 3; j++)
		{
			varying_t_pow[i][j] = ((int)(VAR_power[i][6 + j] * divide));
		}
	}

	delete[] INT_power, UNINT_power, VAR_power;

	if ((sample_time + 1) != 97)
	{
		same_day = 1;
	}

	//clean table (if at one_schedule)
	if (same_day == 1 && real_time == 1)
	{
		printf(" *********real_time mode**********\n");

		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status"); //clean real_status;
		sent_query();

		// snprintf(sql_buffer, sizeof(sql_buffer), "SELECT A%d FROM control_status WHERE control_id = 22 ", sample_time);
		// SOC = turn_value_to_float(0);
		// printf("LINE %d: SOC = %.2f\n", __LINE__, SOC);

		// snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE `LP_BASE_PARM` SET `value` = '%f'  WHERE parameter_id=25 ", SOC);
		// sent_query();

		// snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 25"); //get now_SOC
		// SOC_ini = turn_value_to_float(0);
		// if (SOC_ini > 90)
		// 	SOC_ini = 89.8;

		// printf("LINE %d: NOW REAL SOC:%f\n", __LINE__, SOC_ini);
		// memset(sql_buffer, 0, sizeof(sql_buffer));
	}
	else
	{

		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE control_status"); //clean control_status;
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status"); //clean control_status;
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE cost"); //clean control_status;
		sent_query();
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE solar_fake"); //clean control_status;
		sent_query();

		if (real_time == 0) //if we want only do one day schedule
		{
			printf(" *********one_day mode(signed one day mode)**********\n");
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 24"); //get ini_SOC
			SOC_ini = turn_value_to_float(0);
			printf("LINE %d: ini_SOC:%f\n", __LINE__, SOC_ini);
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}
		else
		{
			printf(" *********one_day mode(caculate next day)**********\n");
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 25"); //get now_SOC
			SOC_ini = turn_value_to_float(0);
			printf("LINE %d: ini_SOC:%f\n", __LINE__, SOC_ini);
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}

		sample_time = 0;
		real_time = 1; //if you don't want do real_time,please commend it.
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %d WHERE parameter_id = 15 ", real_time);
		sent_query();
	}

	/*=========�ۤv�諸================get sample time=============================*/
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 28");
	sample_time = turn_value_to_float(0);
	memset(sql_buffer, 0, sizeof(sql_buffer));
	printf("LINE %d: time_block_sample_time=%d\n", __LINE__, sample_time);
	/*=========�ۤv�諸================get sample time=============================*/

	//done and update_date
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%d-%02d-%02d' WHERE parameter_id = 17 ", now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday);
	sent_query();
	// ---------------------------------------------------------------------------------------------------- OK above ---------------------------------------------------------------------------------------------------- //

	//optimization
	GLPK(interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_ot, uninterrupt_reot, uninterrupt_p, uninterrupt_flag, varying_start, varying_end, varying_ot, varying_reot, varying_flag, varying_t_pow, varying_p_pow, app_count, price, position);

	// =-=-=-=-=-=-=-=-=-=- update Pgrid & Pess & SOC to control status -=-=-=-=-=-=-=-=-=-= //
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 1, app_count + 1); //Pgrid
	sent_query();
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 2, app_count + 3); //Pess
	sent_query();
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 3, app_count + 7); //SOC
	sent_query();

	if ((sample_time + 1) != 97)
		same_day = 1;
	else
		same_day = 0;

	printf("LINE %d: sample_time = %d\n", __LINE__, sample_time);
	printf("LINE %d: next sample_time = %d\n\n", __LINE__, sample_time + 1);
	/*=========�ۤv��sample time +1================ sample time +1 =============================*/

	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%d' WHERE  parameter_id = 28", sample_time + 1);
	sent_query();

	mysql_close(mysql_con);
	return 0;
}

// ---------------------------------------------------------------------------------------------------------------------------------------------------- GLPK ---------------------------------------------------------------------------------------------------------------------------------------------------- //

void GLPK(int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_ot, int *uninterrupt_reot, float *uninterrupt_p, int *uninterrupt_flag, int *varying_start, int *varying_end, int *varying_ot, int *varying_reot, int *varying_flag, int **varying_t_pow, float **varying_p_pow, int app_count, float *price, int *position)
{
	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);

	int *buff = new int[app_count]; //(The number of remaining executions)
	for (i = 0; i < app_count; i++)
	{
		buff[i] = 0;
	}

	int noo;
	//get now time that can used in the real experiment
	if (((now_time.tm_min) % (60 / divide)) != 0)
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)); //divide=4
	else
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));

	printf("LINE %d: sample_noo:%d\n", __LINE__, noo);

	/*============================== ???����?�X}?C() ============================== */
	float *price2 = new float[time_block];
	//float *solar2 = new float[time_block];

	for (int x = 0; x < 24; x++) //(Transform the Price and PV power array from 24 to 96)
	{
		for (int y = x * divide; y < (x * divide) + divide; y++)
		{
			price2[y] = price[x];
		}
	}
	//insert fake data
	//float solar2[96] = { 0 };
	/*float solar2[96] = {					//case2 //X
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.030333,
	 	0.073, 0.086, 0.217667, 0.18,
	 	1.001333, 0.364, 0.110667, 0.264,
	 	0.25833, 0.255, 0.473, 0.661,
	 	0.702, 0.97133, 0.69667, 0.56433,
	 	1.47, 1.58333, 1.067667, 2.015333,
	 	1.871, 1.62233, 1.453667, 2.487,
	 	1.873667, 1.383, 1.968333, 1.819,
	 	1.27, 1.75433, 2.295667, 2.26033,
	 	1.832667, 1.92, 1.23933, 1.571333,
	 	1.439, 1.550667, 1.081667, 0.967667,
	 	0.913, 0.737667, 0.653, 0.52233,
	 	0.437, 0.25033, 0.19433, 0.13233,
	 	0.059667, 0.01667, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0,
	 	0.0, 0.0, 0.0, 0.0
	 };*/

	// case3
	float solar2[96] = {
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.030,
		0.073, 0.086, 0.217, 0.180,
		0.251, 0.364, 0.110, 0.264,
		0.258, 0.255, 0.473, 0.661,
		0.702, 0.971, 0.696, 0.564,
		1.470, 1.583, 1.067, 2.015,
		1.871, 1.622, 1.453, 2.487,
		1.873, 1.383, 1.968, 1.819,
		1.270, 1.754, 2.295, 2.260,
		1.832, 1.920, 1.240, 1.571,
		0.839, 0.550, 0.687, 0.900,
		0.880, 0.667, 0.311, 0.122,
		0.220, 0.250, 0.143, 0.152,
		0.059, 0.016, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0};

	// case1
	/*float solar2[96] = {			
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.003, 0.028, 0.170,
	0.343, 0.320, 0.250, 0.346,
	0.335, 0.346, 0.346, 0.342,
	0.344, 0.344, 0.346, 0.348,
	0.342, 0.348, 0.344, 0.348,
	0.346, 0.343, 0.342, 0.367,
	0.420, 0.528, 0.580, 0.676,
	0.689, 0.722, 0.751, 0.735,
	0.707, 0.746, 0.613, 0.543,
	0.688, 0.738, 0.639, 0.555,
	0.426, 0.378, 0.280, 0.215,
	0.132, 0.095, 0.081, 0.068,
	0.035, 0.023, 0.010, 0.008,
	0.0, 0.005, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0
	};*/

	/*float solar2[96] = {
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.003, 0.034, 0.086, 0.139,
	0.217, 0.335, 0.434, 0.626,
	0.801, 1.000, 1.227, 1.295,
	1.430, 1.040, 1.170, 1.680,
	1.600, 2.200, 2.725, 2.660,
	3.100, 1.992, 2.538, 2.800,
	2.460, 2.680, 3.200, 3.000,
	2.940, 3.000, 2.410, 2.238,
	1.907, 1.615, 0.984, 0.870,
	0.567, 0.584, 1.161, 1.100,
	1.341, 0.870, 0.943, 0.725,
	0.769, 0.563, 0.516, 0.351,
	0.352, 0.291, 0.218, 0.146,
	0.088, 0.030, 0.002, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0,
	0.0, 0.0, 0.0, 0.0
	};*/
	// Z1
	// float Pgrid_max[96] = {
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	0.9232, 0.4084, 0.553, 0.548,
	// 	3.1598, 3.1464, 3.103, 3.1656,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2,
	// 	3.2, 3.2, 3.2, 3.2};

	/*float Pgrid_max[96] ={      //�ݶq�v����bi-di  Z2
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		0.27696, 0.12252, 0.1659, 0.1644, 
		0.63196, 0.62928, 0.6206, 0.63312,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2,
		3.2, 3.2, 3.2, 3.2
	};*/

	if (sample_time == 0)
	{
		for (i = 0; i < time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE solar_day SET value =%.3f WHERE  time_block =%d", solar2[i], i);
			sent_query();
		}
	}
	else
	{
		for (i = 0; i < 96; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM solar_day WHERE time_block = %d", i);
			solar2[i] = turn_value_to_float(0);
		}
	}

	/*=========================(Check the uninterrupt load whether it have been run)=============================*/ //!!�_�W�ɶ��ܼƷ|��
	int flag = 0;

	if (sample_time != 0)
	{
		for (i = 0; i < uninterrupt_num; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE equip_id = '%d'", column, (i + variable - varying_num * 2 - uninterrupt_num + 1));
			for (int j = 0; j < sample_time; j++)
			{
				flag += turn_value_to_int(j);
			}
			uninterrupt_flag[i] = flag;
		}
		for (i = 0; i < varying_num; i++)
		{
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE equip_id = '%d'", column, (i + variable - varying_num * 2 + 1));
			for (int j = 0; j < sample_time; j++)
			{
				flag += turn_value_to_int(j);
			}
			varying_flag[i] = flag;
		}
	}

	/*====================(Get the number of times the device was executed before sample_time)=======================*/
	int coun = 0;

	if (sample_time != 0)
	{
		for (i = 1; i <= app_count; i++)
		{
			coun = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			for (int j = 0; j < sample_time; j++)
			{
				coun += turn_value_to_int(j);
			}
			buff[i - 1] = coun;
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}
	}
	/*============================(Calculate the remaining execution time of appliances)==============================*/
	for (i = 0; i < interrupt_num; i++) //(Interrupt load)
	{
		if ((interrupt_ot[i] - buff[i]) == interrupt_ot[i])
		{
			interrupt_reot[i] = interrupt_ot[i];
		}
		else if (((interrupt_ot[i] - buff[i]) < interrupt_ot[i]) && ((interrupt_ot[i] - buff[i]) > 0))
		{
			interrupt_reot[i] = interrupt_ot[i] - buff[i];
		}
		else if ((interrupt_ot[i] - buff[i]) <= 0)
		{
			interrupt_reot[i] = 0;
		}
	}
	for (j = 0; j < uninterrupt_num; j++)
	{
		if (uninterrupt_flag[j] == 0) //(Uninterrupted load has not yet started)
		{
			uninterrupt_reot[j] = uninterrupt_ot[j];
		}
		if (uninterrupt_flag[j] == 1) //(Uninterrupted load is started (modify load start time))
		{
			if (((uninterrupt_ot[j] - buff[j + interrupt_num]) < uninterrupt_ot[j]) && ((uninterrupt_ot[j] - buff[j + interrupt_num]) > 0))
			{
				uninterrupt_reot[j] = uninterrupt_ot[j] - buff[j + interrupt_num];
				if (uninterrupt_reot[j] != 0)
				{
					uninterrupt_end[j] = sample_time + uninterrupt_reot[j] - 1;
				}
			}
			else if ((uninterrupt_ot[j] - buff[j + interrupt_num]) <= 0)
			{
				uninterrupt_reot[j] = 0;
			}
		}
	}
	for (k = 0; k < varying_num; k++)
	{
		if (varying_flag[k] == 0) //(Varying load has not yet started)
		{
			varying_reot[k] = varying_ot[k];
		}
		if (varying_flag[k] == 1) //(Varying load is started (modify load start time))
		{
			if (((varying_ot[k] - buff[k + interrupt_num + uninterrupt_num]) < varying_ot[k]) && ((varying_ot[k] - buff[k + interrupt_num + uninterrupt_num]) > 0))
			{
				varying_reot[k] = varying_ot[k] - buff[k + interrupt_num + uninterrupt_num];
				if (varying_reot[k] != 0)
				{
					varying_end[k] = sample_time + varying_reot[k] - 1;
				}
			}
			else if ((varying_ot[k] - buff[k + interrupt_num + uninterrupt_num]) <= 0)
			{
				varying_reot[k] = 0;
			}
		}
	}
	/*=========================(Structure initialization of varying load)=============================*/

	int **varying_t_d;
	float **varying_p_d;
	for (i = 0; i < varying_num; i++)
	{
		varying_t_d = NEW2D(varying_num, (time_block - sample_time), int);
		varying_p_d = NEW2D(varying_num, varying_ot[i], float);
		for (m = 0; m < (time_block - sample_time); m++)
		{
			varying_t_d[i][m] = 0;
		}
		for (m = 0; m < varying_ot[i]; m++)
		{
			varying_p_d[i][m] = 0.0;
		}
	}
	/*========================(Energy consumption model of varying load)==============================*/
	for (i = 0; i < varying_num; i++)
	{
		for (j = 0; j < varying_t_pow[i][0]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][0];
		}
		for (j = varying_t_pow[i][0]; j < varying_t_pow[i][0] + varying_t_pow[i][1]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][1];
		}
		for (j = varying_t_pow[i][0] + varying_t_pow[i][1]; j < varying_t_pow[i][0] + varying_t_pow[i][1] + varying_t_pow[i][2]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][2];
		}
	}

	/*========================(Start and end period variable of varying load) =========================== */
	for (i = 0; i < varying_num; i++)
	{
		if ((varying_end[i] - sample_time) >= 0)
		{
			if ((varying_start[i] - sample_time) >= 0)
			{
				for (m = (varying_start[i] - sample_time); m <= (varying_end[i] - sample_time); m++)
				{
					varying_t_d[i][m] = 1;
				}
			}
			else if ((varying_start[i] - sample_time) < 0)
			{
				for (m = 0; m <= (varying_end[i] - sample_time); m++)
				{
					varying_t_d[i][m] = 1;
				}
			}
		}
	}

	/*========================(maximum power of varying load) =========================== */
	double *varying_p_max = new double[varying_num];
	for (i = 0; i < varying_num; i++)
	{
		varying_p_max[i] = 0.0;

		for (j = 0; j < 3; j++)
		{
			if (varying_p_pow[i][j] > varying_p_max[i])
			{
				varying_p_max[i] = varying_p_pow[i][j];
			}
		}
	}

	/*========================(Define appliances for individual time variables matrix)==============================*/
	//	float *s = new float[time_block - sample_time];
	float *s = new float[time_block];
	/*============================(Total planning power matrix)====================================*/
	float **power1 = NEW2D((((time_block - sample_time) * 200) + app_count + 1), (variable * (time_block - sample_time)), float);

	/*============================(GLPK parameter matrix definition)==================================*/
	glp_prob *mip;																												   //200�u�O�O�d�@�Ӥj�@�I���Ŷ��A+1�OSOC���e�Ȫ����
	int *ia = new int[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1];	   //Row
	int *ja = new int[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1];	   //Column
	double *ar = new double[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1]; //structural variable

	/*=============================(GLPK variable definition)=====================================*/
	mip = glp_create_prob();
	glp_set_prob_name(mip, "hardware_algorithm_case");
	glp_set_obj_dir(mip, GLP_MIN); //�̤p�ƥιq��O
	glp_add_rows(mip, (((time_block - sample_time) * 200) + app_count + 1));
	glp_add_cols(mip, (variable * (time_block - sample_time)));
	/*=============================(initial the matrix)======================================*/
	for (m = 0; m < (((time_block - sample_time) * 200) + app_count + 1); m++)
	{
		for (n = 0; n < (variable * (time_block - sample_time)); n++)
		{
			power1[m][n] = 0.0;
		}
	}

	/*============================(Constraints coefficient)==========================================*/ //!!����n���
	//??(Household load minimum energy consumption)
	for (h = 0; h < interrupt_num; h++) //(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i * variable + h] = 1.0;
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i * variable + h] = 1.0;
				}
			}
		}
	}

	//(Decide whether to buy electricity from utility)
	// 0 < Pgrid j < Pgrid max
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[app_count + i][i * variable + app_count] = 1.0;
	}

	//(Balanced function) Pgrid j - Pess j + Ppv j = sum(Pa j)
	// Pa j
	for (h = 0; h < interrupt_num; h++) //(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h] = interrupt_p[h];
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h] = interrupt_p[h];
				}
			}
		}
	}
	for (h = 0; h < uninterrupt_num; h++) //(uninterrupt load)
	{
		if ((uninterrupt_end[h] - sample_time) >= 0)
		{
			if ((uninterrupt_start[h] - sample_time) >= 0)
			{
				for (i = (uninterrupt_start[h] - sample_time); i <= (uninterrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
			else if ((uninterrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
		}
	}
	for (h = 0; h < varying_num; h++) //(varying load)
	{
		if ((varying_end[h] - sample_time) >= 0)
		{
			if ((varying_start[h] - sample_time) >= 0)
			{
				for (i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h + (variable - varying_num)] = 1.0;
				}
			}
			else if ((varying_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (varying_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + h + (variable - varying_num)] = 1.0;
				}
			}
		}
	}
	// Pgrid j
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 2 + app_count + 1 + i][i * variable + app_count] = -1.0;
	}

	//(Uninterrupted load of auxiliary variables), sum = 1
	// sum(δa j) = 1 (uninterrupt loads)
	int counter = 0;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)					 //(When the uninterruptible load is not started, use the original way to fill the matrix)
		{												 //?Y?w��?�X??N��j??��N��??{�X_?????? ��q "start????" ?? "start???? + ???U?�P�X???????"
			if ((uninterrupt_end[h] - sample_time) >= 0) //If it is already started, it will force the schedule to start from "start time" to "start time + left to be executed"
			{
				if ((uninterrupt_start[h] - sample_time) >= 0)
				{
					for (i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * 6 + app_count + 1 + counter][i * variable + h + (variable - uninterrupt_num - (varying_num * 2))] = 1.0;
					}
				}
				else if ((uninterrupt_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * 6 + app_count + 1 + counter][i * variable + h + (variable - uninterrupt_num - (varying_num * 2))] = 1.0;
					}
				}
			}
			counter += 1;
		}
	}
	//(Varying load of auxiliary variables), sum = 1
	// sum(δa j) = 1 (varying loads)
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)					 //(When the varying load is not started, use the original way to fill the matrix)
		{											 //?Y?w��?�X??N��j??��N��??{�X_?????? ��q "start????" ?? "start???? + ???U?�P�X???????"
			if ((varying_end[h] - sample_time) >= 0) ////If it is already started, it will force the schedule to start from "start time" to "start time + left to be executed"
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * 6 + app_count + 1 + counter][i * variable + h + (variable - (varying_num * 2))] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * 6 + app_count + 1 + counter][i * variable + h + (variable - (varying_num * 2))] = 1.0;
					}
				}
			}
			counter += 1;
		}
	}

	// ra j+n >= δa j (uninterrupt loads)
	n = 0;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			for (k = (6 + n), m = 0; k < (6 + n) + uninterrupt_reot[h], m < uninterrupt_reot[h]; k++, m++)
			{
				if ((uninterrupt_end[h] - sample_time) >= 0)
				{
					if ((uninterrupt_start[h] - sample_time) >= 0)
					{
						for (i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i + m) * variable + h + interrupt_num] = 1.0;								  //???i???_?t???M??????
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][i * variable + h + (variable - uninterrupt_num - (varying_num * 2))] = -1.0; //???i???_?t???G????��U????
						}
					}
					else if ((uninterrupt_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i + m) * variable + h + interrupt_num] = 1.0;								  //???i???_?t???M??????
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][i * variable + h + (variable - uninterrupt_num - (varying_num * 2))] = -1.0; //???i???_?t???G????��U????
						}
					}
				}
			}
			n += uninterrupt_reot[h];
		}
		if (uninterrupt_flag[h] == 1)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - sample_time) <= 0)
				{
					for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][i * variable + h + interrupt_num] = 1.0; //???i???_?t???M??????
					}
				}
				n += 1;
			}
		}
	}
	// ra j+n >= δa j (varying loads)
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0) //?b??�X??t????��?�X???
		{
			for (k = (6 + n), m = 0; k < (6 + n) + varying_reot[h], m < varying_reot[h]; k++, m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i + m) * variable + h + (interrupt_num + uninterrupt_num)] = 1.0;
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][i * variable + h + (variable - (varying_num * 2))] = -1.0;
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i + m) * variable + h + (interrupt_num + uninterrupt_num)] = 1.0;
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][i * variable + h + (variable - (varying_num * 2))] = -1.0;
						}
					}
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) <= 0)
				{
					for (i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][i * variable + h + (interrupt_num + uninterrupt_num)] = 1.0;
					}
				}
				n += 1;
			}
		}
	}

	// ψa j+n  >= δa j * σa n
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (k = (6 + n), m = 0; k < (6 + n) + varying_reot[h], m < varying_reot[h]; k++, m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i * variable) + h + (variable - (varying_num * 2))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][((i + m) * variable) + h + (variable - varying_num)] = 1.0; // ψa j+n
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][(i * variable) + h + (variable - (varying_num * 2))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							power1[(time_block - sample_time) * k + app_count + 1 + counter + i][((i + m) * variable) + h + (variable - varying_num)] = 1.0; // ψa j+n
						}
					}
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			//h?X?W
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][(i * variable) + h + interrupt_num + uninterrupt_num] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][(i * variable) + h + (variable - varying_num)] = 1.0; // ψa j+n
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][(i * variable) + h + interrupt_num + uninterrupt_num] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						power1[(time_block - sample_time) * (6 + n) + app_count + 1 + counter + i][(i * variable) + h + (variable - varying_num)] = 1.0; // ψa j+n
					}
				}
			}
			n += 1;
		}
	}

	/*==============================(row)===============================*/
	for (i = 1; i <= interrupt_num; i++) //(Interrupt load)
	{
		glp_set_row_name(mip, i, "");
		glp_set_row_bnds(mip, i, GLP_LO, ((float)interrupt_reot[i - 1]), 0.0); //ok
	}

	// 0 < Pgrid j < Pgrid max
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, (app_count + i), "");
		glp_set_row_bnds(mip, (app_count + i), GLP_UP, 0.0, Pgrid_max);
	}

	//(Balanced function) Pgrid j - Pess j + Ppv j = sum(Pa j)
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 2 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 2 + app_count + 1 + i), GLP_FX, 0.0, 0.0);
		// glp_set_row_bnds(mip, ((time_block - sample_time) * 2 + app_count + 1 + i), GLP_DB, -0.0001, solar2[i - 1 + sample_time]);
	}

	// sum(δa j) = 1 (uninterrupt loads)
	counter = 1;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			glp_set_row_name(mip, ((time_block - sample_time) * 6 + app_count + 1 + counter), "");
			glp_set_row_bnds(mip, ((time_block - sample_time) * 6 + app_count + 1 + counter), GLP_FX, 1.0, 1.0);

			counter += 1;
		}
	}
	// sum(δa j) = 1 (varying loads)
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			//??�X??t???��??��U????, sum = 1
			glp_set_row_name(mip, ((time_block - sample_time) * 6 + app_count + 1 + counter), "");
			glp_set_row_bnds(mip, ((time_block - sample_time) * 6 + app_count + 1 + counter), GLP_FX, 1.0, 1.0);

			counter += 1;
		}
	}
	// ra j+n >= δa j (uninterrupt loads)
	n = 0;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			//???i???_?t???M??????
			for (k = (6 + n); k < (6 + n) + uninterrupt_reot[h]; k++)
			{
				for (i = ((time_block - sample_time) * k + app_count + 1 + counter); i < ((time_block - sample_time) * (k + 1) + app_count + 1 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
			}
			n += uninterrupt_reot[h];
		}
		if (uninterrupt_flag[h] == 1)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				for (i = ((time_block - sample_time) * (6 + n) + app_count + 1 + counter); i < ((time_block - sample_time) * (6 + n) + app_count + 1 + counter + uninterrupt_reot[h]); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 1.0, 1.0);
				}
				for (i = ((time_block - sample_time) * (6 + n) + app_count + 1 + counter + uninterrupt_reot[h]); i < ((time_block - sample_time) * ((6 + n) + 1) + app_count + 1 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
				n += 1;
			}
		}
	}
	// ra j+n >= δa j (varying loads)
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{

			for (k = (6 + n); k < (6 + n) + varying_reot[h]; k++)
			{
				for (i = ((time_block - sample_time) * k + app_count + 1 + counter); i < ((time_block - sample_time) * (k + 1) + app_count + 1 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			for (i = ((time_block - sample_time) * (6 + n) + app_count + 1 + counter); i < (((time_block - sample_time) * (6 + n) + app_count + 1 + counter) + varying_reot[h]); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 1.0, 1.0);
			}
			for (i = (((time_block - sample_time) * (6 + n) + app_count + 1 + counter) + varying_reot[h]); i < ((time_block - sample_time) * ((6 + n) + 1) + app_count + 1 + counter); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
			}
			n += 1;
		}
	}

	// ψa j+n  >= δa j * σa n
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (k = (6 + n); k < (6 + n) + varying_reot[h]; k++)
			{
				for (i = ((time_block - sample_time) * k + app_count + 1 + counter); i < ((time_block - sample_time) * (k + 1) + app_count + 1 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			for (i = ((time_block - sample_time) * (6 + n) + app_count + 1 + counter); i < ((time_block - sample_time) * ((6 + n) + 1) + app_count + 1 + counter); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
			}
			n += 1;
		}
	}

	/*==============================??��i?M??????(column)================================*/ //!!�ܼƭn���
	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (j = 1; j <= app_count; j++)
		{
			glp_set_col_bnds(mip, (j + i * variable), GLP_DB, 0.0, 1.0); // ra
			glp_set_col_kind(mip, (j + i * variable), GLP_BV);
		}
		glp_set_col_bnds(mip, ((app_count + 1) + i * variable), GLP_DB, 0.0, Pgrid_max); //Pgrid
		glp_set_col_kind(mip, ((app_count + 1) + i * variable), GLP_CV);

		for (j = 1; j <= uninterrupt_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 1 + j) + i * variable), GLP_DB, 0.0, 1.0); //???i???_?t????��U?G??????
			glp_set_col_kind(mip, ((app_count + 1 + j) + i * variable), GLP_BV);
		}

		for (j = 1; j <= varying_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 1 + uninterrupt_num + j) + i * variable), GLP_DB, 0.0, 1.0); //??�X??t????��U?G??????
			glp_set_col_kind(mip, ((app_count + 1 + uninterrupt_num + j) + i * variable), GLP_BV);
		}

		for (j = 1; j <= varying_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 1 + uninterrupt_num + varying_num + j) + i * variable), GLP_DB, 0.0, varying_p_max[j - 1]); //??�X??t??��?��?????Pa
			glp_set_col_kind(mip, ((app_count + 1 + uninterrupt_num + varying_num + j) + i * variable), GLP_CV);
		}
	}

	/*==============================??��i??????�X???(column)===============================*/
	for (j = 0; j < (time_block - sample_time); j++)
	{
		glp_set_obj_coef(mip, ((app_count + 1) + j * variable), price2[j + sample_time] * delta_T);
	}
	/*==============================GLPK?g?J��x�X}(ia,ja,ar)===============================*/
	for (i = 0; i < (((time_block - sample_time) * 200) + app_count + 1); i++)
	{
		for (j = 0; j < (variable * (time_block - sample_time)); j++)
		{
			ia[i * ((time_block - sample_time) * variable) + j + 1] = i + 1;
			ja[i * ((time_block - sample_time) * variable) + j + 1] = j + 1;
			ar[i * ((time_block - sample_time) * variable) + j + 1] = power1[i][j];
		}
	}
	/*==============================GLPK????????��x�X}====================================*/
	glp_load_matrix(mip, (((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time)), ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);

	parm.tm_lim = 240000;
	parm.presolve = GLP_ON;
	//not cloudy
	//parm.ps_heur = GLP_ON;
	//parm.bt_tech = GLP_BT_BPH;
	//parm.br_tech = GLP_BR_PCH;

	//cloud
	parm.gmi_cuts = GLP_ON;
	parm.ps_heur = GLP_ON;
	parm.bt_tech = GLP_BT_BFS;
	parm.br_tech = GLP_BR_PCH;

	//no fc+ no sell
	//fc+no sell
	// parm.gmi_cuts = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	// parm.br_tech = GLP_BR_PCH;

	//FC+sell
	//parm.fp_heur = GLP_ON;
	// parm.bt_tech = GLP_BT_BPH;
	//parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);

	z = glp_mip_obj_val(mip);
	printf("\n");
	printf("sol = %f; \n", z);

	if (z == 0.0)
	{
		printf("Error > sol is 0, No Solution, give up the solution\n");
		system("pause");
		//exit(1);
	}

	/*==============================��N?M?????????G???X==================================*/
	int l = 0;

	//get now time that can used in the real experiment
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	else
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	// printf("LINE %d: noo_sample = %d\n", __LINE__, noo);

	for (i = 1; i <= variable; i++)
	{
		h = i;
		l = variable - (app_count - i);
		if (sample_time == 0)
		{
			for (j = 0; j < time_block; j++)
			{

				s[j] = glp_mip_col_val(mip, h);

				// if (i <= app_count && j== noo)
				// {
				// 	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i-1]);
				// 	mysql_query(mysql_con, sql_buffer);
				// 	snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_history (id,status,schedule) VALUES(%d,%d,%d)", position[i - 1], (int)s[j], 1);
				// 	mysql_query(mysql_con, sql_buffer);
				// }

				// =-=-=-=-=-=-=-=-=-=- table 'now status' is showing loads on off status, doing if loop when interrupt and uninterrupt load, else if loop when varying load -=-=-=-=-=-=-=-=-=-= //
				if (i <= interrupt_num + uninterrupt_num && j == noo)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i - 1]);
					sent_query();
				}
				else if ((i > interrupt_num + uninterrupt_num) && (i <= app_count)) //sometimes varying load will have weird
				{
					s[j] = glp_mip_col_val(mip, l);
					// printf("%d. variable:%d  value:%f\n",j,i,s[j]);
					if (s[j] > 0.0)
						s[j] = 1.0;
					if (j == noo)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i - 1]);
						sent_query();
					}
				}

				l = (l + variable);
				h = (h + variable);
			}
			// =-=-=-=-=-=-=-=-=-=- update each variables's A0 ~ A95 in each for loop -=-=-=-=-=-=-=-=-=-= //
			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');", column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			sent_query();
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}

		if (sample_time != 0)
		{
			// =-=-=-=-=-=-=-=-=-=- history about the control status from each control id -=-=-=-=-=-=-=-=-=-= //
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			for (int k = 0; k < sample_time; k++)
			{
				s[k] = turn_value_to_float(k);
			}
			memset(sql_buffer, 0, sizeof(sql_buffer));
			// =-=-=-=-=-=-=-=-=-=- change new result after the sample time -=-=-=-=-=-=-=-=-=-= //
			printf("UPDATE now_status (load status)\nposition\t\n");
			for (i = 0; i < sizeof(position) / sizeof(position[0]); i++)
				printf("%d\t", position[i]);
			printf("\nstatus\t\n");
			for (j = 0; j < (time_block - sample_time); j++)
			{
				s[j + sample_time] = glp_mip_col_val(mip, h);

				if (i <= interrupt_num + uninterrupt_num && j == 0)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j + sample_time], position[i - 1]);
					printf("%d\t", (int)s[j + sample_time]);
					sent_query();
				}
				else if ((i > interrupt_num + uninterrupt_num) && (i <= app_count))
				{
					s[j + sample_time] = glp_mip_col_val(mip, l);
					if (s[j + sample_time] > 0.0)
						s[j + sample_time] = 1.0;

					if (j == 0)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j + sample_time], position[i - 1]);
						printf("%d\t", (int)s[j + sample_time]);
						sent_query();
					}
				}
				l = (l + variable);
				h = (h + variable);
			}
			// =-=-=-=-=-=-=-=-=-=- full result update -=-=-=-=-=-=-=-=-=-= //
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE equip_id = '%d';", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			sent_query();
			memset(sql_buffer, 0, sizeof(sql_buffer));

			// =-=-=-=-=-=-=-=-=-=- result update from the sample time until end timeblock (96) -=-=-=-=-=-=-=-=-=-= //
			for (j = 0; j < sample_time; j++)
			{
				s[j] = 0;
			}
			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO real_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');", column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			sent_query();
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}
	}
	printf("\n");

	glp_delete_prob(mip);
	delete[] ia, ja, ar, s;
	delete[] power1;
	return;
}
