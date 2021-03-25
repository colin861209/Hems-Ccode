# real_control
* Funtions need to know

    * APP_control
      * write commend & read data from UART
      * decode receive data and return

    * HtoA
      * Turn hex to Ascii

    * AtoH
      * Turn Ascii to hex

    * Rectifier_control
      * control rectifier through socket connected by `140.124.42.67`

    * UART_set
      * set UART parmeter ,including baud rate, port stuff ...
---
* Process

    * initial UART

    * variable defination

    * get time
        * get local time

    * SQL parm
        * SQL connect `140.124.42.70` and use `realtime` data set

    * check same day. if it is same day then do nothing
        * get the last operation time from `LP_BASE_PARM` where  parameter name = `上次執行時間`
        * if now time = last operation time -> control will do nothing

    * Get time(A0,A1,A2......)
        * get time division by hour*4 + min/15

    * Get Application parameter
        * get `家電負載本來的number順序` and save it

    * Get Rectifier value
        * get `Pgrid`  from `control_status` by sequence of `control_id`
        * get `Last_Pgrid`  from `control_status` by sequence of `control_id`
        * compare Pgrid & Last_Pgrid ,if varying range is less than 30 ,than don't change
        * else control rectifier by ``high_Rec & low_Rec``
        * and update `LP_BASE_PARM` and set value = Pgrid/1000 , where parameter name =`上次市電輸出`

    * Get Application status and send commend to DIO controller
        * get Application status  from ``now_status`` sequentially
        * and send commend to DIO controller by APP_control

    * Get FC value and send value to FC table
        * get `Pfc`  from `control_status` where parameter id = 5
        * get `fc_enable`  from `fc_control`  where parameter name = `power_enable`
        * if fc_enable = 1 ,then update fc_control with setting ``target_power`` to be Pfc
        * if fc_enable != 1 ,then update fc_control with setting ``target_power`` to be 0.0

---
