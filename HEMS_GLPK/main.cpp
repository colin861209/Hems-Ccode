#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include "HEMS.h" 

//void GLPK(int *);
//common variable
int RT_enable = 0;
int h, i, j, k, m, n = 0;
double z = 0;

// common parameter
int time_block = 0, app_count = 0, variable = 0, divide = 0, sample_time = 0, sa_counter = 0, rasa_counter; float delta_T = 0.0;
float Cbat = 0.0, Vsys = 0.0, SOC_ini = 0.0, SOC_min = 0.0, SOC_max = 0.0, SOC_thres = 0.0, Pbat_min = 0.0, Pbat_max = 0.0, Pgrid_max = 0.0, Psell_max, Delta_battery = 0.0, Pfc_max = 0.0;
float step1_bill = 0.0, step1_sell = 0.0, step1_PESS = 0.0;	
// app parameter		
// app parameter		
int interrupt_num = 0, uninterrupt_num = 0, varying_num = 0, ponit_num = 0;
int stop_enable = 0;
int stop_enable = 0;
char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";

MYSQL *mysql_con = mysql_init(NULL);
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;

int main(void)
{
	/*============================������e�t�ήɶ�==================================*/
	//vs2015
	 // time_t t = time(NULL);
	 // struct tm now_time;
	 // localtime_s(&now_time, &t);
	//linux
	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);
	RT_enable = 0;

	//MARK: one_day scheduling MYSQL read PARM
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;

	int same_day = 0, real_time = 0;
	int same_day = 0, real_time = 0;
	char *s_time = new char[3];
	int *position = new int[16];

	//if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "realtime", 7781, NULL, 0)) == NULL)
	if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "realtime", 6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		system("pause");
		return 0;
	}
	printf("Connect to Mysql sucess!!\n");
	mysql_set_character_set(mysql_con, "utf8");
	//VS2015
	//char sql_buffer[1024] = { NULL };
	//linux
	char sql_buffer[1024] = { '\0' };


	//if just want only run realtime 24 hour.. otherwise you can commend it
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 27");//get now_SOC
	//if just want only run realtime 24 hour.. otherwise you can commend it
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 27");//get now_SOC
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	stop_enable =atoi(mysql_row[0]);
	if (stop_enable == 1)
	{
		printf("A day is over so force to close program....\n");
		return -1;
	}


	//SELECT COLUMN_NAME FROM `COLUMNS` WHERE `COLUMN_NAME` BETWEEN 'A0' AND 'A95'
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	stop_enable =atoi(mysql_row[0]);
	if (stop_enable == 1)
	{
		printf("A day is over so force to close program....\n");
		return -1;
	}


	//SELECT COLUMN_NAME FROM `COLUMNS` WHERE `COLUMN_NAME` BETWEEN 'A0' AND 'A95'
	// ------------------------------- Start to Get value from load_list & LP_BASE_PARM than Update value back ------------------------------- //
	// MARK: each of loads number in table 'load_list'
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 "); // 可中斷 interrupt_num = 12
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	interrupt_num = atoi(mysql_row[0]);
	mysql_free_result(mysql_result);
	printf("interruptable app num:%d\n", interrupt_num);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=2 "); // 不可中斷 uninterrupt_num = 2
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	uninterrupt_num = atoi(mysql_row[0]);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=3 "); // 變動型 uninterrupt_num = 1
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	varying_num = atoi(mysql_row[0]);
	mysql_free_result(mysql_result);
	printf("variable app num:%d\n", varying_num);

	// MARK: s_time => last time execute 2020-0x-0x => time_tmp[3] = {2020, 03, 06} 
		{ base_par[i - 1] = atof(mysql_row[0]);	}
	//  	if different,then enforce start up one_schedule => same day = 0 (一日排程)
	for (i = 1; i <= 17; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "select value from LP_BASE_PARM where parameter_id = '%d'", i);
		mysql_query(mysql_con, sql_buffer);
	//check whether same day or not(if different,then enforce start up one_schedule)
		mysql_result = mysql_store_result(mysql_con);
	 // char *p;
	 // char *token = strtok_s(s_time, "-", &p);
	 // int time_tmp[3], tt = 0;
	 // while (token != NULL)
	 // {
	 // 	time_tmp[tt] = atoi(token);
	 // 	token = strtok_s(NULL, "-", &p);
	 // 	tt++;
	 // }
	 // char *p;
	//linux
	 // char *token = strtok_s(s_time, "-", &p);
	 // int time_tmp[3], tt = 0;
	 // while (token != NULL)
	 // {
	 // 	time_tmp[tt] = atoi(token);
	 // 	token = strtok_s(NULL, "-", &p);
	 // 	tt++;
	 // }

	//linux
	char *token = strtok(s_time, "-");
	int time_tmp[3], tt = 0;
	while (token != NULL)
	{
		time_tmp[tt] = atoi(token);
	{same_day = 1;}	
		tt++;
	//send parameter to variale number
	time_block = base_par[0]; // = 96 
	app_count = interrupt_num + uninterrupt_num + varying_num;	// = 12+2+1 = 15
	ponit_num = 6;
	variable = app_count + 9 + uninterrupt_num + (varying_num * 2) + 2 + 2 * (ponit_num - 1) + 3;	// 15+9+2+(1*2)+2+2*(6-1)+3 = 43
		real_time = 0;
	}
	Vsys = base_par[5]; // = 48
	{same_day = 1;}	

	//send parameter to variale number
	time_block = base_par[0]; // = 96 
	app_count = interrupt_num + uninterrupt_num + varying_num;	// = 12+2+1 = 15
	ponit_num = 6;
	Pbat_max = 1.53;
	Pbat_min = 1.53;
	//Pfc_max = 5.13;
	Pfc_max = 0.0;
	Cbat = base_par[6];	// = 0.318
	Pgrid_max = base_par[12];	
	Psell_max = base_par[13];	
	real_time = (int)base_par[14];	
	Pbat_max = 1.53;
	Pbat_min = 1.53;
	//Pfc_max = 5.13;
	Pfc_max = 0.0;

	Pgrid_max = base_par[12];	
	Psell_max = base_par[13];	
	real_time = (int)base_par[14];	
	/*=============================get sample time=============================*/
	 
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		sample_time = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) ;  
	}
	else
	{
		sample_time = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) ;
	}
	printf("sample_time:%d\n", sample_time);


	base_par[1] = app_count;
	base_par[2] = interrupt_num;
	base_par[3] = uninterrupt_num;
	base_par[4] = varying_num;
	}

	/*=============================get sample time=============================*/
	 
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		sample_time = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) ;  
	}
	else
	{
		sample_time = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) ;
	}
	printf("sample_time:%d\n", sample_time);


	for (i = 1; i <= 13; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%f' WHERE  PARM_id = '%d'", base_par[i - 1], i);
		mysql_query(mysql_con, sql_buffer);
	}
	printf("************system state************* \n");
	printf("time block:%d\n", time_block);
	printf("interrupt numbers:%d\n", interrupt_num);
	printf("uninterrupt numbers:%d\n", uninterrupt_num);
	printf("varying numbers:%d\n", varying_num);
	printf("variable numbers:%d\n", variable);
	printf("app counts:%d\n", app_count);
	printf("system voltage:%.1f\n", Vsys);
	printf("battery capacity:%.3f\n", Cbat);
	printf("SOC min:%.2f\n", SOC_min);
	printf("SOC max:%.2f\n", SOC_max);
	printf("SOC threads:%.2f\n", SOC_thres);
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
	printf("Pbat max:%.3f\n", Pbat_max);
	printf("Pgrid max:%.2f\n", Pgrid_max);
	printf("Psell max:%.2f\n", Psell_max);
	printf("Last run date:%d-%02d-%02d\n", time_tmp[0], time_tmp[1], time_tmp[2]);
	printf("User set for realtime(0->no 1->yes):%d\n", real_time);
	printf("Last running were same day(0->no 1->yes):%d\n", same_day);
	printf("\n");
	// ------------------------------- END of Get value from load_list & LP_BASE_PARM than Update value back ------------------------------- //

	// ------------------------------- Start to Put Mysql data into Matrix ------------------------------- //
	float *price = new float[24];
	float **INT_power = NEW2D(interrupt_num, 4, float);	// INT_power[interrupt_num][4]
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 1 ORDER BY number ASC LIMIT %d,1", i - 1);
	float **VAR_power = NEW2D(varying_num, 9, float);	// VAR_power[varying_num][4]

	// MARK: get all application(interrupt-> uninterrupt-> varying) into array position
	for (i = 0; i < app_count; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		position[i] = atoi(mysql_row[j]);
		//printf("xxxxxxx:%d\n", position[i]);
		mysql_free_result(mysql_result);
	}
	for (i = 1; i < uninterrupt_num + 1; i++)
	// MARK: get each interrupt application column value into multidimesional array INT_power
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 2 ORDER BY number ASC LIMIT %d,1", i - 1);
	for (i = 1; i < interrupt_num + 1; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 1 ORDER BY number ASC LIMIT %d,1", i - 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < 4; j++)
		{
			INT_power[i - 1][j] = atof(mysql_row[j]);
			//	printf("%.2f    ", INT_power[i - 1][j]);
		}
		//printf("\n");
		mysql_free_result(mysql_result);
	for (i = 1; i < varying_num + 1; i++)   //�ܰʫ�


		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time,power1,power2,power3,block1,block2,block3 FROM load_list WHERE group_id = 3 ORDER BY number ASC LIMIT %d,1", i - 1);
	for (i = 1; i < uninterrupt_num + 1; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time,operation_time ,power1 FROM load_list WHERE group_id = 2 ORDER BY number ASC LIMIT %d,1", i - 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < 4; j++)
		{
			UNINT_power[i - 1][j] = atof(mysql_row[j]);
			//printf("%.2f    ", UNINT_power[i - 1][j]);
		}
		//printf("\n");
		mysql_free_result(mysql_result);
	}

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT price_value FROM price WHERE price_period = %d", i - 1);
	for (i = 1; i < varying_num + 1; i++)   //�ܰʫ�
	{

		price[i - 1] = atof(mysql_row[0]);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);

		for (j = 0; j < 9; j++)
		{
			VAR_power[i - 1][j] = atof(mysql_row[j]);
			//printf("%.2f    ", VAR_power[i - 1][j]);
		}
		//printf("\n");
		mysql_free_result(mysql_result);
	}

	// MARK: get each hour price value into array price
	for (i = 1; i < 25; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT price_value FROM price WHERE price_period = %d", i - 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		price[i - 1] = atof(mysql_row[0]);
	int *uninterrupt_flag = new int[uninterrupt_num];			
		mysql_free_result(mysql_result);
	}

	// MARK: interrupt_num = 12
	// 	reot: remain operation time
	//  flag: for the application won't close after open 
	//  t_pow: the varying application each timeblock
	//  p_pow: the varying application each power
	int *interrupt_start = new int[interrupt_num];
	int *interrupt_end = new int[interrupt_num];
	int *interrupt_ot = new int[interrupt_num];
	int *interrupt_reot = new int[interrupt_num];
	float *interrupt_p = new float[interrupt_num];

	// MARK: uninterrupt_num = 2
	int *uninterrupt_start = new int[uninterrupt_num];
	int *uninterrupt_end = new int[uninterrupt_num];
	int *uninterrupt_ot = new int[uninterrupt_num];
	int *uninterrupt_reot = new int[uninterrupt_num];
	float *uninterrupt_p = new float[uninterrupt_num];
	int *uninterrupt_flag = new int[uninterrupt_num];			

	// MARK: varying_num = 1
	int *varying_start = new int[varying_num];
	int *varying_end = new int[varying_num];
	int *varying_ot = new int[varying_num];
	int *varying_reot = new int[varying_num];
	int **varying_t_pow = NEW2D(varying_num, 3, int);
	float **varying_p_pow = NEW2D(varying_num, 3, float);
	int *varying_flag = new int[varying_num];


	// initialize all matrix
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

		uninterrupt_end[j] = 0;
		uninterrupt_ot[j] = 0;
		uninterrupt_reot[j] = 0;
		interrupt_start[i] = ((int)(INT_power[i][0] * divide));	
		interrupt_end[i] = ((int)(INT_power[i][1] * divide)) - 1;
	}
	for (k = 0; k < varying_num; k++)
	{
		varying_start[k] = 0;
		varying_end[k] = 0;
		varying_ot[k] = 0;
		varying_reot[k] = 0;
		varying_flag[k] = 0;
		uninterrupt_end[i] = ((int)(UNINT_power[i][1] * divide)) - 1;
		{
			varying_t_pow[k][j] = 0;
			varying_p_pow[k][j] = 0.0;

		}
	}


	// All the time has been converted to 96blocks, so multiply "divide". 
		varying_start[i] = ((int)(VAR_power[i][0] * divide));	// varying_start[0] = 9
	{
		interrupt_start[i] = ((int)(INT_power[i][0] * divide));	
		interrupt_end[i] = ((int)(INT_power[i][1] * divide)) - 1;
		interrupt_ot[i] = ((int)(INT_power[i][2] * divide));
		interrupt_p[i] = INT_power[i][3];
		printf("%d  %d   %d  %.3f  ", interrupt_start[i], interrupt_end[i], interrupt_ot[i], interrupt_p[i]);
		printf("\n");
	}
	for (i = 0; i < uninterrupt_num; i++)
	{
		uninterrupt_start[i] = ((int)(UNINT_power[i][0] * divide));
		uninterrupt_ot[i] = ((int)(UNINT_power[i][2] * divide));
		uninterrupt_p[i] = UNINT_power[i][3];
		printf("%d  %d   %d  %.3f  ", uninterrupt_start[i], uninterrupt_end[i], uninterrupt_ot[i], uninterrupt_p[i]);

		printf("\n");
	}
	// ------------------------------- End of Put Mysql data into Matrix ------------------------------- //
	for (i = 0; i < varying_num; i++)
	//clean table (if at one_schedule)
	// MARK: if in the same day, only clean the 'real_status' table and get now_SOC
	if (same_day == 1 && real_time == 1)
		varying_ot[i] = ((int)(VAR_power[i][2] * divide));	// varying_ot[0] = 1
		printf("%d  %d   %d  ", varying_start[i], varying_end[i], varying_ot[i]);
		for (j = 0; j < 3; j++)
		{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 25");	//get now_SOC
			varying_p_pow[i][j] = VAR_power[i][3 + j];	// varying_p_pow[0][0~3] = [0.4, 0.5, 0.6] (kW)
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		if ((atof(mysql_row[0]) / 100.0) < 1.00)
		{SOC_ini = atof(mysql_row[0]);}
		if (SOC_ini>90)
		{SOC_ini=89.8;}
		// SOC_ini=0.68;   //for test
		printf("NOW REAL SOC:%f", SOC_ini);
		memset(sql_buffer, 0, sizeof(sql_buffer));
		mysql_free_result(mysql_result);
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status");      //clean real_status;
	
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		{SOC_ini = atof(mysql_row[0]);}
		if (SOC_ini>90)
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE real_status");      //clean control_status;
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE cost");      //clean control_status;
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE solar_fake");      //clean control_status;
		// SOC_ini=0.68;   //for test
		//snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE solar_day");      //clean control_status;
		//mysql_query(mysql_con, sql_buffer);

		if (real_time == 0)   //if we want only do one day schedule
		{
			printf(" *********one_day mode(signed one day mode)**********\n");
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 24");  //get ini_SOC
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			SOC_ini = atof(mysql_row[0]);
			// SOC_ini=0.7;   //for test
			printf("ini_SOC:%f\n", SOC_ini);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			mysql_free_result(mysql_result);
		}
		else
		{
			printf(" *********one_day mode(caculate next day)**********\n");
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 25");//get now_SOC
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			if ((atof(mysql_row[0]) / 100.0) < 1.00)
			{SOC_ini = atof(mysql_row[0]);}
			printf("ini_SOC:%f", SOC_ini);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			mysql_free_result(mysql_result);
		}

	
		real_time = 1;    //if you don't want do real_time,please commend it.
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE control_status");      //clean control_status;
		mysql_query(mysql_con, sql_buffer);

		mysql_query(mysql_con, sql_buffer);
	//done and update_date
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE solar_fake");      //clean control_status;
		mysql_query(mysql_con, sql_buffer);
	//optimization
	GLPK(interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_ot, uninterrupt_reot, uninterrupt_p, uninterrupt_flag, varying_start, varying_end, varying_ot, varying_reot, varying_flag, varying_t_pow, varying_p_pow, app_count, price, position);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			mysql_free_result(mysql_result);
		}
		else
		{
			printf(" *********one_day mode(caculate next day)**********\n");
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 25");//get now_SOC
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			if ((atof(mysql_row[0]) / 100.0) < 1.00)
			{SOC_ini = atof(mysql_row[0]);}
			printf("ini_SOC:%f", SOC_ini);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 6, app_count + 11);
	mysql_query(mysql_con, sql_buffer);


	if ((sample_time+1) != 96)
	{
		same_day = 1;
	}
	else  
	{
		same_day = 0;

		//if just want only run realtime 24 hour.. otherwise you can commend it
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%d' WHERE parameter_id = 27 ",1);
		mysql_query(mysql_con, sql_buffer);
	}

			memset(sql_buffer, 0, sizeof(sql_buffer));
			mysql_free_result(mysql_result);
		}

		sample_time = 0;
		real_time = 1;    //if you don't want do real_time,please commend it.
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %d WHERE parameter_id = 15 ", real_time);
		mysql_query(mysql_con, sql_buffer);
	}


	//done and update_date
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%d-%02d-%02d' WHERE parameter_id = 17 ", now_time.tm_year + 1900, now_time.tm_mon + 1, now_time.tm_mday);
	mysql_query(mysql_con, sql_buffer);

	//optimization
	GLPK(interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_ot, uninterrupt_reot, uninterrupt_p, uninterrupt_flag, varying_start, varying_end, varying_ot, varying_reot, varying_flag, varying_t_pow, varying_p_pow, app_count, price, position);


	//MARK: after finish the GLPK, update other value into control_status
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 1, app_count + 1);
	mysql_query(mysql_con, sql_buffer);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 2, app_count + 3);
	mysql_query(mysql_con, sql_buffer);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 3, app_count + 7);
	mysql_query(mysql_con, sql_buffer);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 4, app_count + 6);
	mysql_query(mysql_con, sql_buffer);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 5, app_count + 10);
	mysql_query(mysql_con, sql_buffer);
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET parameter_id = '%d' WHERE  control_id = '%d'", 6, app_count + 11);
	mysql_query(mysql_con, sql_buffer);


	if ((sample_time+1) != 96)
	{
		same_day = 1;
	}
	else  
	{
		same_day = 0;

		//if just want only run realtime 24 hour.. otherwise you can commend it
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = '%d' WHERE parameter_id = 27 ",1);
		mysql_query(mysql_con, sql_buffer);
	}



	mysql_close(mysql_con);

	system("pause");
	return 0;
}