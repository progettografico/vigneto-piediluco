"""params.py — FONTE DI VERITA' dimensionale del Robot Piscina (tutte le quote in mm).

Allineato a docs/03-bom.md e docs/disegni/. Modifica qui i valori: tutti i pezzi e
l'assieme si aggiornano di conseguenza. Quote indicative, da verificare con i
componenti reali e con il calcolo di galleggiamento (docs/01-progettazione.md §2.2).
"""

# --- Stampa 3D ---------------------------------------------------------------
BED          = 200.0     # lato utile del piano di stampa (mm); pezzi piu' grandi -> in sezioni
CLEAR        = 0.4       # gioco di accoppiamento per parti che si incastrano (mm)

# --- Pontoni (tubo PVC commerciale) -----------------------------------------
PONT_OD      = 75.0      # diametro esterno tubo
PONT_WALL    = 3.0       # spessore parete tubo
PONT_ID      = PONT_OD - 2 * PONT_WALL   # diametro interno (= 69)
PONT_LEN     = 400.0     # lunghezza di un pontone
PONT_SPACING = 210.0     # interasse fra i due pontoni (centro-centro)

# --- Telaio: traverse (profilo quadro commerciale) --------------------------
BEAM_SIZE    = 20.0      # lato del profilo quadro (alluminio/PVC 20x20)
BEAM_LEN     = PONT_SPACING + PONT_OD + 30   # lunghezza traversa (copre i due pontoni)
BEAM_X       = 120.0     # distanza delle due traverse dal centro (±X)

# --- Tappo stampato del pontone ---------------------------------------------
CAP_PLUG_D   = PONT_ID - CLEAR        # tappo che entra nel tubo
CAP_PLUG_LEN = 18.0                   # profondita' di innesto
CAP_FLANGE_D = PONT_OD + 8.0          # flangia di battuta esterna
CAP_FLANGE_T = 4.0                    # spessore flangia
CAP_ORING_W  = 3.0                    # larghezza gola O-ring
CAP_ORING_D  = 2.5                    # profondita' gola O-ring

# --- Sella/cavallotto (fissa il pontone alla traversa) ----------------------
SAD_W        = 40.0      # larghezza sella (lungo l'asse del tubo)
SAD_WALL     = 6.0       # spessore pareti della culla
SAD_HUG      = 0.62      # frazione di circonferenza abbracciata (0.5=mezzo tubo)
SAD_BASE_T   = 6.0       # spessore piastra di base
SAD_SCREW_D  = 4.0       # fori vite M4
SAD_SCREW_OFF= 16.0      # distanza fori dal centro

# --- Piastra ponte (regge box elettronica + pannello solare) ----------------
DECK_L       = 220.0     # lunghezza ponte (X)  -> > BED quindi in 2 sezioni
DECK_W       = 190.0     # larghezza ponte (Y)  (sta nel piano di stampa; le traverse arrivano ai pontoni)
DECK_T       = 5.0       # spessore piastra
DECK_RIB     = 8.0       # altezza nervature di irrigidimento
DECK_JOINT   = 14.0      # sovrapposizione di giunzione fra le due sezioni
BOX_L        = 120.0     # impronta vano elettronica (X)
BOX_W        = 90.0      # impronta vano elettronica (Y)
PANEL_L      = 200.0     # pannello solare (X)
PANEL_W      = 170.0     # pannello solare (Y)
DECK_SCREW_D = 4.0

# --- Supporto di prua (tiene insieme le punte + aggancio cestello) ----------
BOW_T        = 6.0       # spessore
BOW_W        = 30.0      # larghezza braccio
BOW_DROP     = 40.0      # quanto scende verso l'acqua per reggere il cestello

# --- Colori per i render (estetica) -----------------------------------------
COL_PONT   = "#e8a51b"   # pontoni (giallo)
COL_CAP    = "#c8771a"   # tappi
COL_SADDLE = "#5566cc"   # selle
COL_BEAM   = "#9aa6ad"   # traverse
COL_DECK   = "#cfd8dd"   # ponte
COL_PANEL  = "#16315a"   # pannello solare
COL_BOX    = "#7a8b93"   # box elettronica
COL_BOW    = "#3aa0a0"   # supporto prua
