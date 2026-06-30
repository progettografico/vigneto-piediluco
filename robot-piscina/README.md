# 🛶 Robot Piscina — Skimmer di superficie e pulizia bordi

Piccolo robot **galleggiante** autonomo (con telecomando manuale) che pulisce la
**superficie** e i **bordi / linea di galleggiamento** di una piscina privata.
**Non scende in profondità**: galleggia sempre come una piccola barca.

> ⚠️ Acqua + elettronica + batterie LiPo = rischio reale. Leggi
> **[docs/05-sicurezza.md](docs/05-sicurezza.md) PRIMA di costruire e usare il robot.**

---

## Cosa fa

- **Skimmer**: avanza in superficie e convoglia foglie, insetti e polline in un
  **cestello a rete estraibile** a prua.
- **Pulizia bordo**: in modalità perimetro segue il muro e una **spazzola rotante
  laterale** pulisce la linea di galleggiamento (waterline).
- **Autonomo**: copertura a "rimbalzo" della superficie (`AUTO_COVER`) + inseguimento
  del bordo (`AUTO_EDGE`), con stop automatico a batteria scarica.
- **Manuale**: lo piloti dal telefono con un joystick (la modalità manuale ha sempre priorità).

## Com'è fatto (in breve)

| Sottosistema | Scelta |
|---|---|
| Cervello | **ESP32** (WiFi + Bluetooth integrati) |
| Scafo | Catamarano a doppio pontone (galleggiante, stabile) |
| Propulsione | 2 motori DC + eliche, **differential drive** (sterza variando dx/sx) |
| Pulizia | Cestello a rete a prua + spazzola rotante laterale (+ pompa skimmer opzionale) |
| Sensori | Paraurti a micro-switch, sensore distanza laterale (ToF/ultrasuoni), tensione batteria |
| Alimentazione | Pacco 2S LiPo / 18650 in vano stagno (~60–90 min) **+ pannello solare** sul ponte con regolatore di carica MPPT |
| Controllo | L'ESP32 crea un suo **WiFi**; il telefono apre l'app e comanda via WebSocket |

## Struttura del progetto

```
robot-piscina/
├── docs/        # progettazione, elettronica, BOM, montaggio, sicurezza, schema
├── cad/         # modelli 3D parametrici + STL stampabili + render (scafo e pontoni)
├── firmware/    # progetto PlatformIO per ESP32 (+ UI servita dal robot)
└── app/         # app di controllo PWA installabile sullo smartphone
```

> Questo progetto è **auto-contenuto**: puoi copiare la cartella `robot-piscina/`
> in un repository dedicato senza modifiche.

## Quickstart

### 1. Costruzione hardware
1. Procurati i componenti: **[docs/03-bom.md](docs/03-bom.md)**.
2. Monta scafo, motori, elettronica: **[docs/04-montaggio.md](docs/04-montaggio.md)**.
3. Collega tutto secondo **[docs/02-elettronica.md](docs/02-elettronica.md)** e
   **[docs/wiring-diagram.svg](docs/wiring-diagram.svg)**.

### 2. Firmware (ESP32)
```bash
cd firmware
pio run                 # compila
pio run -t upload       # carica il firmware sull'ESP32
pio run -t uploadfs     # carica la UI (cartella data/) nella memoria LittleFS
```
(Installa [PlatformIO](https://platformio.org/) — estensione VS Code o `pip install platformio`.)

### 3. Uso
1. Accendi il robot: crea una rete WiFi **`RobotPiscina`** (password in `firmware/src/config.h`).
2. Collega il telefono a quella rete e apri **`http://192.168.4.1`**
   (oppure installa l'app da `app/` e impostala su quell'indirizzo).
3. Scegli la modalità (Manuale / Superficie / Bordo) e premi **Avvia**.

## Documentazione

| Documento | Contenuto |
|---|---|
| [01-progettazione.md](docs/01-progettazione.md) | Concept, meccanica, galleggiamento, logica di navigazione |
| [02-elettronica.md](docs/02-elettronica.md) | Schema, pinout ESP32, cablaggio, alimentazione |
| [03-bom.md](docs/03-bom.md) | Lista componenti con costi indicativi e alternative |
| [04-montaggio.md](docs/04-montaggio.md) | Istruzioni passo-passo + collaudo (bring-up) |
| [05-sicurezza.md](docs/05-sicurezza.md) | Sicurezza acqua/elettricità/LiPo — **da leggere** |
| [disegni/](docs/disegni/) | Disegni tecnici: spaccati (sezione laterale/trasversale), vista quotata, componenti |

Vedi anche il **render concettuale** ([docs/render-robot.png](docs/render-robot.png)) e lo
**schema di cablaggio** ([docs/wiring-diagram.svg](docs/wiring-diagram.svg)).

## Stato e limiti (fuori scope per ora)
Pulizia del **fondo/pareti in profondità**, ritorno autonomo a una base di ricarica,
visione artificiale e GPS sono **fuori scope** in questa versione, ma l'architettura è
predisposta per aggiungerli in seguito.

## Licenza
Progetto personale/hobbistico. Usa a tuo rischio rispettando le norme locali sulla
sicurezza elettrica e l'uso della piscina.
