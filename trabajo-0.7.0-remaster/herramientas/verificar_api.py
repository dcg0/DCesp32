#!/usr/bin/env python3
"""Audita el contrato entre el JavaScript embebido y las rutas HTTP del sketch."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
INO = ROOT / "ESP32-OBDC2" / "ESP32-OBDC2.ino"
DATA = ROOT / "ESP32-OBDC2" / "dc_data.h"
source = INO.read_text(encoding="utf-8")
header = DATA.read_text(encoding="utf-8")
page = source.split('R"HTML(', 1)[1].split(')HTML";', 1)[0]
js = page.split("<script>", 1)[1].split("</script>", 1)[0]

handlers = set()
for match in re.finditer(r'server\.on\("([^"]+)"(?:,\s*HTTP_(GET|POST))?', source):
    handlers.add((match.group(1), (match.group(2) or "GET").upper()))

calls = set()
for match in re.finditer(r"\bfetch\s*\(\s*(['\"])([^'\"]+)\1\s*(?:,\s*\{([^}]*)\})?", js):
    opts = match.group(3) or ""
    method_match = re.search(r"\bmethod\s*:\s*(['\"])(GET|POST|PUT|DELETE)\1", opts, re.I)
    method = method_match.group(2).upper() if method_match else "GET"
    calls.add((match.group(2), method))

missing = sorted(calls - handlers)
if missing:
    raise SystemExit("FAIL: cliente llama rutas/métodos no registrados: " + repr(missing))

required = {("/data", "GET"), ("/cfg", "GET"), ("/status", "GET")}
if not required.issubset(handlers) or not required.issubset(calls):
    raise SystemExit("FAIL: falta alguna ruta básica /data, /cfg o /status")
for marker in ('\\"d\\":{', '\\"id\\":\\"', '\\"n\\":\\"', '\\"u\\":\\"', '\\"w\\":'):
    if marker not in header:
        raise SystemExit(f"FAIL: dc_data.h no contiene el campo API esperado {marker}")
for marker in ("j.d[p.id]", "fetch('/data'", "fetch('/cfg'"):
    if marker not in js:
        raise SystemExit(f"FAIL: la interfaz no consume el contrato esperado: {marker}")

assets = set()
for match in re.finditer(r"\bsrc\s*=\s*['\"]?(\/[^\s'\">]+)", page, re.I):
    assets.add(match.group(1).split("?", 1)[0])
for match in re.finditer(r"url\(\s*['\"]?(\/[^\s)'\"]+)", page, re.I):
    assets.add(match.group(1).split("?", 1)[0])
missing_assets = sorted(path for path in assets if (path, "GET") not in handlers)
if missing_assets:
    raise SystemExit("FAIL: recursos sin handler GET: " + repr(missing_assets))

fw = re.search(r'#define\s+FW_VERSION\s+"([^"]+)"', source)
print(f"PASS · {fw.group(1) if fw else 'FW sin etiqueta'} · {len(calls)} rutas/métodos cliente registrados en el servidor · recursos web verificados")
print("Rutas consumidas:")
for path, method in sorted(calls):
    print(f"  {method:4} {path}")
print("Contrato comprobado: /data -> j.d[id]; /cfg -> id,n,u,w.")
