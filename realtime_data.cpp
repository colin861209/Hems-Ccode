#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#include <unistd.h>
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART

#include <sys/poll.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <mysql/mysql.h> 

#define ORION_DATA_NUM 17
#define BAT_BIAS 1.35
#define TOTAL_SEC 900.0 //  15 min has 900 sce
//#define TOTAL_SEC 720.0 //  15 min has 720 sce
#define EVERY_SEC 10.0

//process: gcc -o realtime_data realtime_data.c `mysql_config --cflags --libs`


//orion
float Vsys,Psys,Pload,Pbat,SOC,Psolar,Prect,P_1,P_2,P_3=0.0;
//other meter
float Vfc,Pfc,Psell=0.0;
//weather
float lx,lx_power,PV_tmp=0.0;

int now_t,prev_t;
int divide=4;//1 hour have 4 timeblock


//socket
int socket_timeout=0,socket_err=0;
//time
time_t t = time(NULL);
struct tm now_time = *localtime(&t);
//serial
int fd;
int tx_count=0,rx_count=0;

int get_orion();
int get_other();    //get_other is to get FC & Sell_inverter's parameter
int get_weather_data();
int UART_set();
int serial_trans(char *);
int insert_value();
int update_realtime_table();
unsigned char* serial_wr(unsigned char *, int  ,int  );
void auto_increase_table();

int main()
{
	printf("Get previous data....\n");

	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	MYSQL_FIELD mysql_field;

	if ((mysql_real_connect(mysql_con, "localhost", "root", "fuzzy314", "monitor_data", 6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		return -1;
	}
	mysql_set_character_set(mysql_con, "utf8");
	char sql_buffer[1024]={'\0'};

 //if now time didn't exist data, than insert  one data to privent from later access aerror
	int sql_rownum;
	mysql_query(mysql_con,"SELECT COUNT(*) FROM monitor_now");
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);  
	sql_rownum=atoi(mysql_row[0]);
	mysql_free_result(mysql_result);

	if(sql_rownum==0) //monitor_now內沒有data 從monitor_history最新的一筆抓
	{
	 printf("Automatically insert one row data to prevent error(from history latest data)...\n");
	 snprintf(sql_buffer, sizeof(sql_buffer),"select * FROM `monitor_history_%d_%.2d` WHERE datetime = (SELECT max(datetime) as time FROM monitor_history_%d_%.2d)",now_time.tm_year+1900,now_time.tm_mon+1,now_time.tm_year+1900,now_time.tm_mon+1);
	 
	}
	else//monitor_now內有data 從前一比抓資料
	{
	printf("Row data has existed ...\n");
	snprintf(sql_buffer, sizeof(sql_buffer),"select * FROM `monitor_now`  WHERE datetime = (SELECT max(datetime) as time FROM monitor_now )");
	}  

	mysql_query(mysql_con,sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	Vsys = atof(mysql_row[0]);
	Psys = atof(mysql_row[1]);
	Pload = atof(mysql_row[2]);
	Pbat = atof(mysql_row[3]);
	Psolar = atof(mysql_row[4]);
	Prect = atof(mysql_row[5]);
	P_1 = atof(mysql_row[6]);
	P_2 = atof(mysql_row[7]);
	P_3 = atof(mysql_row[8]);
	Vfc = atof(mysql_row[9]);
	Pfc = atof(mysql_row[10]);
	Psell = atof(mysql_row[11]);
	lx = atof(mysql_row[12]);
	lx_power = atof(mysql_row[13]);
	PV_tmp = atof(mysql_row[14]);
	SOC = atof(mysql_row[15]);
	mysql_free_result( mysql_result);
					 

	//get now time that can used in the real experiment
	//now_t 就是現在的timeblock
	if (((now_time.tm_min) % (60 / divide)) != 0)
	{
			now_t = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	else
	{
			now_t = (now_time.tm_hour) * divide + (int)((now_time.tm_min) / (60 / divide));
	}
	printf("now period:%d\n",now_t);

	//prev_t 就是前一個時刻的timeblock
	snprintf(sql_buffer, sizeof(sql_buffer), "SELECT `time_block` FROM `previous_time_block` WHERE id=1");
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	prev_t=atoi(mysql_row[0]);
	printf("previous period:%d\n",prev_t);


 //first get SOCi
 if(now_t==prev_t)//for caculate sum of SOC
 {
	 mysql_query(mysql_con," select `SOC` FROM `monitor_now` WHERE datetime = (SELECT min(datetime) as time FROM monitor_now )");
	mysql_result = mysql_store_result(mysql_con);
	if(mysql_row = mysql_fetch_row(mysql_result))
	{SOC = atof(mysql_row[0]);}
 }
 else
 {
	 mysql_query(mysql_con," select `SOC` FROM `monitor_now` WHERE datetime = (SELECT max(datetime) as time FROM monitor_now )");
	mysql_result = mysql_store_result(mysql_con);
				if(mysql_row = mysql_fetch_row(mysql_result))
				{SOC = atof(mysql_row[0]);}
 }



/*
	if(now_t==prev_t)
	{ mysql_query(mysql_con," select `SOC` FROM `monitor_now` WHERE datetime = (SELECT min(datetime) as time FROM monitor_now )");printf("QQQQQQQQQQ\n");} //for caculate sum of SOC
	else
	{ mysql_query(mysql_con," select `SOC` FROM `monitor_now` WHERE datetime = (SELECT max(datetime) as time FROM monitor_now )");printf("AAAAAAAAAAA\n");}
	mysql_result = mysql_store_result(mysql_con);
	if(mysql_row = mysql_fetch_row(mysql_result))
	{ SOC = atof(mysql_row[0]);}
	else
	{	
	snprintf(sql_buffer, sizeof(sql_buffer), "select `SOC` FROM `monitor_history_%d_%.2d` WHERE datetime = (SELECT max(datetime) as time FROM monitor_history_%d_%.2d)",now_time.tm_year+1900,now_time.tm_mon+1,now_time.tm_year+1900,now_time.tm_mon+1);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	SOC = atof(mysql_row[0]);
	}  
*/
	
	printf("done\n");

	printf("Get other data...\n");  
	//get_other is to get FC & Sell_inverter's parameter
	get_other(); // Vfc Pfc Psell
	printf("done\n");

	printf("Get orion data...\n");
	
	get_orion();

	if(socket_timeout==1)//time out get all value of previous value.
	{     
 /*   mysql_query(mysql_con," select * FROM `monitor_now`  WHERE datetime = (SELECT max(datetime) as time FROM monitor_now )");
		mysql_result = mysql_store_result(mysql_con); 
		mysql_row = mysql_fetch_row(mysql_result);
		Vsys = atof(mysql_row[0]);
		Psys = atof(mysql_row[1]);
		Pload = atof(mysql_row[2]);
		Pbat = atof(mysql_row[3]);  
		Psolar = atof(mysql_row[4]);
		Prect = atof(mysql_row[5]);
		P_1 = atof(mysql_row[6]);
		P_2 = atof(mysql_row[7]);
		P_3 = atof(mysql_row[8]);
		Vfc = atof(mysql_row[8]); */
	}
	printf("done\n");

	printf("Get weather data...\n");  
	get_weather_data();
	printf("done\n");

	printf("Insert value...\n");
	auto_increase_table();
	insert_value();
	printf("done\n");
 
	update_realtime_table();

	snprintf(sql_buffer, sizeof(sql_buffer),"UPDATE `previous_time_block` SET `time_block` = '%d' WHERE `previous_time_block`.`id` = 1;",now_t);
	mysql_query(mysql_con, sql_buffer); 
	
	mysql_close(mysql_con);
	return 0;   
}


int get_orion()
{
	int i=0;
	int sockfd;
	struct sockaddr_in dest;
	struct pollfd fd;
	int ret;

	char recv_buf[11];
	char send_buf[12]={0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x04,0x00,0x00,0x00,0x01}; // TCP/IP modbus default code 
	float data[ORION_DATA_NUM];
	short tmp;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(502);
	dest.sin_addr.s_addr = inet_addr("140.124.42.67");
 
	/* connect time out */
	struct timeval timeout;      
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000;
	if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
	{ printf("setsockopt recv  failed\n"); socket_err=-1;}

	if (setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,sizeof(timeout)) < 0)
	{ printf("setsockopt send  failed\n"); socket_err=-1;} 
 
	if(socket_err!=-1)
	{
	/* Connecting to server */
		socket_err=connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));
		for (i=0;i<ORION_DATA_NUM;i++)     //because read in one quary will have problem, so use multi-quary intead
		{
			send_buf[9]=i;
			send(sockfd,send_buf,sizeof(send_buf),0);
			usleep(2000);
			fd.fd =sockfd; // your socket handler 
			fd.events = POLLIN;
			ret = poll(&fd, 1,1000);
			switch (ret) {
				case -1:
					socket_err=1;
					printf("connect error\n");	     
					break;
				case 0:
					socket_timeout=1;
					printf("time_out\n");
					break;
				default:
					recv(sockfd,recv_buf,sizeof(recv_buf), 0); // get your data
					break;	
			}

			if(socket_timeout ==1)
			{
				break;//if timeout then exit loop    
			}      
			tmp=((recv_buf[9]<<8)|recv_buf[10]);
			if(tmp==0x7fff)
			{data[i]=0.0;}
			else
			{data[i]=(float)(tmp);}
			printf("data:%f\n",data[i]);
		} 
		if(socket_timeout!=1) //if no timeout then do below
		{    
			Vsys=data[13]/100.0; 
			Pload=data[0]/10.0*Vsys;
			// Pbat=data[1]/10.0*Vsys;  // because have some measure errror..so below function instead 
			Psys=data[3];
			Prect=data[2]/10.0*Vsys;
			Psolar=data[10]+data[11];
			P_1=data[4]/10.0*Vsys;
			P_2=data[5]/10.0*Vsys;
			P_3=data[6]/10.0*Vsys;

			Pbat=Prect+Psolar+Pfc-Pload-Psell;

		}
	}
		
	else
	{
		printf("connect error");
	}
	
	/* Close connection */
	close(sockfd);
	
}

int get_other()
{
	int i=0;
	float Ifc=0.0;
	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	int outtime = 5;
	mysql_options(mysql_con,MYSQL_OPT_READ_TIMEOUT ,&outtime);
	if ((mysql_real_connect(mysql_con, "140.124.42.65", "root", "fuzzy314", "meter", 9489, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		return -1; 
	}
	mysql_set_character_set(mysql_con, "utf8");
	char sql_buffer[200] = { 0 };
	sprintf(sql_buffer,"select FC_V,FC_I,FC_P,inverter_V,inverter_I,inverter_P FROM HEMS WHERE datetimes = (SELECT max(datetimes) as time FROM HEMS)"); 
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	
	Vfc=strtod(mysql_row[0],NULL);
	Ifc=strtod(mysql_row[1],NULL);
	//Pfc=strtod(mysql_row[2],NULL);
	if ((Ifc>-0.15)&&(Ifc<=0.0))
	{Ifc=0.0;}
	Pfc=Vfc*Ifc;
	Psell=strtod(mysql_row[5],NULL);
	
	Pfc=Pfc;
	Psell=Psell*1000.0;

	printf("Vfc:%f\n",Vfc);
	printf("Pfc:%f\n",Pfc);
	printf("Psell:%f\n",Psell);
 
	mysql_close(mysql_con);
}

int get_weather_data()
{
	UART_set();
	if (fd != -1)
	{
		 char sql_buffer[1024] = {'\0'};
		 int tx_num=8, rx_num=8;

		 unsigned char solar[8] = {0x02,0x03,0x00,0x00,0x00,0x01,0x84,0x39};
		 unsigned char temp[8] = {0x03,0x03,0x00,0x00,0x00,0x01,0x85,0xE8};

		 unsigned char *solar_result = (unsigned char*)malloc(rx_num);
		 unsigned char *temp_result = (unsigned char*)malloc(rx_num);

		 //serial raead write
		 solar_result=serial_wr(solar,tx_num,rx_num);
		 temp_result=serial_wr(temp,tx_num,rx_num);

		 lx =(solar_result[3]*256+solar_result[4])/10.0; //solar(lumid)
		 lx_power =lx*3.75*10; //solar(power)
		 PV_tmp =(temp_result[3]*256+temp_result[4])/100.0; //temp(C)
		 
		 if (lx>200.0)
		 {lx=0.0;lx_power=0.0;}
		 if (PV_tmp>100.0)
		 {PV_tmp=0.0;}	     
		 printf("lx:%f\n",lx);
		 printf("lx_power:%f\n",lx_power);
		 printf("temp:%f\n",PV_tmp);

	}
	close(fd);
		
}


void auto_increase_table() //every month increase table
{
	
	if((now_time.tm_year+1900)!=1900)
	{
	if (now_time.tm_mday==1 && now_t==0)
	{
				MYSQL *mysql_con = mysql_init(NULL);
				MYSQL_RES *mysql_result;
				MYSQL_ROW mysql_row;
		
				char sql_buffer[1024] = { '\0' };        
				
			 if ((mysql_real_connect(mysql_con, "localhost", "root", "fuzzy314", "monitor_data",6666, NULL, 0)) == NULL)
		{
		printf("Failed to connect to Mysql!\n");
		return ;     
		}
			 snprintf(sql_buffer, sizeof(sql_buffer)," CREATE TABLE `monitor_data`.`monitor_history_%d_%.2d` ( `Vsys` FLOAT NULL , `Psys` FLOAT NULL , `Pload` FLOAT NULL , `Pbat` FLOAT NULL , `Psolar` FLOAT NULL , `Prect` FLOAT NULL , `P_1` FLOAT NULL , `P_2` FLOAT NULL , `P_3` FLOAT NULL , `Vfc` FLOAT NULL , `Pfc` FLOAT NULL , `Psell` FLOAT NULL , `lx` FLOAT NULL , `lx_power` FLOAT NULL , `PV_temp` FLOAT NULL , `SOC` FLOAT NULL ,`id` INT NOT NULL AUTO_INCREMENT , `period` INT NOT NULL , `day` INT NOT NULL,  `datetime` TIMESTAMP on update CURRENT_TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP  , PRIMARY KEY (`id`)) ENGINE = InnoDB CHARSET=utf8 COLLATE utf8_general_ci;",now_time.tm_year+1900,now_time.tm_mon+1);

			mysql_query(mysql_con, sql_buffer);

			mysql_close(mysql_con);
	}
	}

 }
 

unsigned char* serial_wr(unsigned char *tx_data , int tx_num ,int rx_num )
{
	 int i=0;
	 unsigned char tx_buf[tx_num];
	 unsigned char rx_buf[rx_num]; 
	 unsigned char *rx_data = (unsigned char*)malloc(rx_num);
	 for(i=0;i<tx_num;i++)
	 {tx_buf[i]=tx_data[i];}

		tx_count = write(fd,&tx_buf,tx_num);
		usleep(200000);
		rx_count = read(fd,&rx_buf,rx_num);
		

		for( i = 0;i < rx_num;i++)
		{
				printf("0x%x ",rx_buf[i]);
				rx_data[i]=rx_buf[i];
		}
		
		printf("\n");
		
		tcflush(fd, TCIFLUSH); 
		usleep(100000);  
		return rx_data;
		delete[] tx_data,rx_data;
}

int UART_set()
{
		fd=0;
		fd = open("/dev/ttyUSB-solar", O_RDWR | O_NOCTTY | O_NDELAY); 

		if (fd == -1)
		{
			//ERROR - CAN'T OPEN SERIAL PORT
			printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
			return -1;
		}

	 //CONFIGURE THE UART
		struct termios options;
		tcgetattr(fd, &options);
		options.c_cflag = B9600 | CS8 | CLOCAL | CREAD;     //<Set baud rate
		options.c_cflag &= ~PARENB; //No parity     //new
		options.c_cflag &= ~CSTOPB; //Stop bit = 1  //new
			
		options.c_iflag = IGNPAR | ICRNL;  //new
	 // options.c_iflag = IGNPAR;
		options.c_oflag = 0;
		options.c_lflag = 0;
		tcflush(fd, TCIFLUSH);
		tcsetattr(fd, TCSANOW, &options);
		printf("Success to open UART\n");

}

int insert_value()
{
	int i,j=0;	
	int diff_block=0;
	float val[3600][20]={0.0};
	float avg[20]={0.0};
	float Pbat_sum=0.0;
	int row_num;
	int numrows,numfields=0;
	
	//printf("now period:%d\n", now_t);
	
	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;
	MYSQL_FIELD mysql_field;

	char sql_buffer[1024] = { '\0' };
	if ((mysql_real_connect(mysql_con, "localhost", "root", "fuzzy314", "monitor_data",6666, NULL, 0)) == NULL)
	{
			printf("Failed to connect to Mysql!\n");
			return -1;
	}
	
	if(prev_t!=now_t)
	{
			//get data average 	
		snprintf(sql_buffer, sizeof(sql_buffer), "SELECT * FROM `monitor_now` ",i);    
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		numrows = mysql_num_rows(mysql_result);
		numfields = mysql_num_fields(mysql_result);
		j=0;
		while(mysql_row = mysql_fetch_row(mysql_result))
		{
			for(i =0 ; i < numfields-3; i++)
			{
				avg[i]+= atof(mysql_row[i]);
			}
			j++;

		}
		for(i=0;i<numfields-3;i++)
		{
			avg[i]/=j;
		}
		printf("pressssssss:%f\n",prev_t);
		char sql_c[]={"`Vsys`, `Psys`, `Pload`, `Pbat` , `Psolar`, `Prect`, `P_1`, `P_2`, `P_3`, `Vfc`, `Pfc`, `Psell`, `lx`, `lx_power`, `PV_temp` , `SOC` , `period` , `day`"};
		snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `monitor_history_%d_%.2d`(%s) VALUES(%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%f,%d,%d) ",now_time.tm_year+1900,now_time.tm_mon+1,sql_c,avg[0],avg[1],avg[2],avg[3],avg[4],avg[5],avg[6],avg[7],avg[8],avg[9],avg[10],avg[11],avg[12],avg[13],avg[14],SOC,prev_t,now_time.tm_mday);
		mysql_query(mysql_con, sql_buffer);

		mysql_query(mysql_con, "truncate table `monitor_now`");
	}

	char sql_q[]={"`Vsys`, `Psys`, `Pload`, `Pbat`, `Psolar`, `Prect`, `P_1`, `P_2`, `P_3`, `Vfc`, `Pfc`, `Psell`, `lx`, `lx_power`, `PV_temp`, `SOC` "};
	snprintf(sql_buffer, sizeof(sql_buffer), "INSERT INTO `monitor_now`(%s) VALUES(%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%f) ",sql_q,Vsys,Psys,Pload,Pbat,Psolar,Prect,P_1,P_2,P_3,Vfc,Pfc,Psell,lx,lx_power,PV_tmp,SOC);
	mysql_query(mysql_con, sql_buffer);

		//caculate SOC
	mysql_query(mysql_con,"SELECT `Pbat` FROM `monitor_now`" );
	mysql_result = mysql_store_result(mysql_con);
	numrows = mysql_num_rows(mysql_result);
	numfields = mysql_num_fields(mysql_result);   
	while(mysql_row = mysql_fetch_row(mysql_result))
	{
		Pbat_sum+= atof(mysql_row[0]);    
	}

	if(Vsys>0)  // if close orion cause voltage is zero cause infinity SOC value 
	// { SOC=SOC+((Pbat_sum/TOTAL_SEC*EVERY_SEC/divide)/(Vsys*318.0));}
	// { SOC=SOC+((Pbat_sum*EVERY_SEC/TOTAL_SEC/divide)/(54.4*318.0));}    
	{ SOC=SOC+((Pbat_sum*EVERY_SEC/TOTAL_SEC/divide)/(48.0*318.0));}  	    
	
	snprintf(sql_buffer, sizeof(sql_buffer),"UPDATE `monitor_now` SET `SOC` = '%f'  WHERE datetime =(SELECT * from(SELECT max(datetime) from `monitor_now`) monitor_data ) ",SOC);
	mysql_query(mysql_con,sql_buffer);    

	mysql_close(mysql_con);

}

int update_realtime_table()
{
 MYSQL *mysql_con = mysql_init(NULL);
 MYSQL_RES *mysql_result;
 MYSQL_ROW mysql_row;
 MYSQL_FIELD mysql_field;

 char sql_buffer[1024] = { '\0' };
 if ((mysql_real_connect(mysql_con, "localhost", "root", "fuzzy314", "realtime",6666, NULL, 0)) == NULL)
 {
		 printf("Failed to connect to Mysql!\n");
		 return -1;
 }
 if (now_t==0)  //initial SOC
 {
	 snprintf(sql_buffer, sizeof(sql_buffer),"UPDATE `LP_BASE_PARM` SET `value` = '%f'  WHERE parameter_id=24 ",SOC);
	 mysql_query(mysql_con,sql_buffer);
 }	 

//now SOC
 snprintf(sql_buffer, sizeof(sql_buffer),"UPDATE `LP_BASE_PARM` SET `value` = '%f'  WHERE parameter_id=25 ",SOC);
 mysql_query(mysql_con,sql_buffer);

 snprintf(sql_buffer, sizeof(sql_buffer),"UPDATE `LP_BASE_PARM` SET `value` = '%.3f'  WHERE parameter_id=26 ",Psolar/1000.0);
 mysql_query(mysql_con,sql_buffer);

 mysql_close(mysql_con);
}
