#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"


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
}

void loop() {
  // put your main code here, to run repeatedly:

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
  if(state == RED_STATE && ldr_read - previous_ldr > 300) {
    redCount++;
    Blynk.virtualWrite(V3, redCount);
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
