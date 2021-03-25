# fc_control doc
* calc_crc()
   * create crc for serial_wr() based on the commend 
     * crc is a method that can check whether the commend send from PC is correctly recieve by RTU
---
* serial_wr()
   *  sent commend and crc to RTU
   *  recieve data( `now voltage`/`now current`/`now power state`) from RTU
---
* UART_set()
   * connect to modbus RTU
   * set baudrate/timeout/etc...
---
* main() spec
   1. initial `UART`/`Mysql`
   2. get `power_enable` / `power_status` from Mysql
   3. if `power_enable`!=`on` (on = 1 off = 0)
      * set `power_status` in Mysql to 0
      * set `set_current` in Mysql to 0
      * set `now_voltage` in Mysql to 0
      * set `now_current` in Mysql to 0
      * set `now_power` in Mysql to 0
   4. if `power_status`!=`on` (on = 1 off = 0)
      * set voltage = 54(V) in RTU
      * set current = 0.1(A) in RTU
   5. get `target_power`
   6. check whether `target_power` is in safe range
   7. get `now_voltage` `now_current` and calculate `now_power`
    #### *we hope that `now_power` approaches `target_power`*
   8. check whether `now_power`>=`target_power` 
      >if *true*. set a lower current based on `target_power`/`now_voltage` 
      * set new current in RTU
      * get `now_voltage` `now_current` from RTU
      * update `set_current` in Mysql
      * update`now_voltage` in Mysql
      * update `now_current` in Mysql
      * update `now_power` in Mysql
      >if *false*. increase current and iteration until `now_power` approaches `target_power`
      * iteration until (`now_power`>=`target_power`)  
      * set new current in RTU (+4A per iteration)
      * get `now_voltage` `now_current` from RTU
      * update `set_current` in Mysql
      * update`now_voltage` in Mysql
      * update `now_current` in Mysql
      * update `now_power` in Mysql
   9. close connection to RTU and Mysql
  
