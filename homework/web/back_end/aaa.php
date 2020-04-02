<?php
require 'fetch_mysql.php';

$electric_price_array = sqlFetchAssoc($conn, "SELECT `price_value` FROM `price` ", array("price_value"));


$load_list_array = sqlFetchAssoc($conn, "SELECT start_time, end_time, operation_time, power1, number, equip_name  FROM load_list WHERE group_id = 1 && number>=6 && number<10 ", array("start_time","end_time", "operation_time", "power1", "number", "equip_name"));

$interrupt_num = sqlFetchRow($conn, "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 && number>=6 && number<10 ", $oneValue);

$limit_power = sqlFetchRow($conn, "SELECT `value` FROM `LP_BASE_PARM` where `parameter_id` = 13 ", $oneValue);
for($y=0;$y<96;$y++)
{ $limit_capability[$y] = floatval($limit_power); }

$load_status_array = sqlFetchRow($conn, "SELECT * FROM `control_status` ", $controlStatusResult);
mysqli_close($conn);

// electric_price_array
for($y=0; $y<24; $y++){

    for ($i=0;$i<4;$i++)
    { $electric_price[4*$y+$i] = floatval($electric_price_array[$y]); }

}

// load_list_array
for($i=0; $i<count($load_list_array[0]); $i++) {

    $start_time[$i] = intval($load_list_array[0][$i]);
    $end_time[$i] = intval($load_list_array[1][$i]);
    $operation_time[$i] = intval($load_list_array[2][$i]);
    $power1[$i] = floatval($load_list_array[3][$i]);
    $number[$i] = intval($load_list_array[4][$i]);
    $equip_name[$i] = $load_list_array[5][$i];

}

// load_status_array
for($u=0;$u<3;$u++){
    for($y=0;$y<96;$y++){
        $load_power[$u][] = $power1[$u]*$load_status_array[$u][$y];
    }

}

$data_array = [
    "interrupt_num"=>$interrupt_num,
    "electric_price"=>$electric_price,
    "start_time"=>$start_time,
    "end_time"=>$end_time,
    "operation_time"=>$operation_time,
    "power1"=>$power1,
    "limit_capability"=>$limit_capability,
    "load_power"=>$load_power,
    "equip_name"=>$equip_name,
    "load_num"=>$number,
];

    
echo json_encode($data_array);


?>