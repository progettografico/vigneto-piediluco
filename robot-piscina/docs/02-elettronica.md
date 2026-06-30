# 02 — Elettronica e cablaggio

> Schema visivo: **[wiring-diagram.svg](wiring-diagram.svg)**.
> Mappatura pin completa: vedi tabella in **[03-bom.md](03-bom.md)** (replicata in
> `firmware/src/config.h`).

## 1. Schema a blocchi

```
  ☀ Pannello   ┌────────────────────────┐
  solare 6–10W ▶│ Regolatore carica MPPT  │──▶ ricarica la 2S LiPo (autonomo)
               └────────────────────────┘
                 ┌──────────────────────────────┐
   2S LiPo ──┬──▶│  Interruttore + Fusibile 3–5A │
  (7.4 V)    │   └───────────────┬───────────────┘
             │                   │ Vbatt (7.4 V)
             │        ┌──────────┴───────────┐
             │        ▼                      ▼
             │  ┌───────────┐        ┌────────────────┐
             │  │ Buck 5 V  │        │  DRV8833 #1     │──▶ Motore SX (elica)
             │  │ (MP1584)  │        │  (propulsione)  │──▶ Motore DX (elica)
             │  └─────┬─────┘        └────────────────┘
             │        │ 5 V                  ▲ PWM (GPIO 25/26/27/14)
             │        ▼                      │
             │  ┌─────────────┐              │
             │  │   ESP32     │──────────────┘
             │  │  DevKit     │──▶ DRV8833 #2 (spazzola GPIO16/17, pompa GPIO18/19)
             │  │             │──▶ VL53L0X  (I²C: SDA 21 / SCL 22)
             │  │             │◀── Bump front/side (GPIO 34/35 + pull-up 10k)
             │  │             │◀── VBAT_SENSE (GPIO 33 ← partitore)
             │  │             │──▶ LED (GPIO2) / Buzzer (GPIO4)
             │  └─────────────┘
             └──────────────────────▶ partitore di tensione (vedi §4)
```

## 2. Alimentazione

- **Batteria**: 2S LiPo 7.4 V (o 2×18650 in serie). Connettore XT30/JST.
- **Linea motori**: i motori di spinta vanno alimentati **direttamente da Vbatt**
  (7.4 V) tramite il DRV8833, **non** dai 5 V dell'ESP32.
- **Linea logica**: un convertitore **buck step-down a 5 V** (MP1584/LM2596) alimenta
  l'ESP32 dal pin **5V/VIN** e la logica dei driver/sensori (3.3 V li fornisce l'ESP32).
- **Massa comune**: GND di batteria, buck, driver, ESP32 e sensori **tutti collegati
  insieme** (indispensabile per i segnali PWM e I²C).
- **Protezione**: interruttore generale + fusibile 3–5 A subito dopo il "+" batteria.

> ⚠️ Verifica la corrente di stallo dei motori: deve stare **sotto** il limite del
> DRV8833 (~1.5 A/canale continui, 2 A picco). Per motori più potenti usa un driver
> più grande (es. TB6612 o BTS7960) — vedi alternative in BOM.

## 3. Driver motori (DRV8833)

Ogni DRV8833 pilota 2 motori con 2 segnali PWM ciascuno (IN1/IN2, IN3/IN4):

| IN1 (o IN3) | IN2 (o IN4) | Effetto |
|---|---|---|
| PWM | 0 | Avanti (velocità = duty) |
| 0 | PWM | Indietro |
| 0 | 0 | Coast (ruota libera) |
| 1 | 1 | Freno |

- **#1 propulsione**: AIN1/AIN2 = GPIO 25/26 (motore SX), BIN1/BIN2 = GPIO 27/14 (DX).
- **#2 utenze**: IN1/IN2 = GPIO 16/17 (spazzola), IN3/IN4 = GPIO 18/19 (pompa).
- Collega `VM`=Vbatt, `VCC`=3.3 V (logica), `GND` comune, `STBY`/`EEP` a 3.3 V (attivo).

## 4. Lettura batteria (partitore di tensione)

L'ADC dell'ESP32 legge **max 3.3 V**, ma la batteria arriva a ~8.4 V → serve un
partitore che dimezzi (o meno) la tensione su `VBAT_SENSE` (GPIO33):

```
 Vbatt ──[ R1 = 100 kΩ ]──┬──[ R2 = 33 kΩ ]── GND
                          │
                          └──▶ GPIO33 (VBAT_SENSE)
```

Con R1=100k, R2=33k → fattore = 33/(100+33) ≈ **0.248**.
A 8.4 V → ~2.08 V all'ADC (sotto i 3.3 V, sicuro). Nel firmware:
`Vbatt = Vadc / 0.248` (costante `VBAT_DIVIDER` in `config.h`, da tarare col multimetro).

> Aggiungi un piccolo condensatore (100 nF) tra GPIO33 e GND per stabilizzare la lettura.

## 5. Sensori

- **VL53L0X (ToF, I²C)**: SDA→GPIO21, SCL→GPIO22, VCC→3.3 V, GND comune. Montato sul
  **fianco** del robot per misurare la distanza dal muro (inseguimento bordo).
  Alternativa economica: **HC-SR04** ad ultrasuoni (tienilo **sopra** il pelo
  dell'acqua; richiede partitore sul pin ECHO perché emette 5 V).
- **Micro-switch paraurti**: un capo a **GND**, l'altro al GPIO (34 front, 35 side).
  Poiché GPIO34/35 **non hanno pull-up interno**, aggiungi **pull-up esterno 10 kΩ**
  verso 3.3 V su ciascun ingresso. Premuto = livello **basso**.

## 6. Tenuta stagna (regole elettriche)
- Tutta l'elettronica nel **box IP65**; i cavi escono da **pressacavi** sigillati.
- Stagna i motori (sono semi-sommersi): usa motori/pompe **waterproof** o sigilla
  l'albero con grasso al silicone; isola le saldature con guaina termorestringente.
- **Conformal coating** su ESP32 e driver per resistere all'umidità/condensa.
- Porta di **ricarica** esterna con tappo: vedi 04-montaggio e 05-sicurezza.

## 6.bis Ricarica solare ☀

Il **pannello solare** (sul ponte) alimenta un **regolatore di carica MPPT per 2S
Li-ion/LiPo**, che ricarica autonomamente la batteria:

```
Pannello (6–10 W) ──[diodo Schottky]──▶ IN  Regolatore MPPT 2S  BAT ──▶ pacco 2S LiPo
                                              (gestisce tensione/corrente di carica)
```

- Collega il **+** del pannello all'ingresso del regolatore (con **diodo di blocco**
  Schottky se non già integrato, per non scaricare la batteria nel pannello al buio),
  e l'uscita BAT del regolatore **in parallelo al pacco batteria**.
- **Importante**: il regolatore deve essere del tipo giusto per la **chimica e il numero
  di celle** della batteria (2S Li-ion/LiPo). Non collegare un pannello direttamente
  alla batteria senza regolatore.
- La ricarica è **indipendente dal firmware**: l'ESP32 continua a leggere solo la
  tensione della batteria. Se vuoi mostrare lo stato "in carica" nell'app, aggiungi un
  partitore dal pannello a `VSOLAR_SENSE` (GPIO32) e leggine l'ADC (estensione opzionale).
- Tieni il pannello **ben fissato e sigillato**; orientalo verso l'alto per la massima resa.

## 7. Consumi e autonomia (stima)
- Motori di spinta in crociera: ~0.3–0.6 A ciascuno; spazzola/pompa ~0.2–0.4 A.
- ESP32 (WiFi AP): ~0.1–0.2 A.
- Totale medio ~1–1.5 A → con LiPo 2S 2200 mAh ≈ **80–120 min** teorici
  (in pratica ~60–90 min con margine e picchi). Tara la soglia di `LOW_BATTERY`
  a **6.6 V** (3.3 V/cella) per non danneggiare il LiPo.
- ☀ Con il **pannello solare** in pieno sole, parte (o tutta) la corrente di crociera è
  coperta dal sole: l'autonomia effettiva **aumenta** e la batteria si ricarica nei
  momenti di pausa, così il robot può lavorare a lungo durante le ore diurne.
