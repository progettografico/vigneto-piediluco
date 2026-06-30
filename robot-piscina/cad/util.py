"""util.py — primitive parametriche, operazioni booleane, export STL e render PNG.

Centralizza tutto cio' che e' riutilizzato dai pezzi, per non duplicare codice.
Motore geometrico: trimesh + manifold3d (booleani robusti, mesh watertight).
Render: matplotlib (Agg, headless) con shading Lambert e ordinamento per profondita'.
"""
import os
import numpy as np
import trimesh
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d.art3d import Poly3DCollection

HERE   = os.path.dirname(os.path.abspath(__file__))
STLDIR = os.path.join(HERE, "stl")
IMGDIR = os.path.join(HERE, "render")
os.makedirs(STLDIR, exist_ok=True)
os.makedirs(IMGDIR, exist_ok=True)

# --- Primitive ---------------------------------------------------------------
def cyl(d, h, sections=48):
    """Cilindro pieno lungo Z, centrato nell'origine."""
    return trimesh.creation.cylinder(radius=d / 2.0, height=h, sections=sections)

def tube(od, idd, length, sections=64):
    """Tubo (anello) lungo Z, centrato nell'origine."""
    return trimesh.creation.annulus(r_min=idd / 2.0, r_max=od / 2.0,
                                    height=length, sections=sections)

def box(x, y, z):
    """Parallelepipedo centrato nell'origine."""
    return trimesh.creation.box(extents=(x, y, z))

def rounded_box(x, y, z, r, sections=24):
    """Parallelepipedo con spigoli verticali raccordati (raggio r)."""
    r = min(r, x / 2 - 0.01, y / 2 - 0.01)
    parts = [box(x - 2 * r, y, z), box(x, y - 2 * r, z)]
    for sx in (-1, 1):
        for sy in (-1, 1):
            c = cyl(2 * r, z, sections)
            c.apply_translation((sx * (x / 2 - r), sy * (y / 2 - r), 0))
            parts.append(c)
    return union(*parts)

def slot(length, width, height, sections=24):
    """Asola (stadio): rettangolo con estremi arrotondati, lungo X. Per sottrazione."""
    b = box(length, width, height)
    parts = [b]
    for sx in (-1, 1):
        c = cyl(width, height, sections)
        c.apply_translation((sx * length / 2, 0, 0))
        parts.append(c)
    return union(*parts)

# --- Trasformazioni ----------------------------------------------------------
def T(mesh, x=0, y=0, z=0):
    m = mesh.copy(); m.apply_translation((x, y, z)); return m

def R(mesh, ax, deg):
    m = mesh.copy()
    axis = {"x": (1, 0, 0), "y": (0, 1, 0), "z": (0, 0, 1)}[ax]
    m.apply_transform(trimesh.transformations.rotation_matrix(np.radians(deg), axis))
    return m

# --- Booleani ----------------------------------------------------------------
def union(*meshes):
    ms = [m for m in meshes if m is not None]
    return trimesh.boolean.union(ms, engine="manifold") if len(ms) > 1 else ms[0]

def difference(a, *b):
    return trimesh.boolean.difference([a, *b], engine="manifold")

def intersection(*meshes):
    return trimesh.boolean.intersection(list(meshes), engine="manifold")

# --- Export / verifica -------------------------------------------------------
def normalize(mesh):
    """Normalizza la mesh passandola in manifold3d (output manifold per costruzione):
    fonde geometrie coincidenti e produce STL solidi e watertight."""
    return trimesh.boolean.union([mesh], engine="manifold")

def export(mesh, name):
    """Normalizza, salva STL in cad/stl e verifica ricaricando da disco (watertight)."""
    m = normalize(mesh)
    path = os.path.join(STLDIR, name + ".stl")
    m.export(path)
    r = trimesh.load(path)                 # verifica sull'STL reale, non solo in memoria
    e = r.extents
    wt = r.is_watertight
    print(f"  STL {name:22s} watertight={wt!s:5s} "
          f"bbox={e[0]:.0f}x{e[1]:.0f}x{e[2]:.0f}mm vol={r.volume/1000:.1f}cm3")
    return wt

# --- Render ------------------------------------------------------------------
_LIGHT = np.array([0.35, 0.45, 0.82]); _LIGHT = _LIGHT / np.linalg.norm(_LIGHT)

def _hex2rgb(h):
    h = h.lstrip("#")
    return np.array([int(h[i:i + 2], 16) / 255.0 for i in (0, 2, 4)])

def _tess(mesh, max_edge=18.0):
    """Suddivide le facce grandi: il sort per-profondita' del render diventa piu' fine
    (riduce gli artefatti su piani estesi). Le normali restano corrette (facce planari)."""
    from trimesh.remesh import subdivide_to_size
    v, f = subdivide_to_size(mesh.vertices, mesh.faces, max_edge=max_edge)
    m = trimesh.Trimesh(vertices=v, faces=f, process=False)
    return m.triangles, m.face_normals


def render(parts, out, elev=24, azim=-58, title=None, subtitle=None,
           annotations=None, size=(1500, 1050), zoom=1.3):
    """parts: lista di (mesh, colore_hex, alpha). Salva un PNG in cad/render."""
    fig = plt.figure(figsize=(size[0] / 100, size[1] / 100), dpi=100)
    ax = fig.add_subplot(111, projection="3d")
    ax.set_proj_type("persp")

    all_tris, all_cols = [], []
    mn = np.array([1e9, 1e9, 1e9]); mx = -mn
    for mesh, color, alpha in parts:
        tris, n = _tess(mesh)                       # (N,3,3), (N,3)
        shade = 0.32 + 0.68 * np.clip(n @ _LIGHT, 0, 1)
        base = _hex2rgb(color)
        cols = np.clip(base[None, :] * shade[:, None], 0, 1)
        cols = np.concatenate([cols, np.full((len(cols), 1), alpha)], axis=1)
        all_tris.append(tris); all_cols.append(cols)
        v = mesh.vertices
        mn = np.minimum(mn, v.min(0)); mx = np.maximum(mx, v.max(0))

    tris = np.concatenate(all_tris); cols = np.concatenate(all_cols)
    coll = Poly3DCollection(tris, facecolors=cols, edgecolors="none")
    coll.set_zsort("average")
    ax.add_collection3d(coll)

    dx, dy, dz = (mx - mn)
    ax.set_xlim(mn[0], mx[0]); ax.set_ylim(mn[1], mx[1]); ax.set_zlim(mn[2], mx[2])
    ax.set_box_aspect((dx, dy, dz), zoom=zoom)
    ax.view_init(elev=elev, azim=azim)
    ax.set_axis_off()

    if annotations:
        for p1, p2, txt in annotations:
            ax.plot(*zip(p1, p2), color="#b03a1a", lw=1.2)
            mid = (np.array(p1) + np.array(p2)) / 2
            ax.text(*mid, txt, color="#b03a1a", fontsize=10, ha="center")

    if title:
        fig.text(0.04, 0.95, title, fontsize=19, weight="bold", color="#0c2230")
    if subtitle:
        fig.text(0.04, 0.915, subtitle, fontsize=12, color="#5a6b73")

    fig.subplots_adjust(left=0, right=1, bottom=0, top=1)
    fig.savefig(os.path.join(IMGDIR, out), dpi=100, facecolor="white")
    plt.close(fig)
    print(f"  PNG {out}")
