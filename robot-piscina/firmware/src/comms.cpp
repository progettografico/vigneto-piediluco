// comms.cpp — Access Point WiFi + HTTP (serve la UI) + WebSocket (realtime JSON).
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "comms.h"
#include "config.h"

static WebServer        http(HTTP_PORT);
static WebSocketsServer ws(WS_PORT);

// Stato comandi aggiornato dai messaggi WebSocket.
static struct CmdState {
  bool     startReq    = false;
  bool     stopReq     = false;
  bool     manualReq   = false;
  Mode     startMode   = Mode::AUTO_COVER;
  float    manualV     = 0.0f;
  float    manualW     = 0.0f;
  int8_t   brushOv     = -1;
  int8_t   pumpOv      = -1;
  uint32_t lastManualMs = 0;
} st;

static Mode modeFromStr(const char* v) {
  if (!v) return Mode::AUTO_COVER;
  if (!strcmp(v, "manual"))     return Mode::MANUAL;
  if (!strcmp(v, "auto_edge"))  return Mode::AUTO_EDGE;
  return Mode::AUTO_COVER;
}

static const char* contentType(const String& path) {
  if (path.endsWith(".html")) return "text/html";
  if (path.endsWith(".js"))   return "application/javascript";
  if (path.endsWith(".css"))  return "text/css";
  if (path.endsWith(".json")) return "application/json";
  if (path.endsWith(".png"))  return "image/png";
  if (path.endsWith(".svg"))  return "image/svg+xml";
  return "text/plain";
}

static void handleFile() {
  String path = http.uri();
  if (path == "/") path = "/index.html";
  if (LittleFS.exists(path)) {
    File f = LittleFS.open(path, "r");
    http.streamFile(f, contentType(path));
    f.close();
  } else {
    http.send(404, "text/plain", "Not found");
  }
}

// Decodifica un messaggio JSON ricevuto dall'app.
static void onMessage(uint8_t* payload, size_t len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) return;   // JSON non valido → ignora
  const char* cmd = doc["cmd"] | "";

  if (!strcmp(cmd, "manual")) {
    st.manualReq    = true;
    st.manualV      = doc["vx"] | 0.0f;
    st.manualW      = doc["w"]  | 0.0f;
    st.lastManualMs = millis();
  } else if (!strcmp(cmd, "start")) {
    st.startReq   = true;
    st.startMode  = modeFromStr(doc["mode"] | "auto_cover");
  } else if (!strcmp(cmd, "mode")) {
    st.startReq   = true;                            // cambia modalità = riavvia in quella modalità
    st.startMode  = modeFromStr(doc["value"] | "auto_cover");
  } else if (!strcmp(cmd, "stop")) {
    st.stopReq = true;
  } else if (!strcmp(cmd, "brush")) {
    st.brushOv = (doc["on"] | false) ? 1 : 0;
  } else if (!strcmp(cmd, "pump")) {
    st.pumpOv  = (doc["on"] | false) ? 1 : 0;
  }
}

static void onWsEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t len) {
  if (type == WStype_TEXT) onMessage(payload, len);
}

void commsBegin() {
  if (!LittleFS.begin(true)) {
    Serial.println("[comms] LittleFS mount fallito");
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);
  Serial.print("[comms] AP \"" AP_SSID "\" IP: ");
  Serial.println(WiFi.softAPIP());

  http.onNotFound(handleFile);
  http.on("/", handleFile);
  http.begin();

  ws.begin();
  ws.onEvent(onWsEvent);
}

void commsLoop() {
  http.handleClient();
  ws.loop();
}

NavCommand commsGetCommand(uint32_t nowMs) {
  NavCommand c;
  c.startReq  = st.startReq;
  c.startMode = st.startMode;
  c.stopReq   = st.stopReq;
  c.manualReq = st.manualReq;
  c.manualV   = st.manualV;
  c.manualW   = st.manualW;
  c.brushOv   = st.brushOv;
  c.pumpOv    = st.pumpOv;
  // Watchdog: se non arrivano pacchetti joystick da troppo tempo, segnala perdita.
  c.commLost  = (st.lastManualMs == 0) ||
                ((uint32_t)(nowMs - st.lastManualMs) > COMM_TIMEOUT_MS);

  // Consuma gli eventi one-shot (così scattano una sola volta).
  st.startReq  = false;
  st.stopReq   = false;
  st.manualReq = false;
  return c;
}

void commsSendTelemetry(const Telemetry& t) {
  if (ws.connectedClients() == 0) return;
  JsonDocument doc;
  doc["batt"]  = t.battPct;
  doc["vbat"]  = serialized(String(t.vbat, 2));
  doc["state"] = stateName(t.state);
  doc["bumpF"] = t.bumpFront;
  doc["bumpS"] = t.bumpSide;
  doc["dist"]  = (int)t.distMm;
  char buf[160];
  size_t n = serializeJson(doc, buf, sizeof(buf));
  ws.broadcastTXT(buf, n);
}
