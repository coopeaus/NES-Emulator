# Docker Tools

## Setup

If applicable, remove the old `project-linter` image:

```bash
docker rmi project-linter
```

Install the new image:

```bash
docker build --no-cache -t project-linter2 -f docker/ubuntu-Dockerfile .
```

Subsequent rebuilds can be done without the `--no-cache` flag.
You'll want to rebuild after:

- Updating `scripts/entrypoint.sh`
- Adding or removing dependencies in `docker-vcpkg/vcpkg.json`

## Build Project
```bash
docker run --rm \
    -v "$(pwd):/workspace" \
    -v build:/workspace/build \
    -v docker-vcpkg:/workspace/docker-vcpkg \
    -w /workspace \
    project-linter build
```

## Fix Linting and Formatting
```
docker run --rm \
    -v "$(pwd):/workspace" \
    -v build:/workspace/build \
    -v docker-vcpkg:/workspace/docker-vcpkg \
    -w /workspace \
    project-linter lint
```

## Run Tests
```
docker run --rm \
    -v "$(pwd):/workspace" \
    -v build:/workspace/build \
    -v docker-vcpkg:/workspace/docker-vcpkg \
    -w /workspace \
    project-linter test
```
You can isolate a test by adding `<test-name>` string after `test`. See the complete list of test names, run `ctest -N` from the build directory after building the project.
## Bash Mode
```bash
docker run --rm -it --entrypoint /bin/bash \
    -v "$(pwd):/workspace" \
    -v build:/workspace/build \
    -v docker-vcpkg:/workspace/docker-vcpkg \
    -w /workspace \
    project-linter
```

You can also explore the image without mounting any volumes:
```bash
docker run --rm -it --entrypoint /bin/bash \
    -w /workspace \
    project-linter
```

## Scripts
You can do all of the above with `scripts/docker-run.sh`:

- build
- lint
- test
- bash
- bash —no-mount
```bash
scripts/docker-run.sh <command>
```

---
