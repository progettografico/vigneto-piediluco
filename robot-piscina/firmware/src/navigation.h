// navigation.h — macchina a stati di navigazione.
// MODULO PURO: nessuna dipendenza da Arduino, così è testabile sull'host
// (vedi firmware/test/test_navigation.cpp). Il tempo arriva come parametro.
#pragma once
#include <stdint.h>

enum class Mode  { MANUAL, AUTO_COVER, AUTO_EDGE };
enum class State { IDLE, MANUAL, AUTO_COVER, AUTO_EDGE, STOP, LOW_BATTERY };

// Comandi provenienti dall'app (decodificati da comms).
struct NavCommand {
  bool  startReq   = false;          // avvia nella modalità startMode
  Mode  startMode  = Mode::AUTO_COVER;
  bool  stopReq    = false;          // ferma → IDLE
  bool  manualReq  = false;          // pacchetto joystick → forza MANUAL
  float manualV    = 0.0f;           // joystick avanti  [-1..1]
  float manualW    = 0.0f;           // joystick rotazione [-1..1]
  int8_t brushOv   = -1;             // -1 = auto, 0 = off, 1 = on
  int8_t pumpOv    = -1;             // -1 = auto, 0 = off, 1 = on
  bool  commLost   = false;          // watchdog comunicazione scaduto
};

struct NavSensors {
  bool  bumpFront = false;
  bool  bumpSide  = false;
  float distMm    = 8190.0f;         // grande = nessun muro
  float vbat      = 7.4f;
};

struct NavActuators {
  float v     = 0.0f;
  float w     = 0.0f;
  bool  brush = false;
  bool  pump  = false;
};

struct NavContext {
  State    state        = State::IDLE;
  // sotto-stato manovra di evitamento (AUTO_COVER / angoli AUTO_EDGE)
  enum class Avoid { NONE, BACKUP, TURN } avoid = Avoid::NONE;
  uint32_t avoidUntilMs = 0;
  int8_t   turnDir      = 1;          // +1 = destra, -1 = sinistra
  uint32_t rng          = 22695477u;  // stato LCG per angoli pseudo-casuali
  bool     lowLatched   = false;
};

void navInit(NavContext& c, uint32_t seed);

// Esegue un passo della macchina a stati e restituisce i comandi attuatori.
NavActuators navStep(NavContext& c, const NavCommand& cmd,
                     const NavSensors& s, uint32_t nowMs);

const char* stateName(State s);
