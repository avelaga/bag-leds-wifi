/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.
   https://www.teachmemicro.com/simple-nodemcu-web-server/
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>

#ifndef APSSID
#define APSSID "papi"
#define APPSK  "b1rdlaw69"
#endif

#define NUM_LEDS 20
#define DATA_PIN D4
#define LEN 8
#define BUTTON_PIN 2
#define LED_PIN 16

/* Set these to your desired credentials. */
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
int lightmode = 1;
int frames = 0;

ESP8266WebServer server(80);

// http://192.168.4.1
void handleRoot() {
  server.send(200, "text/html", "<h1>You are connected</h1>");
}

void setup() {
  page = "<html><center><h1>THE COOLEST BACKPACK</h1><a href=\"0\"><div style='width:80%;  font-size: 80px; line-height: 30%; margin: 60px;'>Color Strobe</div></a><a href=\"1\"><div style='width:80%;  font-size: 80px; line-height: 30%; margin: 60px;'>Color Fade</div></a><a href=\"2\"><div style='width:80%;  font-size: 80px; line-height: 30%; margin: 60px;'>White Strobe</div></a><a href=\"off\"><div style='width:80%;  font-size: 80px; line-height: 30%; margin: 60px;'>OFF</div></a></center></html>";
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  LEDS.addLeds<WS2812, DATA_PIN, RGB>(leds, NUM_LEDS);
  LEDS.setBrightness(255);
  pinMode(BUTTON_PIN, INPUT);
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
  server.on("/0", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 0;
  });
  server.on("/1", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 1;
  });
  server.on("/2", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, LOW);
    lightmode = 2;
  });
  server.on("/off", []() {
    server.send(200, "text/html", page);
    digitalWrite(LED_PIN, HIGH);
    lightmode = 3;
  });
  server.begin();
  Serial.println("HTTP server started");
}


void checkButton() {
  buttonState = digitalRead(BUTTON_PIN);
  if (buttonState != recent) {
    if (buttonState == 1) {
      Serial.println("CHANGED");
      recent = buttonState;

      if (lightmode == 0) {
        lightmode = 1;
      }
      else {
        lightmode = 0;
      }
    }
    else {
      recent = 0; // reset for letting go of button
    }
  }
}

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
  //  if (inc > LEN) {
  //    isOn = !isOn;
  //    inc = 0;
  //    colSwitch += 20;
  //    hueInc = colSwitch;
  //  } else {
  //    inc++;
  hueInc += 3;
  //  }

  for (int a = NUM_LEDS - 1; a > 0; a--) {
    hue[a] = hue[a - 1];
    if (brightness[a - 1] == 255) {
      brightness[a] = 255;
    }
    else {
      brightness[a] = 0;
    }
  }

//  if (isOn) {
    brightness[0] = 255;
    hue[0] = hueInc;
//  }
//  else {
//    brightness[0] = 0;
//  }

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
    //    colSwitch += 20;
    //    hueInc = colSwitch;
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

void loop() {
  server.handleClient();
  checkButton();
  if (lightmode == 0) { // normal fade
    colorStrobe();
  }
  else if (lightmode == 1) {
    colorFade();
  }
  else if (lightmode == 2) {
    whiteStrobe();
  } else {
    clearLeds();
  }
  FastLED.show();
}
