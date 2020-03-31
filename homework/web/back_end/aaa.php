<?php

$conn = new mysqli('140.124.42.70','root','fuzzy314','wang','6666');
$oneValue = "oneValue";
$aRowInt = "aRowInt";
$aRowFloat = "aRowFloat";
$aRowString = "aRowString";
$multiRow = "multiRow";

function sqlGetValue ($conn, $sql, $key) {

    switch ($key) {
        case "oneValue":
            $result=mysqli_query($conn,$sql);
            $row = mysqli_fetch_row($result);
            $value = floatval($row[0]);
            return $value;
            mysqli_free_result($result);
            break;
        case "aRowInt":
            $result=mysqli_query($conn,$sql);
            while($row = mysqli_fetch_row($result)){
                $array[] = floatval($row[0]);
            }
            return $array;
            mysqli_free_result($result);
            break;
        case "aRowFloat":
            $result=mysqli_query($conn,$sql);
            while($row = mysqli_fetch_row($result)){
                $array[] = floatval($row[0]);
            }
            return $array;
            mysqli_free_result($result);
            break;
        case "aRowString":
            $result=mysqli_query($conn,$sql);
            while($row = mysqli_fetch_row($result)){
                $array[] = $row[0];
            }
            return $array;
            mysqli_free_result($result);
            break;
        case "multiRow":
            $result=mysqli_query($conn,$sql);
            $i=0;
            while($row = mysqli_fetch_row($result)){
                $array[$i][] = floatval($row[0]);
                $i++;
            }
            return $array;
            mysqli_free_result($result);
            break;
        default:
            echo "tap key";
    }
}

$electric_price_array = sqlGetValue($conn, "SELECT `price_value` FROM `price` ", $aRowFloat);
for($y=0; $y<24; $y++){
    for ($i=0;$i<4;$i++){
        $electric_price[4*$y+$i] = $electric_price_array[$y];
    }
}
// $sql="SELECT `price_value` FROM `price` ";
// $result=mysqli_query($conn,$sql);
// while($row = mysqli_fetch_row($result)){
//     if(!$row>0){
//         $status= "can not get pole account";
//         echo json_encode(array("response"=>$status));
//     }else {
//         for ($i=0;$i<4;$i++){
//             $electric_price[] = floatval($row[0]);
//         }
//     }
// }
// mysqli_free_result($result);



// $load_list_time = sqlGetValue($conn, "SELECT start_time, end_time, operation_time, number FROM load_list WHERE group_id = 1 && number>=6 && number<10 ", $aRowInt);
// $load_list_power = sqlGetValue($conn, "SELECT power1 FROM load_list WHERE group_id = 1 && number>=6 && number<10 ", $aRowFloat);
// $equip_name = sqlGetValue($conn, "SELECT equip_name FROM load_list WHERE group_id = 1 && number>=6 && number<10 ", $aRowString);
$sql = "SELECT start_time, end_time, operation_time, power1, number, equip_name  FROM load_list WHERE group_id = 1 && number>=6 && number<10 " ;
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

$interrupt_num = sqlGetValue($conn, "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 && number>=6 && number<10 ", $oneValue);
// $sql = "SELECT count(*) AS numcols FROM load_list WHERE group_id=1 && number>=6 && number<10 " ;
// $result=mysqli_query($conn,$sql);
// $row = mysqli_fetch_row($result);
// $interrupt_num = floatval($row[0]);
// mysqli_free_result($result);

$limit_power = sqlGetValue($conn, "SELECT `value` FROM `LP_BASE_PARM` where `parameter_id` = 13 ", $oneValue);
// $sql = "SELECT `value` FROM `LP_BASE_PARM` where `parameter_id` = 13 ";
// $result = mysqli_query($conn,$sql);
// $row = mysqli_fetch_row($result);
// $limit_power= floatval($row[0]);
// mysqli_free_result($result);

// $load_status_array = sqlGetValue($conn, "SELECT * FROM `control_status` ", $multiRow);
$k=0;
$sql="SELECT * FROM `control_status` ";
$result=mysqli_query($conn,$sql);
while($row=mysqli_fetch_array($result,MYSQLI_NUM)){

    for($i=1;$i<98;$i++){
        $load_status[$k][] = floatval($row[$i]);
    }
    $k++ ;
}
mysqli_free_result($result);

mysqli_close($conn);

for($y=0;$y<96;$y++){
    $limit_capability[$y] = floatval($limit_power); 
}

for($u=0;$u<3;$u++){
    for($y=0;$y<96;$y++){
        $load_power[$u][] = $power1[$u]*$load_status[$u][$y];
    }

}

for($u=0;$u<count($load_power);$u++){
    for($y=0;$y<96;$y++){
        $load_power_in_one[$u][] = $load_power[$u][$y];
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
    // "load_status_array"=>$load_status_array,
    "load_power"=>$load_power,
    "equip_name"=>$equip_name,
    "load_num"=>$number,
    // "electric_price_array"=>$electric_price_array
];

    
echo json_encode($data_array);


?>