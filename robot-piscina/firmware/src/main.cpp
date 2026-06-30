// main.cpp — Robot Piscina: orchestrazione di sensori, navigazione e attuatori.
#include <Arduino.h>
#include <esp_random.h>
#include "config.h"
#include "motors.h"
#include "cleaning.h"
#include "sensors.h"
#include "navigation.h"
#include "comms.h"

static NavContext nav;
static uint32_t    lastTelemetryMs = 0;

static void updateStatusLed(State s, uint32_t nowMs) {
  // idle: lampeggio lento · attivo: acceso · low battery: lampeggio veloce
  uint32_t period;
  switch (s) {
    case State::LOW_BATTERY: period = 150;  break;
    case State::IDLE:        period = 1000; break;
    case State::STOP:        period = 1000; break;
    default:                 period = 0;    break;  // acceso fisso
  }
  bool on = (period == 0) ? true : ((nowMs / period) & 1);
  digitalWrite(PIN_LED_STATUS, on ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[main] Robot Piscina avvio...");

  pinMode(PIN_LED_STATUS, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  motorsBegin();
  cleaningBegin();
  sensorsBegin();
  navInit(nav, esp_random());
  commsBegin();

  Serial.println("[main] pronto. Stato iniziale: IDLE.");
}

void loop() {
  uint32_t nowMs = millis();
  commsLoop();

  // 1) Sensori
  SensorReading sr = sensorsRead();
  NavSensors ns;
  ns.bumpFront = sr.bumpFront;
  ns.bumpSide  = sr.bumpSide;
  ns.distMm    = sr.distMm;
  ns.vbat      = sr.vbat;

  // 2) Comandi dall'app
  NavCommand cmd = commsGetCommand(nowMs);

  // 3) Navigazione (macchina a stati)
  NavActuators act = navStep(nav, cmd, ns, nowMs);

  // 4) Attuatori
  motorsSetDrive(act.v, act.w);
  brushSet(act.brush);
  pumpSet(act.pump);
  updateStatusLed(nav.state, nowMs);

  // 5) Telemetria
  if ((uint32_t)(nowMs - lastTelemetryMs) >= TELEMETRY_MS) {
    lastTelemetryMs = nowMs;
    Telemetry t;
    t.battPct   = sr.battPct;
    t.vbat      = sr.vbat;
    t.state     = nav.state;
    t.bumpFront = sr.bumpFront;
    t.bumpSide  = sr.bumpSide;
    t.distMm    = sr.distMm;
    commsSendTelemetry(t);
  }

  delay(10);   // ~100 Hz di ciclo di controllo
}
