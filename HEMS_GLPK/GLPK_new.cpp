#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include "HEMS.h"  

#define P_1 1.63
#define P_2 2.38
#define P_3 3.52
#define P_4 4.80
#define P_5 5.66
#define P_6 6.41

#define Hydro_Price 0.08
#define Hydro_Cons 0.04 // unit kWh/g
#define FC_START_POWER 0.35 // Pfc start power

#define NEW2D(H, W, TYPE) (TYPE **)new2d(H, W, sizeof(TYPE))

void GLPK(int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_ot, int *uninterrupt_reot, float *uninterrupt_p, int *uninterrupt_flag, int *varying_start, int *varying_end, int *varying_ot, int *varying_reot, int *varying_flag, int **varying_t_pow, float **varying_p_pow, int app_count, float *price, int *position)
{
	/*============================ 獲取當前系統時間(Get system's time) ==================================*/
	time_t t = time(NULL);
	struct tm now_time = *localtime(&t);

	/*============================資料庫讀取存取資料(Database access data)===================================*/
	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;

	if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "realtime", 6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		exit(1);
	}
	mysql_set_character_set(mysql_con, "utf8");
	char sql_buffer[2000] = { 0 };

	int *buff = new int[app_count];	//存放剩餘執行次數(The number of remaining executions)
	for (i = 0; i < app_count; i++)
	{
		buff[i] = 0;
	}

	int noo;
	//get now time that can used in the real experiment
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	else
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	printf("sample:%d\n", noo);


	/*============================== 太陽能陣列() ============================== */
	float solar[24] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.6, 0.8, 1.6, 1.4, 2.1, 2.5, 2.8, 2.7, 1.9, 2.0, 1.4, 0.8, 0.2, 0.0, 0.0, 0.0, 0.0, 0.0 };		//晴天太陽能(Sunny Solar)
	//float solar[24] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.2, 0.5, 0.6, 0.9, 1.3, 1.5, 1.8, 1.6, 0.9, 0.6, 0.3, 0.1, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };			//陰天太陽能  (Cloudy Solar)

	float *price2 = new float[time_block];

	//將電價資訊由24個=>96個(每小時=>每15分鐘) (Transform the Price and PV power array from 24 to 96)
	for (int x = 0; x < 24; x++)							
	{
		for (int y = x*divide; y < (x*divide) + divide; y++)
		{
			price2[y] = price[x];
		}
	}


	float solar2[96] = {
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
    };

	
	
	if (sample_time == 0)
	{
		for (i = 0; i < time_block; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE solar_day SET value =%.3f WHERE time_block =%d", solar2[i], i);
			mysql_query(mysql_con, sql_buffer);
		}
	}
	else
	{	for(i=0;i<96;i++)
		//for (i = 0; i < noo; i++)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM solar_day WHERE time_block=%d", i);
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			solar2[i] = atof(mysql_row[0]);
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM LP_BASE_PARM WHERE parameter_id = 26");
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		solar2[noo] = atof(mysql_row[0]);

		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE solar_day SET value = '%.3f' WHERE  time_block = '%d'", solar2[noo], noo);
		mysql_query(mysql_con, sql_buffer);
	}

	/*=========================檢查不可中斷負載是否已執行(Check the uninterrupt load whether it have been run)=============================*/
	int flag = 0;

	if (sample_time != 0)
	{
		for (i = 0; i < uninterrupt_num; i++)
		{
			flag = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE equip_id = '%d'", column, (i + app_count + 15 + (ponit_num - 1) * 2));
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			for (j = 0; j < sample_time; j++)
			{
				flag += atoi(mysql_row[j]);
			}
			//不可中斷負載輔助變數之旗標, 0:未開始執行, 1:已開始執行(為1代表前一刻設備已開啟)
			//The flag of the uninterrupted load auxiliary variable, 0: not started, 1: started execution (1 for the first time the device is turned on)
			uninterrupt_flag[i] = flag;			
			mysql_free_result(mysql_result);    
		}
		for (i = 0; i < varying_num; i++)
		{
			flag = 0;
			printf("pointnum:%d", ponit_num);
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE equip_id = '%d'", column, (i + app_count + 15 + (ponit_num - 1) * 2 + uninterrupt_num));
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			for (j = 0; j < sample_time; j++)
			{
				flag += atoi(mysql_row[j]);
			}
			// 變動負載輔助變數旗標, 0:未開始執行, 1:已開始執行, 為1代表前一刻設備已開啟
			// The flag of the varying load auxiliary variable, 0: not started, 1: started execution (1 for the first time the device is turned on)
			varying_flag[i] = flag;
			mysql_free_result(mysql_result);
		}
	}

	/*==================== 得到在sample_time以前設備已執行的次數(Get the number of times the device was executed before sample_time) =======================*/
	int coun = 0;

	if (sample_time != 0)
	{
		for (i = 1; i <= app_count; i++)
		{
			coun = 0;

			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);

			for (j = 0; j < sample_time; j++)
			{
				coun += atoi(mysql_row[j]);
			}
			buff[i - 1] = coun;
			memset(sql_buffer, 0, sizeof(sql_buffer));
		}
		mysql_free_result(mysql_result);
	}
	/*============================ 計算家電剩餘執行時間(Calculate the remaining execution time of appliances)==============================*/
	for (i = 0; i < interrupt_num; i++)	//可中斷負載 (Interrupt load)
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
		if (uninterrupt_flag[j] == 0)	// 不可中斷負載尚未啟動 (Uninterrupted load has not yet started)
		{
			uninterrupt_reot[j] = uninterrupt_ot[j];
		}
		if (uninterrupt_flag[j] == 1)	// 不可中斷負載已啟動(則修改負載起迄時間)(Uninterrupted load is started (modify load start time))
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
		if (varying_flag[k] == 0)	// 變動負載尚未啟動 (Varying load has not yet started)
		{
			varying_reot[k] = varying_ot[k];
		}
		if (varying_flag[k] == 1)	// 變動負載已啟動(則修改負載起迄時間)(Varying load is started (modify load start time))
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
	/*========================= 變動負載結構初始化(Structure initialization of varying load) =============================*/

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
	/*======================== 變動負載耗能模型(Energy consumption model of varying load) ==============================*/
	for (i = 0; i < varying_num; i++)
	{
		for (j = 0; j < varying_t_pow[i][0]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][0];
			//printf("varying_p_d[i][j]:%f\n", varying_p_d[i][j]);
		}
		for (j = varying_t_pow[i][0]; j < varying_t_pow[i][0] + varying_t_pow[i][1]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][1];
			//printf("varying_p_d[i][j]:%f\n", varying_p_d[i][j]);
		}
		for (j = varying_t_pow[i][0] + varying_t_pow[i][1]; j < varying_t_pow[i][0] + varying_t_pow[i][1] + varying_t_pow[i][2]; j++)
		{
			varying_p_d[i][j] = varying_p_pow[i][2];
			//printf("varying_p_d[i][j]:%f\n", varying_p_d[i][j]);
		}
	}

	/*======================== 變動型起訖時程變數(Start and end period variable of varying load) =========================== */
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

	/*======================== 變動型功率最大值(maximum power of varying load) =========================== */
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
		//printf("varying_p_max:%f\n", varying_p_max[i]);
	}

	/*======================== 宣告家電個別時段變數矩陣(Define appliances for individual time variables matrix) ==============================*/
	//	float *s = new float[time_block - sample_time];
	float *s = new float[time_block];
	/*============================ 總規劃功率矩陣(Total planning power matrix) ====================================*/
	float **power1 = NEW2D((((time_block - sample_time) * 200) + app_count + 1), (variable * (time_block - sample_time)), float);

	/*============================ GLPK參數矩陣定義(GLPK parameter matrix definition) ==================================*/
	glp_prob *mip;
	int *ia = new int[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1]; 			//Row
	int *ja = new int[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1];			//Column
	double *ar = new double[((((time_block - sample_time) * 200) + app_count + 1) * (variable * (time_block - sample_time))) + 1];		//structural variable

	/*============================== GLPK變數宣告(GLPK variable definition)=====================================*/
	mip = glp_create_prob();
	glp_set_prob_name(mip, "hardware_algorithm_case");
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, (((time_block - sample_time) * 200) + app_count + 1));
	glp_add_cols(mip, (variable * (time_block - sample_time)));

	/*=============================== 初始化矩陣(initial the matrix)======================================*/
	for (m = 0; m < (((time_block - sample_time) * 200) + app_count + 1); m++)
	{
		for (n = 0; n < (variable * (time_block - sample_time)); n++)
		{
			power1[m][n] = 0.0;
		}
	}

	/*=============================== 燃料電池非線性特性曲線(fuel cell's non-linear curve setting)======================================*/
	//piecewise

	int piecewise_num;
	float *data_power = new float[101];         // power
	float *data_power_all = new float[101];     // total power

	float *P_power = new float[ponit_num];         // power
	float *P_power_all = new float[ponit_num];     // total power

	piecewise_num = ponit_num - 1;

	for (i = 1; i <=101; i++)
	{
		if (i == 1)
		{
			data_power[i-1] =  FC_START_POWER;
		}
		else
		{
			data_power[i-1] = (i-1) *(Pfc_max - FC_START_POWER)*0.01 + FC_START_POWER;
		}

		data_power_all[i-1] = data_power[i - 1] /(0.00025732*(pow(data_power[i - 1], 5)) - 0.003463*(pow(data_power[i - 1], 4)) + 0.016041*(pow(data_power[i - 1], 3)) - 0.02706*(pow(data_power[i - 1], 2)) - 0.026982*(pow(data_power[i - 1], 1)) + 0.5692);
		//printf(" aa:%f  value is %f\n", data_power[i-1], data_power_all[i - 1]);
	}


	P_power[0] = 0.0;
	P_power_all[0] = 0.0;
	printf(" x_0:%f  y_0:%f\n", P_power[0], P_power_all[0]);

	for (j = 1; j < ponit_num; j++)
	{
		P_power[j] = data_power[(j - 1)*(100 / (piecewise_num - 1))];
		P_power_all[j] = data_power_all[(j - 1)*(100 / (piecewise_num - 1))];
		printf(" x_%d:%f  y_%d:%f\n",j, P_power[j],j, P_power_all[j]);
	}



	/*============================= 限制式係數(Constraints coefficient) ============================================*/
	//­­¨î¦¡-®a®x­t¸ü³Ì§C¯Ó¯à(Household load minimum energy consumption)
	for (h = 0; h < interrupt_num; h++)	//可中斷負載(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i*variable + h] = 1.0;
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[h][i*variable + h] = 1.0;
				}
			}
		}
	}
	for (h = 0; h < uninterrupt_num; h++)	// 不可中斷負載(uninterrupt load)					
	{
		if (uninterrupt_flag[h] == 0)
		{
			if (((uninterrupt_end[h] - sample_time) >= 0) && (uninterrupt_reot[h] > 0))
			{
				if ((uninterrupt_start[h] - sample_time) >= 0)
				{
					for (i = (uninterrupt_start[h] - sample_time); i <= (uninterrupt_end[h] - sample_time); i++)
					{
						power1[h + interrupt_num][i*variable + h + interrupt_num] = 1.0;
					}
				}
				else if ((uninterrupt_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
					{
						power1[h + interrupt_num][i*variable + h + interrupt_num] = 1.0;
					}
				}
			}
		}
	}
	for (h = 0; h < varying_num; h++)	// 變動負載(varying load)
	{
		if (varying_flag[h] == 0)
		{
			if (((varying_end[h] - sample_time) >= 0) && (varying_reot[h] > 0))
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
					{
						power1[h + interrupt_num + uninterrupt_num][i*variable + h + interrupt_num + uninterrupt_num] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						power1[h + interrupt_num + uninterrupt_num][i*variable + h + interrupt_num + uninterrupt_num] = 1.0;
					}
				}
			}
		}
	}

	// 決定是否輸出市電(Decide whether to buy electricity from utility)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[app_count + i][i*variable + app_count] = 1.0;
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[app_count + i][i*variable + (app_count + 1)] = -Pgrid_max;
	}

	//決定是否賣電	//if no sell
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) + app_count + i][i*variable + (app_count + 5)] = 1.0;
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) + app_count + i][i*variable + (app_count + 1)] = Psell_max;
	}

	//限制隔天SOC必須大於門檻值(Limit the next day SOC must be greater than the threshold)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 2 + app_count][i*variable + (app_count + 2)] = 1.0;
	}

	//計算下一刻SOC(Calculate the next moment SOC)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (j = 0; j <= i; j++)
		{
			power1[(time_block - sample_time) * 2 + app_count + 1 + i][j*variable + (app_count + 2)] = -1.0;
		}
		power1[(time_block - sample_time) * 2 + app_count + 1 + i][i*variable + (app_count + 6)] = Cbat * Vsys / delta_T;				//SOC
	}

	// 平衡式(Balanced function)
	for (h = 0; h < interrupt_num; h++)	//可中斷負載(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h] = interrupt_p[h];
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h] = interrupt_p[h];
				}
			}
		}
	}
	for (h = 0; h < uninterrupt_num; h++)	//不可中斷負載(Interrupt load)
	{
		if ((uninterrupt_end[h] - sample_time) >= 0)
		{
			if ((uninterrupt_start[h] - sample_time) >= 0)
			{
				for (i = (uninterrupt_start[h] - sample_time); i <= (uninterrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
			else if ((uninterrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
		}
	}
	for (h = 0; h < varying_num; h++)	// 變動負載耗能(varying load)
	{
		if ((varying_end[h] - sample_time) >= 0)
		{
			if ((varying_start[h] - sample_time) >= 0)
			{
				for (i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h + (variable - varying_num)] = 1.0;
				}
			}
			else if ((varying_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (varying_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + h + (variable - varying_num)] = 1.0;
				}
			}
		}
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + app_count] = -1.0;							//市電(grid power)
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + (app_count + 2)] = 1.0;						//電池(PESS)
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + (app_count + 5)] = 1.0;						//額外電力(sell power)
	}	//if no sell

	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 3 + app_count + 1 + i][i*variable + (app_count + 9)] = -1.0;							//燃料電池(fuel cell)
	}


	// 充電限制式(Charge limit)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 4 + app_count + 1 + i][i*variable + (app_count + 3)] = 1.0;						//Pess +
		power1[(time_block - sample_time) * 4 + app_count + 1 + i][i*variable + (app_count + 7)] = -Pbat_max;				//Z
	}

	// 放電限制式(Discharge limit)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 5 + app_count + 1 + i][i*variable + (app_count + 4)] = 1.0;						//Pess -
		power1[(time_block - sample_time) * 5 + app_count + 1 + i][i*variable + (app_count + 7)] = Pbat_min;					//Z
	}

	// 電池功率(Battery power)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 6 + app_count + 1 + i][i*variable + (app_count + 2)] = 1.0;						//Pess
		power1[(time_block - sample_time) * 6 + app_count + 1 + i][i*variable + (app_count + 3)] = -1.0;					//Pess +
		power1[(time_block - sample_time) * 6 + app_count + 1 + i][i*variable + (app_count + 4)] = 1.0;						//Pess -
	}

	// (Sell to grid constraint,Psell<=Psolar+Pfc)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 7 + app_count + 1 + i][i*variable + (app_count + 5)] = 1.0;     //Psell
	}
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 7 + app_count + 1 + i][i*variable + (app_count + 9)] = -1.0;	//Pfc
	}

	// =-=-=-=-=-=-=- FC part -=-=-=-=-=-=-=
	//fc constraint
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 8 + app_count + 1 + i][i*variable + (app_count + 9)] = 1.0;		// Pfc=Pfc_on+Pfc_off
		power1[(time_block - sample_time) * 8 + app_count + 1 + i][i*variable + (app_count + 11)] = -1.0;	// Pfc_on
		power1[(time_block - sample_time) * 8 + app_count + 1 + i][i*variable + (app_count + 12)] = -1.0;	// Pfc_off
	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 9 + app_count + 1 + i][i*variable + (app_count + 11)] = 1.0;	//Pfc_on    //Pfc_on<=ufc*Pfc_max
		power1[(time_block - sample_time) * 9 + app_count + 1 + i][i*variable + (app_count + 13)] = -Pfc_max;	// ufc
	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 10 + app_count + 1 + i][i*variable + (app_count + 11)] = 1.0;	//Pfc_on    //Pfc_on>=ufc*Pfc_min
		power1[(time_block - sample_time) * 10 + app_count + 1 + i][i*variable + (app_count + 13)] = -1.5;	// ufc
	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 11 + app_count + 1 + i][i*variable + (app_count + 12)] = 1.0;	//Pfc_off    //Pfc_off<=ufc*0.000000001
		power1[(time_block - sample_time) * 11 + app_count + 1 + i][i*variable + (app_count + 13)] = 0.0;	//ufc
	}


	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 12 + app_count + 1 + i][i*variable + (app_count + 9)] = 1.0; //Pfc j
		
		for (k = 1; k <= piecewise_num; k++)												//X=z1*x1+(x2-x1)*s1...
		{		
			power1[(time_block - sample_time) * 12 + app_count + 1 + i][i*variable + (app_count + 13 + k)] = -P_power[k-1];	//z
			power1[(time_block - sample_time) * 12 + app_count + 1 + i][i*variable + (app_count + 13 + piecewise_num + k)] = -1.0*(P_power[k] - P_power[k-1]);	//λ
		}

	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[(time_block - sample_time) * 13 + app_count + 1 + i][i*variable + (app_count + 10)] = 1.0;	//Pfct j

		for (k = 1; k <= piecewise_num; k++)	//Y=z1*y1+(y2-y1)*s1...
		{
			power1[(time_block - sample_time) * 13 + app_count + 1 + i][i*variable + (app_count + 13 + k)] = -P_power_all[k-1];	//z
			power1[(time_block - sample_time) * 13 + app_count + 1 + i][i*variable + (app_count + 13 + piecewise_num + k)] = -1.0*(P_power_all[k] - P_power_all[k-1]);	//λ
		}

	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (k = 1; k <= piecewise_num; k++)												
		{
			power1[(time_block - sample_time) * 14 + app_count + 1 + i][i*variable + (app_count + 13 + k)] = 1.0; //z       //z1+z2+z3+......=1
		}
	}

	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (k = 1; k <= piecewise_num; k++)
		{
			power1[(time_block - sample_time) * (14 + k) + app_count + 1 + i][i*variable + (app_count + 13 + k)] = -1.0; //z       //λi-zi  <=0  
			power1[(time_block - sample_time) * (14 + k) + app_count + 1 + i][i*variable + (app_count + 13 + piecewise_num + k)] = 1.0;//λ
		}
	}


	int counter;
	// 不可中斷負載之輔助變數(Uninterrupted load of auxiliary variables), sum = 1
	counter = 0;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)	//在不可中斷負載未啟動時，以不可中斷方式填矩陣(When the uninterruptible load is not started, use the original way to fill the matrix)
		{								//­若已啟動就強迫將排程起迄時間 從 "start時刻" 到 "start時刻 + 剩下須執行時間"
			if ((uninterrupt_end[h] - sample_time) >= 0)	//If it is already started, it will force the schedule to start from "start time" to "start time + left to be executed"
			{
				if ((uninterrupt_start[h] - sample_time) >= 0)
				{
					for (i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter][i*variable + h + (variable - uninterrupt_num - (varying_num * 2))] = 1.0;
					}
				}
				else if ((uninterrupt_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter][i*variable + h + (variable - uninterrupt_num - (varying_num * 2))] = 1.0;
					}
				}
			}
			counter += 1;
		}
	}
	//變動負載之輔助變數(Varying load of auxiliary variables), sum = 1
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)	// 在變動負載未啟動時，以不可中斷方式填矩陣(When the varying load is not started, use the original way to fill the matrix)
		{							//若已啟動就強迫將排程起迄時間 從 "start時刻" 到 "start時刻 + 剩下須執行時間"
			if ((varying_end[h] - sample_time) >= 0)	//If it is already started, it will force the schedule to start from "start time" to "start time + left to be executed"
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter][i*variable + h + (variable - (varying_num * 2))] = 1.0;
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter][i*variable + h + (variable - (varying_num * 2))] = 1.0;
					}
				}
			}
			counter += 1;
		}
	}


	n = 0+ piecewise_num;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)	//在不可中斷負載未啟動時
		{
			//不可中斷負載決策變數
			for (k = (15 + n), m = 0; k < (15 + n) + uninterrupt_reot[h], m < uninterrupt_reot[h]; k++, m++)
			{
				if ((uninterrupt_end[h] - sample_time) >= 0)
				{
					if ((uninterrupt_start[h] - sample_time) >= 0)
					{
						for (i = (uninterrupt_start[h] - sample_time); i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i + m)*variable + h + interrupt_num] = 1.0;	// 不可中斷負載決策變數
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][i*variable + h + (variable - uninterrupt_num - (varying_num * 2))] = -1.0;	//不可中斷負載二元輔助變數
						}
					}
					else if ((uninterrupt_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((uninterrupt_end[h] - uninterrupt_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i + m)*variable + h + interrupt_num] = 1.0;									//不可中斷負載決策變數
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][i*variable + h + (variable - uninterrupt_num - (varying_num * 2))] = -1.0;	//不可中斷負載二元輔助變數
						}
					}
				}
			}
			n += uninterrupt_reot[h];
		}
		//在不可中斷負載已啟動時(因起迄時間已被強迫修改)
		if (uninterrupt_flag[h] == 1)
		{
			if ((uninterrupt_end[h] - sample_time) >= 0)
			{
				if ((uninterrupt_start[h] - sample_time) <= 0)
				{
					for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][i * variable + h + interrupt_num] = 1.0;	//不可中斷負載決策變數
					}
				}
				n += 1;
			}
		}
	}
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)	//在變動負載未啟動時
		{
			//變動負載決策變數
			for (k = (15 + n), m = 0; k < (15 + n) + varying_reot[h], m < varying_reot[h]; k++, m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i + m)*variable + h + (interrupt_num + uninterrupt_num)] = 1.0;	//變動負載決策變數
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][i*variable + h + (variable - (varying_num * 2))] = -1.0;	//變動負載二元輔助變數
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i + m)*variable + h + (interrupt_num + uninterrupt_num)] = 1.0;	//變動負載決策變數
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][i*variable + h + (variable - (varying_num * 2))] = -1.0;	//變動負載二元輔助變數
						}
					}
				}
			}
			n += varying_reot[h];
		}
		// 在變動負載已啟動時(因起迄時間已被強迫修改)
		if (varying_flag[h] == 1)
		{
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) <= 0)
				{
					for (i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][i*variable + h + (interrupt_num + uninterrupt_num)] = 1.0;	//變動負載決策變數
					}
				}
				n += 1;
			}
		}
	}


	//變動型負載耗能Pa
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (k = (15 + n), m = 0; k < (15 + n) + varying_reot[h], m < varying_reot[h]; k++, m++)
			{
				if ((varying_end[h] - sample_time) >= 0)
				{
					if ((varying_start[h] - sample_time) >= 0)
					{
						for (i = (varying_start[h] - sample_time); i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i * variable) + h + (variable - (varying_num * 2))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][((i + m) * variable) + h + (variable - varying_num)] = 1.0;    //輔助變數Pa 
						}
					}
					else if ((varying_start[h] - sample_time) < 0)
					{
						for (i = 0; i <= ((varying_end[h] - varying_reot[h] + 1) - sample_time); i++)
						{
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][(i * variable) + h + (variable - (varying_num * 2))] = -1.0 * (((float)varying_t_d[h][i]) * (varying_p_d[h][m]));
							power1[(time_block - sample_time) * k + app_count + 2 + counter + i][((i + m) * variable) + h + (variable - varying_num)] = 1.0;    //輔助變數Pa 
						}
					}
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			//h擴增			
			if ((varying_end[h] - sample_time) >= 0)
			{
				if ((varying_start[h] - sample_time) >= 0)
				{
					for (i = (varying_start[h] - sample_time); i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][(i * variable) + h + interrupt_num + uninterrupt_num] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][(i * variable) + h + (variable - varying_num)] = 1.0;    //輔助變數Pa 
					}
				}
				else if ((varying_start[h] - sample_time) < 0)
				{
					for (i = 0; i <= (varying_end[h] - sample_time); i++)
					{
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][(i * variable) + h + interrupt_num + uninterrupt_num] = -1.0 * ((float)(varying_t_d[h][i]) * (varying_p_d[h][i + buff[h + interrupt_num + uninterrupt_num]]));
						power1[(time_block - sample_time) * (15 + n) + app_count + 2 + counter + i][(i * variable) + h + (variable - varying_num)] = 1.0;    //輔助變數Pa 
					}
				}
			}
			n += 1;
		}
	}



	/*============================== 宣告限制式條件範圍(row) ===============================*/
	//GLPKÅª¦C±q1¶}©l
	//­­¨î¦¡-®a®x­t¸ü³Ì§C¯Ó¯à
	for (i = 1; i <= interrupt_num; i++)	//可中斷負載(Interrupt load)
	{
		glp_set_row_name(mip, i, "");
		glp_set_row_bnds(mip, i, GLP_LO, ((float)interrupt_reot[i - 1]), 0.0);//ok
	}
	for (i = 1; i <= uninterrupt_num; i++)	//不可中斷負載
	{
		if (uninterrupt_flag[i] == 0)
		{
			glp_set_row_name(mip, i + interrupt_num, "");
		    glp_set_row_bnds(mip, i + interrupt_num, GLP_LO, ((float)uninterrupt_reot[i - 1]), ((float)uninterrupt_reot[i - 1]));//ok
		}
	}
	for (i = 1; i <= varying_num; i++)	//變動負載
	{
		if (varying_flag[i] == 0)
		{
			glp_set_row_name(mip, i + interrupt_num + uninterrupt_num, "");
			glp_set_row_bnds(mip, i + interrupt_num + uninterrupt_num, GLP_FX, ((float)varying_reot[i - 1]), ((float)varying_reot[i - 1]));  //ok
		}
	}

	//決定是否輸出市電
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, (app_count + i), "");
		glp_set_row_bnds(mip, (app_count + i), GLP_UP, 0.0, 0.0);
	}

	//if no sell //決定是否賣電
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) + app_count + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) + app_count + i), GLP_UP, 0.0, Psell_max);
	}

	//限制隔天SOC必須大於門檻值 (1)
	glp_set_row_name(mip, ((time_block - sample_time) * 2 + app_count + 1), "");
	if (sample_time == 0)
	{
		glp_set_row_bnds(mip, ((time_block - sample_time) * 2 + app_count + 1), GLP_LO, ((SOC_thres - SOC_ini) * Cbat * Vsys) / delta_T, 0.0);
	}
	else
	{
		glp_set_row_bnds(mip, ((time_block - sample_time) * 2 + app_count + 1), GLP_DB, ((SOC_thres - SOC_ini) * Cbat * Vsys) / delta_T, ((0.89 - SOC_ini) * Cbat * Vsys) / delta_T); //比較能夠解的出來
	}
	

	//限制下一刻SOC
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 2 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 2 + app_count + 1 + i), GLP_FX, (SOC_ini * Cbat * Vsys / delta_T), (SOC_ini * Cbat * Vsys / delta_T));
	}

	//平衡式
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 3 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 3 + app_count + 1 + i), GLP_DB, -0.0001, solar2[i - 1 + sample_time]);    //不會亂放電
	}

	//充電限制式
	for (i = 1; i <= (time_block - sample_time); i++)
	{
	glp_set_row_name(mip, ((time_block - sample_time) * 4 + app_count + 1 + i), "");
	glp_set_row_bnds(mip, ((time_block - sample_time) * 4 + app_count + 1 + i), GLP_UP, 0.0, 0.0);
	}

	//放電限制式
	for (i = 1; i <= (time_block - sample_time); i++)
	{
	glp_set_row_name(mip, ((time_block - sample_time) * 5 + app_count + 1 + i), "");
	glp_set_row_bnds(mip, ((time_block - sample_time) * 5 + app_count + 1 + i), GLP_UP, 0.0, Pbat_min);
	}

	//電池功率
	for (i = 1; i <= (time_block - sample_time); i++)
	{
	glp_set_row_name(mip, ((time_block - sample_time) * 6 + app_count + 1 + i), "");
	glp_set_row_bnds(mip, ((time_block - sample_time) * 6 + app_count + 1 + i), GLP_FX, 0.0, 0.0);
	}

	// (Sell to grid constraint,Psell<=Psolar+Pfc)
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 7 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 7 + app_count + 1 + i), GLP_UP, 0.0, solar2[i - 1 + sample_time]);   //Pfc
	}

	// =-=-=-=-=-=-=- FC part -=-=-=-=-=-=-=
	//pfc=pfc_on+pfc_off
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 8 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 8 + app_count + 1 + i), GLP_FX, 0.0, 0.0);
	}

	//pfc_on<=ufc*Pfc_max
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 9 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 9 + app_count + 1 + i), GLP_UP, 0.0, 0.0);
	}

	//pfc_on>=ufc*pfc_on
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 10 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 10 + app_count + 1 + i), GLP_LO, 0.0, 0.0);
	}

	//pfc_off<=(1-ufc)*pfc_off
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 11 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 11 + app_count + 1 + i), GLP_UP, 0.0, 0.0);
	}

	//pfc=x1z1+(x2-x1)s1......
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 12 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 12 + app_count + 1 + i), GLP_UP, 0.0, 0.0);
	}

	//pfc_tatol=y1z1+(y2-y1)s1......
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 13 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 13 + app_count + 1 + i), GLP_LO, 0.0, 0.0);
	}

	//z1+z2+z3+.....=1
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, ((time_block - sample_time) * 14 + app_count + 1 + i), "");
		glp_set_row_bnds(mip, ((time_block - sample_time) * 14 + app_count + 1 + i), GLP_FX, 1.0, 1.0);
	}

	for (i = 1; i <= (time_block - sample_time); i++)
	{
		for (j = 1; j <= piecewise_num; j++)												//s-z<=0
		{
			glp_set_row_name(mip, ((time_block - sample_time) * (14 + j) + app_count + 1 + i), "");
			glp_set_row_bnds(mip, ((time_block - sample_time) * (14 + j) + app_count + 1 + i), GLP_UP,0.0, 0.0);
		}
	}


	counter = 1;

	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			//不變動負載之輔助變數, sum = 1
			glp_set_row_name(mip, ((time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter), "");
			glp_set_row_bnds(mip, ((time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter), GLP_FX, 1.0, 1.0);

			counter += 1;
		}
	}
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			//變動負載之輔助變數, sum = 1
			glp_set_row_name(mip, ((time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter), "");
			glp_set_row_bnds(mip, ((time_block - sample_time) * (15 + piecewise_num) + app_count + 2 + counter), GLP_FX, 1.0, 1.0);

			counter += 1;
		}
	}

	n = 0 + piecewise_num;
	for (h = 0; h < uninterrupt_num; h++)
	{
		if (uninterrupt_flag[h] == 0)
		{
			//不可中斷負載決策變數
			for (k = (15 + n); k < (15 + n) + uninterrupt_reot[h]; k++)
			{
				for (i = ((time_block - sample_time)*k + app_count + 2 + counter); i < ((time_block - sample_time)*(k + 1) + app_count + 2 + counter); i++)
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
				for (i = ((time_block - sample_time)*(15 + n) + app_count + 2 + counter); i < ((time_block - sample_time)*(15 + n) + app_count + 2 + counter + uninterrupt_reot[h]); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 1.0, 1.0);
				}
				for (i = ((time_block - sample_time)*(15 + n) + app_count + 2 + counter + uninterrupt_reot[h]); i < ((time_block - sample_time)*((15 + n) + 1) + app_count + 2 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
				n += 1;
			}
		}
	}
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			//變動負載決策變數
			for (k = (15 + n); k < (15 + n) + varying_reot[h]; k++)
			{
				for (i = ((time_block - sample_time)*k + app_count + 2 + counter); i < ((time_block - sample_time)*(k + 1) + app_count + 2 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			for (i = ((time_block - sample_time)*(15 + n) + app_count + 2 + counter); i < (((time_block - sample_time)*(15 + n) + app_count + 2 + counter) + varying_reot[h]); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 1.0, 1.0);
			}
			for (i = (((time_block - sample_time)*(15 + n) + app_count + 2 + counter) + varying_reot[h]); i < ((time_block - sample_time)*((15 + n) + 1) + app_count + 2 + counter); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
			}
			n += 1;
		}
	}

	// 變動型負載耗能Pa	
	for (h = 0; h < varying_num; h++)
	{
		if (varying_flag[h] == 0)
		{
			for (k = (15 + n); k < (15 + n) + varying_reot[h]; k++)
			{
				for (i = ((time_block - sample_time)*k + app_count + 2 + counter); i < ((time_block - sample_time)*(k + 1) + app_count + 2 + counter); i++)
				{
					glp_set_row_name(mip, i, "");
					glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
				}
			}
			n += varying_reot[h];
		}
		if (varying_flag[h] == 1)
		{
			for (i = ((time_block - sample_time) * (15 + n) + app_count + 2 + counter); i < ((time_block - sample_time) * ((15 + n) + 1) + app_count + 2 + counter); i++)
			{
				glp_set_row_name(mip, i, "");
				glp_set_row_bnds(mip, i, GLP_LO, 0.0, 0.0);
			}
			n += 1;
		}
	}

	/*============================== 宣告決策變數(column) ================================*/
	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (j = 1; j <= app_count; j++)
		{
			glp_set_col_bnds(mip, (j + i*variable), GLP_DB, 0.0, 1.0);	//負載決策變數
			glp_set_col_kind(mip, (j + i*variable), GLP_BV);
		}
		glp_set_col_bnds(mip, ((app_count + 1) + i*variable), GLP_DB, 0.0, Pgrid_max);	//決定市電輸出功率  一定要大於總負載功率才不會有太大問題
		glp_set_col_kind(mip, ((app_count + 1) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 2) + i*variable), GLP_DB, 0.0, 1.0);	//決定市電是否輸出二元輔助變數
		glp_set_col_kind(mip, ((app_count + 2) + i*variable), GLP_BV);
		glp_set_col_bnds(mip, ((app_count + 3) + i*variable), GLP_DB, -Pbat_min, Pbat_max);	//Pess
		glp_set_col_kind(mip, ((app_count + 3) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 4) + i*variable), GLP_FR, 0.0, Pbat_max);	//Pess +
		glp_set_col_kind(mip, ((app_count + 4) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 5) + i*variable), GLP_FR, 0.0, Pbat_min);	//Pess -
		glp_set_col_kind(mip, ((app_count + 5) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 6) + i*variable), GLP_DB, -0.0000000001, Psell_max);	//額外電力 //if no sell
		glp_set_col_kind(mip, ((app_count + 6) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 7) + i*variable), GLP_DB, SOC_min, SOC_max);	//電池SOC //if no sell
		glp_set_col_kind(mip, ((app_count + 7) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 8) + i*variable), GLP_DB, 0.0, 1.0);	//Z
		glp_set_col_kind(mip, ((app_count + 8) + i*variable), GLP_BV);
		glp_set_col_bnds(mip, ((app_count + 9) + i*variable), GLP_LO, 0.0, 0.0);	//S
		glp_set_col_kind(mip, ((app_count + 9) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 10) + i*variable), GLP_DB, -0.00001, Pfc_max);	//Pfc /////*******
		glp_set_col_kind(mip, ((app_count + 10) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 11) + i*variable), GLP_LO, 0.0, 0.0);	//Total_Pfc /////*******
		glp_set_col_kind(mip, ((app_count + 11) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 12) + i*variable), GLP_DB, -0.00001, Pfc_max);	//Pfc_ON_POWER /////*******
		glp_set_col_kind(mip, ((app_count + 12) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 13) + i*variable), GLP_FX, 0.0, 0.0);	//Pfc_OFF_POWER /////*******
		glp_set_col_kind(mip, ((app_count + 13) + i*variable), GLP_CV);
		glp_set_col_bnds(mip, ((app_count + 14) + i*variable), GLP_DB, 0.0, 1.0);	//ufc /////*******
		glp_set_col_kind(mip, ((app_count + 14) + i*variable), GLP_BV);

		for (j = 1; j <= piecewise_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 14 + j) + i*variable), GLP_DB, 0.0, 1.0);	//z_Pfc /////*******
			glp_set_col_kind(mip, ((app_count + 14 + j) + i*variable), GLP_BV);	//*******
		}

		for (j = 1; j <= piecewise_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 14 + piecewise_num + j) + i*variable), GLP_LO, 0.0, 0.0);	//λ_Pfc /////*******
			glp_set_col_kind(mip, ((app_count + 14 + piecewise_num + j) + i*variable), GLP_CV);	//*******
		}

		for (j = 1; j <= uninterrupt_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 14 + piecewise_num * 2 + j) + i*variable), GLP_DB, 0.0, 1.0);	//不可中斷負載輔助二元變數
			glp_set_col_kind(mip, ((app_count + 14 + piecewise_num * 2 + j) + i*variable), GLP_BV);
		}

		for (j = 1; j <= varying_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 14 + piecewise_num * 2 + uninterrupt_num + j) + i*variable), GLP_DB, 0.0, 1.0);	//變動負載輔助二元變數
			glp_set_col_kind(mip, ((app_count + 14 + piecewise_num * 2 + uninterrupt_num + j) + i*variable), GLP_BV);
		}

		for (j = 1; j <= varying_num; j++)
		{
			glp_set_col_bnds(mip, ((app_count + 14 + piecewise_num * 2 + uninterrupt_num + varying_num + j) + i*variable), GLP_DB, 0.0, varying_p_max[j - 1]);	//變動負載耗能變數Pa	
			glp_set_col_kind(mip, ((app_count + 14 + piecewise_num * 2 + uninterrupt_num + varying_num + j) + i*variable), GLP_CV);
		}

	}

	/*============================== 宣告目標式參數(column) ===============================*/
	for (j = 0; j < (time_block - sample_time); j++)
	{
		glp_set_obj_coef(mip, ((app_count + 1) + j*variable), price2[j + sample_time] * delta_T);							//單目標cost(步驟一)
		glp_set_obj_coef(mip, ((app_count + 6) + j*variable), price2[j + sample_time] * delta_T * (-1));					//單目標cost(步驟一)
		glp_set_obj_coef(mip, ((app_count + 11) + j*variable), Hydro_Price / Hydro_Cons * delta_T);	//FC cost
	}

	/*============================== GLPK寫入矩陣(ia,ja,ar) ===============================*/
	for (i = 0; i < (((time_block - sample_time) * 200) + app_count + 1); i++)
	{
		for (j = 0; j < (variable * (time_block - sample_time)); j++)
		{
			ia[i*((time_block - sample_time)*variable) + j + 1] = i + 1;
			ja[i*((time_block - sample_time)*variable) + j + 1] = j + 1;
			ar[i*((time_block - sample_time)*variable) + j + 1] = power1[i][j];
		}
	}
	/*============================== GLPK讀取資料矩陣 ====================================*/
	glp_load_matrix(mip, (((time_block - sample_time) * 200) + app_count + 1)*(variable * (time_block - sample_time)), ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);
        
	if(sample_time==0)
	{parm.tm_lim = 120000;}
	else
	{parm.tm_lim = 60000;}
	parm.presolve = GLP_ON;   
	//no fc+ no sell
	//fc+no sell 
	parm.gmi_cuts = GLP_ON;	
	parm.bt_tech = GLP_BT_BPH;
	parm.br_tech = GLP_BR_PCH;

	//FC+sell
	//parm.fp_heur = GLP_ON;
	//parm.bt_tech = GLP_BT_BPH;
	//parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);
	z = glp_mip_obj_val(mip);

	

	printf("\n");
	printf("sol = %f; \n", z);

	if (z == 0.0 && glp_mip_col_val(mip, (app_count + 7)) == 0.0)
	{
		printf("No Solotion,give up the solution\n");
		system("pause");
		exit(1);
	}


	/*============================== 將決策變數結果輸出 ==================================*/
	int l=0;
	step1_bill = 0.0, step1_sell = 0.0, step1_PESS = 0.0;

	//get now time that can used in the real experiment
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	else
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	printf("sample:%d", noo);

	for (i = 1; i <= variable; i++)
	{
		h = i;
		l=variable-(app_count-i);
		if (sample_time == 0)
		{
			for (j = 0; j < time_block; j++)
			{

				s[j] = glp_mip_col_val(mip, h);

				if (i <= interrupt_num+uninterrupt_num && j== noo)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i-1]);
					mysql_query(mysql_con, sql_buffer);
				}
				else if ((i > interrupt_num+uninterrupt_num) && (i <= app_count))  //sometimes varying load will have weird 
				{
					s[j] = glp_mip_col_val(mip,l);
					// printf("%d. variable:%d  value:%f\n",j,i,s[j]);
					if(s[j]>0.0){s[j]=1.0;}
					if(j== noo)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i-1]);
						mysql_query(mysql_con, sql_buffer);
					}					
				}

				l = (l + variable);
				h = (h + variable);

			}

			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');"
				, column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			printf("%d,", i);
		}

		if (sample_time != 0)
		{
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE (control_id = '%d')", column, i);
			mysql_query(mysql_con, sql_buffer);
			mysql_result = mysql_store_result(mysql_con);
			mysql_row = mysql_fetch_row(mysql_result);
			for (k = 0; k < sample_time; k++)
			{
				s[k] = atof(mysql_row[k]);
			}
			memset(sql_buffer, 0, sizeof(sql_buffer));

			for (j = 0; j < (time_block - sample_time); j++)
			{
				s[j + sample_time] = glp_mip_col_val(mip, h);

				if (i <= interrupt_num+uninterrupt_num && j== 0)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j+sample_time], position[i-1]);
					printf("%s\n",sql_buffer);
					mysql_query(mysql_con, sql_buffer);
				}
				else if ((i > interrupt_num+uninterrupt_num) && (i <= app_count))
				{
					s[j+ sample_time] = glp_mip_col_val(mip,l);
					// printf("%d. variable:%d  value:%f\n",j,i,s[j+ sample_time]);
					if(s[j+ sample_time]>0.0){s[j+ sample_time]=1.0;}
					if(j==0)
					{
						snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j + sample_time], position[i-1]);
						printf("%s\n",sql_buffer);
						mysql_query(mysql_con, sql_buffer);
					}
				}
				l = (l + variable);
				h = (h + variable);
			
			}

			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE equip_id = '%d';", s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));

			for (j = 0; j < sample_time; j++)
			{
				s[j] = 0;
			}
			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO real_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');"
				, column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));

		}


	}
	printf("\n");


	//update now_state
	float now_grid[96] = { 0.0 },varying_grid[96] = {0.0}, cost[96] = { 0.0 }, sell[96] = { 0.0 }, FC_cost[96] = { 0.0 },Hydrogen_com[96] = { 0.0 }, FC_every_cost[96] = { 0.0 };
	float now_power_result = 0.0, var_grid_result = 0.0, opt_cost_result = 0.0, opt_sell_result = 0.0,  opt_FC_cost_result = 0.0, opt_Hydrogen_result = 0.0, price_sum_now_power=0.0;

	//get all_grid cost(use the time varing price)
	for (j = 0; j < (time_block - sample_time); j++)
	{
		for (i = 1; i < (app_count+1); i++)
		{
			if (i < (interrupt_num+1))
			{
				h = i+variable*j;
				s[j] = glp_mip_col_val(mip, h);
				now_grid[j + sample_time] += (float)(s[j]) * interrupt_p[i - 1] * delta_T;
				varying_grid[j+sample_time] += (float)(s[j]) * interrupt_p[i-1]* price2[j + sample_time] * delta_T;
			}
			else if (i >= (interrupt_num + 1) && i < (interrupt_num+uninterrupt_num+1))
			{
				h = i + variable*j;
				s[j] = glp_mip_col_val(mip, h);
				now_grid[j + sample_time] += (float)(s[j])* uninterrupt_p[i - 1 - interrupt_num] * delta_T;
				varying_grid[j + sample_time] += (float)(s[j])* uninterrupt_p[i - 1 - interrupt_num] * price2[j + sample_time] * delta_T;
			}
			else if (i >= (interrupt_num + uninterrupt_num + 1) && i < (interrupt_num + uninterrupt_num+varying_num+1))
			{
				k = i + 9 + uninterrupt_num + varying_num * 2 + 2 + 2 * (ponit_num - 1)+ 3 + variable*j;						//MUSTMUSTMUST CHANGE(if add rows)
				s[j] = glp_mip_col_val(mip, k);
				now_grid[j + sample_time] += (float)(glp_mip_col_val(mip, i + variable*j))*s[j] * delta_T;
				varying_grid[j + sample_time] += (float)(glp_mip_col_val(mip, i + variable*j))*s[j]* price2[j + sample_time] * delta_T;
			}		
		}

			now_power_result += now_grid[j + sample_time];     //now_power_result¡÷ just all the comsumption energy(for no varying price and coculate leter).
			var_grid_result += varying_grid[j + sample_time];  //var_power_result¡÷ comsumption energy*price.
			printf("%d.power:%f\n",j, now_power_result);
	}

	h = app_count + 1;//cost
	for (j = 0; j < (time_block - sample_time); j++)
	{
		s[j] = glp_mip_col_val(mip, h);
		cost[j + sample_time] = s[j] * price2[j + sample_time] * delta_T;
		opt_cost_result += cost[j + sample_time];
		h = (h + variable);
	}


	h = app_count + 6;//sell
	for (j = 0; j < (time_block - sample_time); j++)
	{
		s[j] = glp_mip_col_val(mip, h);
		sell[j + sample_time] = s[j] * price2[j + sample_time] * delta_T;
		opt_sell_result += sell[j + sample_time];
		//opt_cost_result -= sell[j + sample_time];
		h = (h + variable);
	}

	h = app_count + 11;//FC total cost
	for (j = 0; j < (time_block - sample_time); j++)
	{
		s[j] = glp_mip_col_val(mip, h);
		FC_cost[j + sample_time] = s[j] * Hydro_Price / Hydro_Cons  * delta_T;
		Hydrogen_com[j + sample_time] = s[j] * Hydro_Cons * delta_T;
		opt_FC_cost_result += FC_cost[j + sample_time];
		opt_Hydrogen_result += Hydrogen_com[j + sample_time];
		h = (h + variable);
	}

	h = app_count + 10;//FC cost 
	for (j = 0; j < (time_block - sample_time); j++)
	{
		s[j] = glp_mip_col_val(mip, h);
		FC_every_cost[j + sample_time] = FC_cost[j + sample_time] / (s[j] * delta_T);   //every kW cost
	    //FC_cost[j + sample_time] = FC_cost[j + sample_time]/s[j];
		//printf(fc)
	//	Hydrogen_com[j + sample_time] = s[j] * 0.04 / 0.35* delta_T;
	//	opt_FC_cost_result += FC_cost[j + sample_time];
	//	opt_Hydrogen_result += Hydrogen_com[j + sample_time];
		h = (h + variable);
	}




	// update to sql
	if (sample_time == 0)
	{	
		//now grid 
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 1, now_grid[0], now_grid[1], now_grid[2], now_grid[3], now_grid[4], now_grid[5], now_grid[6], now_grid[7], now_grid[8], now_grid[9], now_grid[10], now_grid[11], now_grid[12], now_grid[13], now_grid[14], now_grid[15], now_grid[16], now_grid[17], now_grid[18], now_grid[19], now_grid[20], now_grid[21], now_grid[22], now_grid[23], now_grid[24], now_grid[25], now_grid[26], now_grid[27], now_grid[28], now_grid[29], now_grid[30], now_grid[31], now_grid[32], now_grid[33], now_grid[34], now_grid[35], now_grid[36], now_grid[37], now_grid[38], now_grid[39], now_grid[40], now_grid[41], now_grid[42], now_grid[43], now_grid[44], now_grid[45], now_grid[46], now_grid[47], now_grid[48], now_grid[49], now_grid[50], now_grid[51], now_grid[52], now_grid[53], now_grid[54], now_grid[55], now_grid[56], now_grid[57], now_grid[58], now_grid[59], now_grid[60], now_grid[61], now_grid[62], now_grid[63], now_grid[64], now_grid[65], now_grid[66], now_grid[67], now_grid[68], now_grid[69], now_grid[70], now_grid[71], now_grid[72], now_grid[73], now_grid[74], now_grid[75], now_grid[76], now_grid[77], now_grid[78], now_grid[79], now_grid[80], now_grid[81], now_grid[82], now_grid[83], now_grid[84], now_grid[85], now_grid[86], now_grid[87], now_grid[88], now_grid[89], now_grid[90], now_grid[91], now_grid[92], now_grid[93], now_grid[94], now_grid[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 18 ", now_power_result);
		mysql_query(mysql_con, sql_buffer);
		
		//variing grid 
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 2, varying_grid[0], varying_grid[1], varying_grid[2], varying_grid[3], varying_grid[4], varying_grid[5], varying_grid[6], varying_grid[7], varying_grid[8], varying_grid[9], varying_grid[10], varying_grid[11], varying_grid[12], varying_grid[13], varying_grid[14], varying_grid[15], varying_grid[16], varying_grid[17], varying_grid[18], varying_grid[19], varying_grid[20], varying_grid[21], varying_grid[22], varying_grid[23], varying_grid[24], varying_grid[25], varying_grid[26], varying_grid[27], varying_grid[28], varying_grid[29], varying_grid[30], varying_grid[31], varying_grid[32], varying_grid[33], varying_grid[34], varying_grid[35], varying_grid[36], varying_grid[37], varying_grid[38], varying_grid[39], varying_grid[40], varying_grid[41], varying_grid[42], varying_grid[43], varying_grid[44], varying_grid[45], varying_grid[46], varying_grid[47], varying_grid[48], varying_grid[49], varying_grid[50], varying_grid[51], varying_grid[52], varying_grid[53], varying_grid[54], varying_grid[55], varying_grid[56], varying_grid[57], varying_grid[58], varying_grid[59], varying_grid[60], varying_grid[61], varying_grid[62], varying_grid[63], varying_grid[64], varying_grid[65], varying_grid[66], varying_grid[67], varying_grid[68], varying_grid[69], varying_grid[70], varying_grid[71], varying_grid[72], varying_grid[73], varying_grid[74], varying_grid[75], varying_grid[76], varying_grid[77], varying_grid[78], varying_grid[79], varying_grid[80], varying_grid[81], varying_grid[82], varying_grid[83], varying_grid[84], varying_grid[85], varying_grid[86], varying_grid[87], varying_grid[88], varying_grid[89], varying_grid[90], varying_grid[91], varying_grid[92], varying_grid[93], varying_grid[94], varying_grid[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 19 ", var_grid_result);
		mysql_query(mysql_con, sql_buffer);

		////minimum cost
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column,3, cost[0], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6], cost[7], cost[8], cost[9], cost[10], cost[11], cost[12], cost[13], cost[14], cost[15], cost[16], cost[17], cost[18], cost[19], cost[20], cost[21], cost[22], cost[23], cost[24], cost[25], cost[26], cost[27], cost[28], cost[29], cost[30], cost[31], cost[32], cost[33], cost[34], cost[35], cost[36], cost[37], cost[38], cost[39], cost[40], cost[41], cost[42], cost[43], cost[44], cost[45], cost[46], cost[47], cost[48], cost[49], cost[50], cost[51], cost[52], cost[53], cost[54], cost[55], cost[56], cost[57], cost[58], cost[59], cost[60], cost[61], cost[62], cost[63], cost[64], cost[65], cost[66], cost[67], cost[68], cost[69], cost[70], cost[71], cost[72], cost[73], cost[74], cost[75], cost[76], cost[77], cost[78], cost[79], cost[80], cost[81], cost[82], cost[83], cost[84], cost[85], cost[86], cost[87], cost[88], cost[89], cost[90], cost[91], cost[92], cost[93], cost[94], cost[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 20 ", opt_cost_result);
		mysql_query(mysql_con, sql_buffer);

		//maximum sell
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 4, sell[0], sell[1], sell[2], sell[3], sell[4], sell[5], sell[6], sell[7], sell[8], sell[9], sell[10], sell[11], sell[12], sell[13], sell[14], sell[15], sell[16], sell[17], sell[18], sell[19], sell[20], sell[21], sell[22], sell[23], sell[24], sell[25], sell[26], sell[27], sell[28], sell[29], sell[30], sell[31], sell[32], sell[33], sell[34], sell[35], sell[36], sell[37], sell[38], sell[39], sell[40], sell[41], sell[42], sell[43], sell[44], sell[45], sell[46], sell[47], sell[48], sell[49], sell[50], sell[51], sell[52], sell[53], sell[54], sell[55], sell[56], sell[57], sell[58], sell[59], sell[60], sell[61], sell[62], sell[63], sell[64], sell[65], sell[66], sell[67], sell[68], sell[69], sell[70], sell[71], sell[72], sell[73], sell[74], sell[75], sell[76], sell[77], sell[78], sell[79], sell[80], sell[81], sell[82], sell[83], sell[84], sell[85], sell[86], sell[87], sell[88], sell[89], sell[90], sell[91], sell[92], sell[93], sell[94], sell[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 21 ", opt_sell_result);
		mysql_query(mysql_con, sql_buffer);

		//minimum FC cost
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 5, FC_cost[0], FC_cost[1], FC_cost[2], FC_cost[3], FC_cost[4], FC_cost[5], FC_cost[6], FC_cost[7], FC_cost[8], FC_cost[9], FC_cost[10], FC_cost[11], FC_cost[12], FC_cost[13], FC_cost[14], FC_cost[15], FC_cost[16], FC_cost[17], FC_cost[18], FC_cost[19], FC_cost[20], FC_cost[21], FC_cost[22], FC_cost[23], FC_cost[24], FC_cost[25], FC_cost[26], FC_cost[27], FC_cost[28], FC_cost[29], FC_cost[30], FC_cost[31], FC_cost[32], FC_cost[33], FC_cost[34], FC_cost[35], FC_cost[36], FC_cost[37], FC_cost[38], FC_cost[39], FC_cost[40], FC_cost[41], FC_cost[42], FC_cost[43], FC_cost[44], FC_cost[45], FC_cost[46], FC_cost[47], FC_cost[48], FC_cost[49], FC_cost[50], FC_cost[51], FC_cost[52], FC_cost[53], FC_cost[54], FC_cost[55], FC_cost[56], FC_cost[57], FC_cost[58], FC_cost[59], FC_cost[60], FC_cost[61], FC_cost[62], FC_cost[63], FC_cost[64], FC_cost[65], FC_cost[66], FC_cost[67], FC_cost[68], FC_cost[69], FC_cost[70], FC_cost[71], FC_cost[72], FC_cost[73], FC_cost[74], FC_cost[75], FC_cost[76], FC_cost[77], FC_cost[78], FC_cost[79], FC_cost[80], FC_cost[81], FC_cost[82], FC_cost[83], FC_cost[84], FC_cost[85], FC_cost[86], FC_cost[87], FC_cost[88], FC_cost[89], FC_cost[90], FC_cost[91], FC_cost[92], FC_cost[93], FC_cost[94], FC_cost[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 22 ", opt_FC_cost_result);
		mysql_query(mysql_con, sql_buffer);

		//minimum hydrogen consumption
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 6, Hydrogen_com[0], Hydrogen_com[1], Hydrogen_com[2], Hydrogen_com[3], Hydrogen_com[4], Hydrogen_com[5], Hydrogen_com[6], Hydrogen_com[7], Hydrogen_com[8], Hydrogen_com[9], Hydrogen_com[10], Hydrogen_com[11], Hydrogen_com[12], Hydrogen_com[13], Hydrogen_com[14], Hydrogen_com[15], Hydrogen_com[16], Hydrogen_com[17], Hydrogen_com[18], Hydrogen_com[19], Hydrogen_com[20], Hydrogen_com[21], Hydrogen_com[22], Hydrogen_com[23], Hydrogen_com[24], Hydrogen_com[25], Hydrogen_com[26], Hydrogen_com[27], Hydrogen_com[28], Hydrogen_com[29], Hydrogen_com[30], Hydrogen_com[31], Hydrogen_com[32], Hydrogen_com[33], Hydrogen_com[34], Hydrogen_com[35], Hydrogen_com[36], Hydrogen_com[37], Hydrogen_com[38], Hydrogen_com[39], Hydrogen_com[40], Hydrogen_com[41], Hydrogen_com[42], Hydrogen_com[43], Hydrogen_com[44], Hydrogen_com[45], Hydrogen_com[46], Hydrogen_com[47], Hydrogen_com[48], Hydrogen_com[49], Hydrogen_com[50], Hydrogen_com[51], Hydrogen_com[52], Hydrogen_com[53], Hydrogen_com[54], Hydrogen_com[55], Hydrogen_com[56], Hydrogen_com[57], Hydrogen_com[58], Hydrogen_com[59], Hydrogen_com[60], Hydrogen_com[61], Hydrogen_com[62], Hydrogen_com[63], Hydrogen_com[64], Hydrogen_com[65], Hydrogen_com[66], Hydrogen_com[67], Hydrogen_com[68], Hydrogen_com[69], Hydrogen_com[70], Hydrogen_com[71], Hydrogen_com[72], Hydrogen_com[73], Hydrogen_com[74], Hydrogen_com[75], Hydrogen_com[76], Hydrogen_com[77], Hydrogen_com[78], Hydrogen_com[79], Hydrogen_com[80], Hydrogen_com[81], Hydrogen_com[82], Hydrogen_com[83], Hydrogen_com[84], Hydrogen_com[85], Hydrogen_com[86], Hydrogen_com[87], Hydrogen_com[88], Hydrogen_com[89], Hydrogen_com[90], Hydrogen_com[91], Hydrogen_com[92], Hydrogen_com[93], Hydrogen_com[94], Hydrogen_com[95]);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 23 ", opt_Hydrogen_result);
		mysql_query(mysql_con, sql_buffer);
		//FC cost every kW
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO cost (control_id,%s) VALUES('%d','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f');"
			, column, 7, FC_every_cost[0], FC_every_cost[1], FC_every_cost[2], FC_every_cost[3], FC_every_cost[4], FC_every_cost[5], FC_every_cost[6], FC_every_cost[7], FC_every_cost[8], FC_every_cost[9], FC_every_cost[10], FC_every_cost[11], FC_every_cost[12], FC_every_cost[13], FC_every_cost[14], FC_every_cost[15], FC_every_cost[16], FC_every_cost[17], FC_every_cost[18], FC_every_cost[19], FC_every_cost[20], FC_every_cost[21], FC_every_cost[22], FC_every_cost[23], FC_every_cost[24], FC_every_cost[25], FC_every_cost[26], FC_every_cost[27], FC_every_cost[28], FC_every_cost[29], FC_every_cost[30], FC_every_cost[31], FC_every_cost[32], FC_every_cost[33], FC_every_cost[34], FC_every_cost[35], FC_every_cost[36], FC_every_cost[37], FC_every_cost[38], FC_every_cost[39], FC_every_cost[40], FC_every_cost[41], FC_every_cost[42], FC_every_cost[43], FC_every_cost[44], FC_every_cost[45], FC_every_cost[46], FC_every_cost[47], FC_every_cost[48], FC_every_cost[49], FC_every_cost[50], FC_every_cost[51], FC_every_cost[52], FC_every_cost[53], FC_every_cost[54], FC_every_cost[55], FC_every_cost[56], FC_every_cost[57], FC_every_cost[58], FC_every_cost[59], FC_every_cost[60], FC_every_cost[61], FC_every_cost[62], FC_every_cost[63], FC_every_cost[64], FC_every_cost[65], FC_every_cost[66], FC_every_cost[67], FC_every_cost[68], FC_every_cost[69], FC_every_cost[70], FC_every_cost[71], FC_every_cost[72], FC_every_cost[73], FC_every_cost[74], FC_every_cost[75], FC_every_cost[76], FC_every_cost[77], FC_every_cost[78], FC_every_cost[79], FC_every_cost[80], FC_every_cost[81], FC_every_cost[82], FC_every_cost[83], FC_every_cost[84], FC_every_cost[85], FC_every_cost[86], FC_every_cost[87], FC_every_cost[88], FC_every_cost[89], FC_every_cost[90], FC_every_cost[91], FC_every_cost[92], FC_every_cost[93], FC_every_cost[94], FC_every_cost[95]);
		mysql_query(mysql_con, sql_buffer);

	}
	else
	{
		//all grid 
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				now_grid[j] = atof(mysql_row[j]);
				now_power_result += now_grid[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, now_grid[0], now_grid[1], now_grid[2], now_grid[3], now_grid[4], now_grid[5], now_grid[6], now_grid[7], now_grid[8], now_grid[9], now_grid[10], now_grid[11], now_grid[12], now_grid[13], now_grid[14], now_grid[15], now_grid[16], now_grid[17], now_grid[18], now_grid[19], now_grid[20], now_grid[21], now_grid[22], now_grid[23], now_grid[24], now_grid[25], now_grid[26], now_grid[27], now_grid[28], now_grid[29], now_grid[30], now_grid[31], now_grid[32], now_grid[33], now_grid[34], now_grid[35], now_grid[36], now_grid[37], now_grid[38], now_grid[39], now_grid[40], now_grid[41], now_grid[42], now_grid[43], now_grid[44], now_grid[45], now_grid[46], now_grid[47], now_grid[48], now_grid[49], now_grid[50], now_grid[51], now_grid[52], now_grid[53], now_grid[54], now_grid[55], now_grid[56], now_grid[57], now_grid[58], now_grid[59], now_grid[60], now_grid[61], now_grid[62], now_grid[63], now_grid[64], now_grid[65], now_grid[66], now_grid[67], now_grid[68], now_grid[69], now_grid[70], now_grid[71], now_grid[72], now_grid[73], now_grid[74], now_grid[75], now_grid[76], now_grid[77], now_grid[78], now_grid[79], now_grid[80], now_grid[81], now_grid[82], now_grid[83], now_grid[84], now_grid[85], now_grid[86], now_grid[87], now_grid[88], now_grid[89], now_grid[90], now_grid[91], now_grid[92], now_grid[93], now_grid[94], now_grid[95], 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_free_result(mysql_result);



		//all grid 
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 2);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				varying_grid[j] = atof(mysql_row[j]);
				var_grid_result += varying_grid[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, varying_grid[0], varying_grid[1], varying_grid[2], varying_grid[3], varying_grid[4], varying_grid[5], varying_grid[6], varying_grid[7], varying_grid[8], varying_grid[9], varying_grid[10], varying_grid[11], varying_grid[12], varying_grid[13], varying_grid[14], varying_grid[15], varying_grid[16], varying_grid[17], varying_grid[18], varying_grid[19], varying_grid[20], varying_grid[21], varying_grid[22], varying_grid[23], varying_grid[24], varying_grid[25], varying_grid[26], varying_grid[27], varying_grid[28], varying_grid[29], varying_grid[30], varying_grid[31], varying_grid[32], varying_grid[33], varying_grid[34], varying_grid[35], varying_grid[36], varying_grid[37], varying_grid[38], varying_grid[39], varying_grid[40], varying_grid[41], varying_grid[42], varying_grid[43], varying_grid[44], varying_grid[45], varying_grid[46], varying_grid[47], varying_grid[48], varying_grid[49], varying_grid[50], varying_grid[51], varying_grid[52], varying_grid[53], varying_grid[54], varying_grid[55], varying_grid[56], varying_grid[57], varying_grid[58], varying_grid[59], varying_grid[60], varying_grid[61], varying_grid[62], varying_grid[63], varying_grid[64], varying_grid[65], varying_grid[66], varying_grid[67], varying_grid[68], varying_grid[69], varying_grid[70], varying_grid[71], varying_grid[72], varying_grid[73], varying_grid[74], varying_grid[75], varying_grid[76], varying_grid[77], varying_grid[78], varying_grid[79], varying_grid[80], varying_grid[81], varying_grid[82], varying_grid[83], varying_grid[84], varying_grid[85], varying_grid[86], varying_grid[87], varying_grid[88], varying_grid[89], varying_grid[90], varying_grid[91], varying_grid[92], varying_grid[93], varying_grid[94], varying_grid[95], 2);
		mysql_query(mysql_con, sql_buffer);
		mysql_free_result(mysql_result);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 19 ", var_grid_result);
		mysql_query(mysql_con, sql_buffer);


		//minimum cost
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 3);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				cost[j] = atof(mysql_row[j]);
				opt_cost_result += cost[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
		, cost[0], cost[1], cost[2], cost[3], cost[4], cost[5], cost[6], cost[7], cost[8], cost[9], cost[10], cost[11], cost[12], cost[13], cost[14], cost[15], cost[16], cost[17], cost[18], cost[19], cost[20], cost[21], cost[22], cost[23], cost[24], cost[25], cost[26], cost[27], cost[28], cost[29], cost[30], cost[31], cost[32], cost[33], cost[34], cost[35], cost[36], cost[37], cost[38], cost[39], cost[40], cost[41], cost[42], cost[43], cost[44], cost[45], cost[46], cost[47], cost[48], cost[49], cost[50], cost[51], cost[52], cost[53], cost[54], cost[55], cost[56], cost[57], cost[58], cost[59], cost[60], cost[61], cost[62], cost[63], cost[64], cost[65], cost[66], cost[67], cost[68], cost[69], cost[70], cost[71], cost[72], cost[73], cost[74], cost[75], cost[76], cost[77], cost[78], cost[79], cost[80], cost[81], cost[82], cost[83], cost[84], cost[85], cost[86], cost[87], cost[88], cost[89], cost[90], cost[91], cost[92], cost[93], cost[94], cost[95], 3);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 20 ", opt_cost_result);
		mysql_query(mysql_con, sql_buffer);

		//maximum sell
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 4);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				sell[j] = atof(mysql_row[j]);
				opt_sell_result += sell[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, sell[0], sell[1], sell[2], sell[3], sell[4], sell[5], sell[6], sell[7], sell[8], sell[9], sell[10], sell[11], sell[12], sell[13], sell[14], sell[15], sell[16], sell[17], sell[18], sell[19], sell[20], sell[21], sell[22], sell[23], sell[24], sell[25], sell[26], sell[27], sell[28], sell[29], sell[30], sell[31], sell[32], sell[33], sell[34], sell[35], sell[36], sell[37], sell[38], sell[39], sell[40], sell[41], sell[42], sell[43], sell[44], sell[45], sell[46], sell[47], sell[48], sell[49], sell[50], sell[51], sell[52], sell[53], sell[54], sell[55], sell[56], sell[57], sell[58], sell[59], sell[60], sell[61], sell[62], sell[63], sell[64], sell[65], sell[66], sell[67], sell[68], sell[69], sell[70], sell[71], sell[72], sell[73], sell[74], sell[75], sell[76], sell[77], sell[78], sell[79], sell[80], sell[81], sell[82], sell[83], sell[84], sell[85], sell[86], sell[87], sell[88], sell[89], sell[90], sell[91], sell[92], sell[93], sell[94], sell[95], 4);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 21 ", opt_sell_result);
		mysql_query(mysql_con, sql_buffer);


		//minimum FC cost
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 5);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				FC_cost[j] = atof(mysql_row[j]);
				opt_FC_cost_result += FC_cost[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, FC_cost[0], FC_cost[1], FC_cost[2], FC_cost[3], FC_cost[4], FC_cost[5], FC_cost[6], FC_cost[7], FC_cost[8], FC_cost[9], FC_cost[10], FC_cost[11], FC_cost[12], FC_cost[13], FC_cost[14], FC_cost[15], FC_cost[16], FC_cost[17], FC_cost[18], FC_cost[19], FC_cost[20], FC_cost[21], FC_cost[22], FC_cost[23], FC_cost[24], FC_cost[25], FC_cost[26], FC_cost[27], FC_cost[28], FC_cost[29], FC_cost[30], FC_cost[31], FC_cost[32], FC_cost[33], FC_cost[34], FC_cost[35], FC_cost[36], FC_cost[37], FC_cost[38], FC_cost[39], FC_cost[40], FC_cost[41], FC_cost[42], FC_cost[43], FC_cost[44], FC_cost[45], FC_cost[46], FC_cost[47], FC_cost[48], FC_cost[49], FC_cost[50], FC_cost[51], FC_cost[52], FC_cost[53], FC_cost[54], FC_cost[55], FC_cost[56], FC_cost[57], FC_cost[58], FC_cost[59], FC_cost[60], FC_cost[61], FC_cost[62], FC_cost[63], FC_cost[64], FC_cost[65], FC_cost[66], FC_cost[67], FC_cost[68], FC_cost[69], FC_cost[70], FC_cost[71], FC_cost[72], FC_cost[73], FC_cost[74], FC_cost[75], FC_cost[76], FC_cost[77], FC_cost[78], FC_cost[79], FC_cost[80], FC_cost[81], FC_cost[82], FC_cost[83], FC_cost[84], FC_cost[85], FC_cost[86], FC_cost[87], FC_cost[88], FC_cost[89], FC_cost[90], FC_cost[91], FC_cost[92], FC_cost[93], FC_cost[94], FC_cost[95], 5);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 22 ", opt_FC_cost_result);
		mysql_query(mysql_con, sql_buffer);


		//hydrogen comsumption
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 6);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				Hydrogen_com[j] = atof(mysql_row[j]);
				opt_Hydrogen_result += Hydrogen_com[j];
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, Hydrogen_com[0], Hydrogen_com[1], Hydrogen_com[2], Hydrogen_com[3], Hydrogen_com[4], Hydrogen_com[5], Hydrogen_com[6], Hydrogen_com[7], Hydrogen_com[8], Hydrogen_com[9], Hydrogen_com[10], Hydrogen_com[11], Hydrogen_com[12], Hydrogen_com[13], Hydrogen_com[14], Hydrogen_com[15], Hydrogen_com[16], Hydrogen_com[17], Hydrogen_com[18], Hydrogen_com[19], Hydrogen_com[20], Hydrogen_com[21], Hydrogen_com[22], Hydrogen_com[23], Hydrogen_com[24], Hydrogen_com[25], Hydrogen_com[26], Hydrogen_com[27], Hydrogen_com[28], Hydrogen_com[29], Hydrogen_com[30], Hydrogen_com[31], Hydrogen_com[32], Hydrogen_com[33], Hydrogen_com[34], Hydrogen_com[35], Hydrogen_com[36], Hydrogen_com[37], Hydrogen_com[38], Hydrogen_com[39], Hydrogen_com[40], Hydrogen_com[41], Hydrogen_com[42], Hydrogen_com[43], Hydrogen_com[44], Hydrogen_com[45], Hydrogen_com[46], Hydrogen_com[47], Hydrogen_com[48], Hydrogen_com[49], Hydrogen_com[50], Hydrogen_com[51], Hydrogen_com[52], Hydrogen_com[53], Hydrogen_com[54], Hydrogen_com[55], Hydrogen_com[56], Hydrogen_com[57], Hydrogen_com[58], Hydrogen_com[59], Hydrogen_com[60], Hydrogen_com[61], Hydrogen_com[62], Hydrogen_com[63], Hydrogen_com[64], Hydrogen_com[65], Hydrogen_com[66], Hydrogen_com[67], Hydrogen_com[68], Hydrogen_com[69], Hydrogen_com[70], Hydrogen_com[71], Hydrogen_com[72], Hydrogen_com[73], Hydrogen_com[74], Hydrogen_com[75], Hydrogen_com[76], Hydrogen_com[77], Hydrogen_com[78], Hydrogen_com[79], Hydrogen_com[80], Hydrogen_com[81], Hydrogen_com[82], Hydrogen_com[83], Hydrogen_com[84], Hydrogen_com[85], Hydrogen_com[86], Hydrogen_com[87], Hydrogen_com[88], Hydrogen_com[89], Hydrogen_com[90], Hydrogen_com[91], Hydrogen_com[92], Hydrogen_com[93], Hydrogen_com[94], Hydrogen_com[95], 6);
		mysql_query(mysql_con, sql_buffer);
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 23 ", opt_Hydrogen_result);
		mysql_query(mysql_con, sql_buffer);

		//FC cost every kW
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM cost WHERE (control_id = '%d')", column, 7);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < time_block; j++)
		{
			if (j < sample_time)
			{
				FC_every_cost[j] = atof(mysql_row[j]);
			}
		}
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE cost set A0 = '%.3f', A1 = '%.3f', A2 = '%.3f', A3 = '%.3f', A4 = '%.3f', A5 = '%.3f', A6 = '%.3f', A7 = '%.3f', A8 = '%.3f', A9 = '%.3f', A10 = '%.3f', A11 = '%.3f', A12 = '%.3f', A13 = '%.3f', A14 = '%.3f', A15 = '%.3f', A16 = '%.3f', A17 = '%.3f', A18 = '%.3f', A19 = '%.3f', A20 = '%.3f', A21 = '%.3f', A22 = '%.3f', A23 = '%.3f', A24 = '%.3f', A25 = '%.3f', A26 = '%.3f', A27 = '%.3f', A28 = '%.3f', A29 = '%.3f', A30 = '%.3f', A31 = '%.3f', A32 = '%.3f', A33 = '%.3f', A34 = '%.3f', A35 = '%.3f', A36 = '%.3f', A37 = '%.3f', A38 = '%.3f', A39 = '%.3f', A40 = '%.3f', A41 = '%.3f', A42 = '%.3f', A43 = '%.3f', A44 = '%.3f', A45 = '%.3f', A46 = '%.3f', A47 = '%.3f', A48 = '%.3f', A49 = '%.3f', A50 = '%.3f', A51 = '%.3f', A52 = '%.3f', A53 = '%.3f', A54 = '%.3f', A55 = '%.3f', A56 = '%.3f', A57 = '%.3f', A58 = '%.3f', A59 = '%.3f', A60 = '%.3f', A61 = '%.3f', A62 = '%.3f', A63 = '%.3f', A64 = '%.3f', A65 = '%.3f', A66 = '%.3f', A67 = '%.3f', A68 = '%.3f', A69 = '%.3f', A70 = '%.3f', A71 = '%.3f', A72 = '%.3f', A73 = '%.3f', A74 = '%.3f', A75 = '%.3f', A76 = '%.3f', A77 = '%.3f', A78 = '%.3f', A79 = '%.3f', A80 = '%.3f', A81 = '%.3f', A82 = '%.3f', A83 = '%.3f', A84 = '%.3f', A85 = '%.3f', A86 = '%.3f', A87 = '%.3f', A88 = '%.3f', A89 = '%.3f', A90 = '%.3f', A91 = '%.3f', A92 = '%.3f', A93 = '%.3f', A94 = '%.3f', A95 = '%.3f' WHERE control_id='%d';"
			, FC_every_cost[0], FC_every_cost[1], FC_every_cost[2], FC_every_cost[3], FC_every_cost[4], FC_every_cost[5], FC_every_cost[6], FC_every_cost[7], FC_every_cost[8], FC_every_cost[9], FC_every_cost[10], FC_every_cost[11], FC_every_cost[12], FC_every_cost[13], FC_every_cost[14], FC_every_cost[15], FC_every_cost[16], FC_every_cost[17], FC_every_cost[18], FC_every_cost[19], FC_every_cost[20], FC_every_cost[21], FC_every_cost[22], FC_every_cost[23], FC_every_cost[24], FC_every_cost[25], FC_every_cost[26], FC_every_cost[27], FC_every_cost[28], FC_every_cost[29], FC_every_cost[30], FC_every_cost[31], FC_every_cost[32], FC_every_cost[33], FC_every_cost[34], FC_every_cost[35], FC_every_cost[36], FC_every_cost[37], FC_every_cost[38], FC_every_cost[39], FC_every_cost[40], FC_every_cost[41], FC_every_cost[42], FC_every_cost[43], FC_every_cost[44], FC_every_cost[45], FC_every_cost[46], FC_every_cost[47], FC_every_cost[48], FC_every_cost[49], FC_every_cost[50], FC_every_cost[51], FC_every_cost[52], FC_every_cost[53], FC_every_cost[54], FC_every_cost[55], FC_every_cost[56], FC_every_cost[57], FC_every_cost[58], FC_every_cost[59], FC_every_cost[60], FC_every_cost[61], FC_every_cost[62], FC_every_cost[63], FC_every_cost[64], FC_every_cost[65], FC_every_cost[66], FC_every_cost[67], FC_every_cost[68], FC_every_cost[69], FC_every_cost[70], FC_every_cost[71], FC_every_cost[72], FC_every_cost[73], FC_every_cost[74], FC_every_cost[75], FC_every_cost[76], FC_every_cost[77], FC_every_cost[78], FC_every_cost[79], FC_every_cost[80], FC_every_cost[81], FC_every_cost[82], FC_every_cost[83], FC_every_cost[84], FC_every_cost[85], FC_every_cost[86], FC_every_cost[87], FC_every_cost[88], FC_every_cost[89], FC_every_cost[90], FC_every_cost[91], FC_every_cost[92], FC_every_cost[93], FC_every_cost[94], FC_every_cost[95], 7);
		mysql_query(mysql_con, sql_buffer);
	}


	//NOW taipower cost
	//float tmp_power[6] = { 0.0 };
	if (now_power_result <= (120.0 / 30.0))
	{
		price_sum_now_power = now_power_result*P_1;
	}
	else if ((now_power_result > (120.0 / 30.0)) & (now_power_result <= 330.0 / 30.0))
	{
		price_sum_now_power = (120.0*P_1 + (now_power_result*30.0 - 120.0)*P_2)/30.0;
	}
	else if ((now_power_result > (330.0 / 30.0)) & (now_power_result <= 500.0 / 30.0))
	{
		price_sum_now_power = (120.0 * P_1 + (330.0 - 120.0) * P_2 + (now_power_result*30.0 - 330.0)*P_3)/30.0;
	}
	else if ((now_power_result > (500.0 / 30.0)) & (now_power_result <= 700.0 / 30.0))
	{
		price_sum_now_power = (120.0 *P_1 + (330.0 - 120.0) *P_2 + (500.0 - 330.0) *P_3 + (now_power_result*30.0 - 500.0)*P_4)/30.0;
	}
	else if ((now_power_result > (700.0 / 30.0)) & (now_power_result <= 1000.0 / 30.0))
	{
		price_sum_now_power = (120.0 * P_1 + (330.0 - 120.0)*P_2 + (500.0 - 330.0) *P_3 + (700.0 - 500.0)*P_4 + (now_power_result*30.0 - 700.0 )*P_5)/30.0;
	}
	else if (now_power_result > (1000.0 / 30.0))
	{
		price_sum_now_power = (120.0 *P_1 + (330.0 - 120.0) *P_2 + (500.0 - 330.0) *P_3 + (700.0 - 500.0) *P_4 + (1000.0 - 700.0) *P_5 + (now_power_result*30.0 - 1000.0 )*P_6)/30.0;
	}
	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value = %f WHERE parameter_id = 18 ", price_sum_now_power);
	mysql_query(mysql_con, sql_buffer);

	printf("now_power_result:%f\n",now_power_result);
	printf("NOW_grid_bill:%f\n", price_sum_now_power);
	printf("ALL_grid_bill:%f\n", var_grid_result);
	printf("Grid_cost:%f\n", opt_cost_result);
	printf("Grid_sell:%f\n", opt_sell_result);
	printf("FC_cost:%f\n", opt_FC_cost_result);
	printf("hydrogen comsumotion:%f\n", opt_Hydrogen_result);

	step1_bill = opt_cost_result- opt_sell_result;
	step1_sell = opt_sell_result;


	//mysql_close(mysql_con);   //do not add 
	glp_delete_prob(mip);




	delete[] ia, ja, ar, s,cost;
	delete[] power1;
	

	return;
}
