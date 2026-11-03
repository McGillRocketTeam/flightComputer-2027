"""Decodes SD Card Data"""
# 2026-10-30 Aneurin H

# pio run -t decode_sd FILENAME

import sys
import yaml
import csv
import struct

TYPE_SIZES_C = {"uint8"  : 1,
              "int8"   : 1,
              "char"   : 1, 
              "uint16" : 2,
              "int16"  : 2, 
              "float"  : 4,
              "int32"  : 4,
              "uint32" : 4}

TYPE_SPECIFERS_C = {"uint8"  : "B",
                    "int8"   : "b",
                    "char"   : "1s", 
                    "uint16" : "H",
                    "int16"  : "h", 
                    "float"  : "f",
                    "int32"  : "i",
                    "uint32" : "I"}

TYPE_NAMES_C = {"uint8"  : "uint8_t",
                "int8"   : "int8_t",
                "char"   : "char", 
                "uint16" : "uint16_t",
                "int16"  : "int16_t", 
                "float"  : "float",
                "int32"  : "int32_t",
                "uint32" : "uint32_t"}

def listParams(data):
    params = []
    seen = set()
    flags = []

    for atomic_name, atomic in data['atomics'].items():
        fields = []
        

        for field in atomic.get('fields', []):
            if(field['name'] in seen):
                continue
            fields.append((field['name'], field['type'], field.get('scale', 1), field.get('add_offset',0)))
            seen.add(field['name'])

        for flag in atomic.get('flags',[]):
            if flag in seen:
                continue
            flags.append(flag)
            seen.add(flag)

        if(len(atomic.get('fields', [])) != 0):
            params.append((atomic_name, fields))



    return params, flags

def parse(params, flags, bytes, filename):
    with open(filename.removesuffix(".MRT") + ".csv", 'w', newline="") as file:
        writer = csv.writer(file)

        # HEADER WRITE
        header = []
        for atomic_name, fields in params:
            header.extend([f[0] for f in fields])
        for flag in flags:
            header.append(flag)
        writer.writerow(header)

        offset = 0
        while offset < len(bytes):
            offset = parseFrame(params, flags, bytes, writer, offset)


def parseFrame(params, flags, bytes, writer, offset):
    bitmask = int.from_bytes(bytes[offset:offset+2], byteorder='little')
    
    frame = dict.fromkeys([f[0] for _, fields in params for f in fields] + flags)

    offset += 2

    i = 0
    while i < 16:
        if(bitmask & (1<<i)):
            if i < len(params):
                for field in params[i][1]:
                    raw = struct.unpack_from('<' + TYPE_SPECIFERS_C[field[1]], bytes, offset)[0]
                    scale = field[2]
                    add_offset = field[3]
                    if field[1] == 'char':
                        frame[field[0]] = raw
                    else:
                        frame[field[0]] = (raw / scale) + add_offset
                    offset += TYPE_SIZES_C[field[1]]
            elif i == len(params):
                # These are our flags
                # + 7 is to ensure that it packs out byte alignment
                flag_bytes = (len(flags) + 7) // 8
                flag_value = int.from_bytes(bytes[offset:offset+flag_bytes], byteorder='little')
                offset += flag_bytes

                for bit, name in enumerate(flags):
                    frame[name] = bool(flag_value & (1<<bit))
        i+=1

    writer.writerow(frame.values())
    return offset

if __name__ == "__main__":
    with open("modules/sd_card/data/sd_fields.yaml") as stream:
        data = yaml.safe_load(stream)
    if(len(sys.argv) == 1):
        raise ValueError("Provide SD File Argument")
    with open(sys.argv[1], 'rb') as sd_data:
        file_bytes = sd_data.read()

    paramList, flags = listParams(data)
    parse(paramList, flags, file_bytes, sys.argv[1])