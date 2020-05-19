#include <EEPROM.h>
#include <avr/pgmspace.h>

const double PWMVoltage[] PROGMEM = {0,0.0093,0.0249,0.0464,0.0616,0.0821,0.0963,0.1163,0.1344,0.1491,0.1691,
0.1852,0.2043,0.2209,0.238,0.2605,0.2717,0.2947,0.3084,0.327,0.3436,0.3627,0.3783,0.3959,0.4169,0.4306,0.4531,
0.4658,0.4868,0.5029,0.5196,0.5401,0.5547,0.5753,0.587,0.6129,0.6241,0.6457,0.6628,0.6784,0.6989,0.7141,0.737,
0.7458,0.7727,0.7845,0.8045,0.824,0.8348,0.8597,0.8685,0.8954,0.9037,0.9306,0.9413,0.9629,0.9824,0.9941,1.0205,
1.0293,1.0557,1.065,1.089,1.1031,1.1188,1.1442,1.1491,1.1784,1.1867,1.2155,1.2273,1.2429,1.2678,1.2752,1.3055,
1.3099,1.3372,1.35,1.3685,1.39,1.3998,1.4272,1.433,1.4643,1.4721,1.4936,1.5147,1.5235,1.5513,1.5552,1.5885,
1.5943,1.6188,1.6354,1.6491,1.6745,1.6804,1.7121,1.7151,1.7444,1.7571,1.7727,1.7991,1.806,1.8363,1.8387,1.87,
1.8773,1.9003,1.9208,1.9291,1.9594,1.9619,1.9946,1.9995,2.0274,2.042,2.0538,2.0826,2.087,2.1193,2.1202,2.152,
2.1623,2.1804,2.2043,2.2111,2.2429,2.2434,2.2786,2.283,2.3094,2.3255,2.3358,2.3661,2.368,2.4027,2.4047,2.433,
2.4506,2.4604,2.4897,2.4927,2.5279,2.5298,2.5577,2.5704,2.5875,2.6119,2.6163,2.6505,2.654,2.6804,2.6921,2.7111,
2.7366,2.7424,2.7742,2.7766,2.8065,2.8152,2.8348,2.8583,2.8656,2.8969,2.9013,2.9301,2.9389,2.9585,2.9829,2.9888,
3.02,3.0239,3.0557,3.0621,3.0831,3.1046,3.1149,3.1437,3.1486,3.1794,3.1857,3.2092,3.2234,3.2395,3.2659,3.2722,
3.306,3.3089,3.3358,3.3485,3.3631,3.3886,3.3969,3.4262,3.4321,3.458,3.4697,3.4888,3.5112,3.523,3.5489,3.5552,
3.5846,3.5943,3.6158,3.6315,3.6461,3.6696,3.6804,3.7058,3.7155,3.741,3.7522,3.7713,3.7918,3.8035,3.8299,3.8397,
3.8646,3.8763,3.8964,3.9154,3.9286,3.9531,3.9638,3.9853,4.0005,4.0161,4.0371,4.0508,4.0738,4.0865,4.108,4.1212,
4.1437,4.1608,4.1755,4.196,4.2116,4.2326,4.2463,4.2644,4.2825,4.3011,4.3182,4.3372,4.3553,4.3715,4.3905,4.4062,
4.4272,4.4423,4.4589,4.4809,4.4961};

byte cal_Mode_Pin = 12;
byte LPF_Cal_Pin = 0;
byte batt1_Pin = 1;
byte ch1CurrentSetPin = 3;

double ch1Voltage;
double LPF_Cal_Voltage;
boolean calModeEN;
byte prev_ch1CurrentPWM;
byte ch1CurrentPWM;
double ch1Current;
double ch1ActualCurrent;
double ch1Power;
byte ch1DualSelect;

byte ch1CurrentAddress = 0;                             /*EEPROM Addresses*/
byte ch1DualSelectAddress = 2;


void setup() {
  Serial.begin(115200);

  pinMode(cal_Mode_Pin, INPUT);
  pinMode(ch1CurrentSetPin, OUTPUT);

  getConfigData();
  analogWrite(ch1CurrentSetPin, ch1CurrentPWM);
}

void loop() {

  ch1Voltage = analogRead(batt1_Pin);
  ch1Voltage = (ch1Voltage * 5.0) / 1023.0;                                       /*convert ch1Voltage from ADC value to Voltage*/
  
  if(ch1CurrentPWM != prev_ch1CurrentPWM)                                       /*if channel 1 current set value is different, write the value*/
  {
    analogWrite(ch1CurrentSetPin, ch1CurrentPWM);
    prev_ch1CurrentPWM = ch1CurrentPWM;
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
    ch1CurrentPWM = (ch1Current / 1000.0 + 0.008505) / (2.0 * 0.017686);     /*ch1Current is in mA, convert to PWM int value*/
    ch1Current = PWMVoltage[ch1CurrentPWM];                                  /*lookup filter voltage value*/
    ch1ActualCurrent = ch1Current * 4000;                                    /*calculate actual current set value (0.5 ohm in parallel) (x * 1000 / 0.25)*/
  }
  else{
    ch1CurrentPWM = (ch1Current / 1000.0 + 0.008505) / (0.017686);           /*ch1Current is in mA, convert to PWM int value for single channel (2x voltage)*/
    ch1Current = PWMVoltage[ch1CurrentPWM];                                  /*lookup filter voltage value*/
    ch1ActualCurrent = ch1Current * 2000;                                    /*calculate actual current set value (0.5 ohm in parallel) (x * 1000 / 0.5)*/
  }
  ch1Power = ch1ActualCurrent * ch1Voltage;                                 /*calculate channel power*/
  storeConfigData();                                                                /*if new setpoints are recieved, store them in EEPROM*/
}

void sendData(){
    Serial.print(ch1Voltage, 2); Serial.print(":");     
    Serial.print(ch1ActualCurrent); Serial.print(":");
    Serial.print(ch1Power); Serial.print(":");
    Serial.println(ch1CurrentPWM);
}

void getData(){      
    //the incoming data is formatted:     
    String dataString = Serial.readStringUntil(':');                                  /*parse the string until the seperator '&' is found*/
    ch1Current = dataString.toInt();                                            /*convert that string into an integar*/
    dataString = Serial.readStringUntil(':');
    ch1DualSelect = dataString.toInt();    
}

void storeConfigData(){
    EEPROM.put(ch1CurrentAddress, ch1CurrentPWM);                            /*store values in EEPROM*/
    EEPROM.put(ch1DualSelectAddress, ch1DualSelect);
}

void getConfigData(){
    EEPROM.get(ch1CurrentAddress, ch1CurrentPWM);
    EEPROM.get(ch1DualSelectAddress, ch1DualSelect);
}
