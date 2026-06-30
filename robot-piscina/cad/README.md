# CAD — Modelli 3D parametrici (scafo e pontoni)

Modelli **parametrici** dello scafo del Robot Piscina: pezzi **stampabili in 3D**
(file STL) + **render** dell'assieme e della vista esplosa. Tutte le quote derivano
da un unico file di parametri, così basta cambiare un valore per aggiornare tutto.

> Questa è la **prima fase di dettaglio** (scafo e pontoni). Seguiranno: vano
> elettronica, sistema di pulizia, propulsione, supporto pannello e assieme completo.

## Render

| Vista | File |
|---|---|
| Assieme | [render/assembly.png](render/assembly.png) |
| Vista dall'alto | [render/assembly_top.png](render/assembly_top.png) |
| **Vista esplosa** | [render/assembly_exploded.png](render/assembly_exploded.png) |
| Singoli pezzi | `render/part_*.png` |

## Pezzi stampabili (STL in `stl/`)

| Pezzo | File | Q.tà | Ingombro | Funzione |
|---|---|---|---|---|
| Tappo pontone | `pontone_endcap.stl` | 4 | Ø83 × 22 mm | Chiude le estremità del tubo (gola O-ring + smusso d'imbocco) |
| Sella / cavallotto | `pontone_saddle.stl` | 4 | 40 × 87 × 57 mm | Fissa il pontone alla traversa (culla + fori M4) |
| Staffa angolare | `corner_bracket.stl` | 4 | 30 × 20 × 30 mm | Collega il ponte alle traverse (L con nervatura) |
| Ponte — sezione | `deck_half.stl` | 2 | 117 × 190 × 5 mm | Piano d'appoggio (giunto a mezzo-legno + asole di drenaggio + fori) |
| Staffa di prua | `bow_bracket.stl` | 2 | 73 × 87 × 87 mm | Collare sul naso del pontone + labbro che regge il cestello |

I **pezzi commerciali** (NON si stampano) sono mostrati solo come riferimento
nell'assieme: tubo PVC Ø75 (pontoni), profilo quadro 20×20 (traverse), pannello solare,
box elettronico.

## Come (ri)generare tutto

Requisiti (Python 3):
```bash
pip install numpy trimesh manifold3d matplotlib
```
Genera STL + render:
```bash
cd cad
python3 build.py
```
Output in `cad/stl/` (STL) e `cad/render/` (PNG). Ogni STL è verificato **watertight**.

## Parametri (modifica `params.py`)
Tutte le quote in mm. I più importanti:
- `PONT_OD`, `PONT_ID`, `PONT_LEN`, `PONT_SPACING` — tubo pontoni e interasse.
- `BED` — lato del piano di stampa (default 200): i pezzi più grandi sono già progettati
  in sezioni che vi rientrano (es. il ponte è in 2 metà unite a mezzo-legno).
- `CLEAR` — gioco di accoppiamento delle parti a incastro (default 0.4).
- `CAP_*`, `SAD_*`, `DECK_*`, `BOW_*` — dettagli dei singoli pezzi.

Struttura del codice:
- `params.py` — fonte di verità dimensionale.
- `util.py` — primitive (tubo, scatola arrotondata, asola), booleani (manifold),
  export STL, render PNG con ombreggiatura.
- `parts.py` — una funzione per pezzo, ognuna ritorna una mesh stampabile.
- `assembly.py` — posiziona pezzi + riferimenti; `explode>0` per la vista esplosa.
- `build.py` — orchestratore.

## Impostazioni di stampa consigliate
- **Materiale**: PETG o ASA (resistenti a UV, acqua e cloro). PLA solo per prove interne.
- **Pareti/perimetri**: 3–4 (i pezzi a contatto con l'acqua devono essere robusti).
- **Riempimento**: 30–40 % (selle e staffe: 50 %).
- **Layer**: 0.2 mm.
- **Orientamento**:
  - *Tappo*: flangia in basso sul piatto (niente supporti).
  - *Sella*: base sul piatto, culla verso l'alto.
  - *Ponte*: piatto disteso.
  - *Staffa di prua*: con il foro del collare orizzontale → servono supporti; in
    alternativa orienta il collare verticale.
- **Tenuta**: i pezzi stampati **non** garantiscono il galleggiamento (lo fanno il tubo
  PVC sigillato + la schiuma). Per i pezzi a contatto con l'acqua, valuta una mano di
  vernice/coating. Verifica sempre il **galleggiamento** col peso reale
  (vedi `../docs/01-progettazione.md`, §2.2).

## Coerenza con il resto del progetto
Le quote sono allineate alla **BOM** (`../docs/03-bom.md`, tubo Ø75, interasse ~21 cm)
e ai disegni in `../docs/disegni/`. Se cambi `params.py`, aggiorna di conseguenza la BOM.
