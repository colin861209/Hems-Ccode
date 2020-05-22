#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <iostream>
#include <mysql.h>
#include <mysql/mysql.h>

#include "SQLFunction.h"

MYSQL *mysql_con = mysql_init(NULL);
MYSQL_RES *mysql_result;
MYSQL_ROW mysql_row;


MYSQL_ROW fetch_row_value() {

	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	mysql_free_result(mysql_result);
	return 0;
}

int turn_int(int row_num) { return atoi(mysql_row[row_num]); }

float turn_float(int row_num) { return atof(mysql_row[row_num]); }

float turn_value_to_float(int row_num) {
	
	fetch_row_value();
	float result = turn_float(row_num);
	return result;
}

int turn_value_to_int(int row_num) {

	fetch_row_value();
	int result = turn_int(row_num);
	return result;
}
