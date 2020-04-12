#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glpk.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include <mysql/mysql.h>
#include "HEMS.h" 


int interrupt_num = 0, uninterrupt_num = 0, app_count = 0, sample_time = 0, variable = 0, divide = 4, time_block = 96, point_num = 6;
int h, i, j, k, m, n = 0;
double z = 0;
float Pgrid_max = 0.0, delta_T = 0.25;
char sql_buffer[2000] = { '\0' };

time_t t = time(NULL);
struct tm now_time = *localtime(&t);

char column[400] = "A0,A1,A2,A3,A4,A5,A6,A7,A8,A9,A10,A11,A12,A13,A14,A15,A16,A17,A18,A19,A20,A21,A22,A23,A24,A25,A26,A27,A28,A29,A30,A31,A32,A33,A34,A35,A36,A37,A38,A39,A40,A41,A42,A43,A44,A45,A46,A47,A48,A49,A50,A51,A52,A53,A54,A55,A56,A57,A58,A59,A60,A61,A62,A63,A64,A65,A66,A67,A68,A69,A70,A71,A72,A73,A74,A75,A76,A77,A78,A79,A80,A81,A82,A83,A84,A85,A86,A87,A88,A89,A90,A91,A92,A93,A94,A95";

MYSQL *mysql_con = mysql_init(NULL);
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;

int main(void) {
    
    
    if ((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "wang", 6666, NULL, 0)) == NULL) {

		printf("Failed to connect to Mysql!\n");
		system("pause");
		return 0;

	}
	printf("Connect to Mysql sucess!!\n");
	mysql_set_character_set(mysql_con, "utf8");

	snprintf(sql_buffer, sizeof(sql_buffer), "TRUNCATE TABLE control_status");      //clean control_status;
	mysql_query(mysql_con, sql_buffer);

    // get num of interrupt group 
    snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 "); 
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	interrupt_num = atoi(mysql_row[0]); // 12
	mysql_free_result(mysql_result);
	printf("interruptable app num:%d\n", interrupt_num);

    // get num of uninterrupt group 
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id=2 "); 
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	uninterrupt_num = atoi(mysql_row[0]); // 2
	mysql_free_result(mysql_result);
	printf("uninterruptable app num:%d\n", uninterrupt_num);

	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT value FROM `LP_BASE_PARM` WHERE parameter_id = %d", 13);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	Pgrid_max = atof(mysql_row[0]);	
	mysql_free_result(mysql_result);

	printf("Pgrid_max:%.2f\n", Pgrid_max);

    app_count = interrupt_num + uninterrupt_num;  // 14
	variable = app_count + 1;  // 買電狀態
	int *position = new int[app_count];
    float **INT_power = NEW2D(interrupt_num, 4, float);
    float **UNINT_power = NEW2D(uninterrupt_num, 4, float);

    for (i = 1; i < interrupt_num + 1; i++) {

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time, operation_time, power1 FROM load_list WHERE group_id = 1 ORDER BY number ASC LIMIT %d,1", i -1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < 4; j++) 
        { INT_power[i - 1][j] = atof(mysql_row[j]);	}
		mysql_free_result(mysql_result);

	}

	for (i = 1; i < uninterrupt_num + 1; i++)   //不可中斷
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT start_time, end_time, operation_time, power1 FROM load_list WHERE group_id = 2 ORDER BY number ASC LIMIT %d,1", i - 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		for (j = 0; j < 4; j++)
		{ UNINT_power[i - 1][j] = atof(mysql_row[j]); }
		mysql_free_result(mysql_result);
	}
	
	float *price = new float[24];
	// interrupt
    int *interrupt_start = new int[interrupt_num];
	int *interrupt_end = new int[interrupt_num];
	int *interrupt_ot = new int[interrupt_num];
	int *interrupt_reot = new int[interrupt_num];
	float *interrupt_p = new float[interrupt_num];
	// uninterrupt
	int *uninterrupt_start = new int[uninterrupt_num];
	int *uninterrupt_end = new int[uninterrupt_num];
	int *uninterrupt_ot = new int[uninterrupt_num];
	int *uninterrupt_reot = new int[uninterrupt_num];
	float *uninterrupt_p = new float[uninterrupt_num];
	int *uninterrupt_flag = new int[uninterrupt_num];

    // initialize INT_power[interrupt num][4] = 0
    for (i = 0; i < interrupt_num; i++) {

		interrupt_start[i] = 0;
		interrupt_end[i] = 0;
		interrupt_ot[i] = 0;
		interrupt_reot[i] = 0;
		interrupt_p[i] = 0.0;

	}
    // interrupt load array: INT_power[interrupt num][4] 
	printf("\ninterrupt multi array: \n");
	printf("St  End  Ot  ReOt\n");
    for (i = 0; i < interrupt_num; i++)	{

		interrupt_start[i] = ((int)(INT_power[i][0] * divide));
		interrupt_end[i] = ((int)(INT_power[i][1] * divide)) - 1;
		interrupt_ot[i] = ((int)(INT_power[i][2] * divide));
		interrupt_p[i] = INT_power[i][3];
		
		printf("%d  %d   %d  %.3f  ", interrupt_start[i], interrupt_end[i], interrupt_ot[i], interrupt_p[i]);
		printf("\n");
	
    }

	// initialize UNINT_power[uninterrupt num][4] = 0
    for (i = 0; i < uninterrupt_num; i++) {

		uninterrupt_start[i] = 0;
		uninterrupt_end[i] = 0;
		uninterrupt_ot[i] = 0;
		uninterrupt_reot[i] = 0;
		uninterrupt_p[i] = 0.0;
		uninterrupt_flag[i] = 0;
	}
    // interrupt load array: INT_power[interrupt num][4] 
	printf("\nuninterrupt multi array: \n");
	printf("St  End   Ot   ReOt\n");
    for (i = 0; i < uninterrupt_num; i++) {

		uninterrupt_start[i] = ((int)(UNINT_power[i][0] * divide));
		uninterrupt_end[i] = ((int)(UNINT_power[i][1] * divide)) - 1;
		uninterrupt_ot[i] = ((int)(UNINT_power[i][2] * divide));
		uninterrupt_p[i] = UNINT_power[i][3];
		printf("%d  %d   %d  %.3f  ", uninterrupt_start[i], uninterrupt_end[i], uninterrupt_ot[i], uninterrupt_p[i]);
		printf("\n");
	
    }
    // price
    for (i = 1; i < 25; i++) {

		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT price_value FROM price WHERE price_period = %d", i - 1);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		price[i - 1] = atof(mysql_row[0]);					
		memset(sql_buffer, 0, sizeof(sql_buffer));
		mysql_free_result(mysql_result);

	}

	printf("\nposition:\n");
	for (i = 0; i < app_count; i++) {
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		position[i] = atoi(mysql_row[0]);
		mysql_free_result(mysql_result);
		printf("%d ",position[i]);
	}
	printf("\n");

    GLPK(interrupt_start, interrupt_end, interrupt_ot, interrupt_reot, interrupt_p, uninterrupt_start, uninterrupt_end, uninterrupt_ot, uninterrupt_reot, uninterrupt_p, uninterrupt_flag, app_count, price, position);
}

void GLPK(int *interrupt_start, int *interrupt_end, int *interrupt_ot, int *interrupt_reot, float *interrupt_p, int *uninterrupt_start, int *uninterrupt_end, int *uninterrupt_ot, int *uninterrupt_reot, float *uninterrupt_p, int *uninterrupt_flag, int app_count, float *price, int *position)
{
	int *buff = new int[app_count];	//存放剩餘執行次數(The number of remaining executions)
	for (i = 0; i < app_count; i++)
	{
		buff[i] = 0;
	}
	//get now time that can used in the real experiment
	int noo;
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide)) + 1;
	}
	else
	{
		noo = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	printf("sampleNoo:%d\n", noo);

	float *price2 = new float[time_block];
	for (int x = 0; x < 24; x++)	
	{
		for (int y = x*divide; y < (x*divide)+divide; y++)
		{
			price2[y] = price[x];
		}
	}
	
	// find flag is 1 or not
	int flag = 0;
	if (sample_time != 0)
	{
		for (i = 0; i < uninterrupt_num; i++)
		{
			// flag status is equip_id 40 41 in original control status
			flag = 0;
			snprintf(sql_buffer, sizeof(sql_buffer), "SELECT %s FROM control_status WHERE equip_id = '%d'", column, (i + app_count + 15 + (point_num - 1) * 2));
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
	}

	//可中斷負載 (Interrupt load)
	for (i = 0; i < interrupt_num; i++)	
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

	//不可中斷負載 (Uninterrupt load)
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
 
	float *s = new float[time_block];
	/*============================ 總規劃功率矩陣(Total planning power matrix) ====================================*/
	float **power1 = NEW2D((((time_block - sample_time) * 1) + app_count), (variable * (time_block - sample_time)), float);

	/*============================ GLPK參數矩陣定義(GLPK parameter matrix definition) ==================================*/
	glp_prob *mip;
	int *ia = new int[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1]; 			// Row
	int *ja = new int[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1];			// Column
	double *ar = new double[((((time_block - sample_time) * 1) + app_count) * (variable * (time_block - sample_time))) + 1];		// structural variable
	/*============================== GLPK變數宣告(GLPK variable definition) =====================================*/
	mip = glp_create_prob();
	glp_set_prob_name(mip, "hardware_algorithm_case");
	glp_set_obj_dir(mip, GLP_MIN);
	glp_add_rows(mip, (((time_block - sample_time) * 1) + app_count));
	glp_add_cols(mip, (variable * (time_block - sample_time)));	

	/*=============================== 初始化矩陣(initial the matrix) ======================================*/
	for (m = 0; m < ((time_block - sample_time) * 1) + app_count; m++)
	{
		for (n = 0; n < (variable * (time_block - sample_time)); n++)
		{
			power1[m][n] = 0.0;
		}
	}

	for (h = 0; h < interrupt_num; h++)		// 可中斷負載(Interrupt load)
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

	// 決定是否輸出市電(Decide whether to buy electricity from utility)
	for (i = 0; i < (time_block - sample_time); i++)
	{
		power1[app_count + i][i*variable + app_count] = -1.0; // Pgrid
	}

	// 平衡式(Balanced function)
	for (h = 0; h < interrupt_num; h++)	// 可中斷負載(Interrupt load)
	{
		if ((interrupt_end[h] - sample_time) >= 0)
		{
			if ((interrupt_start[h] - sample_time) >= 0)
			{
				for (i = (interrupt_start[h] - sample_time); i <= (interrupt_end[h] - sample_time); i++)
				{
					power1[app_count + i][i*variable + h] = interrupt_p[h];
					// printf("[%d][%d] = [%.1f] %dyes\n",app_count + i, i*variable + h, interrupt_p[h],h);
				}
			}
			else if ((interrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (interrupt_end[h] - sample_time); i++)	
				{
					power1[app_count + i][i*variable + h] = interrupt_p[h];
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
					power1[(time_block - sample_time) * 1 + app_count + 1 + i][i*variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
			else if ((uninterrupt_start[h] - sample_time) < 0)
			{
				for (i = 0; i <= (uninterrupt_end[h] - sample_time); i++)
				{
					power1[(time_block - sample_time) * 1 + app_count + 1 + i][i*variable + h + interrupt_num] = uninterrupt_p[h];
				}
			}
		}
	}
	/*============================== 宣告限制式條件範圍(row) ===============================*/
	// GLPK讀列從1開始
	// 限制式-家庭負載最低耗能
	for (i = 1; i <= interrupt_num; i++)	// 可中斷負載(Interrupt load)
	{
		glp_set_row_name(mip, i, "");
		glp_set_row_bnds(mip, i, GLP_LO, ((float)interrupt_reot[i - 1]), 0.0);	// ok
	}

	// 決定是否輸出市電
	for (i = 1; i <= (time_block - sample_time); i++)
	{
		glp_set_row_name(mip, (app_count + i), "");
		glp_set_row_bnds(mip, (app_count + i), GLP_UP, 0.0, 0.0);
	}
	
	/*============================== 宣告決策變數(column) ================================*/
	for (i = 0; i < (time_block - sample_time); i++)
	{
		for (j = 1; j <= app_count; j++)
		{
			glp_set_col_bnds(mip, (j + i*variable), GLP_DB, 0.0, 1.0);	// 負載決策變數
			glp_set_col_kind(mip, (j + i*variable), GLP_BV);
		}
		glp_set_col_bnds(mip, ((app_count + 1) + i*variable), GLP_DB, 0.0, Pgrid_max);	// 決定市電輸出功率  一定要大於總負載功率才不會有太大問題
		glp_set_col_kind(mip, ((app_count + 1) + i*variable), GLP_CV);
	}
	

	/*============================== 宣告目標式參數(column) ===============================*/
	for (j = 0; j < (time_block - sample_time); j++)
	{
		glp_set_obj_coef(mip, (app_count + 1 + j*variable), price2[j + sample_time] * delta_T);		// 單目標cost(步驟一)
	}

	/*============================== GLPK寫入矩陣(ia,ja,ar) ===============================*/
	for (i = 0; i < (((time_block - sample_time) * 1) + app_count); i++)
	{
		for (j = 0; j < (variable * (time_block - sample_time)); j++)
		{
			ia[i*((time_block - sample_time)*variable) + j + 1] = i + 1;
			ja[i*((time_block - sample_time)*variable) + j + 1] = j + 1;
			ar[i*((time_block - sample_time)*variable) + j + 1] = power1[i][j];
		}
	}
	printf("\nGLPK array finish\n\n");	
	/*============================== GLPK讀取資料矩陣 ====================================*/
	glp_load_matrix(mip, (((time_block - sample_time) * 1) + app_count)*(variable * (time_block - sample_time)), ia, ja, ar);

	glp_iocp parm;
	glp_init_iocp(&parm);
	parm.tm_lim = 100000;
        
	parm.presolve = GLP_ON;
	parm.gmi_cuts = GLP_ON;
	parm.fp_heur = GLP_ON;
	parm.bt_tech = GLP_BT_BFS;
	parm.br_tech = GLP_BR_PCH;

	int err = glp_intopt(mip, &parm);
	z = glp_mip_obj_val(mip);
	printf("%f\n",glp_mip_col_val(mip,1));
	printf("%f\n",glp_mip_col_val(mip,2));
	printf("%f\n",glp_mip_col_val(mip,3));
	printf("%.2f\n", glp_mip_col_val(mip,4));

	printf("\n");
	printf("sol = %f; \n", z);

	if (z == 0.0 && glp_mip_col_val(mip, (app_count + 7)) == 0.0)
	{
		printf("No Solotion,give up the solution\n");
		system("pause");
		exit(1);
	}

	/*============================== 將決策變數結果輸出 ==================================*/
	for (i = 1; i <= variable; i++)
	{
		h = i;

		if (sample_time == 0)
		{
			for (j = 0; j < time_block; j++)
			{
				s[j] = glp_mip_col_val(mip, h);

				if (i <= app_count && j== noo)
				{
					snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status set status = %d where id=%d ", (int)s[j], position[i-1]);
					mysql_query(mysql_con, sql_buffer);
					snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_history (id,status,schedule) VALUES(%d,%d,%d)", position[i - 1], (int)s[j], 1);
					mysql_query(mysql_con, sql_buffer);
				}
				h = (h + variable);
			}

			snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO control_status (%s, equip_id) VALUES('%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%.3f','%d');"
				, column, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7], s[8], s[9], s[10], s[11], s[12], s[13], s[14], s[15], s[16], s[17], s[18], s[19], s[20], s[21], s[22], s[23], s[24], s[25], s[26], s[27], s[28], s[29], s[30], s[31], s[32], s[33], s[34], s[35], s[36], s[37], s[38], s[39], s[40], s[41], s[42], s[43], s[44], s[45], s[46], s[47], s[48], s[49], s[50], s[51], s[52], s[53], s[54], s[55], s[56], s[57], s[58], s[59], s[60], s[61], s[62], s[63], s[64], s[65], s[66], s[67], s[68], s[69], s[70], s[71], s[72], s[73], s[74], s[75], s[76], s[77], s[78], s[79], s[80], s[81], s[82], s[83], s[84], s[85], s[86], s[87], s[88], s[89], s[90], s[91], s[92], s[93], s[94], s[95], i);
			mysql_query(mysql_con, sql_buffer);
			memset(sql_buffer, 0, sizeof(sql_buffer));
			printf("%d,", i);
		}
	}
	printf("\n");
	//end
}



