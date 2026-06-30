// test_navigation.cpp — test della logica pura di navigazione, eseguibile sull'host.
// Compila SENZA Arduino:
//   g++ -std=c++17 -I ../src test_navigation.cpp ../src/navigation.cpp -o nav_test && ./nav_test
// Non fa parte della build PlatformIO (cartella non riservata).
#include "navigation.h"
#include "config.h"
#include <cstdio>
#include <cmath>

static int failed = 0;
static void check(bool cond, const char* msg) {
  printf(" [%s] %s\n", cond ? "PASS" : "FAIL", msg);
  if (!cond) failed++;
}

int main() {
  NavContext c; navInit(c, 12345);
  NavSensors s;            // batteria ok, nessun muro
  s.vbat = 7.4f;

  // 1) Stato iniziale IDLE → fermo.
  {
    NavCommand cmd;
    NavActuators a = navStep(c, cmd, s, 0);
    check(c.state == State::IDLE, "parte in IDLE");
    check(a.v == 0 && a.w == 0 && !a.brush && !a.pump, "IDLE: tutto fermo");
  }

  // 2) start AUTO_COVER → crociera dritta, pompa attiva.
  {
    NavCommand cmd; cmd.startReq = true; cmd.startMode = Mode::AUTO_COVER;
    NavActuators a = navStep(c, cmd, s, 100);
    check(c.state == State::AUTO_COVER, "start → AUTO_COVER");
    check(std::fabs(a.v - CRUISE_SPEED) < 1e-4 && std::fabs(a.w) < 1e-4, "AUTO_COVER: avanti dritto");
    check(a.pump, "AUTO_COVER: pompa skimmer attiva");
  }

  // 3) Urto frontale → retromarcia, poi rotazione, poi riprende.
  {
    NavCommand cmd; NavSensors hit = s; hit.bumpFront = true;
    NavActuators a = navStep(c, cmd, hit, 200);
    check(a.v < 0, "dopo urto: retromarcia");
    // durante la retromarcia resta negativo
    a = navStep(c, cmd, s, 200 + BUMP_BACKUP_MS - 50);
    check(a.v < 0, "retromarcia in corso");
    // finita la retromarcia → rotazione (v~0, w!=0)
    a = navStep(c, cmd, s, 200 + BUMP_BACKUP_MS + 10);
    check(std::fabs(a.v) < 1e-4 && std::fabs(a.w) > 1e-4, "dopo retromarcia: rotazione");
    // molto dopo → torna a crociera
    a = navStep(c, cmd, s, 200 + BUMP_BACKUP_MS + TURN_MAX_MS + 100);
    check(std::fabs(a.v - CRUISE_SPEED) < 1e-4, "rotazione finita: torna a crociera");
  }

  // 4) Override manuale: forza MANUAL e guida; commLost ferma.
  {
    NavCommand cmd; cmd.manualReq = true; cmd.manualV = 0.5f; cmd.manualW = -0.3f;
    NavActuators a = navStep(c, cmd, s, 5000);
    check(c.state == State::MANUAL, "joystick → MANUAL");
    check(std::fabs(a.v - 0.5f) < 1e-4 && std::fabs(a.w + 0.3f) < 1e-4, "MANUAL: segue il joystick");
    NavCommand lost; lost.commLost = true;     // nessun nuovo pacchetto
    a = navStep(c, lost, s, 5100);
    check(a.v == 0 && a.w == 0, "watchdog: comm persa → ferma");
  }

  // 5) AUTO_EDGE: controllo proporzionale (segno corretto).
  {
    navInit(c, 7); s.vbat = 7.4f;
    NavCommand start; start.startReq = true; start.startMode = Mode::AUTO_EDGE;
    NavSensors far = s; far.distMm = EDGE_TARGET_MM + 200;   // troppo lontano dal muro
    NavActuators a = navStep(c, start, far, 0);
    check(c.state == State::AUTO_EDGE, "start → AUTO_EDGE");
    check(a.brush, "AUTO_EDGE: spazzola attiva");
    check(a.w > 0, "troppo lontano → gira verso il muro (w>0)");
    NavCommand none;
    NavSensors near = s; near.distMm = EDGE_TARGET_MM - 200; // troppo vicino
    a = navStep(c, none, near, 50);
    check(a.w < 0, "troppo vicino → si allontana (w<0)");
  }

  // 6) Batteria scarica → LOW_BATTERY con latch.
  {
    navInit(c, 1);
    NavCommand run; run.startReq = true; run.startMode = Mode::AUTO_COVER;
    NavSensors low = s; low.vbat = VBAT_LOW - 0.2f;
    NavActuators a = navStep(c, run, low, 0);
    check(c.state == State::LOW_BATTERY, "Vbatt bassa → LOW_BATTERY");
    check(a.v == 0 && a.w == 0 && !a.brush && !a.pump, "LOW_BATTERY: tutto spento");
    // anche se la tensione risale, resta in latch
    NavCommand again; again.startReq = true;
    a = navStep(c, again, s, 100);
    check(c.state == State::LOW_BATTERY, "LOW_BATTERY: latch mantenuto");
  }

  // 7) stop → IDLE.
  {
    navInit(c, 2);
    NavCommand st; st.startReq = true; st.startMode = Mode::AUTO_COVER;
    navStep(c, st, s, 0);
    NavCommand stop; stop.stopReq = true;
    navStep(c, stop, s, 10);            // STOP transitorio
    NavCommand none;
    navStep(c, none, s, 20);            // → IDLE
    check(c.state == State::IDLE, "stop → IDLE");
  }

  printf("\nRisultato: %s (%d fallimenti)\n", failed ? "FALLITO" : "OK", failed);
  return failed ? 1 : 0;
}
