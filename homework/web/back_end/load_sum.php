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

//no-scheduling count
$sql = "SELECT COUNT(*) FROM load_list where `group_id`='0'";
$result = $conn->query($sql);
$row = $result->fetch_row();
$notset_sum=$row[0];
//interrupt_load count
$sql = "SELECT COUNT(*) FROM load_list where `group_id`='1'";
$result = $conn->query($sql);
$row = $result->fetch_row();
$interrupt_sum=$row[0];

//uninterrupt_load count
$sql = "SELECT COUNT(*) FROM load_list where `group_id`='2'";
$result = $conn->query($sql);
$row = $result->fetch_row();

$uninterrupt_sum=$row[0];
//varying_load count
$sql = "SELECT COUNT(*) FROM load_list where `group_id`='3'";
$result = $conn->query($sql);
$row = $result->fetch_row();

$varying_sum=$row[0];
//loads total count
$total_load_sum= $interrupt_sum + $uninterrupt_sum + $varying_sum+$notset_sum;


for ($i=1;$i<=16;$i++) 
{
  $sql = ("SELECT * FROM `load_list` WHERE `number`=".$i."");
  $result = $conn->query($sql);
  $row = $result->fetch_row();
  $load_equip_id[$i]=$row[0];
  $load_group_id[$i]=$row[1];
  $load_start_time[$i]=$row[2];
  $load_end_time[$i]=$row[3];
  $load_operation_time[$i]=$row[4];
  $load_power_1[$i]=$row[5];
  $load_power_2[$i]=$row[6];
  $load_power_3[$i]=$row[7];
  $load_block_1[$i]=$row[8];
  $load_block_2[$i]=$row[9];
  $load_block_3[$i]=$row[10];
  $equip_name[$i]=$row[11];
}

 for ($i=1;$i<=16;$i++)
{
    if ($load_group_id[$i]==0) {
       $load_type[$i]="不加入排程";
    }
    if ($load_group_id[$i]==1) {
       $load_type[$i]="可中斷負載";
    }
    if ($load_group_id[$i]==2) {
       $load_type[$i]="不可中斷負載";
    }
    if ($load_group_id[$i]==3) {
       $load_type[$i]="變動負載";
    }
}


$data_array = [
 //sehedule data
"interrupt_sum"=>$interrupt_sum,	
"uninterrupt_sum"=> $uninterrupt_sum,
"varying_sum"=> $varying_sum,
"notset_sum"=> $notset_sum,
"total_load_sum"=> $total_load_sum,
"load_equip_id"=>$load_equip_id,	
"load_group_id"=> $load_group_id,
"load_start_time"=> $load_start_time,
"load_end_time" => $load_end_time,
"load_operation_time" => $load_operation_time,
"load_power_1" => $load_power_1,
"load_power_2" => $load_power_2,
"load_power_3" => $load_power_3,
"load_block_1" => $load_block_1,
"load_block_2" => $load_block_2,
"load_block_3" => $load_block_3,
"equip_name"=> $equip_name,
"load_type"=> $load_type
];
$demochart_json_en = json_encode($data_array);
echo $demochart_json_en;







?>

