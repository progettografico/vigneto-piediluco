"""build.py — genera TUTTI gli output dello scafo: STL stampabili + render PNG.

Uso:  python3 build.py
Richiede: numpy trimesh manifold3d matplotlib  (pip install ...).
Output:  cad/stl/*.stl  e  cad/render/*.png
"""
import params as p
import parts
import assembly
import util as u

# Nomi leggibili per i titoli dei render
NICE = {
    "pontone_endcap": "Tappo pontone",
    "pontone_saddle": "Sella pontone (cavallotto)",
    "corner_bracket": "Staffa angolare ponte/traversa",
    "deck_half":      "Ponte — sezione (x2)",
    "bow_bracket":    "Staffa di prua (x2)",
}
# Camera per pezzo (elev, azim)
CAM = {
    "pontone_endcap": (28, -50),
    "pontone_saddle": (24, -58),
    "corner_bracket": (26, -54),
    "deck_half":      (40, -60),
    "bow_bracket":    (24, -54),
}


def build_parts():
    print("== Pezzi stampabili (STL + render) ==")
    for name, (fn, col) in parts.PARTS.items():
        m = fn()
        wt = u.export(m, name)
        e = m.extents
        sub = (f"STL: stl/{name}.stl  ·  ingombro {e[0]:.0f}x{e[1]:.0f}x{e[2]:.0f} mm  ·  "
               f"watertight={'si' if wt else 'NO'}")
        elev, azim = CAM[name]
        u.render([(m, col, 1.0)], f"part_{name}.png", elev=elev, azim=azim,
                 title=NICE[name], subtitle=sub, zoom=0.92)


def build_assembly():
    print("== Assieme scafo ==")
    sub = (f"Pontoni Ø{p.PONT_OD:.0f} L{p.PONT_LEN:.0f} · interasse {p.PONT_SPACING:.0f} · "
           f"pannello {p.PANEL_L:.0f}x{p.PANEL_W:.0f} (quote in mm)")
    u.render(assembly.hull(0.0), "assembly.png", elev=24, azim=-58,
             title="Scafo — assieme", subtitle=sub, zoom=1.4)
    u.render(assembly.hull(0.0), "assembly_top.png", elev=58, azim=-72,
             title="Scafo — vista dall'alto", subtitle=sub, zoom=1.4)
    print("== Vista esplosa ==")
    u.render(assembly.hull(1.0), "assembly_exploded.png", elev=18, azim=-58,
             title="Scafo — vista esplosa",
             subtitle="livelli: pontoni · selle · traverse · ponte · pannello", zoom=1.15)


if __name__ == "__main__":
    build_parts()
    build_assembly()
    print("\nFatto. STL in cad/stl/ , render in cad/render/")
