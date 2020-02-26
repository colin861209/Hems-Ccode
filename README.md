# hems-Ccode
Put about realtime_data &amp; real_control

Realtime_data and Real_control run in Crontab per minute.

Realtime_data is to GET THE DATA from the hardware, including the Vsys, Psys, Pload, Pbat, Psolar, Prect, P_1, P_2, P_3, Vfc, Pfc, Psell, lx, lx_power, PV_tmp and SOC. 

Get all this value by many communication protocol,

=> use Modbus RTU to get the lx & lx power & tempture,

=> use Modbus TCP/IP to get the Vsys, Psys, Pload, Pbat, Psolar, Prect, P_1, P_2, P_3

=> use Mysql to get the Fuel Cell and Sell Power.
