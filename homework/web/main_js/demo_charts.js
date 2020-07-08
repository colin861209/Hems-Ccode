 //get time block
    var dt = new Date();
    var now_t= Math.floor(dt.getHours()*4+dt.getMinutes()/15);
    console.log(now_t);

    function set_series_function(multi,series_type,DATA,stack_class,yAxis_locate,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis)
    { 
        if (multi==1)
        {var DATA_NUM = Object.keys(DATA).length;} //get data row num       
        else
        {var DATA_NUM = 1;}//get data row num
        
        for (i=0 ; i < DATA_NUM; i++)
        { 
            chart_series_type.push(series_type);
            if (multi==1)
            {
                chart_series_name.push(Object.keys(DATA)[i]);  
                chart_series_data.push(DATA[Object.keys(DATA)[i]]);
            }
            else
            {
                chart_series_name.push(stack_class);  //same as stack name
                chart_series_data.push(DATA);
            }
            chart_series_stack.push(stack_class);
            chart_series_yAxis.push(yAxis_locate);
        }
    }    

    function show_chart(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis)
    {

            //set all series data
            var series_data = [],
            len = Object.keys(chart_series_name).length,
            i = 0;
    
            for(i;i<(len);i++){
                series_data.push({
                type: chart_series_type[i],
                name: chart_series_name[i],
                data: chart_series_data[i],
                stack: chart_series_stack[i],
                yAxis: chart_series_yAxis[i]
            });
            }      
            //set all chart data
            var charts =Highcharts.chart(chart_info[0], {
            title: {
            text: chart_info[1],
            style: {
            fontWeight: 'bold',
            fontSize: '24px'
            }
            },
            subtitle: {
            text: chart_info[2]
            },
            legend: {
            itemStyle: {
                fontWeight: 'bold',
                fontSize: '18px'
              }
            },
            xAxis: {
            max: 95,    
            title: {text: chart_info[3],style: {
            fontWeight: 'bold',
            fontSize: '16px'
            }},
            categories: [],
            plotLines: [{
            color: 'red', // Color value
            dashStyle: 'ShortDash', // Style of the plot line. Default to solid
            value: now_t, // Value of where the line will appear
            width: 1, // Width of the line   
            }
            ]
            },
            yAxis: [{
            min: 0,
            title: {
            text: chart_info[4],style: {
            fontWeight: 'bold',
            fontSize: '16px'
            }
            }
            }, {
            min: -4,
            max: 4,
            title: {
            text: chart_info[5],
            rotation: 270,
            style: {
            fontWeight: 'bold',
            fontSize: '16px'
            }
            },
            opposite: true
            }]
            ,
            tooltip: {
            //enabled: false
            formatter: function () {
            return '<b>' + this.x + '</b><br/>' +
            this.series.name + ': ' + this.y + '<br/>' +
            'Total: ' + this.point.stackTotal;
            }
            },
            plotOptions: {
            line: {
            dataLabels: {
            enabled: false
            },
            enableMouseTracking: false,
            marker: {
            enabled: false
            }
            },
            column: {
            stacking: 'normal'}
            },
            series: series_data
            }); 

    }
    function show_chart_1(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis)
    {

            //set all series data
            var series_data = [],
            len = Object.keys(chart_series_name).length,
            i = 0;
    
            for(i;i<(len);i++){
                series_data.push({
                type: chart_series_type[i],
                name: chart_series_name[i],
                data: chart_series_data[i],
                stack: chart_series_stack[i],
                yAxis: chart_series_yAxis[i]
            });
            }      
            //set all chart data
            var charts =Highcharts.chart(chart_info[0], {
            title: {
            text: chart_info[1]
            },
            subtitle: {
            text: chart_info[2]
            },
            xAxis: {
            max: 95,    
            title: {text: chart_info[3]},
            categories: [],
            plotLines: [{
            color: 'red', // Color value
            dashStyle: 'ShortDash', // Style of the plot line. Default to solid
            value: now_t, // Value of where the line will appear
            width: 1, // Width of the line   
            }
            ]
            },
            yAxis: [{
            min: 0,
            title: {
            text: chart_info[4]
            }
            }, { 
            title: {
            text: chart_info[5]
            },
            opposite: true
            }]
            ,
            tooltip: {
            //enabled: false
            formatter: function () {
            return '<b>' + this.x + '</b><br/>' +
            this.series.name + ': ' + this.y + '<br/>' +
            'Total: ' + this.point.stackTotal;
            }
            },
            plotOptions: {
            line: {
            dataLabels: {
            enabled: false
            },
            enableMouseTracking: false,
            marker: {
            enabled: false
            }
            },
            column: {
            stacking: 'normal'}
            },
            series: series_data
            }); 

    }
    function show_chart_control_status(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis)
    {

        //set all series data
        var series_data = [],
        len = Object.keys(chart_series_name).length,
        i = 0;

        for(i;i<(len);i++){
            series_data.push({
            type: chart_series_type[i],
            name: chart_series_name[i],
            data: chart_series_data[i],
            stack: chart_series_stack[i],
            yAxis: chart_series_yAxis[i]
        });
        }      
        //set all chart data
        var charts =Highcharts.chart(chart_info[0], {
        title: {
        text: chart_info[1],
        style: {
        fontWeight: 'bold',
        fontSize: '24px'
        }
        },
        subtitle: {
        text: chart_info[2]
        },
        legend: {
        itemStyle: {
            fontWeight: 'bold',
            fontSize: '18px'
            }
        },
        xAxis: {
        max: 95,    
        title: {text: chart_info[3],style: {
        fontWeight: 'bold',
        fontSize: '16px'
        }},
        categories: [],
        plotLines: [{
        color: 'red', // Color value
        dashStyle: 'ShortDash', // Style of the plot line. Default to solid
        value: now_t, // Value of where the line will appear
        width: 1, // Width of the line   
        }
        ]
        },
        yAxis: [{
        min: 0,
        title: {
        text: chart_info[4],style: {
        fontWeight: 'bold',
        fontSize: '16px'
        }
        }
        }, {
        min: 0,
        max: 4,
        title: {
        text: chart_info[5],
        rotation: 270,
        style: {
        fontWeight: 'bold',
        fontSize: '16px'
        }
        },
        opposite: true
        }]
        ,
        tooltip: {
        //enabled: false
        formatter: function () {
        return '<b>' + this.x + '</b><br/>' +
        this.series.name + ': ' + this.y + '<br/>' +
        'Total: ' + this.point.stackTotal;
        }
        },
        plotOptions: {
        line: {
        dataLabels: {
        enabled: false
        },
        enableMouseTracking: false,
        marker: {
        enabled: false
        }
        },
        column: {
            stacking: 'normal'}
        },
        series: series_data
    }); 
    
    }

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
                    simulation_chart_01(this.responseText);
                    simulation_chart_02(this.responseText);
                    monitor_chart_01(this.responseText);
                    monitor_chart_02(this.responseText);
                    table_value(this.responseText);
                    // fc_price_chart(this.responseText);
                }
            };
            xmlhttp.open("GET","back_end/schedule_data.php",true);
            xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded;");
            xmlhttp.send();
        }
    }
    
    // function table_value(ABC)
    // {
    //     var GET_DATA = JSON.parse(ABC);
    //     document.getElementById('1').innerHTML = GET_DATA.sche_table_load_sum;
    //     document.getElementById('2').innerHTML = GET_DATA.sche_table_solar_sum;
    //     document.getElementById('3').innerHTML = GET_DATA.sche_table_buygrid_sum;
    //     document.getElementById('4').innerHTML = GET_DATA.sche_table_sellygrid_sum;
    //     document.getElementById('5').innerHTML = GET_DATA.sche_table_FC_sum;
    //     document.getElementById('6').innerHTML = GET_DATA.sche_table_TL_bill;
    //     document.getElementById('7').innerHTML = GET_DATA.sche_table_var_bill;
    //     document.getElementById('8').innerHTML = GET_DATA.sche_table_buy_bill;
    //     document.getElementById('9').innerHTML = GET_DATA.sche_table_sell_bill;
    //     document.getElementById('10').innerHTML = GET_DATA.sche_table_net_bill;
    //     document.getElementById('11').innerHTML = GET_DATA.sche_table_saving;
    //     document.getElementById('12').innerHTML = GET_DATA.monitor_table_load_sum;
    //     document.getElementById('13').innerHTML = GET_DATA.monitor_table_solar_sum;
    //     document.getElementById('14').innerHTML = GET_DATA.monitor_table_buygrid_sum;
    //     document.getElementById('15').innerHTML = GET_DATA.monitor_table_sellygrid_sum;
    //     document.getElementById('16').innerHTML = GET_DATA.monitor_table_FC_sum;
    //     document.getElementById('17').innerHTML = GET_DATA.monitor_table_TL_bill;
    //     document.getElementById('18').innerHTML = GET_DATA.monitor_table_var_bill;
    //     document.getElementById('19').innerHTML = GET_DATA.monitor_table_buy_bill;
    //     document.getElementById('20').innerHTML = GET_DATA.monitor_table_sell_bill;
    //     document.getElementById('21').innerHTML = GET_DATA.monitor_table_net_bill;
    //     document.getElementById('22').innerHTML = GET_DATA.monitor_table_saving;

    // }

    function simulation_chart_01(ABC)
    {
        //parse to get all json data
        var GET_CHART_DATA = JSON.parse(ABC);

        //define all needed data array
        var chart_info=["container01", "simulation", " ","time","price(TWD)","power(kW)"];
        // var chart_info=["container01", "電價對照圖", "模擬值(simulation)","時間(區間)","電價(TWD)","功率(kW)"];
        var chart_series_type= [];
        var chart_series_name= [];
        var chart_series_data= [];
        var chart_series_stack= [];
        var chart_series_yAxis= [];
        console.log(GET_CHART_DATA);

        /*DATA SET*/
        // set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // // set_series_function(0,"spline",GET_CHART_DATA.sche_Hydro_cost,"sche_Hydro_cost",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"column",GET_CHART_DATA.sche_loadpower_sum,"load_sum",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_buygrid,"sche_buygrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.solar_fake,"solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_battery,"sche_battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_FC,"sche_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_sellgrid,"sche_sellgrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);


        set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_Hydro_cost,"sche_Hydro_cost",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"column",GET_CHART_DATA.sche_loadpower_sum,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.sche_buygrid,"pwr-buy",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.solar_fake,"pwr-solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.sche_battery,"pwr-battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.sche_FC,"pwr-FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.sche_sellgrid,"pwr-sell",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);


        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

        /*Show chart*/
        show_chart(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
            
    }
    function simulation_chart_02(ABC)
    {
        //parse to get all json data
        var GET_CHART_DATA = JSON.parse(ABC);

        //define all needed data array
        var chart_info=["container02", "control_status", " ", "time", "price", "power(kW)"];
        // var chart_info=["container02", "SOC對照圖", "模擬值(simulation)","時間(區間)","SOC","功率(kW)"];
        var chart_series_type= [];
        var chart_series_name= [];
        var chart_series_data= [];
        var chart_series_stack= [];
        var chart_series_yAxis= [];

        /*DATA SET*/
        // set_series_function(0,"spline",GET_CHART_DATA.sche_SOC,"SOC",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"column",GET_CHART_DATA.sche_loadpower_sum,"load_sum",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_buygrid,"sche_buygrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.solar_fake,"solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_battery,"sche_battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_FC,"sche_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_sellgrid,"sche_sellgrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);


        // set_series_function(0,"spline",GET_CHART_DATA.sche_SOC,"SOC",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load-1",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"column",GET_CHART_DATA.sche_loadpower_sum,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_buygrid,"pwr-buy",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.solar_fake,"pwr-solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_battery,"pwr-battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_FC,"pwr-FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.sche_sellgrid,"pwr-sell",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);


        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

        /*Show chart*/
        show_chart_control_status(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
            
    }    



    function monitor_chart_01(ABC)
    {
        //parse to get all json data
        var GET_CHART_DATA = JSON.parse(ABC);

        //define all needed data array
        var chart_info=["container03", "experiment", " ","time","price(TWD)","power(kW)"];
        // var chart_info=["container03", "電價對照圖", "實際值(realvalue)","時間(區間)","電價(TWD)","功率(kW)"];
        var chart_series_type= [];
        var chart_series_name= [];
        var chart_series_data= [];
        var chart_series_stack= [];
        var chart_series_yAxis= [];

        /*DATA SET*/
        // set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // // set_series_function(0,"spline",GET_CHART_DATA.sche_Hydro_cost,"sche_Hydro_cost",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"column",GET_CHART_DATA.monitor_Pload,"monitor_load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Prect,"monitor_buygrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Psolar,"monitor_solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Pbat,"monitor_battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Pfc,"monitor_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Psell,"monitor_sellgrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        

        set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_Hydro_cost,"sche_Hydro_cost",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"column",GET_CHART_DATA.monitor_Pload,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Prect,"pwr-buy",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Psolar,"pwr-solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Pbat,"pwr-battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Pfc,"pwr-FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Psell,"pwr-sell",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);



        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

        /*Show chart*/
        show_chart(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
            
    }  

    function monitor_chart_02(ABC)
    {
        //parse to get all json data
        var GET_CHART_DATA = JSON.parse(ABC);

        //define all needed data array
        var chart_info=["container04", "experiment", " ","time","SOC","power(kW)"];
        // var chart_info=["container04", "SOC對照圖", "實際值(realvalue)","時間(區間)","SOC","功率(kW)"];
        var chart_series_type= [];
        var chart_series_name= [];
        var chart_series_data= [];
        var chart_series_stack= [];
        var chart_series_yAxis= [];

        /*DATA SET*/
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_SOC,"SOC",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"column",GET_CHART_DATA.monitor_Pload,"monitor_load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Prect,"monitor_buygrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Psolar,"monitor_solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Pbat,"monitor_battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.monitor_Pfc,"monitor_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Psell,"monitor_sellgrid",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

        set_series_function(0,"spline",GET_CHART_DATA.monitor_SOC,"SOC",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        // set_series_function(0,"spline",GET_CHART_DATA.sche_Hydro_cost,"sche_Hydro_cost",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"column",GET_CHART_DATA.monitor_Pload,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Prect,"pwr-buy",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Psolar,"pwr-solar",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Pbat,"pwr-battery",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"spline",GET_CHART_DATA.monitor_Pfc,"pwr-FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
        set_series_function(0,"areaspline",GET_CHART_DATA.monitor_Psell,"pwr-sell",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);


        //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

        /*Show chart*/
        show_chart(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
      }

    // function fc_price_chart(ABC)
    // {
    //     //parse to get all json data
    //     var GET_CHART_DATA = JSON.parse(ABC);

    //     //define all needed data array
    //     var chart_info=["container05", "每千瓦之燃料電池花費與時間電價比較圖", "模擬與實際值","時間(區間)","電價(TWD)","功率(kW)"];
    //     var chart_series_type= [];
    //     var chart_series_name= [];
    //     var chart_series_data= [];
    //     var chart_series_stack= [];
    //     var chart_series_yAxis= [];

    //     /*DATA SET*/
    //     set_series_function(0,"line",GET_CHART_DATA.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    //     set_series_function(0,"line",GET_CHART_DATA.sche_FC_price,"sche_FC_price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    //     set_series_function(0,"line",GET_CHART_DATA.monitor_FC_price,"monitor_FC_price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    //     set_series_function(0,"spline",GET_CHART_DATA.sche_FC,"sche_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    //     set_series_function(0,"spline",GET_CHART_DATA.monitor_Pfc,"monitor_FC",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

    //     //set_series_function(1,"column",GET_CHART_DATA.sche_loadpower,"load",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

    //     /*Show chart*/
    //     show_chart_1(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    //   }