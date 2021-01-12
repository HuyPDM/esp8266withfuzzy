
#include <Fuzzy.h>
#include <Adafruit_ADS1015.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Fuzzy.h>
#include <FirebaseESP32.h>
#include <WiFi.h>
#include <EEPROM.h>
#include <Wire.h>
#include <string>
#include <Adafruit_ADS1015.h>
#include "DFRobot_ESP_PH_WITH_ADC.h"

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
DFRobot_ESP_PH_WITH_ADC ph;
#define Firebasename "fuzzylogic-ea9d5-default-rtdb.firebaseio.com"
#define FirebasePass "mhLBv0nvYpLXsJ3Vqlqiu3d0XtzNFGXC2OsisGt1"
#define tenwifi  "Conect_me"
#define mkwifi  "12345678"
#define ONE_WIRE_BUS 0
#define HumSensor 34
#define TurSensor 35
#define I2C_SDA 15
#define I2C_SCL 13
#define led_pin 5
#define pump_pin 14
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
FirebaseData firebaseData;
FirebaseJson json;
//-------------------------------------------------------------------------------
//Fuzzy
Fuzzy * fuzzy = new Fuzzy();
// error = Humiditysensor - Humiditysetup 
FuzzySet *Negative = new FuzzySet(-100, -100, -100, 0);
FuzzySet *Z        = new FuzzySet( -10, 0, 0, 10);
FuzzySet *Positive = new FuzzySet( 0, 100, 100, 100);
// temperature 
FuzzySet *cold     = new FuzzySet(-5, -5, 8, 15);
FuzzySet *cool     = new FuzzySet(8, 18, 18, 28);
FuzzySet *normal   = new FuzzySet(15, 25, 25, 35);
FuzzySet *hot      = new FuzzySet(30, 35, 35, 40);
FuzzySet *veryhot  = new FuzzySet(35, 43, 50, 50);
// fuzzy output
FuzzySet *high     = new FuzzySet(0, 0, 0, 0.5);
FuzzySet *slow     = new FuzzySet(0.3, 0.5, 0.5, 0.9);
FuzzySet *off      = new FuzzySet(0.8, 1, 1, 1);
//-------------------------------------------------------------------------------
void setup_rule(){
  FuzzyInput *eror = new FuzzyInput(1);

  eror->addFuzzySet(Negative);
  eror->addFuzzySet(Z);
  eror->addFuzzySet(Positive);
  fuzzy->addFuzzyInput(eror);

  //fuzzyInput2
  FuzzyInput *temp =new FuzzyInput(2);

  temp->addFuzzySet(cold);
  temp->addFuzzySet(cool);
  temp->addFuzzySet(normal);
  temp->addFuzzySet(hot);
  temp->addFuzzySet(veryhot);
  fuzzy->addFuzzyInput(temp);

  // fuzzy output
  FuzzyOutput *led =new FuzzyOutput(1);

  led->addFuzzySet(high);
  led->addFuzzySet(slow);
  led->addFuzzySet(off);
  fuzzy->addFuzzyOutput(led);
  //---------------------------

  //Fuzzyrule 1
  FuzzyRuleAntecedent *iferorisN = new FuzzyRuleAntecedent();
  iferorisN->joinSingle(Negative);
  FuzzyRuleConsequent *thenWateringH = new FuzzyRuleConsequent();
  thenWateringH->addOutput(high);
  FuzzyRule * fuzzyRule01 = new FuzzyRule (1, iferorisN, thenWateringH);
  fuzzy->addFuzzyRule(fuzzyRule01);
  //rule 2 
  FuzzyRuleAntecedent *iferorisZ_Cold = new FuzzyRuleAntecedent();
  iferorisZ_Cold->joinWithAND(Z, cold);
  FuzzyRuleConsequent *thenWateringS =new FuzzyRuleConsequent();
  thenWateringS ->addOutput(slow);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, iferorisZ_Cold,thenWateringS);
  fuzzy->addFuzzyRule(fuzzyRule02);

  // rule 3
  FuzzyRuleAntecedent *iferorisZ_Cool = new FuzzyRuleAntecedent();
  iferorisZ_Cool->joinWithAND(Z, cool);
  FuzzyRuleConsequent *thenWateringSlow =new FuzzyRuleConsequent();
  thenWateringSlow ->addOutput(slow);
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, iferorisZ_Cool,thenWateringSlow);
  fuzzy->addFuzzyRule(fuzzyRule03);

  // rule 4
  FuzzyRuleAntecedent *iferorisZ_Normal = new FuzzyRuleAntecedent();
  iferorisZ_Normal->joinWithAND(Z, normal);
  FuzzyRuleConsequent *thenWateringoff =new FuzzyRuleConsequent();
  thenWateringoff ->addOutput(off);
  FuzzyRule *fuzzyRule04 = new FuzzyRule(4, iferorisZ_Normal,thenWateringoff);
  fuzzy->addFuzzyRule(fuzzyRule04);
  //rule 5
  FuzzyRuleAntecedent *iferorisZ_Hot = new FuzzyRuleAntecedent();
  iferorisZ_Hot->joinWithAND(Z, hot);
  FuzzyRuleConsequent *thenWateringoff1 =new FuzzyRuleConsequent();
  thenWateringoff1 ->addOutput(off);
  FuzzyRule *fuzzyRule05 = new FuzzyRule(5, iferorisZ_Hot,thenWateringoff1);
  fuzzy->addFuzzyRule(fuzzyRule05);
  //rule 6
  FuzzyRuleAntecedent *iferorisZ_Veryhot = new FuzzyRuleAntecedent();
  iferorisZ_Veryhot->joinWithAND(Z, veryhot);
  FuzzyRuleConsequent *thenWateringoff2 =new FuzzyRuleConsequent();
  thenWateringS ->addOutput(off);
  FuzzyRule *fuzzyRule06 = new FuzzyRule(6, iferorisZ_Veryhot,thenWateringoff2);
  fuzzy->addFuzzyRule(fuzzyRule06);
  //rule 7
  FuzzyRuleAntecedent *iferorisP = new FuzzyRuleAntecedent();
  iferorisP->joinSingle(Positive);
  FuzzyRuleConsequent *thenWateringoff3 =new FuzzyRuleConsequent();
  thenWateringoff3 ->addOutput(off);
  FuzzyRule *fuzzyRule07 = new FuzzyRule(7, iferorisP,thenWateringoff3);
  fuzzy->addFuzzyRule(fuzzyRule07);
  
}
//------------------------------------------------------------------------------------
//rule for controller
OneWire onewire(ONE_WIRE_BUS);
DallasTemperature tempSensors(&onewire);// sử dụng thư viện để đọc cảm biến
float readTemperature(){
  tempSensors.requestTemperatures();
  float Temp= tempSensors.getTempFByIndex(0);
  //tu do F sang do C
  return (Temp-32)*5/9;
  }
float readHumidity(){ 
  float Humidity; 
  for(int i=0;i<=9;i++){
    Humidity+=analogRead(HumSensor);
    delay(50);
  }
  
  Humidity=Humidity/10;
  Serial.println(Humidity);
  Humidity = map(Humidity, 1568, 4095, 0, 100);    // Set giá thang giá trị đầu và giá trị cuối để đưa giá trị về thang từ 0-100. 
  // Cái này sẽ bằng thực nghiệm nhé
  Humidity=100-Humidity; 
  if(Humidity <0)
  {
    Humidity=0;
  }
  else if(Humidity>100){
    Humidity=100;
  }
   return Humidity;
 }
float readTurbidity(){
  float turbidity;
    for(int i=0;i<=9;i++)
  {
    //turbidity +=ads.readADC_SingleEnded(1);
    turbidity += analogRead(TurSensor);
    delay(50);
  }
  Serial.println(turbidity);
  turbidity = (turbidity/10)*(-0.067) + 146.65;
  turbidity = constrain(turbidity, 1, 100);
  return turbidity;
}
float readPHSS(float temperature){
  float  voltage = ads.readADC_SingleEnded(1) / 10; // read the voltage
  Serial.print("voltage:");
  Serial.println(voltage, 4);
  float phValue = ph.readPH(voltage, temperature); // convert voltage to pH with temperature compensation
  Serial.print("pH:");
  Serial.println(phValue, 4);
  return phValue;
}
void setup() {
  
  // put your setup code here, to run once:
  Serial.begin(115200);
  // configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pump_pin, ledChannel);
  pinMode(led_pin,OUTPUT);
  pinMode(pump_pin,OUTPUT);
  pinMode(5,OUTPUT);
  Wire.begin(I2C_SDA, I2C_SCL);
  tempSensors.begin();
  Serial.println("");
  Serial.print("Ket noi den wifi ");
  Serial.println(tenwifi);
  WiFi.begin(tenwifi, mkwifi);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi da duoc ket noi");
  Serial.println("dia chi IP: "); 
  Firebase.begin(Firebasename, FirebasePass);
  Firebase.setString(firebaseData, "/LED", "OFF");
  Firebase.setString(firebaseData, "/Watering", "OFF");
  Firebase.setString(firebaseData, "/autoWatering","OFF");
  Firebase.setInt(firebaseData, "/userHumidity",60);
  setup_rule();
}
String isWatering = "";
String ledOn="";
String autoOn ="";
void loop() {
  int user;
  // get inf in firebase
//  String asd = json.get(firebaseData,"Watering");
  if(Firebase.getString(firebaseData,"/Watering")){
    if(firebaseData.dataType() =="string"){
      isWatering = firebaseData.stringData();
    }
  }
   if(Firebase.getString(firebaseData,"/LED")){
    if(firebaseData.dataType() =="string"){
      ledOn = firebaseData.stringData();
    }
  }
   if(Firebase.getString(firebaseData,"/autoWatering")){
    if(firebaseData.dataType() =="string"){
      autoOn =firebaseData.stringData();
    }
  }
   if(Firebase.getInt(firebaseData,"/userHumidity")){
    if(firebaseData.dataType() =="int"){
      user = firebaseData.intData();
    }
  }
  Serial.println(ledOn);
  Serial.println(autoOn);
  Serial.println(user);
  //int userHum = user.toInt();
  //---------------------------------------------------------------------------
  // read sensor and up to firebase
  float temp = readTemperature();
  int hum = readHumidity();
  float turbidity= readTurbidity();
  float ph1= readPHSS(temp);
  float eror = hum - user;
  Serial.println(temp);
  json.set("/temperature",temp);
  json.set("/ humidity",hum );
  json.set("/ tumidity",turbidity );//nuoc trong 1-5 hoi duc 6-25 duc +25
  json.set("/ ph", ph1);
  Firebase.updateNode(firebaseData,"/Sensor",json);
  //---------------------------------------------------------------------------
  // fuzzy 
  digitalWrite(led_pin,HIGH);
  fuzzy->setInput(1,eror);
  fuzzy->setInput(2,temp);
  fuzzy ->fuzzify();
  float output =fuzzy->defuzzify(1);
  Serial.print("fuzzy output:");
  Serial.println(output);
  //---------------------------------------------------------------------------
  // device 
  if (ledOn == "ON"){
    // led on
    digitalWrite(led_pin,HIGH);
    Serial.println("led is ON");
  }
  else{
    //led off
    digitalWrite(led_pin,LOW);
    Serial.println("led is OFF");
  }
  if (isWatering =="ON"){
    ledcWrite(ledChannel,255);
  }
  else{
    ledcWrite(ledChannel,0);
  }
  if (autoOn =="ON"){
    if(output >=0.8){
      ledcWrite(ledChannel,0);
    }
    else {
      
      float output_temp = (0.8-output)*256.25+50;
      Serial.println(output_temp);
      ledcWrite(ledChannel,output_temp);
    }
  }
  Serial.println("-------------------------------------------------------------------------------------------------");
}
