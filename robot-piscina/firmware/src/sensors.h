// sensors.h — lettura paraurti, distanza laterale e batteria.
#pragma once
#include <stdint.h>

struct SensorReading {
  bool  bumpFront;   // true = urto rilevato davanti
  bool  bumpSide;    // true = urto rilevato di lato
  float distMm;      // distanza dal muro laterale (mm); molto grande = nessun muro
  float vbat;        // tensione batteria (V)
  uint8_t battPct;   // percentuale batteria stimata [0..100]
};

void sensorsBegin();
SensorReading sensorsRead();

// Percentuale batteria da tensione (lineare fra VBAT_EMPTY e VBAT_FULL).
uint8_t batteryPercent(float vbat);
