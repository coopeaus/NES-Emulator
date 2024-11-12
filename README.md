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
# From the root directory:
chmod +x scripts/build.sh scripts/clean.sh scripts/run.sh scripts/test.sh
./scripts/build.sh          # Build project
./scripts/clean.sh          # Clean build cache/executables
./scripts/run.sh <name>     # Run specified executable in build folder
./scripts/test.sh           # Run unit tests
```

Linting and formatting locally:

```bash
# If you have clang-tidy and clang-format installed
find src/ -name '*.cpp' -exec clang-tidy -p build -header-filter='.*' {} \\; # Linting
find src/ -name '*.cpp' -exec clang-format -i {} \; # Apply formatting
```

Linting and formatting with Docker

```bash
docker build -t project-linter -f docker/lint/Dockerfile .

# Linting
docker run project-linter

# Linting and apply formatting
docker run --rm -v "$(pwd)/src":/usr/src/app/src project-linter
```

## Contribution Guidelines

- Start early; don’t wait until the due date to contribute. Respect teammates’ time by submitting quality contributions.
- Lint and format the codebase locally before submitting a PR.
- Research each component thoroughly before working on it.
- Stay updated with the codebase, even if you didn't write it.
- If something is confusing, ask your teammates.
