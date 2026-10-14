#
# pre.py - prepare an MRT flight-log CSV for Renode's csv2resd.
#
# WHY THIS EXISTS
#
# csv2resd does two things: it retypes cells (int/float/string/bytes) and it
# packs them straight into RESD sample fields. It does NOT convert units and it
# does NOT accept floats - RESD fields are integers. A log in engineering units
# with decimals therefore dies in struct.pack against int32.
#
# It also does not tolerate a UTF-8 BOM: csv.DictReader then sees the first
# column as '﻿Timestamp (ns)' and a -t "Timestamp (ns)" lookup fails with
# "<name> is invalid source". Referencing that column by index avoids it, and
# writing the output with encoding='utf-8-sig' here drops the BOM from every file
# we emit.
#
# UNITS
#
#   column        source   x              RESD stores
#   Pressure      hPa      100000         milliPascals (int64)
#   X/Y/Z Accel   mg       1000           micro-g (int32)
#   X/Y/Z AngRate dps      100000         1e-5 deg/s (int32)
#   Temp          degC     1000           milli-degC (int32)
#
# Angular rate is clamped because RESD's int32 tops out at 21474.8 deg/s. Logs
# commonly exceed that through 16-bit wraparound at the source (values pile up
# near 32767), which is a data problem, not a conversion problem - the count of
# clamped values is reported so you notice it.
#
# USAGE
#
#     py pre.py <input.csv> <output.csv>
#

import csv
import math
import sys

# int32 full-scale, used as the clamp ceiling for the vector/scalar int fields.
INT32 = 2147483647

# RESD ANGULAR_RATE is stored in units of 1e-5 rad/s, NOT 1e-5 deg/s. The
# LSM6DSR model proves this: it converts to the driver's deg/s with
# `RadiansToDegrees * sample.AngularRateX / 1e5`, i.e. it multiplies by 180/pi.
# Scaling dps as if the unit were deg/s made every gyro reading 57.3x too large
# (-140 dps was delivered as -8021 dps), which would badly skew any algorithm
# using angular rate. dps -> 1e-5 rad/s is x (1e5 / 57.29578).
DPS_TO_1E5_RAD = 100000.0 * math.pi / 180.0

# column name -> (scale factor, clamp limit in the scaled units, or None)
SCALE = {
    "Pressure":  (100000, None),
    "X Accel":   (1000, INT32),
    "Y Accel":   (1000, INT32),
    "Z Accel":   (1000, INT32),
    "X AngRate": (DPS_TO_1E5_RAD, INT32),
    "Y AngRate": (DPS_TO_1E5_RAD, INT32),
    "Z AngRate": (DPS_TO_1E5_RAD, INT32),
    "Temp":      (1000, INT32),
}

# Missing or non-numeric samples must still be integers, so map them to zero.
BLANKS = ("", "nan", "NaN", "NA", "null", "None")


def main():
    if len(sys.argv) != 3:
        print(__doc__.strip())
        return 1

    src, dst = sys.argv[1], sys.argv[2]

    # utf-8-sig both strips an incoming BOM and keeps us from emitting one.
    with open(src, "rt", encoding="utf-8-sig", newline="") as f:
        reader = csv.reader(f)
        try:
            header = next(reader)
        except StopIteration:
            print("pre: %s is empty" % src)
            return 1

        columns = [(i,) + SCALE[h] for i, h in enumerate(header) if h in SCALE]
        if not columns:
            print("pre: no recognised columns in %s" % src)
            print("pre: expected any of %s" % ", ".join(sorted(SCALE)))
            return 1

        unknown = [h for i, h in enumerate(header) if i not in {c[0] for c in columns}]
        if unknown:
            print("pre: passing through unconverted: %s" % ", ".join(unknown))

        out = csv.writer(open(dst, "wt", newline="", encoding="utf-8"))
        out.writerow(header)

        clamped = 0
        blanks = 0
        rows = 0
        for row in reader:
            for index, factor, limit in columns:
                if index >= len(row) or row[index].strip() in BLANKS:
                    row[index] = "0"
                    blanks += 1
                    continue
                value = int(round(float(row[index]) * factor))
                if limit is not None and not -limit <= value <= limit:
                    value = max(-limit, min(limit, value))
                    clamped += 1
                row[index] = str(value)
            out.writerow(row)
            rows += 1

    print("pre: %d rows, %d columns converted, %d blanks zeroed, %d clamped -> %s"
          % (rows, len(columns), blanks, clamped, dst))
    if clamped:
        print("pre: WARNING %d values hit the int32 limit. If these are angular rate,"
              % clamped)
        print("pre:          values near 32767 usually mean 16-bit wrap in the logger.")
    return 0


if __name__ == "__main__":
    sys.exit(main())