<?php
/*---get date---*/
date_default_timezone_set('Asia/Taipei');
$getDate_year= date("Y");
$getDate_month= date("m");
$getDate_day= date("d");
$getDate_hour=idate("H");
$getDate_min=idate("i");

/*Variable define*/
$timeblock=96;
$divide =$timeblock/24;

/*---Connect to mySQL---*/
$servername = "140.124.42.70";
$username = "root";
$password = "fuzzy314";
$dbname = "wang";
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
/*--- Get sample time ---*/
$sample_time=$getDate_hour*$divide+(int)($getDate_min/(60/$divide));


/*--- Get load table data ---*/
$load_power_row=sql_select_query($conn,'SELECT * FROM `load_list` ORDER BY `group_id` ASC,`number` ASC');
$load_num=sizeof($load_power_row);
$interrupt_num=0;$uninterrupt_num=0;$varing_num=0;$non_num=0;$non_p=0;
for ($i=0; $i<$load_num;$i++) 
{ 
	if(intval($load_power_row[$i]["group_id"])!=0)
	{
		if(intval($load_power_row[$i]["group_id"])==1)
		{$interrupt_num++;}
		elseif(intval($load_power_row[$i]["group_id"])==2)
		{$uninterrupt_num++;}
		elseif(intval($load_power_row[$i]["group_id"])==3)
		{$varing_num++;}
		$position[$i]=$load_power_row[$i]["number"];
 		//echo $position[$i].'<br>';
	}
	else
	{
		$non_position[$non_p]=$load_power_row[$i]["number"];
		//echo 'nonpo:'.$non_position[$non_p].'<br>';
		$non_num++;$non_p++;
	}     			  //non schedual load "num" will be a start bound of a load power
					  //for example: if non_num=2 then the start bound=2  [0,1,2,3....] at '2' it start has a value
}


/*--- Get schedule table data ---*/
// $schedule_load_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.'&& DAY(`datetime`)='.$getDate_day.' limit 0,'.($load_num-$non_num));
$schedule_load_row=sql_select_query($conn,'SELECT * FROM `control_status` limit 0,'.($load_num-$non_num));
$schedule_load_row_count=sql_single_query($conn,'SELECT COUNT(*) FROM control_status;');

//for general load → decision_variable(t)*power=power(t)   (except varying load)
for ($i=0; $i<($load_num-$non_num-$varing_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		$load_P[$i][$k]=$schedule_load_row[$i][$k]*$load_power_row[$i+$non_num]["power1"];
	}
	//echo $load_P[$i][37].'<br>';
}

//for varying load → directilly get row data 
for ($i=0; $i <$varing_num ; $i++) 
{ 
	// $var_p=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.'&& DAY(`datetime`)='.$getDate_day.' limit '.($schedule_load_row_count-$varing_num+$i).',1');
	$var_p=sql_select_query($conn,'SELECT * FROM `control_status` limit '.($schedule_load_row_count-$varing_num+$i).',1');
	for($k=1;$k<($timeblock+1);$k++)
	{
		$load_P[$interrupt_num+$uninterrupt_num+$i][$k]=$var_p[0][$k];
	}
}


//let the schedule table adjest to correct number
for ($i=0; $i<($load_num-$non_num);$i++) 
{
	for($k=1;$k<($timeblock+1);$k++)
	{
		$sche_loadpower[$position[$i+$non_num]][]=floatval($load_P[$i][$k]);  //start after non_num 		
		//$sche_loadpower_sum[$k]=$sche_loadpower_sum[$k]+floatval($load_P[$i][$k]);
	}
}
//get sum of load
for ($i=1; $i<($load_num-$non_num+1);$i++) 
{
	for($k=0;$k<$timeblock;$k++)
	{
		$sche_loadpower_sum[$k]=$sche_loadpower_sum[$k]+floatval($sche_loadpower[$i][$k]);
	}
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
	
/*--- Get solar data(fake)---*/
$solarfake_row=sql_select_query($conn,'SELECT `value` FROM `solar_day`');
for ($i=0; $i<$timeblock ; $i++)
{ 
	$solar_fake[$i]=floatval($solarfake_row[$i]["value"]);
	//echo $solar_fake[$i].'<br>';
}

// $solarfake_row=sql_select_query($conn,'SELECT `power` FROM `solar_fake`');
// for ($i=0; $i<$timeblock ; $i++)
// { 
// 	$solar_fake[$i]=floatval($solarfake_row[$i]["power"]);
// 	//echo $solar_fake[$i].'<br>';
// }


// /*--- Get schedule grid ---*/
// $buygrid_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.'&& DAY(`datetime`)='.$getDate_day.'&& parameter_id=1');
// /*--- Get schedule battery ---*/
// $battery_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.'&& DAY(`datetime`)='.$getDate_day.'&& parameter_id=2');
// /*--- Get schedule SOC ---*/
// $SOC_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.' &&DAY(`datetime`)='.$getDate_day.'&& parameter_id=3');
// /*--- Get schedule sell grid ---*/
// $sellgrid_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.'DAY(`datetime`)='.$getDate_day.'&& parameter_id=4');
// /*--- Get schedule FC ---*/
// $FC_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.' && DAY(`datetime`)='.$getDate_day.'&& parameter_id=5');
// /*--- Get hedrogen cost---*/
// // $Hydro_cost_row=sql_select_query($conn,'SELECT * FROM `cost` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.' && DAY(`datetime`)='.$getDate_day.'&& control_id=5');


/*--- Get schedule grid ---*/
$buygrid_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=1');
/*--- Get schedule battery ---*/
$battery_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=2');
/*--- Get schedule SOC ---*/
$SOC_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=3');
/*--- Get schedule sell grid ---*/
$sellgrid_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=4');
/*--- Get schedule FC ---*/
$FC_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=5');
/*--- Get schedule Total_FC ---*/
$TFC_row=sql_select_query($conn,'SELECT * FROM `control_status` WHERE parameter_id=6');
/*--- Get hedrogen cost---*/
// $Hydro_cost_row=sql_select_query($conn,'SELECT * FROM `cost` WHERE YEAR(`datetime`)='.$getDate_year.' && MONTH(`datetime`)='.$getDate_month.' && DAY(`datetime`)='.$getDate_day.'&& control_id=5');





for ($i=1;$i<($timeblock+1);$i++)
{ 
    //chart
	$sche_buygrid[$i-1]=floatval($buygrid_row[0][$i]);
	$sche_battery[$i-1]=floatval($battery_row[0][$i]);
	$sche_SOC[$i-1]=floatval($SOC_row[0][$i]);
	$sche_sellgrid[$i-1]=floatval($sellgrid_row[0][$i]);
	$sche_FC[$i-1]=floatval($FC_row[0][$i]);
	$sche_TFC[$i-1]=floatval($TFC_row[0][$i]);
	// $sche_Hydro_cost[$i-1]=floatval($Hydro_cost_row[0][$i])*$divide;
    //table
	$sche_table_load_sum+=round($sche_loadpower_sum[$i-1]/$divide,3);
	$sche_table_buygrid_sum+=round($sche_buygrid[$i-1]/$divide,2);
	$sche_table_sellygrid_sum+=round($sche_sellgrid[$i-1]/$divide,2);	
	$sche_table_FC_sum+=round($sche_FC[$i-1]/$divide,2);
	$sche_table_battery_sum+=round($sche_battery[$i-1]/$divide,2);
	$sche_table_solar_sum+=round(($sche_loadpower_sum[$i-1]+$sche_battery[$i-1]+$sche_sellgrid[$i-1]-$sche_buygrid[$i-1]-$sche_FC[$i-1])/$divide,2);
}

//if want to have some modify of grid and fuel cell
for($i=0;$i<96;$i++)
{
	$sche_FC_price[$i]=0.0;
}



for ($i=1;$i<($timeblock+1);$i++)
{
	$sche_buygrid_cost+=$sche_buygrid[$i-1]*$electric_price[$i-1]/4.0;
	$sche_sellgrid_cost+=$sche_sellgrid[$i-1]*$electric_price[$i-1]/4.0;
	$sche_FC_cost+=$sche_TFC[$i-1]*2.0/4.0;
	// echo $i.'.'.($sche_TFC[$i-1]*2.0/4.0).'<br>';
	if(($sche_TFC[$i-1]/$sche_FC[$i-1]*2.0)>3.6)
	{
		$sche_FC_price[$i-1]=round($sche_TFC[$i-1]/$sche_FC[$i-1]*2.0,3);

		// $sche_FC_eff[$i-1]=$sche_FC[$i-1]/$sche_TFC[$i-1];
	}
}

/*--- Get table value---*/
$sche_table_TL_bill=round(sql_single_query($conn,'SELECT `value` FROM `LP_BASE_PARM` WHERE parameter_id=18 '),2);
$sche_table_var_bill=round(sql_single_query($conn,'SELECT `value` FROM `LP_BASE_PARM` WHERE parameter_id=19'),2);
$sche_table_buy_bill=round(sql_single_query($conn,'SELECT `value` FROM `LP_BASE_PARM` WHERE parameter_id=20'),2)+round(sql_single_query($conn,'SELECT `value` FROM `LP_BASE_PARM` WHERE parameter_id=22'),2);
$sche_table_sell_bill=round(sql_single_query($conn,'SELECT `value` FROM `LP_BASE_PARM` WHERE parameter_id=21'),2);

//if want to have some modify of grid and fuel cell
$sche_table_buy_bill=round($sche_buygrid_cost,2)+round($sche_FC_cost,2);
$sche_table_sell_bill=round($sche_sellgrid_cost,2);

$sche_table_net_bill=round($sche_table_buy_bill-$sche_table_sell_bill,2);
if($sche_table_TL_bill!=0) //in case of haven't do real experiment
{$sche_table_saving= round(($sche_table_TL_bill-$sche_table_net_bill)/$sche_table_TL_bill*100.0,2);}

$conn->close();



/*--- Get monitor table data ---*/
/*---Connect to mySQL---*/
$servername = "localhost";
$username = "root";
$password = "fuzzy314";
$dbname = "monitor_data";
$port = "6666";
$conn = new mysqli($servername, $username, $password, $dbname,$port);
if ($conn->connect_error) // Check connection
{
	die("Connection failed: " . $conn->connect_error);
}
mysqli_set_charset($conn,"utf8");//set encode



/*--- Get monitor load data ---*/
// $result = $conn->query('SELECT * FROM  monitor_history_'.$getDate_year.'_'.$getDate_month.' WHERE `datetime` BETWEEN \''.$getDate_year.'-'.$getDate_month.'-'.$getDate_day.' 00:15:00\' AND \''.$getDate_year.'-'.$getDate_month.'-'.$getDate_day.' 23:59:59\'  ORDER BY `datetime` ASC');
// $result = $conn->query('SELECT * FROM  monitor_history_'.$getDate_year.'_'.$getDate_month.' WHERE `datetime` BETWEEN \''.$getDate_year.'-'.$getDate_month.'-'.($getDate_day-1).' 00:15:00\' AND \''.$getDate_year.'-'.$getDate_month.'-'.($getDate_day).' 00:15:00\'  ORDER BY `datetime` ASC');
if($sample_time!=0)
{
	$result = $conn->query('SELECT * FROM  monitor_history_'.$getDate_year.'_'.$getDate_month.' WHERE `datetime` >= \''.$getDate_year.'-'.$getDate_month.'-'.($getDate_day).' 00:15:00\'   ORDER BY `period` ASC');
}
else
{
	$result = $conn->query('SELECT * FROM  monitor_history_'.$getDate_year.'_'.$getDate_month.' WHERE `datetime` >= \''.$getDate_year.'-'.$getDate_month.'-'.($getDate_day-1).' 00:15:00\' AND `datetime` < \''.$getDate_year.'-'.$getDate_month.'-'.($getDate_day).' 00:15:00\'  ORDER BY `period` ASC LIMIT 96');
}

//↑Because of when 00:15:00 will have the first data
$row_cnt = $result->num_rows;
if ($result->num_rows > 0)
{
    while($row = $result->fetch_array(MYSQLI_BOTH)) 
 	{
 		$monitor_row[]=$row;
 	}
 }

 for ($i=0;$i<$row_cnt;$i++)
 {
 	$monitor_Pload[$i]=floatval($monitor_row[$i]["Pload"])/1000.0;
 	$monitor_Psolar[$i]=floatval($monitor_row[$i]["Psolar"])/1000.0;
 	$monitor_Pbat[$i]=floatval($monitor_row[$i]["Pbat"])/1000.0;   
 	$monitor_Prect[$i]=floatval($monitor_row[$i]["Prect"])/1000.0;
 	$monitor_Pfc[$i]=floatval($monitor_row[$i]["Pfc"])/1000.0;
 	$monitor_Psell[$i]=floatval($monitor_row[$i]["Psell"])/1000.0;
 	$monitor_SOC[$i]=floatval($monitor_row[$i]["SOC"]);
 	//table
 	$monitor_table_load_sum+=round($monitor_Pload[$i]/$divide,3);
	$monitor_table_buygrid_sum+=round($monitor_Prect[$i]/$divide,2);
	$monitor_table_sellygrid_sum+=round($monitor_Psell[$i]/$divide,2);	
	$monitor_table_FC_sum+=round($monitor_Pfc[$i]/$divide,2);
	$monitor_table_battery_sum+=round($monitor_Pbat[$i]/$divide,2);
	$monitor_table_solar_sum+=round($monitor_Psolar[$i]/$divide,2);

 }


//Table lamp price caculation
$P_1=1.63;
$P_2=2.38;
$P_3=3.52;
$P_4=4.80;
$P_5=5.66;
$P_6=6.41;
$Hydro_Price=0.08;
$Hydro_Cons=0.04;

if ($monitor_table_load_sum <= (120.0 / 30.0))
{
	$monitor_table_TL_bill = $monitor_table_load_sum*$P_1;
}
else if (($monitor_table_load_sum > (120.0 / 30.0)) & ($monitor_table_load_sum <= 330.0 / 30.0))
{
	$monitor_table_TL_bill = (120.0*$P_1 + ($monitor_table_load_sum*30.0 - 120.0)*$P_2)/30.0;
}
else if (($monitor_table_load_sum > (330.0 / 30.0)) & ($monitor_table_load_sum <= 500.0 / 30.0))
{
	$monitor_table_TL_bill = (120.0 * $P_1 + (330.0 - 120.0) * $P_2 + ($monitor_table_load_sum*30.0 - 330.0)*$P_3)/30.0;
}
else if (($monitor_table_load_sum > (500.0 / 30.0)) & ($monitor_table_load_sum <= 700.0 / 30.0))
{
	$monitor_table_TL_bill = (120.0 *$P_1 + (330.0 - 120.0) *$P_2 + (500.0 - 330.0) *$P_3 + ($monitor_table_load_sum*30.0 - 500.0)*$P_4)/30.0;
}
else if (($monitor_table_load_sum > (700.0 / 30.0)) & ($monitor_table_load_sum <= 1000.0 / 30.0))
{
	$monitor_table_TL_bill = (120.0 * $P_1 + (330.0 - 120.0)*$P_2 + (500.0 - 330.0) *$P_3 + (700.0 - 500.0)*$P_4 + ($monitor_table_load_sum*30.0 - 700.0 )*$P_5)/30.0;
}
else if ($monitor_table_load_sum > (1000.0 / 30.0))
{
	$monitor_table_TL_bill = (120.0 *$P_1 + (330.0 - 120.0) *$P_2 + (500.0 - 330.0) *$P_3 + (700.0 - 500.0) *$P_4 + (1000.0 - 700.0) *$P_5 + ($monitor_table_load_sum*30.0 - 1000.0 )*$P_6)/30.0;
}

//Varing price caculation
for ($i=0;$i<$row_cnt;$i++)
{ 
	$monitor_table_var_bill+=($monitor_Pload[$i]*$electric_price[$i]/$divide);
}

//buy from utility grid
for ($i=0;$i<$row_cnt;$i++)
{ 
	$monitor_table_buygrid_bill+=($monitor_Prect[$i]*$electric_price[$i]/$divide);
}

//sell to utility grid
for ($i=0;$i<$row_cnt;$i++)
{ 
	$monitor_table_sell_bill+=($monitor_Psell[$i]*$electric_price[$i]/$divide);
}

//fuel price caculation
// for ($i = 0; $i < 101; $i++)
// {
// 		$data_power[$i] = $i *5.13*0.01;
// 		$data_power_all[$i] = $data_power[$i] /(0.00015987*(pow($data_power[$i], 5)) - 0.0020536*(pow($data_power[$i], 4)) + 0.0085778*(pow($data_power[$i], 3)) - 0.0096205*(pow($data_power[$i], 2)) - 0.0419*(pow($data_power[$i], 1)) + 0.53932);
// 		// $data_power_all[$i] = $data_power[$i] /(0.00025732*(pow($data_power[$i], 5)) - 0.003463*(pow($data_power[$i], 4)) + 0.016041*(pow($data_power[$i], 3)) - 0.02706*(pow($data_power[$i], 2)) - 0.026982*(pow($data_power[$i], 1)) + 0.5692);
// }
// for ($i = 0; $i < 5; $i++)
// {
// 		$P_power[$i] = $data_power[$i*(100 / 4.0)];
// 		$P_power_all[$i] = $data_power_all[$i*(100 / 4.0)];

// }


for ($i = 1; $i <=101; $i++)
{
	if ($i == 1)
	{
		$data_power[$i-1] = 0.35;
	}
	else
	{
		$data_power[$i-1] = ($i-1) *(5.13 - 0.35)*0.01 + 0.35;
	}

	$data_power_all[$i-1] = $data_power[$i-1] /(0.00025732*(pow($data_power[$i-1], 5)) - 0.003463*(pow($data_power[$i-1], 4)) + 0.016041*(pow($data_power[$i-1], 3)) - 0.02706*(pow($data_power[$i-1], 2)) - 0.026982*(pow($data_power[$i-1], 1)) + 0.5692);
}

$P_power[0] = 0.0;
$P_power_all[0] = 0.0;

for ($j = 1; $j < 6; $j++)
{
	$P_power[$j] = $data_power[($j - 1)*(100 / (5 - 1))];
	$P_power_all[$j] = $data_power_all[($j - 1)*(100 / (5 - 1))];
}



for ($j = 0; $j <$row_cnt; $j++)
{
	for ($i = 0; $i < 4; $i++)
	{
		if(($monitor_Pfc[$j]>=$P_power[$i])&($monitor_Pfc[$j]<$P_power[$i+1]))
		{
			// $monitor_table_fuel_bill+=$monitor_Pfc[$j]*($P_power_all[$i+1]-$P_power_all[$i])/($P_power[$i+1]-$P_power[$i])*$Hydro_Price/$Hydro_Cons/$divide;
				$multi_factor=($monitor_Pfc[$j]-$P_power[$i])/($P_power[$i+1]-$P_power[$i]);   
				$monitor_table_fuel_bill+=($P_power_all[$i]+$multi_factor*($P_power_all[$i+1]-$P_power_all[$i]))*$Hydro_Price/$Hydro_Cons/$divide;
				if ($monitor_Pfc[$j]>=1.0)
				{$monitor_FC_price[$j]=round(($P_power_all[$i]+$multi_factor*($P_power_all[$i+1]-$P_power_all[$i]))/$monitor_Pfc[$j]*$Hydro_Price/$Hydro_Cons,3);}
				else
				{$monitor_FC_price[$j]=0.0;}
		}
	}
}


$monitor_table_TL_bill=round($monitor_table_TL_bill,2);
$monitor_table_var_bill=round($monitor_table_var_bill,2);
$monitor_table_buy_bill=round($monitor_table_buygrid_bill,2)+round($monitor_table_fuel_bill,2);
$monitor_table_sell_bill=round($monitor_table_sell_bill,2);
$monitor_table_fuel_bill=round($monitor_table_fuel_bill,2);
$monitor_table_net_bill=round($monitor_table_buy_bill-$monitor_table_sell_bill,2);
if($monitor_table_TL_bill!=0) //in case of haven't do real experiment
{$monitor_table_saving= round(($monitor_table_TL_bill-$monitor_table_net_bill)/$monitor_table_TL_bill*100.0,2);}


$conn->close();


//adjust position sell grid
for ($i=0;$i<$timeblock;$i++)
{
	$sche_sellgrid[$i]=$sche_sellgrid[$i]*(-1.0);
}	

for ($i=0;$i<$row_cnt;$i++)
{ 
	$monitor_Psell[$i]=$monitor_Psell[$i]*(-1.0);
}




$data_array = [
 //sehedule data
"load_name"=>$load_name,	
"sche_loadpower"=> $sche_loadpower,
"sche_loadpower_sum"=> $sche_loadpower_sum,
"electric_price" => $electric_price,
"solar_fake" => $solar_fake,
"sche_buygrid" => $sche_buygrid,
"sche_battery"=> $sche_battery,
"sche_SOC" => $sche_SOC,
"sche_sellgrid" => $sche_sellgrid,
"sche_FC"=> $sche_FC,
//"sche_Hydro_cost"=> $sche_Hydro_cost,
//monitor data
"monitor_Pload"=>$monitor_Pload,
"monitor_Psolar"=>$monitor_Psolar,
"monitor_Pbat"=>$monitor_Pbat,
"monitor_Prect"=>$monitor_Prect,
"monitor_Pfc"=>$monitor_Pfc,
"monitor_Psell"=>$monitor_Psell,
"monitor_SOC"=>$monitor_SOC,
//sche table show
"sche_table_load_sum"=>$sche_table_load_sum,
"sche_table_buygrid_sum"=>$sche_table_buygrid_sum,
"sche_table_sellygrid_sum"=>$sche_table_sellygrid_sum,
"sche_table_FC_sum"=>$sche_table_FC_sum,
"sche_table_solar_sum"=>$sche_table_solar_sum,
"sche_table_TL_bill"=>$sche_table_TL_bill,
"sche_table_var_bill"=>$sche_table_var_bill,
"sche_table_buy_bill"=>$sche_table_buy_bill,
"sche_table_sell_bill"=>$sche_table_sell_bill,
"sche_table_net_bill"=>$sche_table_net_bill,
"sche_table_saving"=>$sche_table_saving,
//monitor table show
"monitor_table_load_sum"=>$monitor_table_load_sum,
"monitor_table_buygrid_sum"=>$monitor_table_buygrid_sum,
"monitor_table_sellygrid_sum"=>$monitor_table_sellygrid_sum,
"monitor_table_FC_sum"=>$monitor_table_FC_sum,
"monitor_table_solar_sum"=>$monitor_table_solar_sum,
"monitor_table_TL_bill"=>$monitor_table_TL_bill,
"monitor_table_var_bill"=>$monitor_table_var_bill,
"monitor_table_buy_bill"=>$monitor_table_buy_bill,
"monitor_table_sell_bill"=>$monitor_table_sell_bill,
"monitor_table_net_bill"=>$monitor_table_net_bill,
"monitor_table_saving"=>$monitor_table_saving,
//fc price
"sche_FC_price"=>$sche_FC_price,
// "sche_FC_eff"=>$sche_FC_eff,
"monitor_FC_price"=>$monitor_FC_price
// "monitor_FC_eff"=>$monitor_FC_eff
];
$demochart_json_en = json_encode($data_array);
echo $demochart_json_en;


?>