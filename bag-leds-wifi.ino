/*
   Developed by Abhi Velaga
   www.abhi.work
   abhinav.velaga@utexas.edu
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_LIS3DH.h>
#include <Adafruit_Sensor.h>

#define APSSID "papi"
#define APPSK  "milksteak"

#define NUM_LEDS 20
#define LEN 8
#define LED_PIN 16

// Used for software SPI
#define LIS3DH_CLK 13
#define LIS3DH_MISO 12
#define LIS3DH_MOSI 11
// Used for hardware & software SPI
#define LIS3DH_CS 10

// labeled as D4 on nodemcu
#define DATA_PIN D2

// labeled as D7 on node mcu
//#define BUTTON_PIN 3

// wifi credentials
const char *ssid = APSSID;
const char *password = APPSK;

String page = "";
CRGB leds[NUM_LEDS];
int brightness[NUM_LEDS];
int hue[NUM_LEDS];
boolean isOn = false;
int inc = 0;
int hueInc = 0;
int colSwitch = 0;
int buttonState = 0;
int recent = 0;
int lightmode = 0;
int frames = 0;

ESP8266WebServer server(80);
Adafruit_LIS3DH lis = Adafruit_LIS3DH();

// http://192.168.4.1
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
  if (! lis.begin(0x18)) {   // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt find accelerometer");
    while (1) yield();
  }
  Serial.println("Accelerometer found!");
  // lis.setRange(LIS3DH_RANGE_4_G);   // 2, 4, 8 or 16 G! 2G by default
  Serial.print("Range = "); Serial.print(2 << lis.getRange());
  Serial.println("G");
  
  page = "<html><center><a href=\"0\"><div style='position: fixed; width:50%;  height: 50%; left: 0; top: 0; font-size: 80px; display: flex; justify-content: center; align-items: center; color: white; background-color: red;'>Color Strobe</div></a><a href=\"1\"><div style='position: fixed; width:50%;  height: 50%; left: 50%; top: 0; font-size: 80px; display: flex; justify-content: center; align-items: center; background-color: rgb(0,255,0); color: white;'>Color Fade</div></a><a href=\"2\"><div style='position: fixed; width:50%;  height: 50%; left: 0; top: 50%; font-size: 80px; display: flex; justify-content: center; align-items: center; color: black;'>White Strobe</div></a><a href=\"off\"><div style='position: fixed; width:50%;  height: 50%; left: 50%; top: 50%; font-size: 80px; display: flex; justify-content: center; align-items: center; background-color: black; color: white;'>OFF</div></a></center></html>";
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  LEDS.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
//  pinMode(BUTTON_PIN, INPUT);
  delay(1000);
  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  server.on("/", []() {
    server.send(200, "text/html", page);
  });

  // color strobe
  server.on("/0", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 0;
  });

  // color fade
  server.on("/1", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 1;
  });

  // white strobe
  server.on("/2", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 2;
  });

  // off
  server.on("/off", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, HIGH);
    lightmode = 3;
  });

  server.begin();
  Serial.println("HTTP server started");
}

//void checkButton() {
//  buttonState = digitalRead(BUTTON_PIN);
//  if (buttonState != recent) {
//    if (buttonState == 1) {
//      Serial.println("CHANGED");
//      recent = buttonState;
//
//      if (lightmode != 3) {
//        lightmode = 3;
//      }
//      else {
//        lightmode = 0;
//      }
//    }
//    else {
//      recent = 0; // reset for letting go of button
//    }
//  }
//}

void clearLeds() {
  for (int x = 0; x < NUM_LEDS; x++) {
    leds[x] = CHSV(0, 0, 0);
  }
}

void colorStrobe() {
  if (inc > LEN) {
    isOn = !isOn;
    inc = 0;
    colSwitch += 20;
    hueInc = colSwitch;
  } else {
    inc++;
    hueInc += 3;
  }

  for (int a = NUM_LEDS - 1; a > 0; a--) {
    hue[a] = hue[a - 1];
    if (brightness[a - 1] == 255) {
      brightness[a] = 255;
    }
    else {
      brightness[a] = 0;
    }
  }

  if (isOn) {
    brightness[0] = 255;
    hue[0] = hueInc;
  }
  else {
    brightness[0] = 0;
  }

  // fill leds with determined pattern
  for (int x = 0; x < NUM_LEDS; x++) {
    leds[x] = CHSV(hue[x], 255, brightness[x]);
  }

  delay(15);
}

void colorFade() {
  hueInc += 3;

  for (int a = NUM_LEDS - 1; a > 0; a--) {
    hue[a] = hue[a - 1];
    if (brightness[a - 1] == 255) {
      brightness[a] = 255;
    }
    else {
      brightness[a] = 0;
    }
  }

  brightness[0] = 255;
  hue[0] = hueInc;

  // fill leds with determined pattern
  for (int x = 0; x < NUM_LEDS; x++) {
    leds[x] = CHSV(hue[x], 255, brightness[x]);
  }

  delay(15);
}

void whiteStrobe() {
  if (inc > LEN) {
    isOn = !isOn;
    inc = 0;
  }
  inc++;

  if (isOn) {
    for (int x = 0; x < NUM_LEDS; x++) {
      leds[x] = CRGB(255, 255, 255);
    }
  }
  else {
    clearLeds();
  }

  delay(15);
}

void checkAcceleration(){
  /* get a new sensor event, normalized */
  sensors_event_t event;
  lis.getEvent(&event);

  /* Display the results (acceleration is measured in m/s^2) */
//  Serial.print("\t\tX: "); Serial.print(event.acceleration.x);
//  Serial.print(" \tY: "); Serial.print(event.acceleration.y);
//  Serial.print(" \tZ: "); Serial.print(event.acceleration.z);
//  Serial.println(" m/s^2 ");
//
//  Serial.println();
}

void loop() {
  server.handleClient();
//  checkButton();
  checkAcceleration();
  switch (lightmode) {
    case 0:
      colorStrobe();
      break;
    case 1:
      colorFade();
      break;
    case 2:
      whiteStrobe();
      break;
    default:
      clearLeds();
  }
  FastLED.show();
}
