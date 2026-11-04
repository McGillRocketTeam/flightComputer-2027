"""Generates EEPROM Address & Size Maps, as well as Parameter struct from a list of parameters"""
# 2026-10-23 Aneurin H

# pio run -t generate_sd

import sys
from pprint import pprint
from datetime import datetime
from pathlib import Path
import yaml

SCRIPT_DIR = Path(__file__).parent
FILENAME = "MRT_sd_data"
FILENAME_H = SCRIPT_DIR / (FILENAME + ".h")
FILENAME_CPP = SCRIPT_DIR / (FILENAME + ".cpp")


TYPE_SIZES_C = {"uint8"  : 1,
              "int8"   : 1,
              "char"   : 1, 
              "uint16" : 2,
              "int16"  : 2, 
              "float"  : 4,
              "int32"  : 4,
              "uint32" : 4}

TYPE_NAMES_C = {"uint8"  : "uint8_t",
                "int8"   : "int8_t",
                "char"   : "char", 
                "uint16" : "uint16_t",
                "int16"  : "int16_t", 
                "float"  : "float",
                "int32"  : "int32_t",
                "uint32" : "uint32_t"}

"""Generates a list of overall parameters"""
def build_params(data):
    params = []
    flags = []
    flag_count = 0
    seen = set()

    for atomic_name, atomic in data['atomics'].items():
        fields = []

        for field in atomic.get('fields', []):
            if(field['name'] in seen):
                raise ValueError("Schema contains duplicate named entries (" + field['name'] + ")")
            fields.append((field['name'], field['type']))
            seen.add(field['name'])

        for flag in atomic.get('flags', []):
            if flag.upper() in seen:
                raise ValueError("Schema contains duplicate named entries (" + flag.upper() + ")")
            flags.append((flag, flag_count))
            flag_count += 1
            seen.add(flag.upper())
        params.append((atomic_name, fields))
    return params, flags

def generate(params, flags, version):
    out = []
    w = out.append

    size = 0
    atomic_size = 0

    flag_bytes = -(-len(flags)//8)

    w("// AUTO-GENERATED FILE; DO NOT EDIT (Source: generate_sd_data.py)")
    w(f"// Last Generated: {datetime.now()}")
    w("")
    w("#pragma once")
    w("#include <stdint.h>")
    w("#include <stddef.h>")
    w("#include <assert.h>")
    w("")

    w(f"#define ATOMIC_VERSION \"{version}\"")
    # Generate Atomic Types
    w("// Designed for usage with 16-bit atomic bitmask")
    w("enum SDAtomicTypes {")
    pos = 0
    for atomic in params:
        if len(atomic[1]) == 0:
            continue
        w(f"    SD_AT_{atomic[0].upper()} = ({pos}),")
        pos += 1
    w(f"    SD_AT_FLAGS = ({pos}),")
    w(f"    SD_AT_TOTAL = ({1 + pos}),")
    w("};")
    w("")
    w("#define SD_ATOMIC_BITMASK_LEN 2")
    w(f"static_assert(SD_AT_TOTAL <= (SD_ATOMIC_BITMASK_LEN * 8), \"MORE THAN 16 SD ATOMICS DEFINED\");")
    w("")

    # Generate Atomics
    for atomic in params:
        if len(atomic[1]) == 0:
            continue

        w(f"struct __attribute__((__packed__)) {atomic[0]}_data_SD_t {{")
        atomic_size = 0

        for param in atomic[1]:
            if param[1] in TYPE_NAMES_C:
                w(f"    {TYPE_NAMES_C[param[1]]} {param[0]};")
            else:
                raise ValueError("Schema contains type type (" + param[1] + ") not present in generator")

        
            if param[1] in TYPE_SIZES_C:
                atomic_size += TYPE_SIZES_C[param[1]]
            else:
                raise ValueError("Schema contains type (" + param[1] + ") not present in generator")            
        
        size += atomic_size
        w(f"}};")
        w("")
        w(f"struct __attribute__((__packed__)) {atomic[0]}_ptr_SD_t {{")
        for param in atomic[1]:
            w(f"    {TYPE_NAMES_C[param[1]]}* {param[0]};")
        w("};")
        w("")
        w(f"union {atomic[0]}_atomic_SD_t {{")
        w(f"    {atomic[0]}_data_SD_t data;")
        w(f"    uint8_t bytes[{atomic_size}];")
        w("};")
        w("")

    w("")
    # Generate Flag Atomic
    w("struct __attribute__((__packed__)) flags_data_SD_t {")
    for flag in flags:
        w(f"    bool {flag[0]} : 1;")
    w("};")
    w("")
    flag_size = -(-len(flags)//8)
    w("union flags_atomic_SD_t {")
    w("    flags_data_SD_t data;")
    w(f"    uint8_t bytes[{flag_size}];")
    w(f"}};")
    w("")
    w(f"struct __attribute__((__packed__)) flags_ptr_SD_t {{")
    for flag in flags:
        w(f"    bool* {flag[0]};")
    w("};")
    w("")
    for atomic in params:
        if len(atomic[1]) == 0:
            continue
        w(f"inline void fill_{atomic[0]}({atomic[0]}_ptr_SD_t& ptr, {atomic[0]}_atomic_SD_t& out){{")
        for param in atomic[1]:
            w(f"    out.data.{param[0]} = *ptr.{param[0]};")
        w("};")
        w("")

    w(f"inline void fill_flags(flags_ptr_SD_t& ptr, flags_atomic_SD_t& out){{")
    for flag in flags:
        w(f"    out.data.{flag[0]} = *ptr.{flag[0]};")
    w("};")
    w("")

    w("extern const uint16_t SD_AT_SIZE[SD_AT_TOTAL];")
    w("")

    # Generate Flag offsets
    for flag in flags:
        w(f"#define SD_FLAG_{(flag[0].upper())} (1u<<{flag[1]})")
    return "\n".join(out) + "\n"

def generateCPP(params, flags):
    out = []
    w = out.append

    w("// AUTO-GENERATED FILE; DO NOT EDIT (Source: generate_sd_data.py)")
    w(f"// Last Generated: {datetime.now()}")
    w("")
    w(f"#include \"modules/sd_card/data/{FILENAME}.h\"")
    w("")
    w("const uint16_t SD_AT_SIZE[SD_AT_TOTAL]{")
    for atomic in params:
        if len(atomic[1]) == 0:
            continue
        w(f"    sizeof({atomic[0]}_atomic_SD_t),")
    w(f"    sizeof(flags_atomic_SD_t),")
    w("};")

    return "\n".join(out) + "\n"

"""Printout of EEPROM map"""
def printReport(params, flags):
    print(f"[SD Fields]")
    print("[Parameters]")
    pprint(params)
    print("[Flags]")
    pprint(flags)
    size = 0
    for atomic in params:
        for param in atomic[1]:
            # print(f"{param}")
            size += TYPE_SIZES_C[param[1]]
    size += -(-len(flags)//8)
    print(f"[Max Size: {size}]")

if __name__ == "__main__":
    with open("modules/sd_card/data/sd_fields.yaml") as stream:
        data = yaml.safe_load(stream)

    with open(FILENAME_H, "w") as f_h, open(FILENAME_CPP, "w") as f_cpp:
        params, flags = build_params(data)

        f_h.write(generate(params, flags, data['version']))
        f_cpp.write(generateCPP(params, flags))

        printReport(params, flags)
