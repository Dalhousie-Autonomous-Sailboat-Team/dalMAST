///***libraries***\\\

import g4p_controls.*;
import processing.serial.*;
import java.lang.*;

///***global variables***\\\

float arrow_control=0;
float boat_control=0;
float wp_control;
float pitch_control=0;
float roll_control=0;
float wind_control = 0;

int k;

float rudder=0.0;
float sail = 0.0;

float heading = 0.0;
float pitch = 0.0;
float roll = 0.0;
float comp_x = 0.0;
float comp_y = 0.0;
float comp_z = 0.0;

File selection;
boolean fileFound = false;
boolean fileCancel = false;
String fileName;

boolean load_mode = false;
boolean log_mode = false;

PrintWriter GPS_log, COMP_log, WIND_log;

///***Custom Gui Controls***//

GButton RC;
GWindow control_mode;
GWindow test_mode;
GCustomSlider deploy_slider;
GCustomSlider sail_slider; 
GCustomSlider rudder_position; 
GLabel sail_slider_label; 
GLabel rudder_slider_label; 
GLabel deploy_label;
GLabel test_label;


///***Serial definitions***\\\

Serial myPort;
String msgString;
Boolean msgAck;
BufferedReader reader;
String portName; 
String com[];

///***GUI setup***\\\

public void setup(){
  size(500, 450, JAVA2D);
  com = Serial.list();
  if (com.length == 0){
    println("Error, no COM ports are available!");
    exit();
  }
  if (com.length == 1){
     portName=com[0];
     println(portName);
     myPort = new Serial(this, portName, 9600);
     myPort.bufferUntil('\n');
  }
  createGUI();
  RC = new GButton(this, 27, 138, 100, 31);
  RC.setText("REMOTE");
  RC.setTextBold();
  RC.setLocalColorScheme(GCScheme.ORANGE_SCHEME);

  msgAck = false;
}

///***GUI Main***\\\

public void draw(){
  background(255);
  rotateBoat(boat_control);
  rotateWP(wp_control);
  rotateArrow(arrow_control);
  //rotateRoll(roll_control);
  rotateWind(wind_control);
  setPitch(pitch_control, roll_control);
  delay(100);
}

///***COM Select***\\\

public void com_select(GDropList source, GEvent event) {
  if(source == com_port && event == GEvent.SELECTED){ 
    portName = com_port.getSelectedText();
    if (myPort != null) {
      myPort.stop();
    }
    myPort = new Serial(this, portName, 9600);
    myPort.bufferUntil('\n'); 
    println(portName);
  }
}
///*** Data Logging ***\\\

public void log_data_event(GButton source, GEvent event) {
  if(source == log_data && event == GEvent.CLICKED){
    log_mode = true;
    log_data.setText("LOGGING...");
    log_data.setTextBold();
    end_log.setText("STOP LOG");
    end_log.setTextBold();
    String gps_name = ("logs/GPS_" + year() + "_" + month() + "_" + day() + "_" + hour() + "_" + minute() + ".txt");
    String comp_name = ("logs/COMP_" + year() + "_" + month() + "_" + day() + "_" +  hour() + "_" + minute() + ".txt");
    String wind_name = ("logs/WIND_" + year() + "_" + month() + "_" + day() + "_" +  hour() + "_" +minute() + ".txt");
    GPS_log = createWriter(gps_name);
    GPS_log.println("LAT, LON");
    COMP_log = createWriter(comp_name);
    COMP_log.println("X, Y, Z, HEADING, PITCH, ROLL");
    WIND_log = createWriter(wind_name);
    WIND_log.println("ANGLE, SPEED");
  }
}

public void stop_data(GButton source, GEvent event) {
 if(source == end_log && event == GEvent.CLICKED){
  log_mode=false;
  log_data.setText("START LOG");
  log_data.setTextBold();
  end_log.setText("DATA LOGGED");
  end_log.setTextBold();
  GPS_log.close();
  WIND_log.close();
  COMP_log.close();
 
 }
}

///***Animations***\\\

void rotateBoat(float boat_control){
  noStroke();
  fill(139,223,255);
  ellipse(404,357,80,80);
  boat_control = boat_control - 90;
  PImage boat;
  boat = loadImage("boat.png");
  boat.resize(80,50);
  translate(240,225);
  rotate(radians(boat_control));
  image(boat,-40,-25);
  rotate(radians(-boat_control));
  translate(-240,-225);
}

void rotateWP(float wp_control){
  wp_control=wp_control-140;
  translate(240,220);
  rotate(radians(wp_control));
  noStroke();
  fill(255,0,0);
  ellipse(40,50,10,10);
  rotate(radians(-wp_control));
  translate(-240,-220);

}
void rotateArrow(float arrow_control){
  arrow_control = arrow_control + 90;
  PImage arrow;
  arrow = loadImage("arrow.png");
  arrow.resize(100,15);
  translate(240,225);
  rotate(radians(arrow_control));
  image(arrow,-50,-8);
  rotate(radians(-arrow_control));
  translate(-240,-225);
}

void rotateWind(float wind_control){
  wind_control=wind_control-140;
  translate(240,220);
  rotate(radians(wind_control));
  strokeWeight(1);
  stroke(0);
  fill(178,178,178);
  ellipse(40,50,9,9);
  rotate(radians(-wind_control));
  translate(-240,-220);
}

void setPitch(float pitch_control, float roll_control){
  float x = map(pitch_control,180,-180,-PI,PI);
  roll_control = radians(roll_control);
  noStroke();
  fill(232,191,138);
  arc(404,357,80,80, -x + roll_control, PI + roll_control + x, CHORD);
}


///***Remote Mode event***\\\

void handleButtonEvents(GButton button, GEvent event) {
  if (button == RC && event == GEvent.CLICKED) {
    mode_label.setText("REMOTE MODE");
    myPort.write("$DALSAIL,001,1*5e\r\n");    
    
    createWindows();
    RC.setEnabled(false);
    windowClosing(control_mode);
  }
}

///*** Deoply/Test event ***\\\

public void test_decision(GButton source, GEvent event) { 
  if (source == test_button && event == GEvent.CLICKED) {
    createWindows1();
    //myPort.write("$DALSAIL,001,1*5e\r\n");
    test_button.setEnabled(false);
    windowClosing(control_mode);
  }
} 

///***Window Creation***\\\

synchronized public void window_draw(PApplet appc, GWinData data) { 
  appc.background(255); 
  appc.fill(0);
} 

void createWindows() {
  println("Making Window");
  control_mode = GWindow.getWindow(this, "Control Mode", 0, 0, 400, 200, JAVA2D);
  control_mode.addDrawHandler(this, "window_draw");
  control_mode.addOnCloseHandler(this, "windowClosing");
  control_mode.setActionOnClose(GWindow.CLOSE_WINDOW);
  sail_slider = new GCustomSlider(control_mode, 27, 52, 140, 52, "blue18px");
  sail_slider.setShowValue(true);
  sail_slider.setShowLimits(true);
  sail_slider.setLimits(0.0, -90.0, 90.0);
  sail_slider.setNumberFormat(G4P.INTEGER, 2);
  sail_slider.setOpaque(false);
  sail_slider.addEventHandler(this, "sail_position_set");
  sail_slider.setEasing(3);
  rudder_position = new GCustomSlider(control_mode, 240, 52, 140, 53, "blue18px");
  rudder_position.setShowValue(true);
  rudder_position.setShowLimits(true);
  rudder_position.setLimits(0.0, -45.0, 45.0);
  rudder_position.setNumberFormat(G4P.INTEGER, 1);
  rudder_position.setOpaque(false);
  rudder_position.addEventHandler(this, "rudder_position_set");
  rudder_position.setEasing(3);
  sail_slider_label = new GLabel(control_mode, 53, 132, 80, 20);
  sail_slider_label.setText("Sail");
  sail_slider_label.setOpaque(false);
  rudder_slider_label = new GLabel(control_mode, 270, 132, 80, 20);
  rudder_slider_label.setText("Rudder");
  rudder_slider_label.setOpaque(false);
}

void createWindows1() {
  test_mode = GWindow.getWindow(this, "Decision Mode", 0, 0, 400, 200, JAVA2D);
  test_mode.addDrawHandler(this, "window_draw");
  test_mode.addOnCloseHandler(this, "windowClosing");
  test_mode.setActionOnClose(GWindow.CLOSE_WINDOW);
  deploy_slider = new GCustomSlider(test_mode, 150, 100, 100, 40, "blue18px");
  deploy_slider.setLimits(0.5, 0.0, 1.0);
  deploy_slider.setStickToTicks(true);
  deploy_slider.setNumberFormat(G4P.DECIMAL, 1);
  deploy_slider.setLocalColorScheme(GCScheme.RED_SCHEME);
  deploy_slider.setOpaque(false);
  deploy_slider.addEventHandler(this, "deploy_test");
  deploy_label = new GLabel(test_mode, 80, 113, 80, 20);
  deploy_label.setText("DEPLOY");
  deploy_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  deploy_label.setOpaque(false);
  test_label = new GLabel(test_mode, 230, 113, 80, 20);
  test_label.setText("TEST");
  test_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  test_label.setOpaque(false);
}

public void windowClosing(GWindow w){
  println("Window closing");
  RC.setEnabled(true);
}

///***Slider Controls***\\\

void rudder_position_set(GCustomSlider customslider, GEvent event) {
  if (customslider == rudder_position && event == GEvent.VALUE_STEADY){
  rudder = rudder_position.getValueI();
  motorControl();
  }
}

void sail_position_set(GCustomSlider customslider, GEvent event) {
  if (customslider == sail_slider && event == GEvent.VALUE_STEADY){
  sail = sail_slider.getValueI();
  motorControl();
  }
}
void deploy_test(GCustomSlider customslider, GEvent event) { 

  float value = deploy_slider.getValueF();
  if(value==1){
    println("$DALSAIL,002,0*5c\r\n");
    myPort.write("$DALSAIL,002,0*5c\r\n");
  }
  if(value==0) {
    println("$DALSAIL,002,1*5c\r\n");
    myPort.write("$DALSAIL,002,1*5d\r\n");
  }
} 
void slider_set(GCustomSlider customslider, GEvent event) { //_CODE_:period_slider:638714:
  if (customslider == period_slider && event == GEvent.VALUE_STEADY){
    int value = period_slider.getValueI();
    int i;
    String message = ("DALSAIL,005,"+(value));
    byte checksum = byte(message.charAt(0));
    for (i=1;i<message.length();i++){
      checksum ^= byte(message.charAt(i));
    }
    String cksm = String.format("%02x",checksum);
    myPort.write("$DALSAIL,005," + value + "*" + cksm + "\r\n");
    println("$DALSAIL,005," + value + "*" + cksm + "\r\n");
    }
}

void motorControl(){
  int i;
  String message = ("DALSAIL,003," + (int)rudder + "," + (int)sail);
  byte checksum = byte(message.charAt(0));
  for (i=1;i<message.length();i++){
      checksum ^= byte(message.charAt(i));
     }
  String cksm = String.format("%02x",checksum);
  myPort.write("$" + message + "*" + cksm + "\r\n"); 
}
///*** EEprom controls***\\\

void load_data(GButton source, GEvent event) {
  String ack = "DEFAULT"; 
  String message;
  String screen[];
  float lat, lon;
  int lati, loni, radi;
  int idx, next;
  int i;
  Boolean found_ack=false;
  
   eep_label.setText("");
  
  if (source == LOAD && event == GEvent.CLICKED) {
    // Let the user select and file and wait for the dialog to be closed
    selectInput("Select a file to process:", "fileSelected");
    // TODO put a timeout here
    while(!fileFound && !fileCancel) {
      println("waiting...");
      delay(500);
    }
    
    if (fileCancel) {
      fileCancel = false;
      return;
    }
    
    fileFound = false;
    
    screen = loadStrings(fileName);
    eep_label.setText("LOADED DATA FILE");
    
    // Send the message to change the mode
    myPort.write("$DALSAIL,001,2*5d\r");
    
    // Wait for the ack to arrive
    if (!waitAck()) {
      // Error, no acks could be found
      return;
    }
    
    int a;
    for(a=0;a<screen.length;a++){
      String data[] = split(screen[a], ',');
      lat = float(data[0])*1000000;
      lati = int(lat);
     // lat = int(lat);
      lon = float(data[1])*1000000;
      loni = int(lon);
     // lon = int(lon);
      radi = int(data[2]);
      next =int(data[3]);
      idx = a;
      //println("" + lat);
      //println("" + lon);
      //println("" + rad);
      //println("" + next);
      
      message = ("DALSAIL,004,"+ idx + "," + lati + "," + loni + "," + radi + "," + next);
      byte checksum = byte(message.charAt(0));
      for (i=1;i<message.length();i++){
        checksum ^= byte(message.charAt(i));
       }
      String cksm = String.format("%02x",checksum);
      myPort.write("$" + message + "*" + cksm + "\r\n");
      // Wait for an ack
      if (!waitAck()) {
        // Error, no acks could be found
        return;
      }
    }
    
    message = "DALSAIL,001,0";
    byte checksum = byte(message.charAt(0));
    for (i=1;i<message.length();i++){
      checksum ^= byte(message.charAt(i));
     }
    String cksm = String.format("%02x",checksum);    
    // Send the message to change the mode
    myPort.write("$" + message + "*" + cksm + "\r\n");
    
    // Wait for the ack to arrive
    if (!waitAck()) {
      // Error, no acks could be found
      return;
    }
    eep_label.setText("DATA UPLOADED");
  }
}


void fileSelected(File selection) {
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
    fileCancel = true;
  } else {
    println("User selected " + selection.getAbsolutePath());
    fileName = selection.getAbsolutePath().replace('\\', '/');
    fileFound = true;
  }
}

///***Sensor Handling***\\\

void readData(String data_read) {
  if (calculate(data_read)) {
    String[] pieces = split(data_read, ',');
    int data=int(pieces[1]);
    println(data);
    
    switch (data) {
      case 0:
        msgAck = (int(pieces[2]) == 0);
        break;
      case 6: 
        GPSsetLabel(data_read);
        break;
      case 7: 
        WINDsetLabel(data_read);
        break;
      case 8: 
        COMPparseData(data_read);
        break;
      case 9: 
        NAVsetAnimation(data_read);
        break;
      default:
        break;
    }
  } else {
    println("Error...");
  }
}

void GPSsetLabel(String data_read){
  String[] pieces = splitTokens(data_read, ",*");
  if(pieces.length != 5){
    myPort.write("Error reading GPS message\r\n");
    return;
  }
  String mode = pieces[1];
  float lat = float(pieces[2]);
  lat = lat/1000000.0;
  float lon = float(pieces[3]);
  lon = lon/1000000.0;
  String cksm = pieces[4];
  GPS_DATA.setText("" + lat + " lat " + "" + lon + " lon\n");
  if(log_mode){
  GPS_log.println(lat + ", " + lon);
  }
  
}

void WINDsetLabel(String data_read){
  String[] pieces = splitTokens(data_read, ",*");
  if(pieces.length != 5){
    myPort.write("Error reading Wind message\r\n");
    return;
  }
  String cmd = pieces[0];
  String mode = pieces[1];
  float angle = float(pieces[2]);
  angle = angle/10.0;
  wind_control = angle;
  float speed = float(pieces[3]);
  speed = speed/10.0;
  int cksm = int(pieces[4]);
  Wind_DAta.setText("" + angle + " deg\n" + "" + speed + " m/s\n");
  if(log_mode){
  WIND_log.println(angle + ", " + speed);
  }

}
void COMPparseData(String data_read){
  String[] pieces = splitTokens(data_read, ",*");
  if(pieces.length != 7){
    myPort.write("Error reading Compass message\r\n");
    return;
  }
  String cmd = pieces[0];
  String mode = pieces[1];
  String cksm = pieces[6];
  int type = int(pieces[2]);
  float temp;
  switch (type){
    case 0: 
      comp_x = float(pieces[3]);
      comp_x = comp_x/10.0;
      comp_y = float(pieces[4]);
      comp_y = comp_y/10.0;
      comp_z = float(pieces[5]);
      comp_z = comp_z/10.0;
      break;
    case 1:
      break;
    case 2: 
      heading = float(pieces[3]);
      heading = heading/10.0;
      pitch = float(pieces[4]);
      pitch = pitch/10.0;
      pitch_control = pitch;
      roll = float(pieces[5]);
      roll = roll/10.0;
      roll_control = roll;
      break;
    case 3: 
      pitch = float(pieces[3]);
      pitch = pitch/10.0;
      pitch_control = pitch;
      roll = float(pieces[4]);
      roll = roll/10.0;
      roll_control = roll;
      temp = float(pieces[5]);
      temp = temp/10.0; 
      break;
    default: 
      break;
  }

  COMPsetLabel();
}

void COMPsetLabel(){
  Comp_DATa.setText(" " + comp_x + " x " + " " + comp_y + " y " + " " + comp_z + " z\n " + " " + heading + " heading " + " " + pitch + " pitch" + " " + roll +" roll\n");
  boat_control=heading;
 if(log_mode){
  COMP_log.println(comp_x + ", " + comp_y + ", " + comp_z  + ", " + heading  + ", " + pitch + ", " + roll);
}
}

void NAVsetAnimation(String data_read){

  String[] pieces = splitTokens(data_read, ",*");
  if(pieces.length != 11){
    myPort.write("Error reading Navigation message\r\n");
    return;
  }
  String cmd = pieces[0];
  String mode = pieces[1];
  float wp_lat = float(pieces[2]);
  wp_lat = wp_lat/1000000.0;
  float wp_lon = float(pieces[3]);
  wp_lon = wp_lon/1000000.0;
  float wp_rad = float(pieces[4]);
  float distance = float(pieces[5]);
  float bearing = float(pieces[6]);
  bearing = bearing/10.0;
  float course = float(pieces[7]);
  course = course/10.0;
  arrow_control = course;
  wp_control=bearing;
  rudder = int(pieces[8]);
  sail = int(pieces[9]);

  String cksm = pieces[10];

  distance_label.setText("Next waypoint: " + wp_lat + " lat" + " " + wp_lon + " lon\n" + "Distance: " + distance + " m");

}

///***Checksum Handling***\\\

boolean calculate(String data_read){
  byte checksum;
  //println(data_read);
  String[] pieces = splitTokens(data_read, "$*\r\n");
  if(pieces.length != 2){
    return false;
  }
  //println(pieces[1]);
  int cksm = unhex(pieces[1]);
  String msg = pieces[0];
  int i;
  checksum = byte(msg.charAt(0));

  for (i=1;i<msg.length();i++){
    checksum ^= byte(msg.charAt(i));
  }
  //println(String.format("%02x",checksum));

  if(checksum == cksm){
    return true;
  } else {
    println("expected " + hex(checksum) +" \n" + "received " + pieces[1]);
    return false;
  }
}

// Serial callback
void serialEvent(Serial p) {
  try {
    msgString = p.readStringUntil('\n'); 
    readData(msgString);
  } catch(RuntimeException e) {
    e.printStackTrace();
  }
} 

Boolean waitAck() {
  int n;
  for (n = 0; n < 10; n++) {
    if (msgAck) {
      // Indicate that the ack was found
      println("Found the ack!!!!!!");
      // Clear the ack flag
      msgAck = false;
      return true;
    }
    delay(100);
  }
  // If we reach this, no acks were found :(
  println("No acks were received :(");
  return false;
}