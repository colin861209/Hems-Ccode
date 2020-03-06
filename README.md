## hems-Ccode
Introduce about realtime_data &amp; real_control

Realtime_data and Real_control run in Crontab per minute.
+ SPEC
+ *Realtime_data*(put the value into DB /per minutes) -> 
+ *GLPK*(find the best energe using result) -> 
+ *Real_control*(control the recitfier and application load /per minutes)
***
1\. Realtime_Data.cpp

Realtime_data is to *GET THE DATA* from the hardware, including the Vsys, Psys, Pload, Pbat, Psolar, Prect, P_1, P_2, P_3, Vfc, Pfc, Psell, lx, lx_power, PV_tmp and SOC. 

Get all this value by many communication protocol,
* use Modbus RTU to get the lx & lx power & tempture,
* use Modbus TCP/IP to get the Vsys, Psys, Pload, Pbat, Psolar, Prect, P_1, P_2, P_3
* use Mysql to get the Fuel Cell and Sell Power.
***
2\. Real_Control.cpp

Real_control most important thing is to *Control the DIO* and *Power of Rectifier*

* use Modbus ASCII to control the DIO status
* DIO is digital input/output, and we use it to control the application load.
* use Modbus TCP/IP to control the each one power of rectifier.
***
3\. Fc_control.cpp

Fc_control is try to generate the power from Mysql.

* use Modbus RTU to get & set the FC control value.
* FC voltage is setting a constant 54V.
* Set the target power than FC control the current to track the target.
