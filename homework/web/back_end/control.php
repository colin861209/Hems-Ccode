
<?php

/*---Connect to mySQL---*/
$servername = "localhost";
$username = "root";
$password = "fuzzy314";
$dbname = "realtime";
$port = "6666";
$conn = new mysqli($servername, $username, $password, $dbname,$port);
if ($conn->connect_error) // Check connection
{
   die("Connection failed: " . $conn->connect_error);
}
mysqli_set_charset($conn,"utf8");//set encode


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


function sql_single_query($conn,$sql_query)
{
  $result = $conn->query($sql_query);
  if ($result->num_rows > 0)
  {
      while($row = $result->fetch_row()) 
    {
      $value=$row;
    }
    return $value[0];
  }
  else
  {
    //echo $sql_query .'→no data , please check query or DB';
    return $value[0];
  }
  $result->free();
}


/*--- Get load table data ---*/
$load_power_row=sql_select_query($conn,'SELECT * FROM `load_list` ORDER BY `group_id` ASC,`number` ASC');
$load_num=sizeof($load_power_row);


//get webpage status
for ($i=1;$i<=$load_num;$i++)
{
   if($_POST["load_".$i]=="on")
   {$web_status[$i-1]=1;}
   else
   {$web_status[$i-1]=0;}
   echo "load_".$i." is ".$web_status[$i-1]."<br>";
}

$sql_row=sql_select_query($conn,'SELECT * FROM `now_status` ORDER BY `id` ASC');

for($i=0;$i<($load_num);$i++)
{
    $sql_status[$i]=intval($sql_row[$i]["status"]);
}




for($i=1;$i<=$load_num;$i++)
{

   if ($web_status[$i-1]!=$sql_status[$i-1])
   {
      echo $i."is DIFF <br>";
      $sql = "INSERT INTO `control_history`(`id`,`status`) VALUES ( '".$i."' , '1' );";
      echo $sql."<br>";
      $result = $conn->query($sql);
      $sql ="UPDATE `now_status` SET `status`='".$web_status[$i-1]."' WHERE `id`= '".$i."'";
      echo $sql."<br>";
      $result = $conn->query($sql);
   }
}
 
$conn->close();
?>
 
<script>
//redirect the client to the control page
location.href = "/new/control_switch.html";
</script>



