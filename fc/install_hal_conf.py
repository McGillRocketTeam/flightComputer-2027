"""
tools/install_hal_conf.py

PlatformIO `pre:` build script.

Installs the CubeMX/stm32pio-generated stm32f4xx_hal_conf.h from the project
tree into the active stm32cube framework, so it is the configuration actually
compiled into every translation unit.

Why a copy rather than an include-path trick
--------------------------------------------
GCC resolves `#include "stm32f4xx_hal_conf.h"` (inside stm32f4xx_hal.h) by
searching the directory of the *including file* first. The conf therefore only
takes effect if it sits next to stm32f4xx_hal.h. Dropping it into the
framework's Inc directory makes it a proper sibling, so the stock include chain
is used unchanged.

The alternative, `-include stm32f4xx_hal_conf.h`, cannot work: it enters the
chain at the conf file, which reaches stm32f4xx_hal_def.h -> stm32f4xx.h ->
stm32f4xx_hal.h *before* __STM32F4xx_HAL_H is set. hal_def.h includesstm32f4xx.h at its top but defines HAL_StatusTypeDef lower down, so hal.h's
body executes while that type does not yet exist:

    error: unknown type name 'HAL_StatusTypeDef'

Running from a `pre:` hook means the install is repeated on every build, so it
survives `pio pkg update` and framework reinstalls.

platformio.ini
--------------
    extra_scripts = pre:tools/install_hal_conf.py
    build_flags   = ${common.build_flags}
                    -I fc/Inc
"""

import glob
import os
import shutil
import stat

Import("env")  # noqa: F821  -- injected by PlatformIO's SCons environment


def _core_dir():
    """Locate PlatformIO's core (packages) directory across v5/v6 layouts."""
    for token in ("$PROJECT_CORE_DIR", "$PLATFORMIO_CORE_DIR"):
        try:
            candidate = env.subst(token)
        except Exception:
            candidate = None
        if candidate and os.path.isdir(candidate):
            return candidate

    from_env = os.environ.get("PLATFORMIO_CORE_DIR")
    if from_env and os.path.isdir(from_env):
        return from_env

    return os.path.join(os.path.expanduser("~"), ".platformio")


def _hal_inc_dirs(core_dir):
    """All <framework>/Drivers/*_HAL_Driver/Inc directories that are installed."""
    pattern = os.path.join(core_dir, "packages", "framework-stm32cube*")
    found = []
    for framework in sorted(glob.glob(pattern)):
        drivers = os.path.join(framework, "Drivers")
        found.extend(sorted(glob.glob(os.path.join(drivers, "*_HAL_Driver", "Inc"))))
    return found


def _fail(message):
    raise SystemExit("\n[install_hal_conf] ERROR: %s\n" % message)


def _install(source, destination):
    """Copy source over destination, clearing a read-only bit if present."""
    try:
        shutil.copyfile(source, destination)
    except PermissionError:
        os.chmod(destination, os.stat(destination).st_mode | stat.S_IWRITE)
        shutil.copyfile(source, destination)


def main():
    project_dir = env.subst("$PROJECT_DIR")
    source = os.path.join(project_dir, "fc", "Inc", "stm32f4xx_hal_conf.h")

    print("")
    print("| install_hal_conf: project stm32f4xx_hal_conf.h")

    if not os.path.isfile(source):
        _fail(
            "source not found: %s\n"
            "   Generate it from the .ioc first (CubeMX / stm32pio)." % source
        )

    # family guard: stm32f4xx_hal_conf.h only belongs next to stm32f4xx_hal.h
    sibling = os.path.basename(source).replace("_conf.h", ".h")

    core_dir = _core_dir()
    targets = [
        directory
        for directory in _hal_inc_dirs(core_dir)
        if os.path.isfile(os.path.join(directory, sibling))
    ]

    if not targets:
        _fail(
            "no '*_HAL_Driver/Inc' containing %s found under %s\n"
            "   Is `framework = stm32cube...` set for this environment?"
            % (sibling, os.path.join(core_dir, "packages"))
        )

    for directory in targets:
        destination = os.path.join(directory, os.path.basename(source))
        _install(source, destination)
        print("|   installed: %s" % destination)

    print("")


main()