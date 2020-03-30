//get time block
    // var dt = new Date();
    // var now_t= Math.floor(dt.getHours()*4+dt.getMinutes()/15);


    function show_value(str)
    {
        if (str != "")
        {
            if (window.XMLHttpRequest) 
            {
                xmlhttp = new XMLHttpRequest();// code for IE7+, Firefox, Chrome, Opera, Safari
            }
            else 
            {
                xmlhttp = new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
            }            
            xmlhttp.onreadystatechange = function()
            {
                if (this.readyState == 4 && this.status == 200)
                {
                    console.log(this.responseText);
                    show_load(this.responseText);
                }
            };
            xmlhttp.open("GET","back_end/load_sum.php",true);
            xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded;");
            xmlhttp.send();
        }
    }
    


    function show_load(ABC)
    {
        var GET_LOAD_DATA = JSON.parse(ABC);
        var load_num =Number(GET_LOAD_DATA.total_load_sum);
        var load_id =GET_LOAD_DATA.load_equip_id;
        var load_group =GET_LOAD_DATA.load_group_id;
        var load_s =GET_LOAD_DATA.load_start_time;
        var load_e =GET_LOAD_DATA.load_end_time;
        var load_o =GET_LOAD_DATA.load_operation_time;
        var load_p_1 =GET_LOAD_DATA.load_power_1;
        var load_p_2 =GET_LOAD_DATA.load_power_2;
        var load_p_3 =GET_LOAD_DATA.load_power_3;
        var load_b_1 =GET_LOAD_DATA.load_block_1;
        var load_b_2 =GET_LOAD_DATA.load_block_2;
        var load_b_3 =GET_LOAD_DATA.load_block_3;
        var load_name =GET_LOAD_DATA.equip_name;
        var load_type =GET_LOAD_DATA.load_type;

        console.log(load_b_1);
  
        var i;


        var htmlElements = "";

        var tr = document.getElementById('myTable').getElementsByTagName('thead')[0],
            th = document.createElement('tr');
        th.innerHTML =  '<th class="text-center">編號</th><th class="text-center">類別</th><th class="text-center">起始時間</th><th class="text-center">終止時間</th><th class="text-center">執行時間</th><th class="text-center">設備功率</th><th class="text-center">執行區間</th><th class="text-center">設備名稱</th>';
        tr.appendChild(th);


        var table = document.getElementById("myTable").getElementsByTagName('tbody')[0];
        for ( i = 0; i < load_num; i++) 
        {
            var row = table.insertRow(i);
            var cell1 = row.insertCell(0);
            var cell2 = row.insertCell(1);
            var cell3 = row.insertCell(2);
            var cell4 = row.insertCell(3);
            var cell5 = row.insertCell(4);
            var cell6 = row.insertCell(5);
            var cell7 = row.insertCell(6);
            var cell8 = row.insertCell(7);
            cell1.innerHTML = load_id[Object.keys(load_id)[i]];
            cell2.innerHTML = load_type[Object.keys(load_type)[i]];
            cell3.innerHTML = load_s[Object.keys(load_s)[i]];
            cell4.innerHTML = load_e[Object.keys(load_e)[i]];
            cell5.innerHTML = load_o[Object.keys(load_o)[i]];
            if(load_group[Object.keys(load_group)[i]]!='3')
            {
                cell6.innerHTML = load_p_1[Object.keys(load_p_1)[i]];
                cell7.innerHTML = load_b_1[Object.keys(load_b_1)[i]];
            }
            else
            {
                cell6.innerHTML = load_p_1[Object.keys(load_p_1)[i]]+'/'+load_p_2[Object.keys(load_p_2)[i]]+'/'+load_p_3[Object.keys(load_p_3)[i]];
                cell7.innerHTML = load_b_1[Object.keys(load_b_1)[i]]+'/'+load_b_2[Object.keys(load_b_2)[i]]+'/'+load_b_3[Object.keys(load_b_3)[i]];
            }
            cell8.innerHTML = load_name[Object.keys(load_name)[i]];
        }   
        console.log(htmlElements);
        var container = document.getElementById("containers");
        container.innerHTML = htmlElements;
    }