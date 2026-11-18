//Mecha2_IOT_Temperature_Humidity_GoogleSheet_Line
#include "ESP8266WiFi.h"
#include "WiFiClientSecure.h"
#include "DHT.h"                  //DHT22 type of Sensor
#include "Wire.h"                 //OLED_u8g2 SCL = D1 & SDA = D2
#include "U8g2lib.h"              //OLED_u8g2 SCL = D1 & SDA = D2

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);//OLED_u8g2 SCL = D1 & SDA = D2

#include <MCP3208.h>              
#include <SPI.h>                  

// --- แก้ไขการเรียกใช้งาน MCP3208 ให้ตรงกับ Library บน PlatformIO ---
MCP3208 adc;                      

// --- แก้ไขจาก D9 เป็น D0 ป้องกันบอร์ดค้างตอนจ่ายไฟ ---
#define Relay D0                  //ON FAN MOTOR
#define DHTPIN D4                 //pin connect DHT
#define DHTTYPE DHT22             //DHT22 type of Sensor
DHT dht(DHTPIN, DHTTYPE);         //DHT22 type of Sensor
#define ANALOG_PIN A0             //ปรับค่าความชื้น 1-10
int analogValue = 0;              //ปรับค่าความชื้น 2-10
int xx = 0;                       //เงื่อนคำสั่งรีสตาร์ท ESP8266 4-1

String readString;
const char* ssid = "MECHA_AP";
const char* password = "MECHA_@123@456";           
unsigned long lastDebounceTime = 0;   
unsigned long debounceDelay = 50;     
const char* host = "script.google.com";
const int httpsPort = 443;
WiFiClientSecure client;
const char* fingerprint = "46 B2 C3 44 9C 59 09 8B 01 B6 F8 BD 4C FB 00 74 91 2F EF F6";
String GAS_ID = "AKfycbw2-LTm8azCAzMKx8Q3z-CSGiAIZfjZrg5qxIJ5";  // Replace by your GAS service id 
int bb = 0;
int cc = 0;
int dd = 0;
int ee = 0;
int vv = 0;                                //WiFi

// --- ประกาศชื่อฟังก์ชัน (Prototypes) ป้องกัน Error: was not declared in this scope ---
void safeDelay(unsigned long ms);
void sendData(float t, float h, float c, float d, float e, float f);

void setup(){
  Serial.begin(115200);
  safeDelay(1000);                                               
  u8g2.begin();                                                  
  u8g2.enableUTF8Print();                                        
  dht.begin();                                                   
  
  // กำหนดขา CS ให้ MCP3208
  adc.begin(D3);                                                   
  
  pinMode(Relay, OUTPUT);                                        
  digitalWrite(Relay, 0);                                         
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);                                    
  WiFi.status(); safeDelay(5000); WiFi.status(); safeDelay(5000);

  Serial.print("IP = ");
  Serial.println(WiFi.localIP());
  
  // --- แก้ไขเงื่อนไขการเช็ค WiFi ---
  if (WiFi.status() != WL_CONNECTED) { 
    Serial.println("connected...No"); 
    vv = 1; 
    xx = 0; 
  }  
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.println("connected...OK"); 
  }        
}
    
void loop(){ 
  if (vv >= 1){ safeDelay(3000); }
  if (vv <= 0){ safeDelay(12000); } 

  analogValue = analogRead(ANALOG_PIN);                             
  int val = analogRead(ANALOG_PIN);                                
  int val1 = map(val, 0, 1023, 0, 500);                           
  double r1 = 0.05;                                               
  int r2 = val1;                                                  
  int r3 = 1;                                                     
  double area_h = r1 * r2 * r3;                                       

  // --- แก้ไขคำสั่งอ่านค่า MCP3208 เป็น readADC ---
  int va2 = adc.readADC(0);                                     
  int va22 = map(va2, 0, 4094, 0, 500);                           
  double rr1 = 0.05;                                              
  int rr2 = va22;                                                 
  int rr3 = 1;                                                    
  double area_t = rr1 * rr2 * rr3;                                    

  Serial.print("bb = "); Serial.println(bb);
  Serial.print("cc = "); Serial.println(cc);
  Serial.print("ee = "); Serial.println(ee);
  Serial.print("vv = "); Serial.println(vv);
  Serial.print("xx = "); Serial.println(xx);
  Serial.print("millis = "); Serial.println(millis());
  Serial.print("IP = "); Serial.println(WiFi.localIP());

  if (xx == 1) {
    Serial.println("Restarting system DHT Sensor");
    ESP.restart();                                              
  }

  float h = (dht.readHumidity() + 5) - area_h;                      
  float t = (dht.readTemperature() + 10) - area_t;                  
  float f = dht.readTemperature(true);
  float c = 0;                                                  
  float d = 0;                                                  
  float e = 0;                                                  
  
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!")); 
    xx = 1;  
    return;

    // h = 0; 
    // t = 0;
    // f = 0;
  }
  
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);       

  // --- แก้ไขเงื่อนไขการเช็ค WiFi ใหม่ ---
  if (WiFi.status() == WL_CONNECTED) { vv = 0; bb = 0; cc = 0; } 
  if (WiFi.status() != WL_CONNECTED) { cc = 1; } 
  if (cc > vv) { bb++; } 
  if (bb > 6) { ESP.restart(); } 
 
  if (vv <= 0) {  
    if(t <= 21.99) { digitalWrite(Relay, 1); c = 1; }  
    if(t >= 22) { digitalWrite(Relay, 0); c = 2; }  
    if(h >= 68) { digitalWrite(Relay, 1); c = 1; }  
    if(h <= 65) { digitalWrite(Relay, 0); c = 2; }  
  }
  
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_ncenB18_tr); 
    u8g2.setCursor(0, 20);
    u8g2.print("H: ");
    u8g2.print(h);
    u8g2.print(" %"); 
    u8g2.setCursor(0, 48);
    u8g2.print("T: ");
    u8g2.print(t);
    u8g2.print(" °C"); 
    u8g2.setFont(u8g2_font_ncenB08_tr); 
    u8g2.setCursor(0, 63);
    u8g2.print(vv);
    u8g2.print(".Wifi IP");
    u8g2.print(WiFi.localIP());
  } while ( u8g2.nextPage() );
  
  Serial.print("Analog value: "); Serial.print("\t");
  Serial.print("Humidity: "); Serial.print(h); Serial.print("\t");
  Serial.print("Temp: "); Serial.println(t);
  Serial.println("..............................."); 

  if (vv <= 0) { 
    ee++;
    if(ee >= 300) { 
      sendData(h, t, c, d, e, f); 
      ee = 0; 
    }
  }
}     

// Function for Send data into Google Spreadsheet
void sendData(float t, float h, float c, float d, float e, float f)
{
  Serial.print("connecting to ");
  Serial.println(host);
  
  // --- ตั้งค่าให้ข้ามการตรวจ Fingerprint (ใช้กับ Google Sheet ดีที่สุด) ---
  client.setInsecure(); 

  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) { 
      delay(100); 
      Serial.print(".");
    }
    return;
  }

  // ลบ block client.verify เดิมออก เพราะเราใช้ setInsecure() แล้ว

  String string_testDataA =  String(t, DEC); 
  String string_testDataB =  String(h, DEC); 
  String string_testDataC =  String(c, DEC); 
  String string_testDataD =  String(d, DEC);
  String string_testDataE =  String(e, DEC);
  String string_testDataF =  String(f, DEC);    
  String url = "/macros/s/" + GAS_ID + "/exec?testDataA=" + string_testDataA + "&testDataB=" + string_testDataB + "&testDataC=" + string_testDataC + "&testDataD=" + string_testDataD + "&testDataE=" + string_testDataE + "&testDataF=" + string_testDataF;
  
  Serial.print("requesting URL: ");
  Serial.println(url);
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
} 

// --- ฟังก์ชัน safeDelay ป้องกัน Watchdog Reset ---
void safeDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    yield();   
    delay(1);  
  }
}