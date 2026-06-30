// sensors.cpp — paraurti (micro-switch), distanza laterale (VL53L0X), batteria (ADC).
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>
#include "sensors.h"
#include "config.h"

static VL53L0X tof;
static bool    tofOk = false;

// Filtro esponenziale sulla tensione batteria (riduce il rumore dell'ADC).
static float vbatFilt = VBAT_FULL;

void sensorsBegin() {
  pinMode(PIN_BUMP_FRONT, INPUT);   // pull-up esterno 10k (GPIO34/35 senza interno)
  pinMode(PIN_BUMP_SIDE,  INPUT);

  analogReadResolution(12);
  analogSetPinAttenuation(PIN_VBAT_SENSE, ADC_11db);  // range pieno ~0..3.3 V

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  tof.setTimeout(100);
  tofOk = tof.init();
  if (tofOk) {
    tof.startContinuous();
  }

  // Inizializza il filtro con una lettura reale.
  int raw = analogRead(PIN_VBAT_SENSE);
  float vadc = (raw / ADC_MAX) * ADC_VREF;
  vbatFilt = vadc / VBAT_DIVIDER;
}

static bool readBump(uint8_t pin) {
  int level = digitalRead(pin);
  return BUMP_ACTIVE_LOW ? (level == LOW) : (level == HIGH);
}

uint8_t batteryPercent(float vbat) {
  float pct = (vbat - VBAT_EMPTY) / (VBAT_FULL - VBAT_EMPTY) * 100.0f;
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return (uint8_t)(pct + 0.5f);
}

SensorReading sensorsRead() {
  SensorReading r;
  r.bumpFront = readBump(PIN_BUMP_FRONT);
  r.bumpSide  = readBump(PIN_BUMP_SIDE);

  // Distanza laterale.
  if (tofOk) {
    uint16_t mm = tof.readRangeContinuousMillimeters();
    r.distMm = tof.timeoutOccurred() ? 8190.0f : (float)mm;
  } else {
    r.distMm = 8190.0f;   // sensore assente → "nessun muro"
  }

  // Batteria (con filtro).
  int raw = analogRead(PIN_VBAT_SENSE);
  float vadc = (raw / ADC_MAX) * ADC_VREF;
  float v = vadc / VBAT_DIVIDER;
  vbatFilt += 0.20f * (v - vbatFilt);   // EMA
  r.vbat = vbatFilt;
  r.battPct = batteryPercent(vbatFilt);

  return r;
}
