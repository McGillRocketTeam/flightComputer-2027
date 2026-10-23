"""Python script for running arbitrary tasks via PIO"""
Import("env")

env.AddCustomTarget(
    name="generate_eeprom_map",
    dependencies=None,
    actions=['python modules/nonvolatile/generate_eeprom_map.py'],
    title="Generate/Update EEPROM map",
    description="Re-generates the auto-generated EEPROM map in MRT_eeprom_map.h"
)