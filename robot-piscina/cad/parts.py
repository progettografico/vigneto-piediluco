"""parts.py — pezzi stampabili dello scafo (funzioni parametriche).

Ogni funzione restituisce una mesh trimesh nella sua orientazione di STAMPA
(asse Z verso l'alto, appoggio sul piano z=0 dove sensato). Le quote vengono da
params.py. I pezzi commerciali (tubo, profilo) sono modellati solo come riferimento
nell'assieme, non si stampano.
"""
import numpy as np
import params as p
import util as u
from util import cyl, tube, box, rounded_box, slot, T, R, union, difference


# --- Helper locali ----------------------------------------------------------
def vhole(d, h):
    """Foro cilindrico verticale (lungo Z) per sottrazione."""
    return cyl(d, h, 24)

def xtube_cut(d, length):
    """Cilindro lungo X (per scavare il canale del pontone)."""
    return R(cyl(d, length, 64), "y", 90)


# === 1. Tappo del pontone ====================================================
def pontone_endcap():
    fl = T(cyl(p.CAP_FLANGE_D, p.CAP_FLANGE_T, 64), 0, 0, p.CAP_FLANGE_T / 2)
    plug = T(cyl(p.CAP_PLUG_D, p.CAP_PLUG_LEN, 64),
             0, 0, p.CAP_FLANGE_T + p.CAP_PLUG_LEN / 2)
    cap = union(fl, plug)
    # gola O-ring a meta' del plug
    gz = p.CAP_FLANGE_T + p.CAP_PLUG_LEN * 0.5
    ring = T(tube(p.CAP_PLUG_D + 2, p.CAP_PLUG_D - 2 * p.CAP_ORING_D, p.CAP_ORING_W, 64),
             0, 0, gz)
    cap = difference(cap, ring)
    # smusso di imbocco in cima al plug (cono)
    top = p.CAP_FLANGE_T + p.CAP_PLUG_LEN
    cone = trimesh_cone(p.CAP_PLUG_D / 2 + 1, 3.5)
    cap = difference(cap, T(cone, 0, 0, top - 3.5 + 0.01))
    return cap


def trimesh_cone(r, h):
    """Cono usato come smusso (punta in alto), per sottrazione in cima."""
    import trimesh
    m = trimesh.creation.cone(radius=r, height=h, sections=48)
    return m  # base a z=0, punta a z=h


# === 2. Sella / cavallotto pontone -> traversa ===============================
def pontone_saddle():
    R0 = p.PONT_OD / 2.0
    Rclear = R0 + p.CLEAR / 2
    R_out = R0 + p.SAD_WALL
    arm_top_z = -R0 * np.cos(np.pi * p.SAD_HUG)      # quanto risalgono le braccia
    base_bot_z = -(R0 + p.SAD_BASE_T)
    h = arm_top_z - base_bot_z
    body = T(rounded_box(p.SAD_W, 2 * R_out, h, 4), 0, 0, (arm_top_z + base_bot_z) / 2)
    # canale del tubo
    body = difference(body, xtube_cut(2 * Rclear, p.SAD_W + 4))
    # imbocco a V per lo snap-fit (svasa l'apertura superiore)
    mouth = T(box(p.SAD_W + 4, 2 * Rclear, 10), 0, 0, arm_top_z + 5 - 0.5)
    body = difference(body, mouth)
    # fori vite verticali nella base
    for sx in (-1, 1):
        body = difference(body, T(vhole(p.SAD_SCREW_D, p.SAD_BASE_T * 3),
                                  sx * p.SAD_SCREW_OFF, 0, base_bot_z + p.SAD_BASE_T / 2))
    # porta il pezzo con la base appoggiata a z=0
    return T(body, 0, 0, -base_bot_z)


# === 3. Staffa angolare (deck <-> traversa / telaio) =========================
def corner_bracket():
    t = 4.0; arm = 30.0; w = p.BEAM_SIZE
    horiz = T(box(arm, w, t), arm / 2, 0, t / 2)            # braccio orizzontale
    vert = T(box(t, w, arm), t / 2, 0, arm / 2)             # braccio verticale
    gusset = make_gusset(arm * 0.7, t)                      # rinforzo triangolare
    br = union(horiz, vert, gusset)
    # foro M4 nel braccio orizzontale (asse Z)
    br = difference(br, T(vhole(4.0, t * 4), arm * 0.62, 0, t / 2))
    # foro M4 nel braccio verticale (asse X)
    hole_x = R(cyl(4.0, t * 4, 24), "y", 90)
    br = difference(br, T(hole_x, t / 2, 0, arm * 0.62))
    return br


def make_gusset(s, t):
    """Triangolo di rinforzo nel piano XZ, spessore t lungo Y."""
    import trimesh
    verts = np.array([[0, -t/2, 0], [s, -t/2, 0], [0, -t/2, s],
                      [0,  t/2, 0], [s,  t/2, 0], [0,  t/2, s]])
    faces = np.array([[0,1,2],[3,5,4],[0,2,5],[0,5,3],
                      [1,4,5],[1,5,2],[0,3,4],[0,4,1]])
    return trimesh.Trimesh(vertices=verts, faces=faces, process=True)


# === 4. Piastra ponte (in 2 sezioni con giunto a mezzo-legno) ================
def deck_half():
    half_len = p.DECK_L / 2 + p.DECK_JOINT / 2
    plate = T(box(half_len, p.DECK_W, p.DECK_T), 0, 0, p.DECK_T / 2)
    # mezzo-legno sull'estremo del giunto (rimuove la meta' inferiore)
    jx = half_len / 2
    lap = T(box(p.DECK_JOINT, p.DECK_W, p.DECK_T / 2 + 0.01),
            jx - p.DECK_JOINT / 2, 0, p.DECK_T / 4)
    plate = difference(plate, lap)
    # fori di giunzione nella zona di sovrapposizione
    for sy in (-1, 1):
        plate = difference(plate, T(vhole(p.DECK_SCREW_D, p.DECK_T * 4),
                                    jx - p.DECK_JOINT / 2, sy * p.DECK_W * 0.3, p.DECK_T / 2))
    # asole di drenaggio (l'acqua non deve ristagnare sul ponte)
    for sy in (-1, 1):
        s = R(slot(60, 6, p.DECK_T * 3), "z", 90)
        plate = difference(plate, T(s, -half_len * 0.18, sy * 55, p.DECK_T / 2))
    # fori di fissaggio alle traverse (vicino al bordo esterno X)
    edge_x = half_len - 12
    for sy in (-1, 1):
        plate = difference(plate, T(vhole(p.DECK_SCREW_D, p.DECK_T * 4),
                                    edge_x, sy * (p.DECK_W / 2 - 14), p.DECK_T / 2))
    return plate


# === 5. Staffa di prua (una per pontone, stampata x2; regge il cestello) =====
def bow_bracket():
    R0 = p.PONT_OD / 2
    wall = 6.0
    collar_w = 36.0                      # lungo l'asse del tubo (X)
    arm_len = 34.0
    # collare chiuso attorno al naso del pontone (si infila dall'estremita')
    collar = rounded_box(collar_w, p.PONT_OD + 2 * wall, p.PONT_OD + 2 * wall, 5)
    collar = difference(collar, xtube_cut(p.PONT_OD + p.CLEAR, collar_w + 4))
    lip_t = 10.0                         # labbro piu' spesso del foro (no tangenze)
    xlip = collar_w / 2 + arm_len + lip_t / 2
    # braccio in avanti, sul dorso del pontone (arriva fino al labbro)
    arm = T(box(arm_len + lip_t, p.BOW_W, p.BOW_T),
            collar_w / 2 + (arm_len + lip_t) / 2, 0, R0 + wall - p.BOW_T / 2)
    # labbro verso il basso (appoggio del cestello)
    lip = T(box(lip_t, p.BOW_W, p.BOW_DROP), xlip, 0, R0 + wall - p.BOW_DROP / 2)
    body = union(collar, arm, lip)
    # foro nel labbro per l'asta del cestello (lungo Y), Ø6 in parete da 10 -> 2 mm/lato
    rod = R(cyl(6, p.BOW_W + 6, 24), "x", 90)
    body = difference(body, T(rod, xlip, 0, R0 + wall - p.BOW_DROP * 0.7))
    return body


# Registro: nome -> (funzione, colore)
PARTS = {
    "pontone_endcap":   (pontone_endcap,  p.COL_CAP),
    "pontone_saddle":   (pontone_saddle,  p.COL_SADDLE),
    "corner_bracket":   (corner_bracket,  p.COL_BEAM),
    "deck_half":        (deck_half,       p.COL_DECK),
    "bow_bracket":      (bow_bracket,     p.COL_BOW),
}
