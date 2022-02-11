/* =========================================================
 * ====                   WARNING                        ===
 * =========================================================
 * The code in this tab has been generated from the GUI form
 * designer and care should be taken when editing this file.
 * Only add/edit code inside the event handlers i.e. only
 * use lines between the matching comment tags. e.g.

 void myBtnEvents(GButton button) { //_CODE_:button1:12356:
     // It is safe to enter your event code here  
 } //_CODE_:button1:12356:
 
 * Do not rename this tab!
 * =========================================================
 */


public void auto_mode(GButton source, GEvent event) { //_CODE_:AUTO:585811:
  if (source == AUTO && event == GEvent.CLICKED) {
    mode_label.setText("AUTONOMOUS MODE");
    myPort.write("$DALSAIL,001,0*5f\r\n");
      // Wait for the ack to arrive
    if (!waitAck()) {
      // Error, no acks could be found
      return;
    }
  }
} //_CODE_:AUTO:585811:

public void imgButton1_click1(GImageButton source, GEvent event) { //_CODE_:imgButton1:622628:
  println("imgButton1 - GImageButton >> GEvent." + event + " @ " + millis());
} //_CODE_:imgButton1:622628:




// Create all the GUI controls. 
// autogenerated do not edit
public void createGUI(){
  G4P.messagesEnabled(false);
  G4P.setGlobalColorScheme(GCScheme.BLUE_SCHEME);
  G4P.setCursor(ARROW);
  surface.setTitle("Sailboat Data");
  togGroup1 = new GToggleGroup();
  togGroup2 = new GToggleGroup();
  LOAD = new GButton(this, 363, 228, 100, 30);
  LOAD.setText("LOAD AND SEND WP");
  LOAD.setTextBold();
  LOAD.setLocalColorScheme(GCScheme.YELLOW_SCHEME);
  LOAD.addEventHandler(this, "load_data");
  AUTO = new GButton(this, 28, 174, 100, 30);
  AUTO.setText("AUTONOMOUS");
  AUTO.setTextBold();
  AUTO.setLocalColorScheme(GCScheme.RED_SCHEME);
  AUTO.addEventHandler(this, "auto_mode");
  north = new GLabel(this, 200, 150, 80, 20);
  north.setText("N");
  north.setTextBold();
  north.setOpaque(false);
  west = new GLabel(this, 134, 212, 80, 20);
  west.setText("W");
  west.setTextBold();
  west.setOpaque(false);
  south = new GLabel(this, 200, 275, 80, 20);
  south.setText("S");
  south.setTextBold();
  south.setOpaque(false);
  east = new GLabel(this, 265, 212, 80, 20);
  east.setText("E");
  east.setTextBold();
  east.setOpaque(false);
  eep_label = new GLabel(this, 330, 142, 147, 55);
  eep_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  eep_label.setOpaque(false);
  mode_label = new GLabel(this, 3, 61, 149, 56);
  mode_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  mode_label.setOpaque(false);
  imgButton1 = new GImageButton(this, 186, 167, 110, 110, new String[] { "circle.png", "circle.png", "circle.png" } );
  imgButton1.addEventHandler(this, "imgButton1_click1");
  WIND_label = new GLabel(this, 179, 55, 40, 30);
  WIND_label.setTextAlign(GAlign.LEFT, GAlign.MIDDLE);
  WIND_label.setText("WIND: ");
  WIND_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  WIND_label.setOpaque(false);
  GPS_label = new GLabel(this, 179, 14, 36, 30);
  GPS_label.setTextAlign(GAlign.LEFT, GAlign.MIDDLE);
  GPS_label.setText("GPS: ");
  GPS_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  GPS_label.setOpaque(false);
  COMP_label = new GLabel(this, 175, 95, 49, 30);
  COMP_label.setTextAlign(GAlign.LEFT, GAlign.MIDDLE);
  COMP_label.setText("COMP: ");
  COMP_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  COMP_label.setOpaque(false);
  GPS_DATA = new GLabel(this, 230, 10, 239, 32);
  GPS_DATA.setOpaque(false);
  Wind_DAta = new GLabel(this, 231, 48, 242, 36);
  Wind_DAta.setOpaque(false);
  Comp_DATa = new GLabel(this, 231, 92, 240, 35);
  Comp_DATa.setOpaque(false);
  distance_label = new GLabel(this, 32, 220, 80, 97);
  distance_label.setLocalColorScheme(GCScheme.RED_SCHEME);
  distance_label.setOpaque(false);
  togGroup1 = new GToggleGroup();
  period_slider = new GCustomSlider(this, 190, 384, 100, 45, "blue18px");
  period_slider.setShowValue(true);
  period_slider.setShowLimits(true);
  period_slider.setLimits(30, 0, 60);
  period_slider.setNumberFormat(G4P.INTEGER, 0);
  period_slider.setLocalColorScheme(GCScheme.RED_SCHEME);
  period_slider.setOpaque(false);
  period_slider.addEventHandler(this, "slider_set");
  period = new GLabel(this, 201, 363, 80, 20);
  period.setText("PERIOD");
  period.setLocalColorScheme(GCScheme.RED_SCHEME);
  period.setOpaque(false);
  test_button = new GButton(this, 200, 322, 80, 30);
  test_button.setText("STATE");
  test_button.setTextBold();
  test_button.addEventHandler(this, "test_decision");
  label1 = new GLabel(this, 367, 402, 80, 20);
  label1.setText("PITCH/ROLL");
  label1.setOpaque(false);
  com_port = new GDropList(this, 27, 16, 90, 220, 10);
  com_port.setItems(com, 0);
  com_port.addEventHandler(this, "com_select");
  log_data = new GButton(this, 324, 266, 80, 30);
  log_data.setText("BEGIN DATA LOGGING");
  log_data.setTextBold();
  log_data.setLocalColorScheme(GCScheme.CYAN_SCHEME);
  log_data.addEventHandler(this, "log_data_event");
  end_log = new GButton(this, 411, 266, 80, 30);
  end_log.setText("END DATA LOGGING");
  end_log.setTextBold();
  end_log.setLocalColorScheme(GCScheme.CYAN_SCHEME);
  end_log.addEventHandler(this, "stop_data");
  julia = new GLabel(this, 13, 411, 109, 36);
  julia.setText("created by\nJULIA SARTY");
  julia.setTextItalic();
  julia.setLocalColorScheme(GCScheme.PURPLE_SCHEME);
  julia.setOpaque(false);
}

// Variable declarations 
// autogenerated do not edit

GToggleGroup togGroup2; 
GButton LOAD; 
GButton AUTO; 
GLabel north; 
GLabel west; 
GLabel south; 
GLabel east; 
GLabel eep_label; 
GLabel mode_label; 
GImageButton imgButton1; 
GLabel WIND_label; 
GLabel GPS_label; 
GLabel COMP_label; 
GLabel GPS_DATA; 
GLabel Wind_DAta; 
GLabel Comp_DATa; 
GLabel distance_label; 
GToggleGroup togGroup1; 
GCustomSlider period_slider; 
GLabel period; 
GButton test_button; 
GLabel label1; 
GDropList com_port; 
GButton log_data; 
GButton end_log; 
GLabel julia; 