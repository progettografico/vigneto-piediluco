// config.h — pin e costanti del Robot Piscina.
// Fonte di verità per il pinout: vedi docs/03-bom.md (tabella pin).
#pragma once

// ---------------------------------------------------------------------------
// Pin motori di propulsione (DRV8833 #1) — PWM
// ---------------------------------------------------------------------------
#define PIN_MOT_L_IN1   25   // Motore SX avanti
#define PIN_MOT_L_IN2   26   // Motore SX indietro
#define PIN_MOT_R_IN1   27   // Motore DX avanti
#define PIN_MOT_R_IN2   14   // Motore DX indietro

// Inverti se in collaudo un motore gira al contrario (vedi 04-montaggio F.3)
#define MOT_L_REVERSED  false
#define MOT_R_REVERSED  false

// ---------------------------------------------------------------------------
// Pin utenze di pulizia (DRV8833 #2)
// ---------------------------------------------------------------------------
#define PIN_BRUSH_IN1   16   // Spazzola laterale
#define PIN_BRUSH_IN2   17
#define PIN_PUMP_IN3    18   // Pompa skimmer (opzionale)
#define PIN_PUMP_IN4    19

// ---------------------------------------------------------------------------
// Sensori
// ---------------------------------------------------------------------------
#define PIN_I2C_SDA     21   // VL53L0X
#define PIN_I2C_SCL     22
#define PIN_BUMP_FRONT  34   // micro-switch (solo input, pull-up esterno 10k)
#define PIN_BUMP_SIDE   35   // micro-switch (solo input, pull-up esterno 10k)
#define PIN_VBAT_SENSE  33   // ADC1, partitore di tensione
#define PIN_LED_STATUS  2
#define PIN_BUZZER      4

// Micro-switch premuto = livello basso (a massa con pull-up)
#define BUMP_ACTIVE_LOW  true

// ---------------------------------------------------------------------------
// PWM
// ---------------------------------------------------------------------------
#define PWM_FREQ_HZ     20000   // 20 kHz: silenzioso, fuori dall'udibile
#define PWM_RES_BITS    8       // 0..255

// ---------------------------------------------------------------------------
// Batteria (2S LiPo)
// ---------------------------------------------------------------------------
#define VBAT_DIVIDER    0.248f  // R2/(R1+R2)=33k/133k — TARA col multimetro
#define ADC_VREF        3.30f   // tensione di riferimento ADC (V)
#define ADC_MAX         4095.0f // ADC a 12 bit
#define VBAT_LOW        6.60f   // stop di sicurezza (3.3 V/cella)
#define VBAT_FULL       8.40f   // 4.2 V/cella
#define VBAT_EMPTY      6.40f   // per il calcolo percentuale

// ---------------------------------------------------------------------------
// Navigazione / comportamento
// ---------------------------------------------------------------------------
#define CRUISE_SPEED    0.60f   // velocità di crociera [0..1]
#define TURN_SPEED      0.55f   // velocità di rotazione [0..1]
#define BACKUP_SPEED    0.45f   // velocità di retromarcia [0..1]

#define BUMP_BACKUP_MS  700     // durata retromarcia dopo urto
#define TURN_MIN_MS     500     // rotazione minima dopo urto
#define TURN_MAX_MS     1100    // rotazione massima (angolo pseudo-casuale)
#define WALL_NEAR_MM    180     // muro "vicino" in AUTO_COVER → evita

#define EDGE_TARGET_MM  250     // distanza dal bordo in AUTO_EDGE
#define EDGE_KP         0.0040f // guadagno proporzionale (per mm di errore)
#define EDGE_SPEED      0.40f   // avanzamento in inseguimento bordo

// ---------------------------------------------------------------------------
// Comunicazione / rete
// ---------------------------------------------------------------------------
#define AP_SSID         "RobotPiscina"
#define AP_PASSWORD     "piscina123"   // CAMBIALA (min 8 caratteri)
#define WS_PORT         81
#define HTTP_PORT       80
#define COMM_TIMEOUT_MS 1000    // watchdog: in MANUAL ferma i motori se l'app tace
#define TELEMETRY_MS    200     // invio telemetria (~5 Hz)
