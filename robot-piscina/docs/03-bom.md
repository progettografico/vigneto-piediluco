# 03 — Lista componenti (BOM)

Budget di riferimento: **piscina privata media (~8×4 m), 100–200 €**.
Prezzi indicativi (mercato hobbistico EU, IVA inclusa) — variano nel tempo.

## Tabella componenti

| # | Componente | Specifiche consigliate | Q.tà | Costo ind. | Note / alternative |
|---|---|---|---|---|---|
| 1 | **ESP32 DevKit** | ESP32-WROOM-32, USB-C | 1 | 6–10 € | WiFi+BT integrati. Alt: ESP32-S3. |
| 2 | **Driver motori DC** | DRV8833 (doppio, ≤1.5 A/canale) | 2 | 2–4 € cad. | 1 per i 2 motori di spinta, 1 per spazzola+pompa. Alt: TB6612FNG. |
| 3 | **Motoriduttori DC** | 6 V "TT motor" o pompe di sentina 12 V | 2 | 3–8 € cad. | Spinta differenziale. Per maggiore spinta: pompe di sentina riusate come thruster. |
| 4 | **Eliche** | Ø ~40–50 mm su albero 2 mm | 2 | 2–5 € cad. | Su misura dell'albero motore. Alt: eliche stampate 3D. |
| 5 | **Motorino spazzola** | DC 6 V con riduttore lento | 1 | 4–8 € | Per la spazzola laterale. |
| 6 | **Spazzola rotante** | rullo/spazzola nylon Ø ~30 mm | 1 | 3–6 € | Alt: spazzola per pulizia stampata 3D + setole. |
| 7 | **Pompa skimmer (opz.)** | mini pompa sommergibile 5–6 V | 1 | 4–8 € | Aspira la superficie verso il cestello. Opzionale (raccolta passiva possibile). |
| 8 | **Sensore distanza laterale** | VL53L0X (ToF I²C) | 1 | 3–6 € | Inseguimento bordo. Alt: HC-SR04 (ultrasuoni, fuori acqua). |
| 9 | **Micro-switch a leva** | paraurti anteriore/laterale | 2–3 | 0.5 € cad. | Rilevano urto contro il muro. |
| 10 | **Pacco batteria** | 2S LiPo 7.4 V 2000–3000 mAh **oppure** 2×18650 + holder | 1 | 12–20 € | Con connettore XT30/JST. |
| 11 | **Regolatore 5 V** | step-down (buck) MP1584 / LM2596, 3 A | 1 | 1–3 € | Da Vbatt a 5 V per ESP32/logica. |
| 12 | **Caricabatterie LiPo** | bilanciato 2S (IMAX B3/B6 o simili) | 1 | 8–20 € | **Mai** caricare senza bilanciamento/sorveglianza. |
| 13 | **Interruttore + fusibile** | switch principale + fusibile 3–5 A | 1 | 2–4 € | Sezionamento e protezione. |
| 14 | **Scatola stagna (IP65)** | box ABS con guarnizione | 1 | 5–10 € | Vano elettronica. |
| 15 | **Pressacavi / passacavi** | PG7 a tenuta | 3–5 | 0.5 € cad. | Uscita cavi motori dal box. |
| 16 | **Scafo / pontoni** | tubo PVC Ø75 mm + tappi **oppure** foam HD | — | 5–15 € | Doppio pontone catamarano. Alt: stampa 3D + schiuma. |
| 17 | **Rete + telaio cestello** | rete a maglia fine + cornice | 1 | 3–6 € | Cestello detriti estraibile. |
| 18 | **Minuteria** | viti inox, O-ring, silicone, fascette, cavi | — | 5–10 € | Inox per ambiente umido/cloro. |
| 19 | **Conformal coating** | spray protettivo per PCB | 1 | 6–10 € | Protegge l'elettronica dall'umidità. |
| 20 | **Pannello solare** | 6 V (o 9 V) **5–10 W**, laminato ETFE/epossidico waterproof | 1 | 12–25 € | Montato sul ponte. Dimensione tipica ~20×17 cm. Più W = ricarica più rapida. |
| 21 | **Regolatore di carica solare** | modulo **MPPT per Li-ion 2S** (es. basato su CN3722) | 1 | 6–14 € | Carica la batteria dal pannello in autonomia, senza firmware. Deve corrispondere alla chimica/celle della batteria. |
| 22 | **Diodo di blocco** | Schottky (es. SB560) | 1 | 0.5 € | Evita lo scaricamento della batteria nel pannello al buio (molti regolatori lo includono già). |

**Totale indicativo: ~130–215 €** (la pompa skimmer e la stampa 3D sono opzionali).

> ☀ **Sistema solare**: il pannello sul ponte alimenta il regolatore di carica, che
> mantiene carica la batteria mentre il robot galleggia/lavora di giorno. In pieno sole
> un pannello da 5–10 W **estende sensibilmente l'autonomia** e tiene il pacco in carica
> nei periodi di inattività. La gestione della carica è **autonoma** (la fa il regolatore):
> il firmware continua solo a leggere la tensione batteria, senza modifiche.

## Strumenti necessari
Saldatore + stagno, multimetro, trapano, cacciaviti, pistola a caldo/silicone,
cavo USB-C, (opz.) stampante 3D.

## Mappatura pin ESP32 (riferimento per il firmware)

Questa tabella è la **fonte di verità** replicata in `firmware/src/config.h`.

| Funzione | Segnale | GPIO ESP32 | Note |
|---|---|---|---|
| Motore SX | `AIN1` / `AIN2` (PWM) | 25 / 26 | Driver propulsione DRV8833 #1 |
| Motore DX | `BIN1` / `BIN2` (PWM) | 27 / 14 | Driver propulsione DRV8833 #1 |
| Spazzola | `IN1` / `IN2` | 16 / 17 | Driver utenze DRV8833 #2, canale A |
| Pompa skimmer | `IN3` / `IN4` | 18 / 19 | Driver utenze DRV8833 #2, canale B |
| Sensore ToF | `SDA` / `SCL` (I²C) | 21 / 22 | VL53L0X |
| Bump anteriore | `BUMP_FRONT` | 34 | Input, micro-switch a massa (pull-up) |
| Bump laterale | `BUMP_SIDE` | 35 | Input, micro-switch a massa (pull-up) |
| Lettura batteria | `VBAT_SENSE` | 33 | ADC1, partitore di tensione |
| LED stato | `LED_STATUS` | 2 | LED onboard |
| Buzzer (opz.) | `BUZZER` | 4 | Avvisi acustici |
| Carica solare (opz.) | `VSOLAR_SENSE` | 32 | ADC1, partitore dal pannello — solo se vuoi mostrare "in carica" nell'app (non usato dal firmware base) |

> GPIO **34/35/36/39 sono solo input** (niente pull-up interno → usa resistenze di
> pull-up esterne da 10 kΩ sui micro-switch). Vedi `docs/02-elettronica.md`.
