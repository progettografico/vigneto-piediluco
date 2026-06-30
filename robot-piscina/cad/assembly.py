"""assembly.py — assieme dello scafo (pezzi stampati + riferimenti commerciali).

Sistema di riferimento: X = prua/poppa (lunghezza), Y = baglio (trasversale),
Z = alto. Centro pontoni a z=0. I pezzi commerciali (tubo, profilo traverse,
pannello, box) sono disegnati come riferimento per capire come tutto si incastra.
"""
import numpy as np
import params as p
import parts
import util as u
from util import cyl, tube, box, T, R, union, difference

R0 = p.PONT_OD / 2
SAD_TOP = R0 + p.SAD_BASE_T                       # quota piano della sella (sopra il tubo)
BEAM_Z  = SAD_TOP + p.BEAM_SIZE / 2              # asse traversa
DECK_Z  = SAD_TOP + p.BEAM_SIZE + p.DECK_T / 2   # piano inferiore del ponte


def _saddle_mounted():
    """Sella ruotata in posa di montaggio: culla verso il basso, piano in alto."""
    return R(parts.pontone_saddle(), "x", 180)


def hull(explode=0.0):
    """Ritorna una lista di (mesh, colore, alpha) con tutto lo scafo posizionato.
    explode>0 distanzia i livelli lungo Z per la vista esplosa."""
    G = 70.0 * explode
    out = []

    # --- Pontoni (tubo PVC, riferimento) ---
    for sy in (-1, 1):
        t = R(tube(p.PONT_OD, p.PONT_ID, p.PONT_LEN), "y", 90)
        out.append((T(t, 0, sy * p.PONT_SPACING / 2, 0), p.COL_PONT, 1.0))

    # --- Tappi pontone (stampati) ---
    cap = parts.pontone_endcap()
    for sy in (-1, 1):
        capP = R(cap, "y", -90)   # plug verso -X
        out.append((T(capP, p.PONT_LEN / 2 + p.CAP_FLANGE_T, sy * p.PONT_SPACING / 2, 0),
                    p.COL_CAP, 1.0))
        capM = R(cap, "y", 90)    # plug verso +X
        out.append((T(capM, -p.PONT_LEN / 2 - p.CAP_FLANGE_T, sy * p.PONT_SPACING / 2, 0),
                    p.COL_CAP, 1.0))

    # --- Selle (stampate) sopra i pontoni, in corrispondenza delle traverse ---
    sad = _saddle_mounted()
    for sx in (-1, 1):
        for sy in (-1, 1):
            out.append((T(sad, sx * p.BEAM_X, sy * p.PONT_SPACING / 2, G * 1),
                        p.COL_SADDLE, 1.0))

    # --- Traverse (profilo quadro, riferimento) ---
    for sx in (-1, 1):
        b = box(p.BEAM_SIZE, p.BEAM_LEN, p.BEAM_SIZE)
        out.append((T(b, sx * p.BEAM_X, 0, BEAM_Z + G * 2), p.COL_BEAM, 1.0))

    # --- Ponte: 2 sezioni stampate con giunto centrale ---
    half = parts.deck_half()
    hx = (p.DECK_L / 2 + p.DECK_JOINT / 2) / 2 - p.DECK_JOINT / 4
    out.append((T(half, +hx, 0, DECK_Z + G * 3), p.COL_DECK, 1.0))
    out.append((T(R(half, "z", 180), -hx, 0, DECK_Z + G * 3), p.COL_DECK, 1.0))

    # --- Staffe di prua (stampate, x2) ---
    bow = parts.bow_bracket()
    for sy in (-1, 1):
        out.append((T(bow, p.PONT_LEN / 2 - 60, sy * p.PONT_SPACING / 2, 0),
                    p.COL_BOW, 1.0))

    # --- Pannello solare sopra il ponte, su distanziali (riferimento, opaco) ---
    panel = box(p.PANEL_L - 6, p.PANEL_W - 6, 6)
    out.append((T(panel, 0, 0, DECK_Z + p.DECK_T / 2 + 16 + G * 4), p.COL_PANEL, 1.0))

    # --- Box elettronica nella BAIA centrale, sotto il ponte (riferimento) ---
    ebox = box(p.BOX_L, p.BOX_W, 50)
    out.append((T(ebox, 0, 0, 18), p.COL_BOX, 1.0))   # layer 0: resta in posa anche nell'esploso

    return out
