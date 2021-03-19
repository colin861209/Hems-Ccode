# realtime_data

* get_orion
  * Use modbus TCP/IP protocol get data from real appliances
  * Content
    * Pload (power of loads)
    * Prect (power of rectifier when buy grid)
    * Psys  (power of HEMS system)
    * P_1   (power of inverter 2)
    * P_2   (power of inverter 1)
    * P_3   (power of inverter 3)
    * Psolar(power of PV)
    * Vsys  (voltage of HEMS system)
    * Pbat  (power of battery)

* get_other
  * Connect `140.124.42.65` to get Vfc, Ifc and Psell
  * NOTE: 2021/3/9 move `140.124.42.65` to supply new Server internet, so annotation this function.
  
* get_weather_data
  * Use modbus RTU to get lx, lx_power and PV_temp from PV pannel on 8F
  * Information calculation of `lx, lx_power and PV_temp` was asking company by previous Ku

* UART_set
  * ttyUSB setting, including boud rate etc...
* serial_trans
* insert_value
  * insert all data to table `monitor_now` 
  * NOTE: if out of 15 min, average all table `monitor_now` data to insert table `monitor_history_{year}_{month}` and truncate `monitor_now`
  
* update_realtime_table
  * Update `SOC` and `Psolar` to table `LP_BASE_PARM`
  
* serial_wr
  * Function of `receive data` through RS485

* auto_increase_table
  * Increase table in table `monitor_data` when next month