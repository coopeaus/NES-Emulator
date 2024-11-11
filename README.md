# NES-Emulator

## Build Instructions

```bash
mkdir -p build && cd build
cmake .. && make
```

## Run Executables

From the `build` folder:

```bash
./emu    # Run main executable
ctest    # Run unit tests
```

## Optional Scripts

Make scripts executable and use them to build, clean, run, or test:

```bash
chmod +x build.sh clean.sh run.sh test.sh
./build.sh          # Build project
./clean.sh          # Clean build cache/executables
./run.sh <name>     # Run specified executable in build folder
./test.sh           # Run unit tests
```

## Contribution Guidelines

- Start early; don’t wait until the due date to contribute. Respect teammates’ time by submitting quality contributions.
- Research each component thoroughly before working on it.
- Stay updated with the codebase, even if you didn't write it.
- If you fall behind, ask for help.
