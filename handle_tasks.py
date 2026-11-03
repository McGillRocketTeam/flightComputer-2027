"""Python script for running arbitrary tasks via PIO"""
Import("env")

env.AddCustomTarget(
    name="generate_eeprom_map",
    dependencies=None,
    actions=['"$PYTHONEXE" modules/nonvolatile/generate_eeprom_map.py'],
    title="Generate/Update EEPROM map",
    description="Re-generates the auto-generated EEPROM map in MRT_eeprom_map.h"
)

env.AddCustomTarget(
    name="generate_sd",
    dependencies=None,
    actions=['"$PYTHONEXE" -m pip install pyyaml','"$PYTHONEXE" modules/sd_card/data/generate_sd_data.py'],
    title="Generate/Update SD Card data & parser",
    description="Re-generates the auto-generated SD Card data and parser files, ensuring they are synchronized"
)

env.AddCustomTarget(
    name="decode_sd",
    dependencies=None,
    actions=['"$PYTHONEXE" modules/sd_card/data/decode_sd.py $PROGRAM_ARGS'],
    title="Decode SD Card data",
    description="Extracts the SD card file, and writes it to a CSV"
)