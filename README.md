# NES-Emulator
![emu-header](https://github.com/user-attachments/assets/9021bfb8-f8e5-40e8-b874-d518e7e28e00)

📖 [Summary](#summary) - ✨ [Features](#features) - 📽️ [Demo](#demo) - 🚀 [Quick Start](#quick-start) - ⚙️[Dev Setup Guides](#dev-setup-guides) - 🛠️[Dev Tooling](#dev-tooling) - ⚠️ [Known Issues](#known-issues) 

## Summary
Welcome to our emulation project! We built a cross-platform emulator of the Nintendo Entertainment System (NES), which was released in 1983 in Japan and 1986 in North America. The project is written in C++ and runs on Windows, Mac, and Linux. The project originated as a university capstone project and we developed it over the course of three semesters, with the goal of better understanding emulation and computer architecture. It currently supports around 80% of commercial titles

## Features
- Runs on MacOS, Linux, and Windows
- Full support for mappers 0, 1, 2, 3 and partial support for mapper 4.
- Drag-and-drop ROMs
- In-game save support
- Saving and loading state
- Single gamepad support, with options to customize key bindings
- Debugging windows for the CPU, PPU, and other systems

## Demo
https://github.com/user-attachments/assets/8c82d557-ba3a-446c-9619-4796924df4f7

## Quick Start
As an end product, we provide a binary executable product for each of our target systems (macOS ARM, Windows, and Linux).

To run the emulator:
- Download the latest [release](https://github.com/coopeaus/NES-Emulator/releases) for your target system
- macOS / Linux:
  - `cd` into the downloaded folder and run `chmod +x emu` and `./emu`
  - If macOS blocks the executable, you can flag it as safe in **System Settings** -> **Privacy & Security**
  - Run `./emu` again
- Windows:
  - From the terminal, run `editbin /STACK:16777216 emu.exe` (see [Known Issues](#known-issues))
  - Run the emulator with `.\emu.exe`


## Dev Setup Guides
- [Windows Setup (CLion)](https://github.com/coopeaus/NES-Emulator/blob/main/docs/CLion_Windows.md)
- [MacOS Setup (CLion)](https://github.com/coopeaus/NES-Emulator/main/docs/CLion_MacOS.md)
- [MacOS Setup (CLI)](https://github.com/coopeaus/NES-Emulator/main/docs/CLI_MacOS.md)

## Dev Tooling
- [Building, Linting, Testing, and Formatting with Docker](https://github.com/coopeaus/NES-Emulator/blob/main/docs/Docker_Tools.md)
- [Building, Linting, Testing, and Formatting from the CLI](https://github.com/coopeaus/NES-Emulator/blob/main/docs/CLI_Tools.md)

## Known Issues
- Windows executables run into stack overflow issues and require a manual increase in the default stack size.
- Super Mario Bros 3 freezes when attempting to load the world map. It's an issue with the mapper 4 implementation and other titles may be effected.
- No support for a second gamepad or other peripherals.

