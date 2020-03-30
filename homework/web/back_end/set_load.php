<?php
    require_once("load_sum.php");
    $set_load_[1]=$_POST["equip_number"];
    $set_load_[2]=$_POST["type"];
    $set_load_[3]=$_POST["starttime"];
    $set_load_[4]=$_POST["endtime"];
    $set_load_[5]=$_POST["operationtime"];
    $set_load_[6]=$_POST["loadpower"];
    $set_load_[7]=$_POST["loadblock"];
    $set_load_[8]=$_POST["loadname"];
    // for ($i=1; $i <=8; $i++) {
    //     echo $set_load_[$i]."<br>";
    // }
    //10 可中斷 20 不可中斷 30 變動型
     if ($set_load_[2]==10) {
         $set_load_[2]=1;
         $sql = ("UPDATE `load_list` SET `group_id`='".$set_load_[2]."',`start_time`='".$set_load_[3]."',`end_time`='".$set_load_[4]."',`operation_time`='".$set_load_[5]."',`power1`='".$set_load_[6]."',`power2`='NULL',`power3`='NULL',`block1`='NULL',`block2`='NULL',`block3`='NULL',`equip_name`='".$set_load_[8]."' WHERE `number`='".$set_load_[1]."'");
         $result = $conn->query($sql);
     }
     if ($set_load_[2]==20) {
         $set_load_[2]=2;
         $sql = ("UPDATE `load_list` SET `group_id`='".$set_load_[2]."',`start_time`='".$set_load_[3]."',`end_time`='".$set_load_[4]."',`operation_time`='".$set_load_[5]."',`power1`='".$set_load_[6]."',`power2`='NULL',`power3`='NULL',`block1`='NULL',`block2`='NULL',`block3`='NULL',`equip_name`='".$set_load_[8]."' WHERE `number`='".$set_load_[1]."'");
         $result = $conn->query($sql);
     }
     
    //將字串變陣列
     if ($set_load_[2]==30) {
         $set_load_[2]=3;
         $str01=$set_load_[6];
         $str11=explode("/",$str01);
         $str02=$set_load_[7];
         $str22=explode("/",$str02);
         
         $sql = ("UPDATE `load_list` SET `group_id`='".$set_load_[2]."',`start_time`='".$set_load_[3]."',`end_time`='".$set_load_[4]."',`operation_time`='".$set_load_[5]."',`power1`='".$str11[0]."',`power2`='".$str11[1]."',`power3`='".$str11[2]."',`block1`='".$str22[0]."',`block2`='".$str22[1]."',`block3`='".$str22[2]."',`equip_name`='".$set_load_[8]."' WHERE `number`='".$set_load_[1]."'");
         $result = $conn->query($sql);
     }

     if ($set_load_[2]==40) {
         $set_load_[2]=0;
            
         $sql = ("UPDATE `load_list` SET `group_id`='".$set_load_[2]."',`start_time`='".$set_load_[3]."',`end_time`='".$set_load_[4]."',`operation_time`='".$set_load_[5]."',`power1`='".$set_load_[6]."',`power2`='NULL',`power3`='NULL',`block1`='NULL',`block2`='NULL',`block3`='NULL',`equip_name`='".$set_load_[8]."' WHERE `number`='".$set_load_[1]."'");
         $result = $conn->query($sql);
     }
    ?>
 <script>
    //將用戶端重新導向到控制頁面
    location.href = "/new/set_data.html";
    </script>
