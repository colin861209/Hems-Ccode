<?PHP
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
$interrupt_num=0;$uninterrupt_num=0;$varying_num=0;$non_num=0;$non_p=0;
for ($i=0; $i<$load_num;$i++) 
{ 
  if(intval($load_power_row[$i]["group_id"])!=0)
  {
    $equip_num[$i-$non_num]=intval($load_power_row[$i]["number"]);
    $equip_name[$i-$non_num]=$load_power_row[$i]["equip_name"];

    if(intval($load_power_row[$i]["group_id"])==1)
    {$interrupt_num++;}
    elseif(intval($load_power_row[$i]["group_id"])==2)
    {$uninterrupt_num++;}
    elseif(intval($load_power_row[$i]["group_id"])==3)
    {$varying_num++;}
  }
  else
  {
    $non_equip_num[$i]=intval($load_power_row[$i]["number"]);
    $non_equip_name[$i]=$load_power_row[$i]["equip_name"];
    $non_num++;
  }             //non schedual load "num" will be a start bound of a load power
            //for example: if non_num=2 then the start bound=2  [0,1,2,3....] at '2' it start has a value
}


$switch_row=sql_select_query($conn,'SELECT * FROM `now_status` ORDER BY `id` ASC');

for($i=0;$i<($load_num-$non_num);$i++)
{
    $equip_status[$i]=intval($switch_row[$equip_num[$i]-1]["status"]);
}

for($i=0;$i<$non_num;$i++)
{
    $non_equip_status[$i]=intval($switch_row[$non_equip_num[$i]-1]["status"]);
}




$data_array = [
 //sehedule data
"interrupt_num"=>$interrupt_num,	
"uninterrupt_num"=> $uninterrupt_num,
"varying_num"=> $varying_num,
"non_num"=> $non_num,
"load_num"=> $load_num,
"equip_num"=> $equip_num,
"equip_name"=> $equip_name,
"equip_status"=> $equip_status,
"non_equip_num"=> $non_equip_num,
"non_equip_name"=> $non_equip_name,
"non_equip_status"=> $non_equip_status
];
$demochart_json_en = json_encode($data_array);
echo $demochart_json_en;


?>

