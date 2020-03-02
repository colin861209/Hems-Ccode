#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include <errno.h>

#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <mysql/mysql.h>

// to run it you may do > g++ -o fc_control fc_control.cpp `mysql_config --cflags` `mysql_config --libs` to compile

//註解 by Lee_you_cheng
#define DIO_COUNT 2
#define HEAD 0x40 // '@'->0x40
#define U_ADDR 0x30 // '0'->0x30  
#define L_ADDR 0x34 // '4'->0x34   ID is from 4 ,5,6.....
#define CR 0x0D    // '/r'->0x0D
#define SAFE_TARGET_POWER 4300 //the highest power limit

// unsigned int *APP_control(int app_count, int *app_arr);
// unsigned char HtoA(unsigned char a);
// unsigned char AtoH(unsigned char a);
// void Rectifier_control(unsigned char high, unsigned char low);
unsigned short calc_crc(unsigned char *, int ); //modbusRTU CRC Check function
float serial_wr(unsigned char *,int); //modbusRTU connect function
int uchar_size(unsigned char *); 
int UART_set(); //raspberry pi usb port connect function

// common parameter
int i = 0, j = 0,fd = 0;
int UART_err=0;


/*===========================commend definition==============================*/
	unsigned char cmd_on[6] = {0x11,0x06,0x00,0x01,0x00,0x00};
	unsigned char cmd_off[6] = {0x11,0x06,0x00,0x01,0x00,0x01};
	unsigned char cmd_get_v[6] = {0x11,0x03,0x01,0x01,0x00,0x01};
	unsigned char cmd_get_i[6] = {0x11,0x03,0x01,0x03,0x00,0x01};
	unsigned char cmd_get_status[6] = {0x11,0x03,0x01,0x07,0x00,0x01};
	unsigned char cmd_set_v[6] = {0x11,0x06,0x00,0x03,0x15,0x18};	//54V(constant)
	unsigned char cmd_set_i[6] = {0x11,0x06,0x00,0x05,0x00,0x0A};	//0.1A

	// serial_wr(cmd_on,sizeof(cmd_on)/sizeof(cmd_on[0]));
	// serial_wr(cmd_off,sizeof(cmd_off)/sizeof(cmd_off[0]));
	// serial_wr(cmd_get_v,sizeof(cmd_get_v)/sizeof(cmd_get_v[0]));
	// serial_wr(cmd_get_i,sizeof(cmd_get_i)/sizeof(cmd_get_i[0]));
	// serial_wr(cmd_get_status,sizeof(cmd_get_status)/sizeof(cmd_get_status[0]));


int main(void)
{	
	/*=============================variable======================================*/

	int power_status=0,power_enable=0;
	char sql_buffer[1024] = {0};
	float target_power=0.0,target_current=0.0,now_power=0.0,now_voltage=0.0,now_current=0.0,set_voltage=0.0,set_current=0.0;


	/*============================initial UART==================================*/
	//if connect error,then exit program.
	UART_set(); //connector with raspberry pi usb port
	if(UART_err==1)
	{
		printf("UART connect error, so exit the program...\n");
		return -1;
	}
	printf("UART connect success...\n");


	/*============================initial mysql==================================*/
	//if connect error,then shutdown and finally exit program.
	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;

	if((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "realtime",6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql, DCDC will shutdown and exit program... \n");
		serial_wr(cmd_off,sizeof(cmd_off)/sizeof(cmd_off[0]));
		return -1;
	}
	printf("mysql connect success...\n");

	/*============================check enable power==================================*/
	snprintf(sql_buffer, sizeof(sql_buffer), "select value from fc_control where parameter_id = '%d'", 1);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	power_enable = atoi(mysql_row[0]); //get power_enable value from mysql (0:off 1:on)

	snprintf(sql_buffer, sizeof(sql_buffer), "select value from fc_control where parameter_id = '%d'", 2);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	power_status = atoi(mysql_row[0]); //get power_status value from mysql


	/*=======================initial commend====================*/
	if(power_enable!=1)  //force to shutdown
	{
		 printf("Power on setting is disabled, DCDC will shutdown and exit program... \n");
		 serial_wr(cmd_off,sizeof(cmd_off)/sizeof(cmd_off[0]));
		 //clean mysql value 
		 power_status=0;set_current=0;now_voltage=0;now_current=0;now_power=0;
		 snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%d WHERE parameter_id=%d ", power_status,2);
    	 mysql_query(mysql_con, sql_buffer);
    	 snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", set_current,4);
    	 mysql_query(mysql_con, sql_buffer);
    	 snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_voltage,6);
    	 mysql_query(mysql_con, sql_buffer);
    	 snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_current,7);
    	 mysql_query(mysql_con, sql_buffer);
    	 snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_power,8);
    	 mysql_query(mysql_con, sql_buffer);
		 return -1;
	}

	if(power_status!=1) //if power on , than don't have to repeat set.
	{
		printf("Beginning of power on, so initial the value... \n");
		serial_wr(cmd_set_v,sizeof(cmd_set_v)/sizeof(cmd_set_v[0]));
	 	serial_wr(cmd_set_i,sizeof(cmd_set_i)/sizeof(cmd_set_i[0]));
	 	power_status=(int)(serial_wr(cmd_on,sizeof(cmd_on)/sizeof(cmd_on[0])));
	if(power_status==0)
    	{
		printf("DCDC doesn't get source power,so can't establish the connet, let the power status as off...  \n");
		return -1;
    	}
    	else
    	{power_status=1;}
    	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%d WHERE parameter_id=%d ", power_status,2);
    	mysql_query(mysql_con, sql_buffer);
	}


	/*=======================get target value and check value====================*/

	snprintf(sql_buffer, sizeof(sql_buffer), "select value from fc_control where parameter_id = '%d'", 3);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	target_power = atof(mysql_row[0]); // get target_power value from mysql

	//target_power=1700; //set target_power by myself
	//target_power=0;
	
	if(target_power>SAFE_TARGET_POWER) //check the target_power is higher than target power or not
	{
		printf("Target power %.2fW is too high, to protect the DCDC,so skip the commend and exit... \n",target_power);
		return -1;
	}
	else
	{
		printf("Target power %.2fW is okey, continue processing ... \n",target_power);		
	}

	target_power=0;

	/*=======================get now value====================*/

	now_voltage=serial_wr(cmd_get_v,sizeof(cmd_get_v)/sizeof(cmd_get_v[0])); //get now voltage from fuel cell by mudbusRTU
	now_current=serial_wr(cmd_get_i,sizeof(cmd_get_i)/sizeof(cmd_get_i[0])); //get now current from fuel cell by mudbusRTU
	now_power=now_voltage*now_current; //get now power
	printf("now voltage:%f V\n",now_voltage);
	printf("now current:%f A\n",now_current);
	printf("now power:%f W\n",now_power);

	/*=======================increase value====================*/
	printf("Tracking target power...\n");

	if(now_power>=target_power) //if now_power >= target_power then must stop plus power anymore
	{
		printf("Target power is lower than now power...\n");
		set_current=target_power/now_voltage;
		if (set_current<=0.1)	//prevent too low current cause fault
		{set_current=0.1;}

		cmd_set_i[4]=(unsigned char)(set_current*100.0/256.0);	//high bit 256=8bit
		cmd_set_i[5]=(unsigned char)(set_current*100.0);		//low bit

		serial_wr(cmd_set_i,sizeof(cmd_set_i)/sizeof(cmd_set_i[0])); //set i value
		now_voltage=serial_wr(cmd_get_v,sizeof(cmd_get_v)/sizeof(cmd_get_v[0])); //get now_voltage again after set i value
		now_current=serial_wr(cmd_get_i,sizeof(cmd_get_i)/sizeof(cmd_get_i[0])); //get now_current again after set i value
		now_current=set_current; //replace now_current by set_current
		now_power=now_current*now_voltage; //calculation now_power again
		printf("now_voltage:%.2f V now_current:%.2f A now_power:%.2f W\n",now_voltage,now_current,now_power);

		//update value to mysql
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", set_current,4);
    	mysql_query(mysql_con, sql_buffer);
    	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_voltage,6);
    	mysql_query(mysql_con, sql_buffer);
    	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_current,7);
    	mysql_query(mysql_con, sql_buffer);
    	snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_power,8);
    	mysql_query(mysql_con, sql_buffer);
	}
	else
	{
		int increase_time=0; //calculate how many times the while doing
		while(now_power<target_power) //if now_power<target_power then plus now_power again
		{
			printf("Target power is higher than now power...\n");
			printf("step:%d\n",increase_time);
			now_voltage=serial_wr(cmd_get_v,sizeof(cmd_get_v)/sizeof(cmd_get_v[0]));
			now_current=serial_wr(cmd_get_i,sizeof(cmd_get_i)/sizeof(cmd_get_i[0]));
			target_current=target_power/now_voltage;
			set_current=now_current;
			//if now_current is so closer with target_current then don't plus too much number for set_current
			if(target_current-now_current>4.0)
			{
				set_current+=4.0;
				//set_current+=10.0;
			}
			else
			{
				set_current+=(target_current-now_current);
			}

			cmd_set_i[4]=(unsigned char)(set_current*100.0/256.0);	//high bit
			cmd_set_i[5]=(unsigned char)(set_current*100.0);		//low bit
			serial_wr(cmd_set_i,sizeof(cmd_set_i)/sizeof(cmd_set_i[0]));
			now_voltage=serial_wr(cmd_get_v,sizeof(cmd_get_v)/sizeof(cmd_get_v[0]));
			now_current=serial_wr(cmd_get_i,sizeof(cmd_get_i)/sizeof(cmd_get_i[0]));
			now_power=now_voltage*now_current;
			printf("now_voltage:%.2f V now_current:%.2f A now_power:%.2f W\n",now_voltage,now_current,now_power);

			//update value
			snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", set_current,4);
    		mysql_query(mysql_con, sql_buffer);
    		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_voltage,6);
    		mysql_query(mysql_con, sql_buffer);
    		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_current,7);
    		mysql_query(mysql_con, sql_buffer);
    		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", now_power,8);
    		mysql_query(mysql_con, sql_buffer);
		
		usleep(4500000);
    		sleep(1);	//every step have 1 sec delay, let FC work successful.
    		increase_time++;
		}
	}


	mysql_close(mysql_con);
    close(fd); 
	return 0;
}


int UART_set()
{
	fd = open( "/dev/ttyUSB0", O_RDWR| O_NOCTTY| O_NDELAY);
	struct termios tty;
	struct termios tty_old;
	memset (&tty, 0, sizeof tty);

	/* Error Handling */
	 if ( tcgetattr ( fd, &tty ) != 0 )
 	{
  	 printf("UART get bad connection, please check the wire or plug\n");
  	 std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
  	 UART_err=1;
  	 return -1;
 	}

	/* Save old tty parameters */
	tty_old = tty;

	/* Set Baud Rate */
	cfsetospeed (&tty, (speed_t)B9600);
	cfsetispeed (&tty, (speed_t)B9600);

	/* Setting other Port Stuff */
	tty.c_cflag     &=  ~PARENB;            // Make 8n1
	tty.c_cflag     &=  ~CSTOPB;
	tty.c_cflag     &=  ~CSIZE;
	tty.c_cflag     |=  CS8;

	tty.c_cflag     &=  ~CRTSCTS;           // no flow control
	tty.c_cc[VMIN]   =  0;                  // read doesn't block
	tty.c_cc[VTIME]  =  5;                  // 0.5 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush( fd, TCIFLUSH );
	if ( tcsetattr ( fd, TCSANOW, &tty ) != 0)
	{
 	  std::cout << "Error " << errno << " from tcsetattr" << std::endl;
 	  UART_err=1;
	}
}


unsigned short calc_crc(unsigned char *buf, int length)
{
   unsigned short crc = 0xFFFF;
   int i,j;
   unsigned char LSB;
    for (i = 0; i < length; i++)
    {
      crc ^= buf[i];
        for (j = 0; j < 8; j++)
        {
           LSB= crc & 1;
           crc = crc >> 1;
                  
           if (LSB)
           { crc ^= 0xA001; }
        }
    }
   return ((crc & 0xFF00) >> 8)|((crc & 0x0FF) << 8 );
}

float serial_wr(unsigned char *cmd_buf,int cmd_length)
{
	usleep(300000);
	int tx_count=0,rx_count=0;
  	unsigned short short_tmp;
	unsigned char tx_buf[cmd_length+2];//addition 2 are CRC
	unsigned char rx_buf[10]={0}; //set as 20byte limit
	float result=0.0;

	// printf("length:%d\n",cmd_length);


	for (i=0;i<cmd_length;i++)
	{
		tx_buf[i] = cmd_buf[i];
	}


 	short_tmp=calc_crc(cmd_buf,cmd_length); //creat crc
    tx_buf[cmd_length]= short_tmp >> 8;
    tx_buf[cmd_length+1]= short_tmp ;

	// printf("send\n");
	// for (i=0;i<(cmd_length+2);i++)
	// {
	// 	printf("0x%x ",tx_buf[i]);
	// }
	// printf("\n");

    tx_count = write(fd,&tx_buf,cmd_length+2);// write
	usleep(300000);
    rx_count = read(fd,&rx_buf,20);// read

	// printf("recv\n");
 //    for (i=0;i<10;i++) 
 //    {
 //       printf("0x%x\n",rx_buf[i]);
 //    }
 //    printf("\n");


   //get now voltage
    if((tx_buf[2]==0x01)&&(tx_buf[3]==0x01))
    {
		result=(((float)(rx_buf[3]))*256+(float)(rx_buf[4]))/100.0;
		return result;
    }
   
   //get now current
    if((tx_buf[2]==0x01)&&(tx_buf[3]==0x03))
    {
    	result=(((float)(rx_buf[3]))*256+(float)(rx_buf[4]))/100.0;
		return result;
    }

    //get now power state
    if((tx_buf[2]==0x00)&&(tx_buf[3]==0x01))
    {
    	result=(float)(rx_buf[0]);   //Because if no response(get all zero value) it means the source power doesn't on.
		return result;
    }

}

