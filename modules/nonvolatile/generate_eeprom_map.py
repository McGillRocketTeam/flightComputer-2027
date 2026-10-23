"""Generates EEPROM Address & Size Maps, as well as Parameter struct from a list of parameters"""
# 2026-10-23 Aneurin H

import sys
from pprint import pprint
from datetime import datetime
from pathlib import Path

NUM_BLOCKS = 4
BLOCK_SIZE = 256
PAGE_SIZE = 16

SCRIPT_DIR = Path(__file__).parent
FILENAME = SCRIPT_DIR / "MRT_eeprom_map.h"

TYPE_SIZES = {"uint8_t"  : 1, 
              "uint16_t" : 2, 
              "float"    : 4,
              "uint32_t" : 4}

BLOCKS = {
    0: [("flight_stage", "uint8_t"),
        ("radio_freq", "float"),
        ("radio_cr", "uint8_t"),
        ("radio_sf", "uint8_t"),
        ("radio_bw", "uint8_t"),
        ("fc_state_reset", "uint8_t"),
        ("fc_state_iwdg", "uint8_t"),
        ("fc_state_WU", "uint8_t"),
    ],
    1: [("prop_armed", "uint8_t"),
        ("recov_armed", "uint8_t"),
        ("sd_open", "uint8_t"),
        ("camera_rec","uint8_t"),
        ("true_apogee", "float"),
        ("true_apogee_time", "uint32_t"),
        ("recov_pad_altitude", "float"),
        ("recov_flag", "uint8_t"),
    ],
    2: [("rtc_day", "uint16_t"),
        ("rtc_hour", "uint8_t"),
        ("rtc_minute", "uint8_t"),
        ("rtc_second", "uint8_t"),
        ("rtc_subsec", "uint16_t"),
    ],
    3: []}

"""Generates a list of overall parameters, where each entry is a dict containing name, type, block, size, blockoffset, and address"""
def build_params():
    params = []
    seen = set()
    for block, fields in sorted(BLOCKS.items()):
        if block >= NUM_BLOCKS:
            sys.exit(f"Block: {block} out of bounds for NUM_BLOCKS: {NUM_BLOCKS}")
        
        # Block relative offset
        offset = 0
        for name, type in fields:
            size = TYPE_SIZES.get(type, 0)

            if(name in seen):
                sys.exit(f"Parameter {name} has a duplicated entry")
            seen.add(name)

            if size < 1:
                sys.exit(f"Parameter {name} has invalid or 0-size")

            if((offset + size) > BLOCK_SIZE):
                # Driver does not implement continuous writes beyond page boundaries, so data must remain in a single block
                sys.exit(f"Parameter {name} extends beyond block boundaries")
            if((offset // PAGE_SIZE) != ((offset + size - 1) // PAGE_SIZE)):
                # Checks all parameters are on a single PAGE_SIZE page. This is to reduce bus transfers
                sys.exit(f"Parameter {name} extends beyond page boundaries in block {block}. Start {offset}, End {(offset + size)}")
            
            addr = (block << 8) + offset

            params.append(dict(name=name, type=type, block=block, size=size, offset = offset, addr=addr))
            offset += size

    return params

def generate(params):
    out = []
    w = out.append
    
    # Max width of names, used for justification
    w_name  = max(len(p["name"]) for p in params)
    w_type = max(len(p['type']) for p in params)

    w("// AUTO-GENERATED FILE; DO NOT EDIT (Source: generate_eeprom_map.py)")
    w(f"// Last Generated: {datetime.now()}")
    w("")
    w("#pragma once")
    w("#include <stdint.h>")
    w("#include <stddef.h>")
    w("#include <assert.h>")
    w("#include <nonvolatile/24xx08/24xx08_address.h>")
    w("#define PACKED_STRUCT typedef struct __attribute__((packed))")
    w("")
    w("// Check Generator Config against driver. These must match for generator to reject block and page violations correctly")
    w(f"static_assert(EEPROM_NUM_BLOCKS == {NUM_BLOCKS}, \"Generator NUM_BLOCKS does not match\");")
    w(f"static_assert(EEPROM_SIZE_BYTES == {NUM_BLOCKS * BLOCK_SIZE}, \"Generator Block Size does not match\");")
    w(f"static_assert(EEPROM_PAGE_SIZE == {PAGE_SIZE}, \"Generator Page Size does not match\");")
    w("")

    # Param struct creation
    w("// START PARAM STRUCT")
    w("PACKED_STRUCT{")
    for p in params:
        w(f"    {p['type']:<{w_type}} {p['name']};")

    w(f"}} EEPROM_STRUCT;")
    w("// END PARAM STRUCT")
    w("")

    # Param Enum Generation
    w("// START PARAM ENUM")
    w("enum class NV_Param : uint8_t {")
    for p in params:
        w(f"    {p['name'].upper()}," + " " * (w_name - len(p['name'])) + f" // Block: {p['block']}")
    w("    NUM_PARAMS,")
    w("};")
    w("// END PARAM ENUM")
    w("")

    # Param Address Mappings
    w("// START PARAM ADDRESS TABLE")
    w("static uint16_t nv_param_addresses [(uint8_t) NV_Param::NUM_PARAMS]{")
    for p in params:
        w(f"    [(uint8_t) NV_Param::{p['name'].upper()}] " + " " * (w_name-len(p['name'])) + f" = 0x{p['addr']:X},")
    w("};")
    w("// END PARAM ADDRESS TABLE")
    w("")

    # Param Size Mappings
    w("// START PARAM SIZE TABLE")
    w("static uint16_t nv_param_sizes [(uint8_t) NV_Param::NUM_PARAMS]{")
    for p in params:
        w(f"    [(uint8_t) NV_Param::{p['name'].upper()}] " + " " * (w_name-len(p['name'])) + f" = {p['size']},")
    w("};")
    w("// END PARAM SIZE TABLE")
    w("")
    w("// END GENERATED CONTENT")

    return "\n".join(out) + "\n"


if __name__ == "__main__":
    with open(FILENAME, "w") as f:
        f.write(generate(build_params()))

    # pprint(generate(build_params()))