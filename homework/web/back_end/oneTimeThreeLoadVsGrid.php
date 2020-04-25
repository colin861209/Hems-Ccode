<?php
require 'fetch_mysql.php';

$electric_price_array = sqlFetchAssoc($conn, "SELECT `price_value` FROM `price` ", array("price_value"));

$load_list_array = sqlFetchAssoc($conn, "SELECT start_time, end_time, operation_time, power1, power2, power3, block1, block2, block3, number, equip_name  FROM load_list ", array("start_time","end_time", "operation_time", "power1", "power2", "power3", "block1", "block2", "block3", "number", "equip_name"));

$interrupt_num = sqlFetchRow($conn, "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 ", $oneValue);
$uninterrupt_num = sqlFetchRow($conn, "SELECT count(*) AS numcols FROM load_list WHERE group_id=2 ", $oneValue);
$varying_num = sqlFetchRow($conn, "SELECT count(*) AS numcols FROM load_list WHERE group_id=3 ", $oneValue);

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

for($i=0; $i<$interrupt_num + $uninterrupt_num + $varying_num; $i++) {

    $start_time[$i] = intval($load_list_array[0][$i]);
    $end_time[$i] = intval($load_list_array[1][$i]);
    $operation_time[$i] = intval($load_list_array[2][$i]);
    $power1[$i] = floatval($load_list_array[3][$i]);
    $power2[$i] = floatval($load_list_array[4][$i]);
    $power3[$i] = floatval($load_list_array[5][$i]);
    $block1[$i] = intval($load_list_array[6][$i] * 4);
    $block2[$i] = intval($load_list_array[7][$i] * 4);
    $block3[$i] = intval($load_list_array[8][$i] * 4);
    $number[$i] = intval($load_list_array[9][$i]);
    $equip_name[$i] = $load_list_array[10][$i];

}

for ($i=0; $i<$varying_num; $i++) {
    for($y=0; $y<$block1[$interrupt_num+$uninterrupt_num]; $y++){
        $varying_power[$i][$y] = $power1[$interrupt_num + $uninterrupt_num];
    }
    for($y=0; $y<$block2[$interrupt_num+$uninterrupt_num]; $y++){
        $varying_power[$i][$y+$block1[$interrupt_num+$uninterrupt_num]] = $power2[$interrupt_num + $uninterrupt_num];
    }
    for($y=0; $y<$block3[$interrupt_num+$uninterrupt_num]; $y++){
        $varying_power[$i][$y+$block1[$interrupt_num+$uninterrupt_num]+$block2[$interrupt_num+$uninterrupt_num]] = $power3[$interrupt_num + $uninterrupt_num];
    }
}
// load_status_array
for($u=0; $u<$interrupt_num+$uninterrupt_num+$varying_num; $u++){

    if($u<$interrupt_num+$uninterrupt_num){

        for($y=0;$y<96;$y++)
        { $load_power[$u][] = $power1[$u]*$load_status_array[$u][$y]; }

    } else {
        $z=0;
        for($y=0;$y<96;$y++){
            if($load_status_array[$u][$y] == 1){
                $load_power[$u][] = $varying_power[$u-$interrupt_num-$uninterrupt_num][$z]*$load_status_array[$u][$y];
                $z++;
            } else {
                $load_power[$u][] = 0.0;
            }
            
        }
    }
    
}

$data_array = [
    "interrupt_num"=>$interrupt_num,
    "uninterrupt_num"=>$uninterrupt_num,
    "varying_num"=>$varying_num,
    "electric_price"=>$electric_price,
    "start_time"=>$start_time,
    "end_time"=>$end_time,
    "operation_time"=>$operation_time,
    "power1"=>$power1,
    "power2"=>$power2,
    "power3"=>$power3,
    "block1"=>$block1,
    "block2"=>$block2,
    "block3"=>$block3,
    "limit_capability"=>$limit_capability,
    "load_power"=>$load_power,
    "equip_name"=>$equip_name,
    "load_num"=>$number,
];

    
echo json_encode($data_array);


?>