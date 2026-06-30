# 01 — Progettazione

## 1. Obiettivo e requisiti

Robot **galleggiante** per piscina privata media (~8×4 m) che:
- raccoglie i detriti che **galleggiano** in superficie (foglie, insetti, polline);
- pulisce la **linea di galleggiamento** (waterline) e il bordo;
- **non** scende in profondità: galleggia sempre;
- funziona in **autonomia** (copertura + bordo) con **override manuale** da smartphone;
- ha autonomia di **60–90 minuti** e si ferma da solo a batteria scarica.

Vincoli: economico (~100–200 €), costruibile a mano, elettronica **ESP32**, sicuro
in ambiente acqua + cloro.

## 2. Architettura meccanica

> 📐 Disegni tecnici (spaccati, vista quotata, componenti): cartella
> [disegni/](disegni/). Render concettuale: [render-robot.png](render-robot.png).
> 🧩 **Modelli 3D parametrici + STL stampabili** dello scafo: cartella
> [../cad/](../cad/) (assieme, vista esplosa, pezzi stampabili).

### 2.1 Scafo: catamarano a doppio pontone
Due pontoni paralleli garantiscono **stabilità** (difficile da ribaltare) e una
**baia centrale** dove montare elettronica e cestello.

```
        prua (cestello a rete)
   ┌───────[ RETE ]───────┐
   │  ▓▓pontone sx▓▓       │
   │   ┌───────────────┐   │   ← baia centrale:
   │   │  BOX STAGNO    │   │     box elettronica + batteria
   │   │  (ESP32, drv,  │   │
   │   │   batteria)    │   │
   │   └───────────────┘   │
   │  ▓▓pontone dx▓▓       │
   └──[elica sx]─[elica dx]┘
         poppa (propulsione)
```

- **Pontoni**: tubo PVC Ø75 mm sigillato ai tappi, **oppure** foam ad alta densità,
  **oppure** stampa 3D riempita di schiuma (ridondanza di galleggiamento).
- **Baricentro basso**: batteria (l'elemento più pesante) montata in basso e centrata.
- **Dimensioni indicative**: ~35–45 cm lunghezza, ~30 cm larghezza. Piccolo e leggero
  (target < 1.5 kg) per spinta modesta e lunga autonomia.

### 2.2 Galleggiamento (dimensionamento rapido)
Spinta di Archimede: ogni litro di volume **immerso** sostiene ~1 kg.
Per un robot da ~1.5 kg con **margine 2×** servono ~3 litri di volume dei pontoni
**sotto la linea di galleggiamento**.

Esempio con pontoni in tubo PVC Ø75 mm (raggio 3.75 cm → area ~44 cm²):
- volume per metro di tubo ≈ 4.4 L/m;
- due pontoni da 40 cm ≈ 2 × 1.76 L ≈ **3.5 L** totali di volume disponibile.

Il robot affonda solo finché la spinta eguaglia il peso → con 1.5 kg si immerge
~1.7 L su 3.5 disponibili: **galleggia con ampio margine** e resta alto sull'acqua.
> Regola pratica: punta a far immergere **meno della metà** del volume dei pontoni a
> pieno carico, così resta riserva per detriti bagnati nel cestello.

### 2.3 Propulsione — differential drive
Due motori indipendenti con elica a poppa:
- entrambi avanti → il robot va dritto;
- uno più veloce dell'altro → curva;
- uno avanti e uno indietro → rotazione sul posto.

Niente timone, niente parti mobili di sterzo. La logica è in `motors.cpp`
(comando `(v, ω)` → spinta sinistra/destra).

### 2.4 Raccolta detriti (skimmer)
- **Cestello a rete estraibile** montato sulla **prua**, leggermente sotto il pelo
  dell'acqua: avanzando, il robot "spinge" i detriti galleggianti dentro la rete.
- **Pompa skimmer opzionale**: aspira l'acqua superficiale verso il cestello,
  aumentando l'efficacia con acqua ferma. Senza pompa la raccolta è **passiva**
  (funziona comunque grazie al moto in avanti).
- Il cestello si sfila per svuotarlo.

### 2.5 Pulizia bordo (waterline)
- **Spazzola rotante laterale** (rullo di nylon) sul fianco del robot, all'altezza
  della linea di galleggiamento.
- In modalità `AUTO_EDGE` il robot mantiene una distanza fissa dal muro (sensore ToF
  laterale) e la spazzola, in rotazione, struscia la waterline rimuovendo il film di
  sporco/calcare leggero.

## 3. Elettronica (sintesi — dettagli in 02-elettronica.md)
- **ESP32** come unico controllore (WiFi AP + WebSocket + logica).
- **2× DRV8833**: uno per i 2 motori di spinta, uno per spazzola + pompa.
- **Sensori**: 2–3 micro-switch paraurti, 1 sensore distanza laterale (VL53L0X),
  partitore di tensione per la batteria.
- **Alimentazione**: 2S LiPo → buck 5 V per la logica; motori direttamente da Vbatt.
- ☀ **Ricarica solare**: un **pannello solare** sul ponte alimenta un **regolatore di
  carica MPPT** che mantiene carica la batteria mentre il robot galleggia/lavora di
  giorno, estendendo l'autonomia. La gestione è autonoma e non richiede firmware.

## 4. Logica di navigazione (macchina a stati)

Implementata in `firmware/src/navigation.cpp`. Stati:

| Stato | Comportamento |
|---|---|
| `IDLE` | Fermo, motori spenti, in attesa di comandi. |
| `MANUAL` | Segue il joystick dell'app. **Priorità massima**: l'app può entrarci sempre. |
| `AUTO_COVER` | Copertura superficie: va dritto; se un bump scatta o il muro è vicino, **arretra, ruota di un angolo (pseudo-casuale) e riparte**. Pompa/cestello attivi. |
| `AUTO_EDGE` | Inseguimento bordo: mantiene una distanza target dal muro col sensore ToF (controllo proporzionale sulla rotazione). Spazzola attiva. |
| `STOP` | Arresto comandato: motori a zero, poi torna `IDLE`. |
| `LOW_BATTERY` | Tensione sotto soglia: ferma tutto, segnala (LED/buzzer) e resta lì in sicurezza. |

Transizioni principali:
```
IDLE ──start(mode)──▶ AUTO_COVER / AUTO_EDGE / MANUAL
qualsiasi ──cmd manual──▶ MANUAL
qualsiasi ──stop──▶ STOP ─▶ IDLE
qualsiasi ──Vbatt<soglia──▶ LOW_BATTERY (latch)
```

### 4.1 Strategia di copertura `AUTO_COVER`
Algoritmo "a rimbalzo" (stile robot aspirapolvere): semplice, robusto, senza mappa.
1. Avanza dritto a velocità di crociera.
2. Su urto (bump) o muro vicino (ToF): ferma → **arretra** brevemente →
   **ruota** di un angolo variabile (per non ripetere lo stesso percorso) → riparte.
3. Ripete fino a `stop` o batteria scarica.

### 4.2 Inseguimento bordo `AUTO_EDGE`
Controllo **proporzionale**: errore = distanza misurata − distanza target dal muro;
la rotazione ω corregge l'errore mantenendo il robot parallelo al bordo, mentre
avanza lentamente e la spazzola lavora. I bump anteriori gestiscono gli angoli della
piscina (arretra + ruota verso l'interno).

## 5. Comunicazione robot ↔ app
- L'ESP32 crea un **Access Point** (`RobotPiscina`, IP `192.168.4.1`).
- Server **WebSocket** + UI servita da LittleFS. Messaggi **JSON**:
  - **app → robot**: `manual {vx, w}`, `mode {auto_cover|auto_edge|manual}`,
    `start`, `stop`, `brush {on}`, `pump {on}`.
  - **robot → app** (~5 Hz): `{batt, mode, state, bump, dist}`.
- Vedi protocollo dettagliato in `firmware/src/comms.cpp` e nell'app.

## 6. Sicurezza by design
Vano stagno, fusibile + interruttore generale, **watchdog di comunicazione**
(se l'app si disconnette in `MANUAL`, i motori si fermano dopo ~1 s), stop automatico
a batteria scarica. Dettagli e regole d'uso in `05-sicurezza.md`.
