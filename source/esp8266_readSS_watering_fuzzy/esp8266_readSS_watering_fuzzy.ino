#include <OneWire.h>
#include <DallasTemperature.h>
#include <Fuzzy.h>
#include <FirebaseArduino.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include <string>

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2 
#define D5 14
#define D6 12
#define D7 13
#define D8 150

/* 
 * chọn  chân D4 là đầu vào cho dây analog của cảm biến nhiệt
 */
 OneWire onewire(D4);

DallasTemperature tempSensors(&onewire);// sử dụng thư viện để đọc cảm biến
int Humidity;
float Temperature, turbidity, voltage_ph, ph_Value;
String fireStatus = ""; 
String wateringStatus = "";
String userHumidity ="";
bool isWatering=0;
void Isr_turnoff_maybom();
float eror_Humidity; 
String autowatering= "";
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
  fuzzy->addFuzzyRule(fuzzyRule02);
  
}
const String tenwifi = "Conect_me";
const String mkwifi = "12345678";


  float readTemperature(){
  tempSensors.requestTemperatures();
  float Temp= tempSensors.getTempFByIndex(0);
  //tu do F sang do C
  return (Temp-32)*5/9;
}
float readPH(){
  for (int i=0;i<10; i++)
    {
      voltage_ph += analogRead(D2);
      delay(50);
    }
    voltage_ph = voltage_ph *5.0/10 /1024;// khang nhieu
   
    return  voltage_ph * -5.70 + 21.34 ; // chuyen vol -> ph
}
void setup() {

  
  Serial.begin(115200);
  Serial.println("");
  Serial.print("Ket noi den wifi ");
  Serial.println(tenwifi);
  WiFi.begin(tenwifi, mkwifi);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi da duoc ket noi");
  Serial.println("dia chi IP: "); 
  
  Firebase.begin("esp8266-e128d.firebaseio.com", "1f72YSTwTZ9um4AZWlZl8nABO2rz86agcVVgLASG");
  pinMode (D1,INPUT);
  pinMode(D6,OUTPUT);
  pinMode(D7,OUTPUT);
   pinMode (D2,INPUT);// analog signal ph 
  tempSensors.begin();// đọc cảm biến nhiệt
  Firebase.setString("LED", "OFF");
  Firebase.setString("Watering", "OFF");
  Firebase.setString("autoWatering","OFF");
  Firebase.setString("userHumidity","00");
  setup_rule();
}

void loop() {
  Temperature = 0;
  Humidity = 0;
  turbidity = 0;
  ph_Value = 0;
  autowatering = Firebase.getString("autoWatering");
  wateringStatus = Firebase.getString("Watering");
  fireStatus = Firebase.getString("LED");
  if(wateringStatus == "ON"){
    // code fuzzy here to find time watering
  }
  else if (wateringStatus == "OFF"){
    //stop watering
  }
  else {
    Serial.println("sai cu phap xin nhap ON/OFF");
    }
  if (fireStatus == "ON") {
    Serial.println("led turned ON");
    digitalWrite(D7,HIGH);
  }
  else if (fireStatus == "OFF"){
    Serial.println("led turned OFF");
    digitalWrite(D7,LOW);
  }
  else{
    Serial.println("sai cu phap! vui long chi nhap ON hoac OFF");
  }
  ph_Value = readPH();
  Serial.print(ph_Value);
  Serial.print("^ph");
  Serial.print('\n');
  delay(200);
  Temperature = readTemperature();
  Serial.print(Temperature);
  Serial.print('C');
  delay(200);
  Serial.print('\n');
  delay(200);  
  for(int i=0;i<=9;i++){
    Humidity+=analogRead(A0);
    delay(50);
  }
  Humidity=Humidity/10;
  Humidity = map(Humidity, 350, 1023, 0, 100);    // Set giá thang giá trị đầu và giá trị cuối để đưa giá trị về thang từ 0-100. 
  // Cái này sẽ bằng thực nghiệm nhé
  Humidity=100-Humidity; 
   Serial.print(Humidity);
   Serial.print('%');
   Serial.print('\n');
   delay(200);
  //cam bien nhiet
  Temperature = readTemperature();
  userHumidity = Firebase.getString("userHumidity");
  Serial.print("user humidity" + userHumidity +"\n");
  
  if(autowatering == "ON"){
    float temp= userHumidity.toFloat();//atof(userHumidity);
    eror_Humidity = Humidity-temp;
    Serial.print("eror is : " );
    Serial.println( eror_Humidity);
    fuzzy->setInput(1,eror_Humidity);
    fuzzy->setInput(2,Temperature);
    fuzzy ->fuzzify();

    float output =fuzzy->defuzzify(1);
    Serial.print("output is " );
    Serial.println(output);
    delay(200);
  }
  for(int i=0;i<=9;i++)
  {
    turbidity +=analogRead(D0);
    delay(50);
  }
  turbidity = (turbidity/10)*5/1024; 
  Serial.print(turbidity);
  Serial.print('\n');
  // send inf to firebase 
  Firebase.setFloat("humidity", Humidity);
  Firebase.setFloat("Temperature", Temperature);
  Firebase.setFloat("Turbidity", turbidity);
  Firebase.setFloat("PH ", ph_Value);
  delay(200);
}
