# 🚀 Flight Computer Software 2027

The McGill Rocket Team is a student run design team at McGill University in Montreal, Canada. We are in the process of designing our fifth generation hybrid rocket. In this new design, we are integrating the eighth generation fully student designed Flight Computer (FC). <br>
The FC Software (FCSW) project contains all the software that runs on the FC. This contains device drivers, communication and storage handlers. This repository is meant to show a real-world embedded system. <br>
All verification and validation of this code per the mission requirements is the responsibility of the mission team. <br>
The FC software project is a completely student run codebase and there are **absolutely no guarantees to the safety or correctness of this system.**

## 🤝 Contributing

This code is meant to be built by students at McGill University. If you are not a student, then please open an issue with your ideas for contributions and we'll see what we can do together. Below is a brief overview for you to get started. Before contributing to the codebase, however, please read the [contribution guidelines](https://github.com/McGillRocketTeam/flightComputer-2027/blob/main/CONTRIBUTING.md) (**IN FULL**).<br>

### Getting Started

There are a few tools that you should download before beginning.

- A Git utility like GitHub desktop can be helpful for managing branches and commits.
- You should also always have some sort of note taking ability when coding. This could be a simple piece of paper or something like [Obsidian](https://obsidian.md/). You should jot down anything interesting that happens or the reasoning you have for writing what you're writing. This will help other people understand what you did (including yourself when you come back in a week).

To contribute, make sure you are [setup](#-setup) and can [compile](#-build-and-run) the project.

### Branches

We use branches extensively to keep track of our work and develop seperate features. <br>
`main` is our main brach and is protected. At **NO** point (and I mean absolutely **ZERO SITUATIONS**) should you be commiting directly to `main`. <br>
`dev` is our development branch and is our internal main branch. Once verified, all new features should be merged into `dev`.<br><br>

If you are starting work on a new issue, please create a new branch off of `dev` titled in the following format `<issue_number>_<issue_name>`. For example, if I'm working on issue #12 titled "create README", then that branch would be called `12_createREADME`.<br>

### Merging

Git merging can be a real pain if not done correctly. That is why you should **NEVER** merge into `dev` without a pull request. To open a PR, see the [docs](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request). When opening a PR, please specify the main changes, and a way that we can test them. All PR's should require a review from the FCSW lead (currently @Andreea-Nicole) and you should assign yourself so you get updates.

### Issues

We use GitHub issues to keep track of new features and bugs. If you encounter a bug when developing, take note and open an issue with the `bug` tag when you have time. In the description, write the bare minimum steps needed to recreate the bug and any hypotheses you have for why it might occur.

## 💾 Setup

Ensure the following are installed [VSCode](https://code.visualstudio.com/), [Python](https://www.python.org/downloads/) (Install any version, we only need `pip`), (Optional)[STM32CubeMX](https://www.st.com/en/development-tools/stm32cubemx.html) (6.17.0). Then clone this repository (see [branches](#branches) for more information about branches):

```bash
git clone git@github.com:McGillRocketTeam/flightComputer-2027.git
```

Open VSCode, and install the [PlatformIO IDE](platformio.org) extension from the Extensions menu.

PlatformIO is the IDE solution that we will use to compile, upload, debug, and test code on embedded targets.

## 📦 Build and Run

The FCSW project will build using the PlatformIO IDE build system, there are two ways to build the FC code using this system.

### VSCode build (recommended)

Building the code with PlatformIO from VSCode is relatively straightforward:

1. Open the repo in VSCode.
2. Ensure PlatformIO is initialized by clicking the PlatformIO alien icon from the Extensions sidebar
3. Once PlatformIO is initialized, ensure the Status Bar (the bottom most row of icons) contains `Default (flightComputer-2027)`, this is the environment that we will build. The default is the main Flight Computer.
4. Run the `PlatformIO: Build` command from either the Command Palette or the checkmark icon on the Status Bar
5. The code will build!

### Command Line (advanced)

If you would like to compile this project from the command line, PlatformIO allows us to do that as well!

1. Navigate to the repo
2. Run `pio run -e flightComputer`
3. The code will build!

### Flashing Code

Connect the ST-Link to the target over SWD and confirm that the MCU is detected before flashing.

We will flash using PlatformIO, via either the `PlatformIO: Upload` command in VSCode (the -> in the Status Bar), or the `pio run -e flightComputer -t upload` command from the CLI.

### Reconfiguing MCU with STM32CubeMX

In order to make changes to the MCU pinout or peripheral configuration, we will use STM32CubeMX. However, we must first install `stm32pio` which is a utility that allows us to use PlatformIO with automatically generated HAL files.

This is not required to build, run, or develop the FC code. This is only used to reconfigure the microcontroller.

1. Install the stm32pio program:`pip install stm32pio`.
2. **Change directory to target subproject**, and run `stm32pio init -b BOARD`, where `BOARD` is `flightComputer` for `fc` and `propulsionController` for `prop_top` and`prop_bottom` subprojects.\
3. Open the corresponding `.ioc` file in STM32CubeMX, make changes, and save the file. **DO NOT CLICK GENERATE**
4. Run `stm32pio generate` to generate HAL code and initialize project structure, this will take a while.
5. There will be a warning about selecting another toolchain, ignore it.

For assistance with the target hardware:

1. **You are a member of MRT:** Contact the FCSW lead on our slack channel and I will assist you in setting up the hardware needed to run this code.
2. **You are not a member of MRT:** Open an issue with your situation, and we'll see what we can do to assist.
