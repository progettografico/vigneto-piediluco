// navigation.cpp — implementazione della macchina a stati (modulo puro).
#include "navigation.h"
#include "config.h"
#include <math.h>

static inline float clampf(float x, float lo, float hi) {
  return x < lo ? lo : (x > hi ? hi : x);
}

// LCG deterministico: numero pseudo-casuale (testabile, niente Arduino).
static uint32_t rngNext(NavContext& c) {
  c.rng = c.rng * 1664525u + 1013904223u;
  return c.rng;
}
static int8_t randSign(NavContext& c) {
  return (rngNext(c) & 0x10000u) ? 1 : -1;
}
static uint32_t randTurnMs(NavContext& c) {
  uint32_t span = (uint32_t)(TURN_MAX_MS - TURN_MIN_MS);
  return TURN_MIN_MS + (rngNext(c) % (span + 1u));
}

void navInit(NavContext& c, uint32_t seed) {
  c = NavContext{};
  if (seed != 0) c.rng = seed;
}

const char* stateName(State s) {
  switch (s) {
    case State::IDLE:        return "idle";
    case State::MANUAL:      return "manual";
    case State::AUTO_COVER:  return "auto_cover";
    case State::AUTO_EDGE:   return "auto_edge";
    case State::STOP:        return "stop";
    case State::LOW_BATTERY: return "low_battery";
  }
  return "?";
}

// Avvia (o prosegue) una manovra di evitamento: retromarcia poi rotazione.
static void startAvoid(NavContext& c, const NavSensors& s, uint32_t nowMs) {
  c.avoid = NavContext::Avoid::BACKUP;
  c.avoidUntilMs = nowMs + BUMP_BACKUP_MS;
  // Direzione di rotazione: se l'ostacolo è sul lato (o muro vicino) gira via,
  // altrimenti scegli un verso pseudo-casuale per variare il percorso.
  if (s.bumpSide || s.distMm < WALL_NEAR_MM) c.turnDir = -1;  // muro/lato a destra → gira a sinistra
  else                                       c.turnDir = randSign(c);
}

// Esegue la manovra di evitamento in corso; ritorna true finché è attiva.
static bool runAvoid(NavContext& c, NavActuators& a, uint32_t nowMs) {
  if (c.avoid == NavContext::Avoid::BACKUP) {
    if ((int32_t)(nowMs - c.avoidUntilMs) >= 0) {
      c.avoid = NavContext::Avoid::TURN;
      c.avoidUntilMs = nowMs + randTurnMs(c);
    } else {
      a.v = -BACKUP_SPEED; a.w = 0.0f;
      return true;
    }
  }
  if (c.avoid == NavContext::Avoid::TURN) {
    if ((int32_t)(nowMs - c.avoidUntilMs) >= 0) {
      c.avoid = NavContext::Avoid::NONE;
    } else {
      a.v = 0.0f; a.w = (float)c.turnDir * TURN_SPEED;
      return true;
    }
  }
  return false;
}

// Applica le forzature manuali di spazzola/pompa (override dall'app).
static void applyOverrides(const NavCommand& cmd, NavActuators& a) {
  if (cmd.brushOv >= 0) a.brush = (cmd.brushOv == 1);
  if (cmd.pumpOv  >= 0) a.pump  = (cmd.pumpOv  == 1);
}

NavActuators navStep(NavContext& c, const NavCommand& cmd,
                     const NavSensors& s, uint32_t nowMs) {
  NavActuators a;   // default: tutto fermo/spento

  // 1) Sicurezza batteria (latch). Lettura valida solo se > 3 V (altrimenti
  //    sensore scollegato → non far scattare un falso allarme).
  if (c.lowLatched || (s.vbat > 3.0f && s.vbat < VBAT_LOW)) {
    c.lowLatched = true;
    c.state = State::LOW_BATTERY;
    return a;   // motori e utenze a zero
  }

  // 2) Stop comandato.
  if (cmd.stopReq) {
    c.state = State::STOP;
  }

  // 3) Override manuale (priorità massima): un pacchetto joystick forza MANUAL.
  if (cmd.manualReq) {
    c.state = State::MANUAL;
  }

  // 4) Avvio in una modalità.
  if (cmd.startReq) {
    switch (cmd.startMode) {
      case Mode::MANUAL:     c.state = State::MANUAL;     break;
      case Mode::AUTO_COVER: c.state = State::AUTO_COVER; break;
      case Mode::AUTO_EDGE:  c.state = State::AUTO_EDGE;  break;
    }
    c.avoid = NavContext::Avoid::NONE;
  }

  // 5) Comportamento per stato.
  switch (c.state) {
    case State::STOP:
      // Transitorio: ferma e torna IDLE al passo successivo.
      c.state = State::IDLE;
      break;

    case State::IDLE:
    case State::LOW_BATTERY:
      break;   // tutto a zero

    case State::MANUAL:
      if (cmd.commLost) { a.v = 0; a.w = 0; }   // watchdog: comunicazione persa
      else { a.v = clampf(cmd.manualV, -1.f, 1.f); a.w = clampf(cmd.manualW, -1.f, 1.f); }
      applyOverrides(cmd, a);
      break;

    case State::AUTO_COVER:
      a.pump = true;                  // skimmer attivo
      if (c.avoid == NavContext::Avoid::NONE &&
          (s.bumpFront || s.bumpSide || s.distMm < WALL_NEAR_MM)) {
        startAvoid(c, s, nowMs);
      }
      if (!runAvoid(c, a, nowMs)) {
        a.v = CRUISE_SPEED; a.w = 0.0f;   // crociera dritta
      }
      applyOverrides(cmd, a);
      break;

    case State::AUTO_EDGE:
      a.brush = true; a.pump = true;
      if (c.avoid == NavContext::Avoid::NONE && s.bumpFront) {
        startAvoid(c, s, nowMs);        // angolo della piscina
      }
      if (!runAvoid(c, a, nowMs)) {
        // Inseguimento proporzionale del bordo a distanza target.
        float err = s.distMm - (float)EDGE_TARGET_MM;   // >0 = troppo lontano
        a.v = EDGE_SPEED;
        a.w = clampf(EDGE_KP * err, -TURN_SPEED, TURN_SPEED);  // gira verso il muro
      }
      applyOverrides(cmd, a);
      break;
  }

  return a;
}
