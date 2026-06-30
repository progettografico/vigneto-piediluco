// cleaning.cpp — spazzola e pompa pilotate on/off via DRV8833 #2.
// Per semplicità on/off a piena velocità (un solo verso); basta per la pulizia.
#include <Arduino.h>
#include "cleaning.h"
#include "config.h"

void cleaningBegin() {
  pinMode(PIN_BRUSH_IN1, OUTPUT);
  pinMode(PIN_BRUSH_IN2, OUTPUT);
  pinMode(PIN_PUMP_IN3, OUTPUT);
  pinMode(PIN_PUMP_IN4, OUTPUT);
  cleaningStop();
}

void brushSet(bool on) {
  digitalWrite(PIN_BRUSH_IN1, on ? HIGH : LOW);
  digitalWrite(PIN_BRUSH_IN2, LOW);   // IN1=1,IN2=0 → avanti ; 0,0 → ferma
}

void pumpSet(bool on) {
  digitalWrite(PIN_PUMP_IN3, on ? HIGH : LOW);
  digitalWrite(PIN_PUMP_IN4, LOW);
}

void cleaningStop() {
  brushSet(false);
  pumpSet(false);
}
