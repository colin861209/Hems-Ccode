//get time block
var dt = new Date();
var now_t= Math.floor(dt.getHours()*4+dt.getMinutes()/15);
console.log(now_t);

var request = new XMLHttpRequest();
    request.open('GET', 'back_end/oneTimeInterruptVsGrid.php');
    request.onload = function () {
        // var ourData = JSON.parse(request.responseText);
        var LOOP_NUM = JSON.parse(this.responseText).load_power.length;
        var i=0;
        console.log(LOOP_NUM);
        InterruptVsGrid(this.responseText);
        muti_divs(this.responseText);
        for(i=0;i<LOOP_NUM;i++)
        { each_load(this.responseText,i); } 
    }
request.send();

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

function set_each_load_function(multi,series_type,DATA,ID,stack_class,yAxis_locate,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis)
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
                chart_series_name.push(ID[Object.keys(ID)[i]]);   
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

function show_each_load(chart_info,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis,chart_upband,chart_lowband)
{
    //set all series data
    var series_data = [];
    len = Object.keys(chart_series_name).length;
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
    plotBands: [{
    color: 'pink', // Color value
    from: chart_upband, // Start of the plot band
    to: chart_lowband // End of the plot band
    }],
    max: 95,    
    title: {text: chart_info[3]},
    categories: []
    },
    yAxis: [{
    min: 0,
    title: {
    text: chart_info[4]
    }
    }, {
    // min: -4,
    // max: 4,   
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

function InterruptVsGrid(ABC)
{
    //parse to get all json data
    var data = JSON.parse(ABC);
    //define all needed data array
    var chart_info=["InterruptVsGrid", "Interrupt Vs Grid", " ", "time", "price(TWD)", "power(kW)"];
    var chart_series_type= [];
    var chart_series_name= [];
    var chart_series_data= [];
    var chart_series_stack= [];
    var chart_series_yAxis= [];

    set_series_function(0,"line",data.electric_price,"price",0,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    set_series_function(0,"line",data.limit_capability,"limit-power",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    set_series_function(1,"column",data.load_power,"load-1",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    // set_series_function(0,"spline",data.load_power[1],"load-2",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);
    // set_series_function(0,"spline",data.load_power[2],"load-3",1,chart_series_type,chart_series_name,chart_series_data,chart_series_stack,chart_series_yAxis);

    /*Show chart*/
    show_chart(chart_info, chart_series_type, chart_series_name, chart_series_data, chart_series_stack, chart_series_yAxis);
        
}

function each_load(ABC,num)
{
    //parse to get all json data
    console.log("each_load"+num)
    var GET_CHART_DATA = JSON.parse(ABC);
    var this_load=GET_CHART_DATA.load_power;
    var this_ID=GET_CHART_DATA.load_num;
    var this_name=GET_CHART_DATA.equip_name;
    var this_s_time=GET_CHART_DATA.start_time;
    var this_e_time=GET_CHART_DATA.end_time;
    var start = this_s_time[num]*4;
    var end = this_e_time[num]*4-1;
    
    //define all needed data array
    var chart_info=["con_"+num, this_name[num],"模擬值(simulation)","時間(區間)","電價(TWD)","功率(kW)"];
    // console.log(chart_info)
    var chart_series_type= [];
    var chart_series_name= [];
    var chart_series_data= [];
    var chart_series_stack= [];
    var chart_series_yAxis= [];

    /*DATA SET*/
    set_each_load_function(0, "line", GET_CHART_DATA.electric_price, null, "price", 0, chart_series_type, chart_series_name, chart_series_data, chart_series_stack, chart_series_yAxis);
    set_each_load_function(0, "column", this_load[num], this_ID[num], this_ID[num], 1, chart_series_type, chart_series_name, chart_series_data, chart_series_stack, chart_series_yAxis);
    /*Show chart*/
    show_each_load(chart_info, chart_series_type, chart_series_name, chart_series_data, chart_series_stack, chart_series_yAxis, start, end);
        
}

function muti_divs(ABC)
{
    var GET_LOAD_DATA = JSON.parse(ABC);
    console.log(GET_LOAD_DATA);
    var LOAD_NUM = GET_LOAD_DATA.load_power.length;
    var i;
    var htmlElements = "";
    for ( i = 0; i < LOAD_NUM; i++) 
    {
        htmlElements += '<div id=con_'+i+' style="min-width: 310px; height: 420px; margin: 0 auto"> </div>';
    }   
    // console.log(htmlElements);
    var container = document.getElementById("containers");
    container.innerHTML = htmlElements;
}
