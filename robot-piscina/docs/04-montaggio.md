# 04 — Montaggio e collaudo

> Leggi **[05-sicurezza.md](05-sicurezza.md)** prima di iniziare. Lavora sempre con
> batteria **scollegata** finché non indicato.

## Fase A — Scafo

1. **Pontoni**: taglia 2 spezzoni di tubo PVC Ø75 mm da ~40 cm. Sigilla i tappi alle
   estremità con colla PVC + silicone. Verifica la **tenuta** immergendoli in acqua:
   non devono entrare bolle/acqua.
2. **Telaio**: unisci i due pontoni con 2 traversine (PVC o alluminio) a ~30 cm di
   interasse, creando la baia centrale. Usa viti **inox**.
3. **Baia centrale**: crea un piano per il box elettronica e una guida per il
   **cestello a rete** estraibile a prua.
4. **Prova di galleggiamento a vuoto**: deve galleggiare alto e in piano.

## Fase B — Propulsione

1. Monta i 2 motori a **poppa**, uno per pontone, con l'albero/elica appena sotto il
   pelo dell'acqua. Sigilla l'albero (grasso al silicone) se il motore non è waterproof.
2. Calza le eliche; controlla che girino libere e nello **stesso verso "avanti"**
   (lo correggerai via software invertendo i pin, se serve).

## Fase C — Pulizia

1. **Cestello**: monta la cornice con rete a maglia fine sulla prua, leggermente
   immersa; deve **sfilarsi** per lo svuotamento.
2. **Spazzola laterale**: fissa il motorino con il rullo di nylon su un fianco,
   all'altezza della **linea di galleggiamento**.
3. **Pompa skimmer** (opzionale): posiziona l'aspirazione a pelo d'acqua, mandata
   verso il cestello.

## Fase D — Elettronica (box stagno)

1. Salda e cabla **seguendo [02-elettronica.md](02-elettronica.md)** e
   [wiring-diagram.svg](wiring-diagram.svg). Consigliato montare prima **su banco**
   (fuori dal box) e collaudare, poi trasferire nel box.
2. Inserisci ESP32, i 2 DRV8833, il buck 5 V e il partitore batteria nel **box IP65**.
3. Fai uscire i cavi dei motori/sensori dai **pressacavi**; sigilla.
4. Applica **conformal coating** su ESP32 e driver (a elettronica spenta e asciutta).
5. Monta **interruttore + fusibile** in linea con il "+" batteria; prevedi una
   **porta di ricarica** esterna con tappo.

## Fase E — Caricamento firmware

```bash
cd firmware
pio run                 # compila
pio run -t upload       # carica il firmware (ESP32 via USB)
pio run -t uploadfs     # carica la UI (cartella data/) in LittleFS
```
Apri il monitor seriale per i log: `pio device monitor -b 115200`.

## Fase F — Collaudo (bring-up) — IN SICUREZZA

Esegui **a secco** (robot fuori dall'acqua, eliche libere) salvo dove indicato.

### F.1 Alimentazione
- [ ] Con multimetro: il buck eroga **5.0 V** stabili.
- [ ] L'ESP32 si accende e crea la rete WiFi **`RobotPiscina`**.

### F.2 Comunicazione
- [ ] Telefono connesso alla rete; `http://192.168.4.1` apre la UI.
- [ ] La telemetria (batteria, stato) si aggiorna nell'app.

### F.3 Motori (a secco, eliche libere!)
- [ ] In **MANUAL**, joystick avanti → entrambe le eliche spingono "avanti".
      Se una gira al contrario, inverti i suoi 2 pin in `config.h` (o i fili).
- [ ] Joystick a sinistra/destra → rotazione corretta.
- [ ] **Watchdog**: chiudi l'app durante MANUAL → i motori si fermano entro ~1 s.

### F.4 Sensori
- [ ] Premendo i micro-switch, lo stato `bump` cambia nell'app/log.
- [ ] Avvicinando la mano al sensore laterale, `dist` diminuisce.
- [ ] Confronta `batt` col multimetro e **tara `VBAT_DIVIDER`** in `config.h`.

### F.5 Pulizia
- [ ] La spazzola gira; la pompa (se presente) aspira.

### F.6 Prove in acqua (sorvegliato)
- [ ] **Test tenuta**: lascia il robot **spento** in acqua qualche minuto; apri il box
      e verifica che sia **asciutto** prima di alimentare.
- [ ] Prima navigazione in **MANUAL** a bassa velocità.
- [ ] Prova **AUTO_COVER**: deve arretrare e ruotare quando tocca il bordo.
- [ ] Prova **AUTO_EDGE**: deve seguire il muro a distanza ~costante.
- [ ] Verifica lo **stop automatico** a batteria scarica (soglia in `config.h`).

## Taratura fine (in `firmware/src/config.h`)
- `CRUISE_SPEED`, `TURN_SPEED`: velocità di crociera/rotazione.
- `EDGE_TARGET_MM`, `EDGE_KP`: distanza dal bordo e guadagno del controllo.
- `BUMP_BACKUP_MS`, `TURN_MS`: durata di retromarcia/rotazione in AUTO_COVER.
- `VBAT_LOW`, `VBAT_DIVIDER`: soglia batteria scarica e fattore del partitore.
