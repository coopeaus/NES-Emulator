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
find core/ -name '*.cpp' -exec clang-tidy -p build -header-filter='.*' {} \\; # Linting
find core/ -name '*.cpp' -exec clang-format -i {} \; # Apply formatting
```

Linting and formatting with Docker

```bash
# Build the docker image
docker build -t project-linter -f docker/alpine-Dockerfile .

# Build the project
docker run -v $(pwd):/workspace -w /workspace project-linter build

# Linting/formatting checks (this will apply fixes when run locally)
docker run -v $(pwd):/workspace -w /workspace project-linter lint
```

Docker build information is stored in a separate build_container directory. The build.sh
script will determine which directory to use based on the environment.

## Testing

Testing locally

```bash
# From the root directory:
# To run all tests
./scripts/test.sh

# To run an individual test, add the test name as an argument
./scripts/test.sh "CPUTestFixture.IMM" # Immediate addressing test
./scripts/test.sh "CPUTestFixture.x29" # Test opcode 29, AND Immediate
```

Testing with Docker

```bash
# Ensure that the container is built, and that you have build the project as described
# in the above section.

# To run all tests
docker run -v $(pwd):/workspace -w /workspace project-linter test

# To run an individual test, add the test name as an argument
docker run -v $(pwd):/workspace -w /workspace project-linter test  "CPUTestFixture.IMM" # Immediate addressing test
```
