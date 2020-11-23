#include <Wire.h>
#include <Adafruit_ADS1015.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>

#define D5 14
#define D6 12
#define D7 13
#define D8 15

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */

#include <OneWire.h>
#include <DallasTemperature.h>
unsigned int t, tRsClient;

int nhietDo ,doAm, anhSang, luongMua;

#define ONE_WIRE_BUS 2          // Cài đặt chân GPIO sử dụng
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

//include thư viện để kiểm tra free RAM trên con esp8266
extern "C" {
  #include "user_interface.h"
}

SocketIOClient client;
WiFiUDP Udp;

//const char* ssid = "AndroidAP62FC";
//const char* password = "Toan3010";
const char* ssid = "abcxyx";
const char* password = "1234abcd@";


// Server Ip
char host[] = "vuonrauminhtrung.herokuapp.com";
//char host[] = "192.168.43.156";
// Server port
//int port = 3000;
int port = 80;

extern String RID;
extern String Rfull;

unsigned long previousMillis = 0;
long interval = 5000;

String data;

int doAmDC, nhietDoDC, anhSangDC, luongMuaDC;

bool bomTuoiSoft = 0, bomThoatSoft = 0, maiCheSoft = 0, quatThoangSoft = 0;

bool bomTuoi = 0, bomThoat = 0, maiChe = 0, quatThoang = 0;

int bomTuoiHard = 0, bomThoatHard = 0, maiCheHard = 0, quatThoangHard = 0;

void setup(void) 
{
  Wire.begin(D2,D1);
  Serial.begin(115200);
  Serial.println("Hello!");
  
  pinMode(0,INPUT_PULLUP); 
  pinMode(D5,OUTPUT); //Mai che
  pinMode(D6,OUTPUT); //Quat thoang
  pinMode(D7,OUTPUT); //Bom tuoi
  pinMode(D8,OUTPUT); //Bom thoat

  ads.begin();
  sensors.begin(); // Bắt đầu đọc cảm biến
  
    //Việc đầu tiên cần làm là kết nối vào mạng Wifi
    Serial.print("Ket noi vao mang ");
    Serial.println(ssid);
 
    //Kết nối vào mạng Wifi
    WiFi.begin(ssid, password);
 
    //Chờ đến khi đã được kết nối
    while (WiFi.status() != WL_CONNECTED) { //Thoát ra khỏi vòng 
        delay(500);
        Serial.print('.');
    }
 
    Serial.println();
    Serial.println(F("Da ket noi WiFi"));
    Serial.println(F("Dia chi IP cua ESP8266 (Socket Client ESP8266): "));
    Serial.println(WiFi.localIP());
 
    if (!client.connect(host, port)) {
        Serial.println(F("Ket noi den socket server that bai!"));
        return;
    }
}

void loop(void) 
{
  doAm=0;
  anhSang = 0;
  luongMua = 0;
  //Xu ly anh sang
  for (int i = 0; i < 10; i++) // Đọc giá trị cảm biến 10 lần và lấy giá trị trung bình
  {
    anhSang += ads.readADC_SingleEnded(3);;
    delay(50);
  }
  anhSang = anhSang / 10; 
  anhSang = map(anhSang, 25, 17685, 0, 100); //Tối:0  ==> Sáng 100%

  if(anhSang > anhSangDC)
    {
      maiCheSoft = 1;   
    }
    else{
      maiCheSoft = 0;
    }

   if(maiCheHard == 1)
    {
      digitalWrite(D5, HIGH);   
      maiChe = 1;
    }
    else{
      if(maiCheSoft == 1){
        digitalWrite(D5, HIGH);
        maiChe = 1;
      }
      else{
        digitalWrite(D5, LOW);
        maiChe = 0;
      }
    }
  
  //Xu ly luong mua
  for (int i = 0; i < 10; i++) // Đọc giá trị cảm biến 10 lần và lấy giá trị trung bình
  {
    luongMua += ads.readADC_SingleEnded(2);;
    delay(50);
  }
  luongMua = luongMua / 10; 
  luongMua = map(luongMua, 25, 17685, 0, 100); //Tối:0  ==> Sáng 100%
  if(luongMua > luongMuaDC)
    {
      bomThoatSoft = 1;   
    }
    else{
      bomThoatSoft = 0;
    }

   if(bomThoatHard == 1)
    {
      digitalWrite(D8, HIGH);   
      bomThoat = 1;
    }
    else{
      if(bomThoatSoft == 1){
        digitalWrite(D8, HIGH);
        bomThoat = 1;
      }
      else{
        digitalWrite(D8, LOW);
        bomThoat = 0;
      }
    }

    
  //Xu ly do am
  for(int i=0;i<=9;i++){
    doAm+=analogRead(A0);
    delay(50);
  }
  doAm=doAm/10;
  doAm = map(doAm, 350, 1023, 0, 100);    // Set giá thang giá trị đầu và giá trị cuối để đưa giá trị về thang từ 0-100. 
  // Cái này sẽ bằng thực nghiệm nhé
  doAm=100-doAm;                            // Tính giá trị phần trăm thực. Chuyển điện thế từ 3.3V ( khô ) thành 3.3V ( ẩm )
  
  if(doAm > doAmDC)
    {
      quatThoangSoft = 1;   
    }
    else{
      quatThoangSoft = 0;
    }

   if(quatThoangHard == 1)
    {
      digitalWrite(D6, HIGH);   
      quatThoang = 1;
    }
    else{
      if(quatThoangSoft == 1){
        digitalWrite(D6, HIGH);
        quatThoang = 1;
      }
      else{
        digitalWrite(D6, LOW);
        quatThoang = 0;
      }
    }

  //Xu ly nhiet do
  if(millis()-t >1000) {
    sensors.requestTemperatures(); 
    nhietDo = sensors.getTempCByIndex(0);
    if(nhietDo > nhietDoDC)
    {
      bomTuoiSoft = 1;   
    }
    else{
      bomTuoiSoft = 0;
    }

   if(bomTuoiHard == 1)
    {
      digitalWrite(D7, HIGH);   
      bomTuoi = 1;
    }
    else{
      if(bomTuoiSoft == 1){
        digitalWrite(D7, HIGH);
        bomTuoi = 1;
      }
      else{
        digitalWrite(D7, LOW);
        bomTuoi = 0;
      }
    }
    /*Serial.print("Nhiet do: ");
    Serial.print(nhietDo); 
    Serial.println("*C");
    Serial.print("Anh sang: ");
    Serial.print(anhSang);
    Serial.println(" %");
    Serial.print("Do am: ");
    Serial.print(doAm);
    Serial.println('%');
    Serial.print("Luong mua: ");
    Serial.print(luongMua);
    Serial.println('%');
    Serial.println(bomThoatSoft);
    Serial.println(bomThoatHard);*/  
  }
  
    t=millis();

  unsigned long currentMillis = millis();
   if(currentMillis - previousMillis > interval)
   {
        //Xu ly dieu chinh
        client.send("DocXongNhietDo", "sensorNhietDo", String(nhietDo));
      client.send("DocXongDoAm", "sensorDoAm", String(doAm));
      client.send("DocXongAnhSang", "sensorAnhSang", String(anhSang));
      client.send("DocXongLuongMua", "sensorLuongMua", String(luongMua));
      client.send("TrangThaiBomThoat", "motorBomThoat", String(bomThoat));
      client.send("TrangThaiQuatThoang", "motorQuatThoang", String(quatThoang));
      client.send("TrangThaiMaiChe", "motorMaiChe", String(maiChe));
      client.send("TrangThaiBomTuoi", "motorBomTuoi", String(bomTuoi));
        previousMillis = currentMillis;
   }
   if (client.monitor()) {
        if (RID == "MuonDoc")
        {
          client.send("DocXongNhietDo", "sensorNhietDo", String(nhietDo));
          client.send("DocXongDoAm", "sensorDoAm", String(doAm));
          client.send("DocXongAnhSang", "sensorAnhSang", String(anhSang));
          client.send("DocXongLuongMua", "sensorLuongMua", String(luongMua));
          client.send("TrangThaiBomThoat", "motorBomThoat", String(bomThoat));
          client.send("TrangThaiQuatThoang", "motorQuatThoang", String(quatThoang));
          client.send("TrangThaiMaiChe", "motorMaiChe", String(maiChe));
          client.send("TrangThaiBomTuoi", "motorBomTuoi", String(bomTuoi));
        }

        if (RID == "MuonDocDC")
        {
          client.send("DocXongNhietDoDC", "sensorNhietDoDC", String(nhietDoDC));
          client.send("DocXongDoAmDC", "sensorDoAmDC", String(doAmDC));
          client.send("DocXongAnhSangDC", "sensorAnhSangDC", String(anhSangDC));
          client.send("DocXongLuongMuaDC", "sensorLuongMuaDC", String(luongMuaDC));
          client.send("TrangThaiBomThoatHard", "motorBomThoatHard", String(bomThoatHard));
          client.send("TrangThaiQuatThoangHard", "motorQuatThoangHard", String(quatThoangHard));
          client.send("TrangThaiMaiCheHard", "motorMaiCheHard", String(maiCheHard));
          client.send("TrangThaiBomTuoiHard", "motorBomTuoiHard", String(bomTuoiHard));
        }
        if (RID == "MuonChinhAnhSang")
        {
          anhSangDC = Rfull.toInt();
        }
        if (RID == "MuonChinhDoAm")
        {
          doAmDC = Rfull.toInt();
        }
        if (RID == "MuonChinhNhietDo")
        {
          nhietDoDC = Rfull.toInt();
        }
        if (RID == "MuonChinhLuongMua")
        {
          luongMuaDC = Rfull.toInt();
        }
        if (RID == "MuonChinhBomThoat")
        {
          bomThoatHard = Rfull.toInt();
        }
        if (RID == "MuonChinhQuatThoang")
        {
          quatThoangHard = Rfull.toInt();
        }
        if (RID == "MuonChinhMaiChe")
        {
          maiCheHard = Rfull.toInt();
        }
        if (RID == "MuonChinhBomTuoi")
        {
          bomTuoiHard = Rfull.toInt();
        }
    }
    //Kết nối lại!
    if(millis()-tRsClient >25000) {
      client.reconnect(host,port);
      Serial.println("Ket noi lai.");
      tRsClient=millis();
    }
}
