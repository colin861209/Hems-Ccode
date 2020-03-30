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
$dbname = "realtime";
$port = "6666";
$conn = new mysqli($servername, $username, $password, $dbname,$port);
if ($conn->connect_error) // Check connection
{
	die("Connection failed: " . $conn->connect_error);
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
$load_sche_seq_row=sql_select_query($conn,'SELECT * FROM `load_list` ORDER BY `group_id` ASC,`number` ASC');
$load_num=sizeof($load_sche_seq_row);
$interrupt_num=0;$uninterrupt_num=0;$varing_num=0;$non_num=0;$non_p=0;
for ($i=0; $i<$load_num;$i++) 
{ 
	if(intval($load_sche_seq_row[$i]["group_id"])!=0)
	{
		if(intval($load_sche_seq_row[$i]["group_id"])==1)
		{$interrupt_num++;}
		elseif(intval($load_sche_seq_row[$i]["group_id"])==2)
		{$uninterrupt_num++;}
		elseif(intval($load_sche_seq_row[$i]["group_id"])==3)
		{$varing_num++;}
		$position[$i]=intval($load_sche_seq_row[$i]["number"]);
 		// echo $position[$i].'<br>';
	}
	else
	{
		$non_position[$non_p]=$load_sche_seq_row[$i]["number"];
		//echo 'nonpo:'.$non_position[$non_p].'<br>';
		$non_num++;$non_p++;
	}
	     			  //non schedual load "num" will be a start bound of a load power
					  //for example: if non_num=2 then the start bound=2  [0,1,2,3....] at '2' it start has a value
}

$load_num_seq_row=sql_select_query($conn,'SELECT * FROM `load_list` ORDER BY `number` ASC');
$load_num=sizeof($load_num_seq_row);
for ($i=0; $i<$load_num;$i++) 
{ 
	if(intval($load_num_seq_row[$i]["group_id"])!=0)
	{
 		$load_name[$i]=$load_num_seq_row[$i]["equip_name"];
 		$load_ID[$i]=$load_num_seq_row[$i]["number"];
 		$load_s_time[$i]=$load_num_seq_row[$i]["start_time"];
 		$load_e_time[$i]=$load_num_seq_row[$i]["end_time"];
	}
}


/*--- Get schedule table data ---*/
$schedule_load_row=sql_select_query($conn,'SELECT * FROM `control_status` limit 0,'.($load_num-$non_num));
$schedule_load_row_count=sql_single_query($conn,'SELECT COUNT(*) FROM control_status;');

//for general load → decision_variable(t)*power=power(t)   (except varying load)
for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		//$load_P[$i][$k]=$schedule_load_row[$i][$k]*$load_power_row[$position[$i]+1]["power1"];
		$load_P[$i][$k]=$schedule_load_row[$i][$k]*$load_sche_seq_row[$i+$non_num]["power1"];
		// $load_P[$i][$k]=$schedule_load_row[$position[$i]-1][$k];
	}
}

//for varying load → directilly get row data 
for ($i=0; $i <$varing_num ; $i++) 
{ 
	$var_p=sql_select_query($conn,'SELECT * FROM `control_status` limit '.($schedule_load_row_count-$varing_num+$i).',1');
	for($k=1;$k<($timeblock+1);$k++)
	{
		$load_P[$interrupt_num+$uninterrupt_num+$i][$k]=$var_p[$i][$k];
	}
}


//convert to number sequence
//inintial array
for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		$convert_array[$i][$k]=0.0;
	}
}


for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		$convert_array[$position[$i+$non_num]-1][$k]=floatval($load_P[$i][$k]);
	}
}

for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		$sche_loadpower[$i][]=$convert_array[$i][$k];
	}
}


//get sum of load
for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=0;$k<$timeblock;$k++)
	{
		$sche_loadpower_sum[$k]=$sche_loadpower_sum[$k]+$sche_loadpower[$i][$k];
	}
}


// /*--- Get electricity pricing---*/
$pricing_row=sql_select_query($conn,'SELECT `price_value` FROM `price`');
for ($i=0; $i <($timeblock/4) ; $i++)
{ 
	for ($k=$i*$divide; $k <$i*$divide+$divide ; $k++)
	{ 
		$electric_price[$k]=floatval($pricing_row[$i]["price_value"]);
	}
}
	
// /*--- Get solar data(fake)---*/
// $solar_row=sql_select_query($conn,'SELECT `value` FROM `solar_day`');
// for ($i=0; $i<$timeblock ; $i++)
// { 
// 	$solar_fake[$i]=floatval($solar_row[$i]["value"]);
// }


$conn->close();


$data_array = [
 //sehedule data
"load_P"=>$load_P,
"load_name"=>$load_name,
"load_ID"=>$load_ID,
// "solar_fake"=>$solar_fake,
"electric_price"=>$electric_price,
"convert_array"=>$convert_array,
"sche_loadpower"=> $sche_loadpower,
"sche_loadpower_sum"=> $sche_loadpower_sum,
"load_s_time"=>$load_s_time,
"load_e_time"=>$load_e_time,
];
$demochart_json_en = json_encode($data_array);
echo $demochart_json_en;

?>