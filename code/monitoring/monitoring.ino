#include <SPI.h>
#include <Wire.h>
#include <SPIFFS.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
 
// Network
const char* ssid = "******";
const char* password = "*******";

// AsyncWebServer object on port 80
AsyncWebServer server(80);


#define OLED_SDA 21
#define OLED_SCL 22
#define CURRENT_S 14
#define VOLTAGE_S 33

/////////////////////////////////////Attribute/////////////////////////////////
char solar[] = "Solar";
double solarVoltage;
double solarCurrent;

/////////////////////////////////////Attribute/////////////////////////////////

const int voltageSensor = 33;
const int currentSensor = 14;

float vOUT;
float vIN;

float solarPower;

float R1 = 30000.0;
float R2 = 7500.0;
int value = 0;


/////////////////////////////////////Einheiten////////////////////////////////////
char ampere[] = "mA";
char voltage[] = "V";
char power[] = "W";

/////////////////////////////////////////////////////////////////////////////////////

Adafruit_SH1106 display(21, 22);

//////////////////////////////////////Sensormethoden + Leistung/////////////////////
String readVoltage() {
  return String(spannungsmessung());
}

String readCurrent() {
  return String(strommessung());
}

String calculatePower() {
  return String(leistungsBerechnung());
}

float leistungsBerechnung(){
  solarPower = solarVoltage*(solarCurrent/1000);
  return solarPower;
}

float spannungsmessung(){
  value = analogRead(voltageSensor);
  vOUT = (value * 3.9) / 4095.0;
  vIN = vOUT / (R2/(R1+R2));
  solarVoltage = vIN;
  delay(50);
  return solarVoltage;
}

float strommessung() {
unsigned int x=0;
float AcsValue=0.0,Samples=0.0,AvgAcs=0.0,AcsValueF=0.0;

  for (int x = 0; x < 150; x++){ 
  AcsValue = analogRead(currentSensor);     /  
  Samples = Samples + AcsValue;  
  delay (3); 
  }

  AvgAcs = Samples/150.0;
  AcsValueF = (2.5 - (AvgAcs * (3.925 / 4096.0)) )/0.185;
  solarCurrent = AcsValueF*(-10);

  delay(50);

  return solarCurrent;
}
/////////////////////////////////////////////////////////////////////////////////////

void setup()   {                
  Serial.begin(115200);
  /* initialize OLED with I2C address 0x3C */
  display.begin(SH1106_SWITCHCAPVCC, 0x3C); 
  display.clearDisplay();

if(!SPIFFS.begin()){
  Serial.println("Fehler bei SPIFF connection");
  while(1);
}

// Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

// Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

// Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html");
  });
  server.on("/voltage", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readVoltage().c_str());
  });
  server.on("/current", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readCurrent().c_str());
  });
  server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", calculatePower().c_str());
  });

  // Start server
  server.begin();
}

void loop() { 
  display.setTextSize(2);
  display.setTextColor(WHITE);

  solarCurrent = strommessung();
  solarVoltage = spannungsmessung();
  solarPower = leistungsBerechnung();


  display.setCursor(0,0);
  display.print("P= ");
  display.print(solarPower);
  display.print(power);
  
  display.setCursor(0,20);
  display.print("U= ");
  display.print(solarVoltage);
  display.print(voltage);
  display.print(" ");
  
  display.setCursor(0,40);
  display.print("I= ");
  display.print(solarCurrent);
  display.print(ampere);

  display.display();
  delay(100);
  display.clearDisplay();
}
