/*
  TOGGLE
	title
	names
  Updated v 1.1
*/
function Toggle(title,names=["OFF","ON"]){
    var item = new Item(title);
    item.setType("TOGGLE");
    var div_id = item.div_id;
    var unique = item.unique;
    var holder = item.container;
    var names = names; //should be 2-long array of values for when switch is low or high
    var val; //holds toggle value right now at any given moment
    var built = false;
    var title_disp; //title of module
    var slider; //div containing slider
    var holder; //div containing title, value, and slider
    var slider_input; //actual "checkbox"
    var label; //needed for css rendering for=divid of slider_input
    var value_div;  //value displayed in module
    var setup = function(){
      item.setSize(120,100);
      value_div = document.createElement('div');
      value_div.setAttribute("id",div_id+unique+"_value");
      value_div.innerHTML = names[0];
      value_div.setAttribute("class","toggle_value");
      slider = document.createElement('div');
      slider.setAttribute("class","ckbx-style-8");
      slider_input = document.createElement('input');
      slider_input.setAttribute("type","checkbox");
      slider_input.setAttribute("name",div_id+unique+"_checkbox");
      slider_input.setAttribute("id",div_id+unique+"_checkbox");
      slider_input.setAttribute("value",1);
      slider_input.setAttribute("name", div_id+unique+"_checkbox");
      label = document.createElement("label");
      label.setAttribute("for",div_id+unique+"_checkbox");
      holder.setAttribute("class", "toggle");
      holder.appendChild(value_div);
      holder.appendChild(slider);
      slider.appendChild(slider_input);
      slider.appendChild(label);
      built = true;

    }
    item.update = function(value) {
      if(value) slider_input.checked = true;
      else slider_input.checked = false;
      value_div.innerHTML = names[slider_input.checked?1:0];
    }
    this.update = function(value){
      if(value) slider_input.checked = true;
      else slider_input.checked = false;
      value_div.innerHTML = names[slider_input.checked?1:0];
    }
    item.get = function() {
      return slider_input.checked?1:0;
    }
    this.value = function(){
        return slider_input.checked?1:0;
    }
    setup();
    var checko = function(element){
      if (slider_input.checked){
      }else{
      }
      value_div.innerHTML = names[slider_input.checked?1:0];
      document.dispatchEvent(ui_change);
      item.logCall("update");
      item.log(LOG.DATA,names[slider_input.checked?1:0]);

      item.action(slider_input.checked?1:0);
    }
    slider_input.addEventListener('change', checko, false);

};
