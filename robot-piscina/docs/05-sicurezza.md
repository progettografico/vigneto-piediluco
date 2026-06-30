# 05 — Sicurezza ⚠️

**Leggi tutto prima di costruire e usare il robot.** Acqua, elettricità e batterie al
litio insieme comportano rischi reali di **incendio, ustioni, folgorazione e
annegamento**. Sei responsabile dell'uso. In caso di dubbio, fermati e chiedi aiuto a
una persona esperta.

## 1. Persone e piscina
- **Nessuno deve fare il bagno mentre il robot è in funzione.** Recupera sempre il
  robot prima che qualcuno entri in acqua.
- Tieni **bambini e animali** lontani dal robot in funzione (eliche e spazzola in
  movimento possono ferire).
- Spegni e recupera il robot prima di qualsiasi manutenzione o pulizia del cestello.

## 2. Batterie LiPo (la parte più pericolosa)
- **Mai caricare LiPo incustodite.** Usa un **caricabatterie bilanciato** adatto al 2S
  e una superficie ininfiammabile (o sacchetto LiPo-safe).
- **Non scaricare** sotto ~3.0 V/cella: il firmware si ferma a `VBAT_LOW` (default
  ~6.6 V = 3.3 V/cella). Non disabilitare questa protezione.
- **Non usare** batterie gonfie, forate, bagnate o danneggiate: smaltiscile secondo le
  norme locali.
- Carica e conserva la batteria **fuori** dal robot quando possibile, in luogo asciutto
  e fresco.
- Monta sempre **fusibile (3–5 A)** e **interruttore generale** in linea col "+".

## 3. Acqua + elettronica
- Tutta l'elettronica va nel **box stagno (IP65)**; verifica la **tenuta a secco**
  prima di ogni uso (test del box vuoto in acqua).
- **Bassa tensione soltanto** (≤ ~12 V cc). **Mai** portare la tensione di rete (230 V)
  vicino alla piscina o al robot. La **ricarica si fa lontano dall'acqua**.
- Se entra acqua nel box: **scollega subito la batteria**, asciuga, ispeziona e
  verifica la LiPo prima di riutilizzare.
- Applica **conformal coating** e isola tutte le saldature.

## 4. Compatibilità con la piscina
- Materiali a contatto con l'acqua **resistenti al cloro** e atossici (inox, PVC,
  nylon, foam adatto). Evita parti che rilasciano sostanze o ruggine.
- Verifica che spazzola e scafo **non graffino** il rivestimento (liner/piastrelle):
  usa setole morbide e bordi smussati.
- Non lasciare il robot in acqua per periodi prolungati o non sorvegliati.

## 5. Sicurezze nel firmware (non rimuoverle)
- **Watchdog di comunicazione**: in `MANUAL`, se l'app si disconnette, i motori si
  fermano entro ~1 s (`COMM_TIMEOUT_MS`).
- **Stop a batteria scarica**: stato `LOW_BATTERY` con latch; ferma motori e utenze.
- **Stop comandato**: il pulsante Stop dell'app porta sempre il robot in `IDLE`.
- All'avvio il robot parte in `IDLE` (fermo): nessun movimento senza comando esplicito.

## 6. Checklist rapida prima di ogni utilizzo
- [ ] Box stagno chiuso e asciutto, pressacavi serrati.
- [ ] Batteria carica, integra, ben fissata; fusibile e interruttore a posto.
- [ ] Eliche e spazzola libere da detriti; cestello inserito.
- [ ] Nessuna persona/animale in acqua.
- [ ] Test rapido in MANUAL prima di passare in automatico.
- [ ] A fine uso: spegni, recupera, asciuga, rimuovi/ricarica la batteria sorvegliata.

## 7. Smaltimento
Batterie ed elettronica vanno conferite negli appositi punti di raccolta (RAEE/pile),
**mai** nei rifiuti indifferenziati né in acqua.
