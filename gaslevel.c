#define sensor 34
#define buzzer 2
#define DHTPIN 4 
#define BLYNK_TEMPLATE_ID "TMPL6SvUgu889"
#define BLYNK_TEMPLATE_NAME "Gas Level Monitoring"
#define RX 16   
#define TX 17   

#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <DHTesp.h>
#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include "CTBot.h"


CTBot mybot;


LiquidCrystal_I2C lcd(0x27, 16, 2);
BlynkTimer timer;


char auth[] = "8xHhtgmNIPYzjdq-OH_LOxWH9xJmr4kN";
const int64_t chat_id = 5279401514; 

char ssid[] = "SOQII";
char pass[] = "anakbaik231";

DHTesp dht;

TinyGPSPlus gps;
HardwareSerial GPSModule(1);

bool botConnectedSent = false; 

void setup() {
  
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  lcd.init();
  lcd.backlight();
  pinMode(buzzer, OUTPUT);

  dht.setup(DHTPIN, DHTesp::DHT22);
  
  GPSModule.begin(9600, SERIAL_8N1, 16, 17); 

  mybot.wifiConnect(ssid, pass);
  mybot.setTelegramToken("7515411372:AAGNM3O6qy8j2WyjFDFIeEC5rPiXMwdZqcg");

  if (!botConnectedSent) {
    mybot.sendMessage(chat_id, "Bot Connected | Kelompok 3");
    botConnectedSent = true; 
  }

  lcd.setCursor(1, 0);
  lcd.print("System Loading");
  for (int a = 0; a <= 15; a++) {
    lcd.setCursor(a, 1);
    lcd.print(".");
    delay(500);
  }
  lcd.clear();
}

void GASLevelAndGPSData() {
  int value = analogRead(sensor);
  value = map(value, 0, 4095, 0, 100);

  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();

  lcd.setCursor(0, 0);
  lcd.print("GAS Level : ");
  lcd.print(value);

  if (value >= 50) {
    digitalWrite(buzzer, HIGH);
    lcd.setCursor(0, 1);
    lcd.print("Status: Warning ");
    WidgetLED LED(V1);
    LED.on();

    Serial.print("GAS Level: ");
    Serial.print(value);
    Serial.print(" | Temperature: ");
    Serial.print(temperature);
    Serial.print(" | Humidity: ");
    Serial.println(humidity);

    float latitude = 0, longitude = 0;

    while (GPSModule.available() > 0) {
      if (gps.encode(GPSModule.read())) {
        if (gps.location.isUpdated()) {
          latitude = gps.location.lat();
          longitude = gps.location.lng();
          break;
        }
      }
    }

    String warningMessage = "Peringatan! Gas level terlalu tinggi.\n";
    warningMessage += "Gas Level: " + String(value) + "\n";
    warningMessage += "Temperature: " + String(temperature) + " C\n";
    warningMessage += "Humidity: " + String(humidity) + " %\n";
    if (latitude != 0 && longitude != 0) {
      warningMessage += "Lokasi: https://www.google.com/maps?q=" + String(latitude, 6) + "," + String(longitude, 6);
    } else {
      warningMessage += "Lokasi GPS belum tersedia.";
    }

    mybot.sendMessage(chat_id, warningMessage);

  } else {
    digitalWrite(buzzer, LOW);
    lcd.setCursor(0, 1);
    lcd.print("Status: Aman    ");
    WidgetLED LED(V1);
    LED.off();
  }
  Blynk.virtualWrite(V0, value); 
  Blynk.virtualWrite(V2, temperature);
}

void loop() {
  GASLevelAndGPSData();
  Blynk.run();
  delay(1000);
}
