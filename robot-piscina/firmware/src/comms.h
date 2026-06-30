// comms.h — WiFi Access Point, server HTTP (UI da LittleFS), WebSocket, JSON.
#pragma once
#include <stdint.h>
#include "navigation.h"

struct Telemetry {
  uint8_t battPct  = 0;
  float   vbat     = 0.0f;
  State   state    = State::IDLE;
  bool    bumpFront = false;
  bool    bumpSide  = false;
  float   distMm   = 0.0f;
};

void commsBegin();
void commsLoop();                         // da chiamare spesso nel loop()

// Restituisce il comando corrente; consuma gli eventi "one-shot" (start/stop).
// nowMs serve per calcolare il watchdog (commLost) sul joystick manuale.
NavCommand commsGetCommand(uint32_t nowMs);

// Invia la telemetria ai client connessi (chiamare ~5 Hz).
void commsSendTelemetry(const Telemetry& t);
