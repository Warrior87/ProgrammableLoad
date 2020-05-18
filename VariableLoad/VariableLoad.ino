/*TO DO*/

#include <EEPROM.h>


byte cal_Mode_Pin = 12;
byte LPF_Cal_Pin = 0;
byte batt1_Pin = 1;
byte ch1CurrentSetPin = 3;

double batt1Voltage;
double LPF_Cal_Voltage;
boolean calModeEN;
int prev_ch1CurrentSetVal;
int ch1CurrentSetVal;
byte ch1DualSelect;

byte ch1CurrentSetValAddress = 0;                             /*EEPROM Addresses*/
byte ch1DualSelectAddress = 2;


void setup() {
  Serial.begin(115200);

  pinMode(cal_Mode_Pin, INPUT);
  pinMode(ch1CurrentSetPin, OUTPUT);

  getConfigData();
}

void loop() {

  batt1Voltage = analogRead(batt1_Pin);
  batt1Voltage = (batt1Voltage * 5.0) / 1023.0;                                       /*convert batt1Voltage from ADC value to Voltage*/
  
  if(calModeEN)                                                                       /*if calibration mode is enabled, get the calibration pin value*/
  {
    LPF_Cal_Voltage = analogRead(LPF_Cal_Pin);
  }
  
  if(ch1CurrentSetVal != prev_ch1CurrentSetVal)                                       /*if channel 1 current set value is different, write the value*/
  {
    analogWrite(ch1CurrentSetPin, ch1CurrentSetVal);
    prev_ch1CurrentSetVal = ch1CurrentSetVal;
  }
  
  if(Serial.available() > 0){                                                         /*if there is data from the serial monitor, recieve it*/
    getData();
    computeSetValues();
  }
  else                                                                                /*otherwise, send the current data*/
  {
    sendData();    
  }
  delay(100);
}

void computeSetValues(){
  if(ch1DualSelect){
    ch1CurrentSetVal = (ch1CurrentSetVal / 1000.0 + 0.008505) / (2.0 * 0.017686);     /*ch1CurrentSetVal is in mA, convert to PWM int value*/
  }
  else{
    ch1CurrentSetVal = (ch1CurrentSetVal / 1000.0 + 0.008505) / (0.017686);           /*ch1CurrentSetVal is in mA, convert to PWM int value for single channel (2x voltage)*/
  }
  storeConfigData();                                                                /*if new setpoints are recieved, store them in EEPROM*/
}

void sendData(){
    Serial.print(batt1Voltage, 2); Serial.print(":"); 
    Serial.print(((LPF_Cal_Voltage * 5.0) / 1023.0),4); Serial.print(":");                /*print the feedback ADC Value as a voltage*/
    Serial.print(ch1DualSelect); Serial.print(":");
    Serial.println(ch1CurrentSetVal);
}

void getData(){      
    //the incoming data is formatted:     
    String dataString = Serial.readStringUntil(':');                                  /*parse the string until the seperator '&' is found*/
    prev_ch1CurrentSetVal = ch1CurrentSetVal;
    ch1CurrentSetVal = dataString.toInt();                                            /*convert that string into an integar*/
    dataString = Serial.readStringUntil(':');
    ch1DualSelect = dataString.toInt();    
}

void storeConfigData(){
    EEPROM.put(ch1CurrentSetValAddress, ch1CurrentSetVal);                            /*store values in EEPROM*/
    EEPROM.put(ch1DualSelectAddress, ch1DualSelect);
}

void getConfigData(){
    EEPROM.get(ch1CurrentSetValAddress, ch1CurrentSetVal);
    EEPROM.get(ch1DualSelectAddress, ch1DualSelect);
}
