#define BLYNK_TEMPLATE_ID "TMPLdroSeriT"
#define BLYNK_DEVICE_NAME "fog"
#define BLYNK_AUTH_TOKEN "R-SI-kqRSzWd13Rm8Eo82W77NXYKI0I6"
#include <Blynk.h>
#define BLYNK_PRINT Serial

#include "BlynkSimpleEsp32.h"
#include <Arduino_JSON.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define ACTIVE_STATE 0
#define LOW_STATE 1
#define UNACTIVE_STATE 2

#define LDR 33
#define RED 23
#define GREEN 18
#define YELLOW 19
#define RESISTOR 14

#define BUTTON 17

#define A 22
#define B 21
#define C 32
#define D 16
#define E 25
#define FLED 26
#define G 27

bool sete_segmentos[10][7] = { 
{ 1,1,1,1,1,1,0 }, // = Digito 0
{ 0,1,1,0,0,0,0 }, // = Digito 1
{ 1,1,0,1,1,0,1 }, // = Digito 2
{ 1,1,1,1,0,0,1 }, // = Digito 3
{ 0,1,1,0,0,1,1 }, // = Digito 4
{ 1,0,1,1,0,1,1 }, // = Digito 5
{ 1,0,1,1,1,1,1 }, // = Digito 6
{ 1,1,1,0,0,0,0 }, // = Digito 7
{ 1,1,1,1,1,1,1 }, // = Digito 8
{ 1,1,1,1,0,1,1 }, // = Digito 9
};

int previous_ldr = 0;

int state = LOW_STATE;
int batteryPercentage = 57;
int ldrCount = 0;
int batteryUsage = 0; // quando chegar a mil, perde 1

int isRedActive = 1;
int isYellowActive = 1;
int isGreenActive = 1;
int isResistorActive = 1;

int redKW = 0;
int yellowKW = 0;
int greenKW = 0;
int resistorKW = 0;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "CINGUESTS";
char pass[] = "acessocin";
char serverLogsName[] = "http://172.22.65.102:1200/logs/";
char serverAnalyzeName[] = "http://172.22.65.102:1200/analyze";

BlynkTimer timer;

BlynkTimer fogTimer;

BLYNK_WRITE(V0){
  isRedActive = param.asInt();
}

BLYNK_WRITE(V1){
  isYellowActive = param.asInt();
}

BLYNK_WRITE(V2){
  isGreenActive = param.asInt();
}

BLYNK_WRITE(V3){
  isResistorActive = param.asInt();
}

BLYNK_WRITE(V4){
  int isOn = param.asInt();
  if (isOn) {
    state = ACTIVE_STATE;
  } else {
    state = UNACTIVE_STATE;
  }
}

BLYNK_WRITE(V10){
  analyze();
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  pinMode(LDR, INPUT); //LDR
  pinMode(BUTTON, OUTPUT);

  pinMode(RED, OUTPUT); //vermelho
  pinMode(YELLOW, OUTPUT); //amarelo
  pinMode(GREEN, OUTPUT); //verde

  pinMode(A, OUTPUT);
  pinMode(B, OUTPUT);
  pinMode(C, OUTPUT);
  pinMode(D, OUTPUT);
  pinMode(E, OUTPUT);
  pinMode(FLED, OUTPUT);
  pinMode(G, OUTPUT);

  Serial.begin(115200);
  Serial.println("Começou");

  WiFi.begin(ssid, pass);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  delay(100);
  Blynk.begin(auth,ssid,pass);
  Blynk.virtualWrite(V0, isRedActive);
  Blynk.virtualWrite(V1, isYellowActive);
  Blynk.virtualWrite(V2, isGreenActive);
  Blynk.virtualWrite(V3, isResistorActive);
  Blynk.virtualWrite(V4, !(state == UNACTIVE_STATE));
  
  timer.setInterval(10L, updateUI);
  timer.setInterval(30000L, sendStatus);
}

void loop() {
  Blynk.run();
  timer.run();
}

void sendStatus() {
  httpPOSTRequest();
}

void updateUI() {
  readButton();
  writeDisplay(batteryPercentage/10);
  checkLDR();
  
  switch(state) {
  case ACTIVE_STATE:
    displayActors();
    useBattery();
  break;
  case UNACTIVE_STATE:
    lowActors();
  break;
  case LOW_STATE:
    lowActors();
    checkBattery();
  break;
  }  
}

void readButton() {
  int b_press = digitalRead(BUTTON);
  if (b_press == HIGH) {
    if (state == UNACTIVE_STATE) {
      state = ACTIVE_STATE;
      Blynk.virtualWrite(V4, 1);
    } else {
      state = UNACTIVE_STATE;
      Blynk.virtualWrite(V4, 0);
    }
    delay(500);
  }
}

void checkBattery() {
  if (batteryPercentage > 3) {
    state = ACTIVE_STATE;
    Blynk.virtualWrite(V4, 1);
  }
}

void printBattery() {
  Serial.println("Bateria - " + String(batteryPercentage) + "%");
}

void displayActors() {
  digitalWrite(RED, isRedActive);
  digitalWrite(YELLOW, isYellowActive);
  digitalWrite(GREEN, isGreenActive);
  digitalWrite(RESISTOR, isResistorActive);
}

void lowActors() {
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  digitalWrite(RESISTOR, LOW);
}

void useBattery() {
  if (isRedActive) {
    batteryUsage += 2;
    redKW += 2;
  }
  if (isYellowActive) {
    batteryUsage += 2;
    yellowKW += 2;
  }
  if (isGreenActive) {
    batteryUsage += 1;
    greenKW += 1;
  }
  if (isResistorActive) {
    batteryUsage += 3;
    resistorKW += 3;
  }

  if (batteryUsage >= 1000) {
    batteryPercentage -= 1;
    printBattery();
    batteryUsage = 0;
    writeBatteryValue();
  }

  if (batteryPercentage <= 0) {
    Serial.println("A bateria está muito fraca... Carregue mais!");
     state = LOW_STATE;
  }
}

void writeBatteryValue() {
  Blynk.virtualWrite(V5, batteryPercentage);
  Blynk.virtualWrite(V6, redKW / 1000);
  Blynk.virtualWrite(V7, yellowKW / 1000);
  Blynk.virtualWrite(V8, greenKW / 1000);
  Blynk.virtualWrite(V9, resistorKW / 1000);
}

void writeDisplay(int number) {
  bool* display = sete_segmentos[number];
  digitalWrite(A, display[0]);
  digitalWrite(B, display[1]);
  digitalWrite(C, display[2]);
  digitalWrite(D, display[3]);
  digitalWrite(E, display[4]);
  digitalWrite(FLED, display[5]);
  digitalWrite(G, display[6]);
}

void checkLDR() {
  int ldr_read = analogRead(LDR);
  if(ldr_read > 300) {
    ldrCount += 1;
    if (ldrCount >= 100) {
      if (batteryPercentage < 100) {
        batteryPercentage += 1;
        printBattery();
        writeBatteryValue();
      }
      ldrCount = 0;
    }
  }
  previous_ldr = ldr_read;
}

void httpPOSTRequest() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
  
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverLogsName);

    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    // Data to send with HTTP POST
    String httpRequestData = "{\"redKW\":" + String(redKW) + ",\"yellowKW\":" + String(yellowKW) + ",\"greenKW\":" + String(greenKW) + ",\"resistorKW\":" + String(resistorKW) + "}";
    Serial.println("Enviando " + httpRequestData);

    // If you need an HTTP request with a content type: application/json, use the following:
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(httpRequestData);

    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
   
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
      
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

void analyze() {
  //Check WiFi connection status
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
  
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverAnalyzeName);


    // If you need an HTTP request with a content type: application/json, use the following:
    http.setTimeout(5);
    int httpResponseCode = http.GET();

    // If you need an HTTP request with a content type: text/plain
    //http.addHeader("Content-Type", "text/plain");
    //int httpResponseCode = http.POST("Hello, World!");
   
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
      
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}
