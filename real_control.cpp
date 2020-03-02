#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <mysql.h>
#include <iostream>
#include <errno.h>
// function a merge to 'master'
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <unistd.h>         //Used for UART
#include <fcntl.h>          //Used for UART
#include <termios.h>        //Used for UART
#include <mysql/mysql.h>

// to run it you may do > g++ -o ctrl real_control.cpp `mysql_config --cflags` `mysql_config --libs` to compile

// this is for testing git push
#define DIO_COUNT 2
#define HEAD 0x40 // '@'->0x40
#define U_ADDR 0x30 // '0'->0x30  
#define L_ADDR 0x34 // '4'->0x34   ID is from 4 ,5,6.....
#define CR 0x0D    // '/r'->0x0D


unsigned int *APP_control(int app_count, int *app_arr);
unsigned char HtoA(unsigned char a);
unsigned char AtoH(unsigned char a);
void Rectifier_control(unsigned char high, unsigned char low);
int UART_set();

// common parameter
int i = 0, j = 0,fd = 0;


int main(void)
{
	/*============================initial UART==================================*/
	UART_set();

	/*============================variable defination==================================*/
	int sample_time = 0, interrupt_num=0, uninterrupt_num=0 , varying_num=0,app_count=0,fc_enable=0;
	int Pgrid = 0,Last_Pgrid=0;
	int app_tmp = 0;

	unsigned char high_Rec = 0x00, low_Rec = 0x00;  //use unsigned char because if value>128 then overflow,so,we use unsigned char(0~255) 
	float *base_par = new float[13];
	float Pfc=0.0;

	char sql_buffer[1024] = {0};
	char A_tmp[4] = {0};

	/*============================get time==================================*/
    time_t t = time(NULL);
	struct tm tm = *localtime(&t);

	/*============================== SQL parm===============================*/
	MYSQL *mysql_con = mysql_init(NULL);
	MYSQL_RES *mysql_result;
	MYSQL_ROW mysql_row;

	if((mysql_real_connect(mysql_con, "140.124.42.70", "root", "fuzzy314", "realtime",6666, NULL, 0)) == NULL)
	{
		printf("Failed to connect to Mysql!\n");
		return -1;
	}
	printf("Connect to Mysql sucess!!\n");
	mysql_set_character_set(mysql_con, "utf8");

    /*==========check same day. if it is sameday thne do nothing.==========*/
	char *s_time = new char[3];
	snprintf(sql_buffer, sizeof(sql_buffer), "select value from LP_BASE_PARM where 	parameter_id = '%d'", 17);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	s_time = mysql_row[0];

    char *token;
    int time_tmp[3], tt = 0;
	token = strtok(s_time,"-");
	while(token != NULL)
	{
		time_tmp[tt] = atoi(token);
		token = strtok(NULL,"-");
		tt++;
	}
        
	printf("Date:%d.%d.%d\n",time_tmp[0],time_tmp[1],time_tmp[2]);
	if ((time_tmp[0] != (tm.tm_year + 1900)) || (time_tmp[1] != (tm.tm_mon + 1)) || (time_tmp[2] != (tm.tm_mday)))
	{
		printf("No today's schedual. So control will do nothing.....\n");
		// return -1;
	}
	printf("Control processing......\n");

	/*======================Get time(A0,A1,A2......)======================*/
	if (((tm.tm_min) % 15) != 0)
	{sample_time = (tm.tm_hour) * 4 + (int)((tm.tm_min) / 15);}
	//{sample_time = (tm.tm_hour) * 4 + (int)((tm.tm_min) / 15) + 1;}
	else
	{sample_time = (tm.tm_hour) * 4 + (int)((tm.tm_min) / 15);}

	//sample_time-=1;// to offset
	//Transform
	snprintf(A_tmp, sizeof(A_tmp), "A%d", sample_time);
	printf("simple time:%s\n",A_tmp);

	printf("/*Get APP value*/\n");

	/*============================Get Application parameter============================*/
    //SQL query
    snprintf(sql_buffer, sizeof(sql_buffer), "SELECT count(*) AS numcols FROM load_list WHERE group_id<>0 "); //get app_count = 15
    mysql_query(mysql_con, sql_buffer);
    mysql_result = mysql_store_result(mysql_con);
    mysql_row = mysql_fetch_row(mysql_result);
    app_count = atoi(mysql_row[0]);
    mysql_free_result(mysql_result);
    printf("app count:%d\n",app_count );

 	int position[app_count];
    for (i = 0; i < app_count; i++)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "select number from load_list WHERE group_id<>0 ORDER BY group_id ASC,number ASC LIMIT %d,1", i);
		mysql_query(mysql_con, sql_buffer);
		mysql_result = mysql_store_result(mysql_con);
		mysql_row = mysql_fetch_row(mysql_result);
		position[i] = atoi(mysql_row[j]); //position用來存家電負載本來的number順序
		printf("equip sequence:%d\n", position[i]);
		mysql_free_result(mysql_result);
	}

	//Define app_arr array
	int *app_arr = new int[DIO_COUNT*8];
    memset(app_arr,0, sizeof(int)*DIO_COUNT*8);             
    unsigned int *app_get = new unsigned int[app_count];
	printf("/*Get Rectifier value*/\n");

	/*============================Get Rectifier value============================*/
	snprintf(sql_buffer, sizeof(sql_buffer), "select %s from control_status where control_id = %d", A_tmp, app_count + 1);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	Pgrid = atof(mysql_row[0])*1000;
	mysql_free_result(mysql_result);
	printf("Pgrid:%d W\n", (int)(Pgrid));
	
	snprintf(sql_buffer, sizeof(sql_buffer), "select value from LP_BASE_PARM where parameter_id = %d", 16);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	Last_Pgrid = atof(mysql_row[0]) * 1000;
	mysql_free_result(mysql_result);
	printf("last Pgrid:%d W\n", (int)(Last_Pgrid));
	
	//test 
	//Pgrid = 200;

	if ((Pgrid - 5) <= (Last_Pgrid) && (Last_Pgrid) <= (Pgrid + 5))     //if varying range is less than 30 ,than don't change
	{
	  printf("P_grid Remain....\n");
	}
	else	
	{
	  //caculate grid power ( Because we control 2 Rectifier same time,so power must be half )
	  high_Rec = (unsigned char)(Pgrid/2/256);
	  low_Rec = (unsigned char)(Pgrid/2);	
	  //send control
	  Rectifier_control(high_Rec, low_Rec);
	  //update last rectifier's value            
	  snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE LP_BASE_PARM SET value=%.3f WHERE parameter_id=%d ", (float)Pgrid/1000,16);
      mysql_query(mysql_con, sql_buffer);
	}

	/*============================Get Application status and send commend to DIO controller============================*/
	for (i = 0;i<(DIO_COUNT * 8);i++)
	{
	    snprintf(sql_buffer, sizeof(sql_buffer), "select status from now_status where id = '%d'", i+1);
        mysql_query(mysql_con, sql_buffer);
        mysql_result = mysql_store_result(mysql_con);
        mysql_row = mysql_fetch_row(mysql_result);
        app_arr[i] = atoi(mysql_row[0]);
        printf("app_arr[%d]:%d\n",i,app_arr[i]);
	}                	
	mysql_free_result(mysql_result);
	//app_arr[15]=1;
	if (app_count<=(DIO_COUNT * 8))
	{
	  app_get = APP_control(app_count, app_arr);
        /*for(i=0;i<(DIO_COUNT * 8);i++)
	    { 
	    	// snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE now_status SET status=%d WHERE id=%d ",app_get[i] ,i+1);
            //printf("AAAAAAAAAAA:%s\n",sql_buffer);      
	      	mysql_query(mysql_con, sql_buffer);  
        }*/
           
     //     for (i=0;i<app_count;i++)
	    // {
     //        snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE control_status SET A%d ='%d' WHERE control_id='%d'",sample_time,app_get[position[i]-1],i+1);
     //        mysql_query(mysql_con, sql_buffer);
	    // }
	}
	else
	{
	  printf("error!Application is too much...\n");

	  return -1;
	}

	/*============================Get FC value and send value to FC table============================*/
	snprintf(sql_buffer, sizeof(sql_buffer), "select %s from control_status where parameter_id = %d", A_tmp, 5);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	Pfc = atof(mysql_row[0])*1000.0;
	mysql_free_result(mysql_result);
	printf("Pfc:%f W\n", Pfc);

	snprintf(sql_buffer, sizeof(sql_buffer), "select value from fc_control where parameter_id = %d", 1);
	mysql_query(mysql_con, sql_buffer);
	mysql_result = mysql_store_result(mysql_con);
	mysql_row = mysql_fetch_row(mysql_result);
	fc_enable = atoi(mysql_row[0]);
	mysql_free_result(mysql_result);

	if(fc_enable==1)
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", Pfc,3);
    	mysql_query(mysql_con, sql_buffer);
	}
	else
	{
		snprintf(sql_buffer, sizeof(sql_buffer), "UPDATE fc_control SET value=%.2f WHERE parameter_id=%d ", 0.0,3);
    	mysql_query(mysql_con, sql_buffer);
	}

	mysql_close(mysql_con);
	close(fd);
	delete[] app_arr, base_par,app_get;

       
	return 0;
}
//-------------------------- int Main endline --------------------------//
unsigned int *APP_control(int app_count,int *app_arr)
{
	int *app = new int[DIO_COUNT*8];
    int tx_count=0,rx_count;
	unsigned char *hex_d = new unsigned char[8];
	unsigned char *asc_MSB = new unsigned char[DIO_COUNT];
	unsigned char *asc_LSB = new unsigned char[DIO_COUNT];
    unsigned char *rx_buf = new unsigned char[6];
    unsigned int *rx_get = new unsigned int[DIO_COUNT*8];
	unsigned char tmp = 0x00;
    memset(rx_buf, '\0', 6);

	//app[0] = 1; app[1] = 1; app[2] = 1;; app[5] = 1;//app[5]=1;

    // Decode to @AA(Data),(Data)->xxxxh Left->MSB Right->LSB
	hex_d[0] = 0x01; hex_d[1] = 0x02; hex_d[2] = 0x04; hex_d[3] = 0x08; hex_d[4] = 0x10; hex_d[5] = 0x20; hex_d[6] = 0x40; hex_d[7] = 0x80;
	//one for funciton get eight DIO status
	for (j = 0; j<DIO_COUNT; j++)
	{
		usleep(200000);
		for (i = 0; i<8; i++)
		{
           // app_arr[i+j*8]=1;
		   app[i + j * 8] = app_arr[i + j * 8];
           printf("APP%d\n",app[i+j*8]);
		   tmp += hex_d[i] * app[i + j * 8];
		}
		printf("0x%x ", tmp >> 4);     //ex: 0x42-> 0x4
		printf("0x%x\n", tmp & 0x0f);   //ex: 0x42-> 0x2

	    // HEX to ASCII
		asc_MSB[j] = HtoA(tmp >> 4);
		asc_LSB[j] = HtoA(tmp & 0x0f);

		printf("0x%x ", asc_MSB[j]);   //ex: 0x4-> 0x34
		printf("0x%x\n ", asc_LSB[j]);   //ex: 0x2-> 0x32

		//clear tmp
		tmp = 0x00;
		//Defined set commend
		unsigned char set_commend[6] = { HEAD, U_ADDR, L_ADDR+j, asc_MSB[j], asc_LSB[j], CR };
	    // For test: unsigned char set_commend[6] = {0x40,0x30,L_ADDR+j,0x34,0x32,0x0D}; @043432, @05xxxx
        printf("Addr:%x\n",L_ADDR+j);
        //Submit commend
        tx_count = write(fd, &set_commend, 6);
        //delay
		usleep(200000); 
		//recieve data
        rx_count = read(fd,rx_buf,6);
		//print result
        printf("Trans DATA:\n");
        for(i=0;i<6;i++)
        {
            printf("0x%x ",rx_buf[i]);
        }
		printf("\n");
		//clean rx_buf
        memset(rx_buf, '\0', 6);
		//delay time (must have)
        usleep(200000);


		//Defined get commend
        unsigned char get_state[4] = { HEAD, U_ADDR, L_ADDR+j, CR };
        //Submit commend
		tx_count = write(fd, &get_state, 4);
 		//Delay
		usleep(200000);
		//sleep(1);
        //Recieve data
		rx_count = read(fd,rx_buf,6);
		//Print result
        printf("Read DATA:\n");
		for(i=0;i<6;i++)
		{
            printf("0x%x ",rx_buf[i]);
		}
        printf("\n");                
            
		//Decode from DIO
		unsigned char add_sum =0;
        add_sum=AtoH(rx_buf[2])|(AtoH(rx_buf[1])<<4); //ex: 0x34(4) 0x33(3) -> 0x4 | 0x30 -> 0x34
		for (i =0;i<(8);i++)
		{
	 		printf("value:%x\n",(add_sum & hex_d[i])/hex_d[i]);
        	rx_get[i+j*8]= ((add_sum & hex_d[i])/hex_d[i]);
		}
		printf("add_sum:0x%x\n",add_sum);
		//clean rx_buf
 		memset(rx_buf, '\0', 6);
	}
	//set not use
	//for(i=app_count;i<(DIO_COUNT*8);i++)
	//{
    	//rx_get[i]=0;	
	//}
    for(i=0;i<16;i++)
    { printf("rx_get:%d\n",rx_get[i]);}
	//return get result
	return rx_get;
	//delete arrays
	delete[] app, hex_d, asc_MSB, asc_LSB,rx_buf,rx_get;
}

unsigned char HtoA(unsigned char a)
{
	if (a<0x0a)	// 0x0a(hex) = 10(dec) 
	{
  	  a = a + 0x30;    //ascii '0'-> 0x30  '1'->0x31 ....
	  return a;
	}
	else
	{
	  a = a - 0x0a + 0x41;  //ascii 'A'-> 0x41  'B'->0x42 ....
	  return a;
	}
}

unsigned char AtoH(unsigned char a)
{
	if (0x46>=a & a>=0x41)
	{
		a = a -0x41+0x0a ;  //ascii 'A'-> 0x41  'B'->0x42 ....
		return a;
	}
	else if (0x39>=a & a>=0x30)
	{
		a = a - 0x30 ;  //ascii '0'-> 0x30  '1'->0x31 ....
		return a;
	}
}


void Rectifier_control(unsigned char high, unsigned char low)
{
	int sockfd;
	struct sockaddr_in dest;
	    
	/* create socket */
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	    
	/* initialize value in dest */    
	bzero(&dest, sizeof(dest));
	dest.sin_family = PF_INET;
	dest.sin_port = htons(502);
	dest.sin_addr.s_addr = inet_addr("140.124.42.67");
	    
	/* Connecting to server */
	connect(sockfd, (struct sockaddr*)&dest, sizeof(dest));

	/* Set rectifier's value */
	unsigned char electric[] = { 0x00,0x01,0x00,0x00,0x00,0x06,0x01,0x06,0x00,0x03,high,low};

	send(sockfd,(const char*)electric,sizeof(electric),0);
	printf("Pigrid :0x%x 0x%x\n", high, low);
    close(sockfd);
}

int UART_set()
{
	fd = open( "/dev/ttyUSB-DIO", O_RDWR| O_NOCTTY| O_NDELAY)  ;
	struct termios tty;
	struct termios tty_old;
	memset (&tty, 0, sizeof tty);

	/* Error Handling */
	 if ( tcgetattr ( fd, &tty ) != 0 )
 	{
  	 std::cout << "Error " << errno << " from tcgetattr: " << strerror(errno) << std::endl;
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
	tty.c_cc[VTIME]  =  10;                  // 1 seconds read timeout
	tty.c_cflag     |=  CREAD | CLOCAL;     // turn on READ & ignore ctrl lines

	/* Make raw */
	cfmakeraw(&tty);

	/* Flush Port, then applies attributes */
	tcflush( fd, TCIFLUSH );
	if ( tcsetattr ( fd, TCSANOW, &tty ) != 0)
	{
 	  std::cout << "Error " << errno << " from tcsetattr" << std::endl;
	}
}


