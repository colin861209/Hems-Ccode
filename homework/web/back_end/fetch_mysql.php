<?php

$conn = new mysqli('140.124.42.70','root','fuzzy314','wang','6666');
$oneValue = "oneValue";
$aRow = "aRow";
$controlStatusResult = "controlStatusResult";

function sqlFetchRow ($conn, $sql, $key) {

    switch ($key) {
        case "oneValue":
            // no convert data type, is a string
            $result=mysqli_query($conn,$sql);
            $row = mysqli_fetch_row($result);
            $value = $row[0];
            return $value;
            mysqli_free_result($result);
            break;

        case "aRow":
            // no convert data type, is a string
            $result=mysqli_query($conn,$sql);
            while($row = mysqli_fetch_row($result)){
                $array = $row;
            }
            return $array;
            mysqli_free_result($result);
            break;

        case "controlStatusResult":
            // convert data type to float
            $k=0;
            $result=mysqli_query($conn,$sql);
            while($row=mysqli_fetch_array($result,MYSQLI_NUM)){

                for($i=1;$i<97;$i++)
                { $array[$k][] = floatval($row[$i]); }
                $k++ ;

            }
            return $array;
            mysqli_free_result($result);
            break;

        default:
            echo "tap key";
    }
}

function sqlFetchAssoc($conn, $sql, $key) {

    $result=mysqli_query($conn,$sql);
    $count=mysqli_num_rows($result);
    if(count($key) == 1){
        for($i=0;$i<$count;$i++){

            $row = mysqli_fetch_assoc($result);
            $array[$i] = $row[$key[0]];

        }
        // return one dimensional array
    } 
    elseif(count($key) > 1) {
        for($i=0;$i<$count;$i++){
            
            $row = mysqli_fetch_assoc($result);
            for($k=0; $k<count($key); $k++) 
            { $array[$k][$i] = $row[$key[$k]]; }
            
        }
        // return two dimensional array
    }
    
    return $array;
    mysqli_free_result($result);
}
?>