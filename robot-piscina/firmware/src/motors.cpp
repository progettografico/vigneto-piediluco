// motors.cpp — differential drive su DRV8833 con PWM (LEDC).
// Compatibile con Arduino-ESP32 core 2.x e 3.x.
#include <Arduino.h>
#include "motors.h"
#include "config.h"

// --- Wrapper LEDC compatibile fra le versioni del core ------------------------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  static inline void pwmAttach(uint8_t pin) { ledcAttach(pin, PWM_FREQ_HZ, PWM_RES_BITS); }
  static inline void pwmWrite(uint8_t pin, uint32_t duty) { ledcWrite(pin, duty); }
#else
  // Nel core 2.x si usano canali numerici associati ai pin.
  static uint8_t _chOf(uint8_t pin) {
    switch (pin) {
      case PIN_MOT_L_IN1: return 0;
      case PIN_MOT_L_IN2: return 1;
      case PIN_MOT_R_IN1: return 2;
      case PIN_MOT_R_IN2: return 3;
      default: return 0;
    }
  }
  static inline void pwmAttach(uint8_t pin) {
    uint8_t ch = _chOf(pin);
    ledcSetup(ch, PWM_FREQ_HZ, PWM_RES_BITS);
    ledcAttachPin(pin, ch);
  }
  static inline void pwmWrite(uint8_t pin, uint32_t duty) { ledcWrite(_chOf(pin), duty); }
#endif

static const uint32_t PWM_MAX = (1u << PWM_RES_BITS) - 1u;

static inline float clampf(float x, float lo, float hi) {
  return x < lo ? lo : (x > hi ? hi : x);
}

// Pilota un singolo motore con spinta s in [-1..+1] su una coppia di pin DRV8833.
static void driveSide(uint8_t pinFwd, uint8_t pinRev, float s, bool reversed) {
  if (reversed) s = -s;
  s = clampf(s, -1.0f, 1.0f);
  uint32_t duty = (uint32_t)(fabsf(s) * PWM_MAX + 0.5f);
  if (s >= 0) { pwmWrite(pinFwd, duty); pwmWrite(pinRev, 0); }
  else        { pwmWrite(pinFwd, 0);    pwmWrite(pinRev, duty); }
}

void motorsBegin() {
  pwmAttach(PIN_MOT_L_IN1);
  pwmAttach(PIN_MOT_L_IN2);
  pwmAttach(PIN_MOT_R_IN1);
  pwmAttach(PIN_MOT_R_IN2);
  motorsStop();
}

void motorsSetDrive(float v, float w) {
  // Miscela differenziale: sinistra = v + w, destra = v - w.
  float left  = clampf(v + w, -1.0f, 1.0f);
  float right = clampf(v - w, -1.0f, 1.0f);
  driveSide(PIN_MOT_L_IN1, PIN_MOT_L_IN2, left,  MOT_L_REVERSED);
  driveSide(PIN_MOT_R_IN1, PIN_MOT_R_IN2, right, MOT_R_REVERSED);
}

void motorsStop() {
  motorsSetDrive(0.0f, 0.0f);
}
