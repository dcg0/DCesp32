#!/bin/bash
cd "$(dirname "$0")/.."
python3 - <<'PY'
import os
out=["#pragma once","#include <Arduino.h>"]
for n in ("logo_web","portada_web"):
    d=open(f"imagenes/{n}.webp","rb").read()
    out.append(f"const size_t {n}_len={len(d)};")
    out.append(f"const uint8_t {n}_data[] PROGMEM={{"+",".join(str(b) for b in d)+"};")
open("ESP32-OBDC2/dc_logos.h","w").write("\n".join(out)+"\n")
print("dc_logos.h regenerado")
PY
