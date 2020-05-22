#ifndef SQLFunction_H
#define SQLFunction_H

extern MYSQL *mysql_con;
extern MYSQL_RES *mysql_result;
extern MYSQL_ROW mysql_row;
extern char sql_buffer[2000];

MYSQL_ROW fetch_row_value();
int turn_int(int row_num);
float turn_float(int row_num);
int turn_value_to_int(int row_num);
float turn_value_to_float(int row_num);

#endif 