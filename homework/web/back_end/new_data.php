<?php

/*---get date---*/
date_default_timezone_set('Asia/Taipei');
$getDate_year= date("Y");
$getDate_month= date("m");
$getDate_day= date("d");

/*Variable define*/
$timeblock=96;
$divide =$timeblock/24;

/*---Connect to mySQL---*/
$servername = "localhost";
$username = "root";
$password = "fuzzy314";
$dbname = "monitor_data";
$port = "6666";
$conn = new mysqli($servername, $username, $password, $dbname,$port);

if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}
mysqli_set_charset($conn,"utf8");//set encode

/*--Select query function (by KUO-WU)--*/
function sql_select_query($conn,$sql_query)
{
  $result = $conn->query($sql_query);
  if ($result->num_rows > 0)
  {
      while($row = $result->fetch_array(MYSQLI_BOTH)) 
    {
      $array[]=$row;
    }
    return $array;
  }
  else
  {
    //echo $sql_query .'→no data , please check query or DB';
    return $array;
  }
  $result->free();
}

function sql_row_select_query($conn,$sql_query)
{
  if ($result = $conn->query($sql_query))
  {
    while($row = $result->fetch_row())
    {
      $array=$row;
    }
    return $array;
  }
  else
  {
    //echo $sql_query .'→no data , please check query or DB';
    return $array;
  }
  $result->free();
}

  //orion資訊
$row_data =sql_row_select_query($conn,"SELECT `Vsys`,`Pload`,`Pbat`,`Psolar`,`Prect`,`Pfc`,`Psell`,`lx`,`SOC` FROM `monitor_now` WHERE datetime = (SELECT max(datetime) as datetime FROM `monitor_now`)");

  $Vsys=$row_data[0];
  $Pload=$row_data[1];
  $Pbat=$row_data[2];
  $Psolar=$row_data[3];
  $Prect=$row_data[4];
  $Pfc=$row_data[5];
  $Psell=$row_data[6];
  $lx=$row_data[7];
  $SOC=$row_data[8]*100.0;
  
  $conn->close();


/*---Connect to mySQL---*/
$servername = "localhost";
$username = "root";
$password = "fuzzy314";
$dbname = "realtime";
$port = "6666";
$conn = new mysqli($servername, $username, $password, $dbname,$port);

if (mysqli_connect_errno()) {
    printf("Connect failed: %s\n", mysqli_connect_error());
    exit();
}
mysqli_set_charset($conn,"utf8");//set encode




/*--- Get solar data---*/
$solar_row=sql_select_query($conn,'SELECT `value` FROM `solar_day`');
for ($i=0; $i<$timeblock ; $i++)
{ 
  $solar[$i]=floatval($solar_row[$i]["value"]);
  //echo $solar_fake[$i].'<br>';
}

/*--- Get electricity pricing---*/
$pricing_row=sql_select_query($conn,'SELECT `price_value` FROM `price`');
for ($i=0; $i <($timeblock/4) ; $i++)
{ 
  for ($k=$i*$divide; $k <$i*$divide+$divide ; $k++)
  { 
    $electric_price[$k]=floatval($pricing_row[$i]["price_value"]);
    //echo $electric_price[$k].'<br>';
  }
}



  $conn->close();





  $data_array_en = [
   "Vsys" => $Vsys,
   "Pload" => $Pload,
   "Pbat" => $Pbat,
   "Psolar"=> $Psolar,
   "Prect"=> $Prect,
   "Pfc" => $Pfc,
   "Psell" => $Psell,
   "lx" => $lx,
   "SOC" => $SOC,
   "solar" => $solar,
   "electric_price" => $electric_price
   ];
   // $data_json_en = json_encode($data_array_en,JSON_FORCE_OBJECT);
   $data_json_en = json_encode($data_array_en);
   echo $data_json_en;
  // echo "getProfile($data_json_en)"; 

?>



  