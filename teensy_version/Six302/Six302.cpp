#include "Six302.h"

CommManager::CommManager(int sp, int rp) {
  incoming_count = 0; //set these to 0
  outgoing_count = 0;
  csv = false;
  step_period = sp;
  report_period = rp;
  report_count = 0;
  report_num_iter = rp/sp;
  handshake = false;
  client_connected = false;
  connection_status = IDLE;
  overhead_meas = 0;
  in_message_index=0;
}

bool CommManager::initialize(){
  Serial.begin(115200);
}


bool CommManager::addSlider(char* name,float low,float high,float step_size,bool toggle, float* linker){
  if(strchr(name,'&')!=NULL){
    if(VERBOSE)Serial.println("Name of GUI Entity cannot contain & symbol!");
    return false;
  }
  int toggle_int = toggle?1:0;
  int n = sprintf(build_strings[incoming_count+outgoing_count],"S&%s&%.3g&%.3g&%.3g&%d",name,low,high,step_size,toggle_int);
  if(VERBOSE)Serial.print(" Buildstring for Slider: "); Serial.print(n); Serial.println(" characters long");
  incoming_data[incoming_count] = linker; //remember linked-to-variable
  incoming_count ++; //incrememnt the known amount of outgoing data
  return true;
}

bool CommManager::addPlot(char* name,float v_low,float v_high,int steps_displayed, float* linker, int plots){
  if(strchr(name,'&')!=NULL){
    if(VERBOSE)Serial.println("Name of GUI Entity cannot contain & symbol!");
    return false;
  }
  int n = sprintf(build_strings[incoming_count+outgoing_count],"P&%s&%d&%.3g&%.3g&%d",name,plots,v_low,v_high,steps_displayed);
  if(VERBOSE)Serial.print(" Buildstring for Plot: "); Serial.print(n); Serial.println(" characters long");
  outgoing_data[outgoing_count] = linker; //store and remember address to tied variable
  outgoing_size[outgoing_count] = plots; //store and remember if single/multiple plot
  outgoing_count ++;
  return true;
}

bool CommManager::addToggle(char* name, float* linker){
  if(strchr(name,'&')!=NULL){
    if(VERBOSE)Serial.println("Name of GUI Entity cannot contain & symbol!");
    return false;
  }
  int n = sprintf(build_strings[incoming_count+outgoing_count],"T&%s",name);
  if(VERBOSE)Serial.print(" Buildstring for Toggle: "); Serial.print(n); Serial.println(" characters long");
  incoming_data[incoming_count] = linker; //remember linked-to-variable
  incoming_count ++; //incrememnt the known amount of outgoing data 
  return true;
}

/*Checks if CSV has been added yet...if not it adds to build string
 */
bool CommManager::addCSV(){
  if (!csv){
    csv=true;
  } 
}

/*Calling this function: 
 (1) Checks to see if there are any GUI-originating commands, and extracts them, updating variables embedded in main code
 (2) Updates the reporting counter and if needed, transmits the data currently existing in the system variables
 */
int CommManager::overhead(){
  return (int)overhead_meas;
  
}
 
bool CommManager::step(){
  if (client_connected && handshake && in_message_index>0) { // if there's bytes to read from the client (but only if it ,
    String data = in_message[in_message_index];
    if (connection_status ==RUNNING && data.substring(0,1)=="U"){
      int current_index=0;
      for (int count=0; count<incoming_count;count++){
        int new_index = data.indexOf(",",current_index+1);
        String raw;
        if (new_index==-1) raw = data.substring(current_index+1);
        else raw = data.substring(current_index+1,new_index);
        *incoming_data[count] = raw.toFloat(); //convert to float and assign to appropriate user variable
        current_index = new_index; 
      }
    }else{
      if(VERBOSE)Serial.println("Ignoring set");
    }
  }
  if(report_count >= report_num_iter){ //time to check connection and/or report data
    //if(VERBOSE)Serial.println(overhead());
    report_count =0; //reset
    //if (VERBOSE) Serial.print("Connection status:" ); Serial.println(connection_status);
    switch (connection_status){
      case IDLE:
        client = server.available();
        if (client){ //found a new one...reset everything
          client_connected = false; //reset
          handshake = false; //reset
          connection_status = NEW_CLIENT;
        }
        break;
      case NEW_CLIENT:
        if (client.connected()){
          client_connected = true;
          connection_status = CONNECTED;
        }else{
          connection_status = IDLE; //failure
        }
        break;
      case CONNECTED:
        if (webSocketServer.handshake(client)){
          handshake = true;
          connection_status = BUILDING;
          build_iterator = -1;
        } else{
          connection_status = IDLE; //failure
        }
        break;
      case BUILDING:
        if (build_iterator==-1){
          Serial.write("BUILDING");
        }else if (build_iterator == incoming_count+outgoing_count){
          // Will remove CSV choice for now...just hardcode it
          //if (csv) webSocketServer.sendData("END~CSV"); //end build but tack on csv option
//        //else 
          Serial.write("END"); //end build with no csv option
          connection_status = UPDATING; //we're done and into running mode.
        }else{
          webSocketServer.sendData(build_strings[build_iterator]); //send each build string separately
          if (VERBOSE)Serial.println(build_strings[build_iterator]);
        }
        build_iterator++;
        break;
      case UPDATING:
        if (client.connected()){ //check if we're still connected!
          sprintf(data_to_send,"I&[");
          for (int i = 0; i<incoming_count; i++){
            sprintf(data_to_send+strlen(data_to_send),"%.2f%s",*(incoming_data[i]),i<incoming_count-1?",":"");
          }
          sprintf(data_to_send+strlen(data_to_send),"]");
          Serial.write(data_to_send);
          if(VERBOSE)Serial.println(data_to_send);
          connection_status = RUNNING;
        }
        break;
      case RUNNING:
        if (client.connected()){ //check if we're still connected!
          sprintf(data_to_send,"O&[");
          for (int i=0;i<outgoing_count; i++){
            //sprintf(data_to_send+strlen(data_to_send),"[");
            for (int j=0;j<outgoing_size[i];j++){
              sprintf(data_to_send+strlen(data_to_send),"%.2f%s",*(outgoing_data[i]+j),j<outgoing_size[i]-1?",":"");
            }
            sprintf(data_to_send+strlen(data_to_send),"%s",i<outgoing_count-1?",":"");
          }
          sprintf(data_to_send+strlen(data_to_send),"]");
          if (VERBOSE) Serial.print("PRE SEND");
          Serial.write(data_to_send);
          //if (VERBOSE) Serial.println("POST SEND");
        }else{ //lost our connection
          client_connected = false; //done
          handshake = false; //done
          connection_status = IDLE;
        }
        break;
    }
  }else{
    report_count++;
  
  }
  overhead_meas = step_period-(micros()-timeo);
  Serial.println(overhead_meas);
  while (micros()-timeo<step_period);
  timeo = micros(); //update time
}


void serialEvent() {
  in_message[in_message_index] = "";
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    in_message[in_message_index] += inChar;
    if (inChar == '\n') {
      in_message_index++;
      return;
    }
  }
}

