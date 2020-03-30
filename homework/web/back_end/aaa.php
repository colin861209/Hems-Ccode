<?php

$conn = new mysqli('140.124.42.70','root','fuzzy314','wang','6666');

$sql="SELECT `price_value` FROM `price` ";
$result=mysqli_query($conn,$sql);
while($row = mysqli_fetch_row($result)){
    if(!$row>0){
        $status= "can not get pole account";
        echo json_encode(array("response"=>$status));
    }else {
        for ($i=0;$i<4;$i++){
            $electric_price[] = floatval($row[0]);
        }
    }
}
mysqli_free_result($result);

$sql = "SELECT start_time, end_time, operation_time, power1, number, equip_name  FROM load_list WHERE group_id = 1 && number<6" ;
$result=mysqli_query($conn,$sql);
while($row = mysqli_fetch_row($result)){
    if(!$row>0){
        $status= "can not get pole account";
        echo json_encode(array("response"=>$status));
    }else {
        $start_time[] = intval($row[0]);
        $end_time[] = intval($row[1]);
        $operation_time[] = intval($row[2]);
        $power1[] = floatval($row[3]);
        $number[] = intval($row[4]);
        $equip_name[] = $row[5];
    }
}
mysqli_free_result($result);

$sql = "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 && number<6 " ;
$result=mysqli_query($conn,$sql);
$row = mysqli_fetch_row($result);
$interrupt_num = floatval($row[0]);
mysqli_free_result($result);

$sql = "SELECT `value` FROM `LP_BASE_PARM` where `parameter_id` = 13 ";
$result = mysqli_query($conn,$sql);
$row = mysqli_fetch_row($result);
$limit_power= floatval($row[0]);
mysqli_free_result($result);

$k=0;
$sql="SELECT * FROM `control_status` ";
$result=mysqli_query($conn,$sql);

// $row=mysqli_fetch_array($result,MYSQLI_NUM);
while($row=mysqli_fetch_array($result,MYSQLI_NUM)){

    for($i=2;$i<98;$i++){
        $load_status[$k][] = floatval($row[$i]);
    }
    $k++ ;
}
mysqli_free_result($result);
mysqli_close($conn);

for($y=0;$y<96;$y++){
    $limit_capability[$y] = $limit_power; 
}

for($u=0;$u<3;$u++){
    for($y=0;$y<96;$y++){
        $load_power[$u][] = $power1[$u]*$load_status[$u][$y];
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
    "load_status"=>$load_status,
    "load_power"=>$load_power,
    "equip_name"=>$equip_name,
    "load_num"=>$number
];

    
echo json_encode($data_array);


?>