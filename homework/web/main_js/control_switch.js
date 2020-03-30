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
                    generate_switch(this.responseText);
                }
            };
            xmlhttp.open("GET","/new/back_end/control_switch.php",true);
            xmlhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded;");
            xmlhttp.send();
        }
    }
    


    function generate_switch(ABC)
    {
        var GET_LOAD_DATA = JSON.parse(ABC);

        var interrupt_num =Number(GET_LOAD_DATA.interrupt_num);
        var uninterrupt_num =Number(GET_LOAD_DATA.uninterrupt_num);
        var varying_num =Number(GET_LOAD_DATA.varying_num);
        var non_num =Number(GET_LOAD_DATA.non_num);
        var load_num =Number(GET_LOAD_DATA.load_num);
        var equip_num =GET_LOAD_DATA.equip_num;
        var equip_name =GET_LOAD_DATA.equip_name;
        var equip_status =GET_LOAD_DATA.equip_status;
        var non_equip_num =GET_LOAD_DATA.non_equip_num;
        var non_equip_name =GET_LOAD_DATA.non_equip_name;
        var non_equip_status =GET_LOAD_DATA.non_equip_status;


        console.log(equip_status);
  
        var i;
        var htmlElements = "";
        var alert_char="";

        for ( i = 0; i < load_num; i++) 
        {
        //class
        if(i<interrupt_num)
        {var form_div =document.getElementById("interrupt");alert_char="alert-warning";}
        else if(i>=interrupt_num && i<interrupt_num+uninterrupt_num)
        {var form_div =document.getElementById("uninterrupt");alert_char="alert-info"}
        else if(i>=interrupt_num+uninterrupt_num && i<interrupt_num+uninterrupt_num+varying_num)
        {var form_div =document.getElementById("varying");alert_char="alert-danger";}
        else
        {var form_div =document.getElementById("non");alert_char="alert-success";}

        //outside(block)
        var form_div_block =document.createElement("DIV");
        form_div_block.className="col-md-12 col-sm-12 col-xs-12 text-center alert "+alert_char;

        //head(number)
        var form_div_head =document.createElement("DIV");
        form_div_head.className="col-md-12 col-sm-12 col-xs-12 text-center";
        var form_h_number =document.createElement("H5");
        form_h_number.className=alert_char;
        if(i<load_num-non_num)
        {var number_text = document.createTextNode("NO. "+equip_num[Object.keys(equip_num)[i]]); }
        else
        {var number_text = document.createTextNode("NO. "+non_equip_num[Object.keys(non_equip_num)[i-(load_num-non_num)]]);}
        form_h_number.appendChild(number_text);
        form_div_head.appendChild(form_h_number);
        form_div_block.appendChild(form_div_head);
        form_div.appendChild(form_div_block);

        //content
        var form_div_content =document.createElement("DIV");
        form_div_content.className="col-md-12 col-sm-12 col-xs-12 text-center align-items-center justify-content-center";
        //name
        var form_div_name =document.createElement("DIV");
        form_div_name.className="col-md-6 col-sm-6 col-xs-12 align-self-center";
        var form_h_name =document.createElement("H3");
        form_h_name.className="align-middle"+alert_char;
        if(i<load_num-non_num)
        {var name_text = document.createTextNode(equip_name[Object.keys(equip_name)[i]]); }
        else
        {var name_text = document.createTextNode(non_equip_name[Object.keys(non_equip_name)[i-(load_num-non_num)]]);}
        form_h_name.appendChild(name_text);
        form_div_name.appendChild(form_h_name);
        form_div_content.appendChild(form_div_name);

        //toggle
        var form_div_toggle =document.createElement("DIV");
        form_div_toggle.className="col-md-6 col-sm-6 col-xs-12 align-self-center";
        var form_input_toggle =document.createElement("INPUT");
        //toggle attribute set
        form_input_toggle.setAttribute("type","checkbox");
        if(i<load_num-non_num)
        {form_input_toggle.setAttribute("name","load_"+equip_num[Object.keys(equip_num)[i]]);}
        else
        {form_input_toggle.setAttribute("name","load_"+non_equip_num[Object.keys(non_equip_num)[i-(load_num-non_num)]]);}
        form_input_toggle.setAttribute("data-toggle","toggle");
        form_input_toggle.setAttribute("data-onstyle","success");
        form_input_toggle.setAttribute("data-offstyle","danger");
        form_input_toggle.setAttribute("data-size","large");


        if(i<load_num-non_num)
        {
            if(equip_status[Object.keys(equip_status)[i]]==1){form_input_toggle.checked=true;}
            else{form_input_toggle.checked=false;}
        }
        else
        {
            if(non_equip_status[Object.keys(non_equip_status)[i-(load_num-non_num)]]==1){form_input_toggle.checked=true;}
            else{form_input_toggle.checked=false;}
        }
      
        form_div_toggle.appendChild(form_input_toggle);
        form_div_content.appendChild(form_div_toggle);
        //name + toggle appendchild set
        form_div_block.appendChild(form_div_content);
        form_div.appendChild(form_div_block);
        }

        //call bootstrapToggle function for style
        $(function() {
            $('[type="checkbox"]').bootstrapToggle('destroy');
            $('[type="checkbox"]').bootstrapToggle();
         })
       


    }