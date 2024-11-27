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
docker build -t project-linter -f docker/alpine-Dockerfile .

# Linting/formatting checks (this will apply formatting when run locally)
docker run -v $(pwd):/workspace -w /workspace project-linter lint
```

Building with Docker

```bash
docker run -v $(pwd):/workspace -w /workspace project-linter build
```

Docker build information is stored in a separate build_container directory. The build.sh
script will determine which directory to use based on the environment.