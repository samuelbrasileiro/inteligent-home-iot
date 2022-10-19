#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define ACTIVE_STATE 0
#define LOW_STATE 1
#define UNACTIVE_STATE 2

#define LDR 33
#define RED 23
#define GREEN 18
#define YELLOW 19

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
int batteryPercentage = 0;
int ldrCount = 0;
int batteryUsage = 0; // quando chegar a mil, perde 1

int isRedActive = 1;
int isYellowActive = 1;
int isGreenActive = 1;
int isResistorActive = 1;

byte server[] = {192, 168, 15, 11};

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

  WiFi.begin("CINGUEST", "acessocin");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to the wifi");
  Serial.println("Press the button if you want to initialize the security system");
}

void loop() {
  // put your main code here, to run repeatedly:
  readButton();
  writeDisplay(batteryPercentage/100);
  checkLDR();
  //watchServer();
  
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
  break;
  }
  delay(10);
}

void readButton() {
  int b_press = digitalRead(BUTTON);
  if (b_press == HIGH) {
    state = ACTIVE_STATE;
    delay(500);
  }
}

void displayActors() {
  digitalWrite(RED, isRedActive);
  digitalWrite(YELLOW, isYellowActive);
  digitalWrite(GREEN, isGreenActive);
  //digitalWrite(RESISTOR, isResistorActive);
}

void lowActors() {
  digitalWrite(RED, LOW);
  digitalWrite(YELLOW, LOW);
  digitalWrite(GREEN, LOW);
  //digitalWrite(RESISTOR, LOW);
}

void useBattery() {
  if (isRedActive) {
    batteryUsage += 1;
  }
  if (isYellowActive) {
    batteryUsage += 1;
  }
  if (isGreenActive) {
    batteryUsage += 1;
  }
  if (isResistorActive) {
    batteryUsage += 3;
  }

  if (batteryUsage >= 1000) {
    batteryPercentage -= 1;
    batteryUsage = 0;
  }

  if (batteryPercentage <= 0) {
     state = LOW_STATE;
  }
}

void writeBatteryValue() {
      WiFiClient cliente;
      bool st = cliente.connect(server, 2045);
      cliente.println("SETBATTERYVALUE "+ String(batteryPercentage));
      while (cliente.available() == 0) {
        continue;
      }
      String reply = "";
      while (cliente.available()) {
        reply += (char)  cliente.read();
      }  
}

void watchServer() {
      WiFiClient cliente;
      bool st = cliente.connect(server, 2045);
      
      while (cliente.available() == 0) {
        continue;
      }
      String reply = "";
      while (cliente.available()) {
        reply += (char)  cliente.read();
      }

      if (reply == "SET RED 0") {
        isRedActive = 0;
      } else if (reply == "SET RED 1") {
        isRedActive = 1;
      } else if (reply == "SET YELLOW 0") {
        isYellowActive = 0;
      } else if (reply == "SET YELLOW 1") {
        isYellowActive = 1;
      } else if (reply == "TURN OFF") {
        state = UNACTIVE_STATE;
      } else if (reply == "TURN ON") {
        state = ACTIVE_STATE;
      }
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
  if(ldr_read - previous_ldr > 300) {
    ldrCount += 1;
    if (ldrCount >= 100) {
      batteryPercentage += 1;
      ldrCount = 0;
      writeBatteryValue();
    }
  }
  previous_ldr = ldr_read;
}

void reset() {
  digitalWrite(A, 0);
  digitalWrite(B, 0);
  digitalWrite(C, 0);
  digitalWrite(D, 0);
  digitalWrite(E, 0);
  digitalWrite(FLED, 0);
  digitalWrite(G, 0);
}
