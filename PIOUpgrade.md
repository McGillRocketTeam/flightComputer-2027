# **FC PIO Upgrade**

## Theory

PlatformIO allows us to compile, upload, debug, and test the code. PlatformIO supports STM32 MCUs, and allows us to define custom boards using them, however, its does not natively integrate with STM32CubeMX (ST's utility to generate HAL files, and manage MCU configuration).\
\
The solution to this is to use [stm32pio](https://github.com/ussserrr/stm32pio), as [recommended by PlatformIO](https://docs.platformio.org/en/latest/frameworks/stm32cube.html#using-with-stm32cubemx). This is a separate program that will invoke generation from CubeMX, and then restructure the generated files into the Inc & Src directories, as PlatformIO expects.\
\
Therefore, Inc & Src will be generated HAL files for each project, MRT will contain MRT board-specific code. and the top-level Drivers folder will contain low-level drivers for the various sensors and devices the MRT boards control. Middlewares are located in {project}/lib, and are a fixed, non-regenerating copy of the libary downloaded by CubeMX.\
\
An important note: the "Generate" button in CubeMX will not invoke this process, and thus HAL code should only be generated via the stm32pio command.

### Note: Middlewares

`stm32pio` does not include the Middleware files for middlewares such as FreeRTOS or FatFS, as such they are static copies from the STM32CubeMX installation. The irrelevant configuration options/files have been removed, as would be done if they were generated. There are some adjustments that must be applied each build in order to achieve this (linker arguements for the FPU), they are implemented as pre-scripts in Python.\
\
This is a known [restriction](https://github.com/ussserrr/stm32pio#restrictions) of `stm32pio`

## Run Sheet

Starting from the blank directory structure as follows:

- flightComputer-2027
  - boards
  - Drivers
  - fc
    - fc.ioc
  - prop\_bottom
    - prop\_bottom.ioc
  - prop\_top
    - prop\_top.ioc
  - platformio.ini

### Install VSCode

[Download](https://code.visualstudio.com/), and install for your operating system/architecture. If you already have a VSCode install, you can just modify that one.

### Install STM32CubeMX

Only necessary if you are making changes to the MCU configuration\
\
FIND A VERSION TO PIN AGAINST

### Install PlatformIO

Install "PlatformIO" from the VSCode Extensions menu.

PlatformIO is the IDE solution that we will use to compile, upload, debug, and test code on embedded targets.

### Install `stm32pio`

Install the stm32pio program:
`pip install stm32pio`.

### Initialize `stm32pio` projects

- **Change directory to each subproject**, and run `stm32pio init`\
- Specify the board in the stm32pio.ini file (`flightComputer` for `fc` and `propulsionController` for `prop_top` and `prop_bottom`)\
- Run `stm32pio generate` to generate HAL code and initialize project structure, this will take a while.
- There will be a warning about selecting another toolchain, ignore it.

### Initialize PlatformIO projects

Ensure that the platformIO.ini configuration file is present at the top-leve, and that PlatformIO is initialized, indicated by a set of PlatformIO icons appearing on the status bar at the bottom of the screen, or by the PIO Home window opening.

If it is not, simply click the PlatformIO alien icon on the extensions (left) sidebar

### Build Project

The build command for PlatformIO is `pio run -e {target}`. The defined targets are `flightComputer`, `prop_top`, & `prop_bottom`. Alternatively the checkmark icon will start a build.\
\
It must be run from the top-level directory, the one containing platformio.ini\
\
First build must be online, as the project will pull down a dependency (Eigen) from the PlatformIO registry.
