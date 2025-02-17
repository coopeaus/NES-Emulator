#!/bin/bash
set -euo pipefail

cd "$(dirname "$BASH_SOURCE[0]")/.." || exit 1

# Docker image name
IMAGE="project-linter"

# Common Docker flags for commands that mount the current workspace.
DOCKER_COMMON=(
  --rm
  -v "$(pwd):/workspace"
  -v build:/workspace/build
  -v docker-vcpkg:/workspace/docker-vcpkg
  -w /workspace
)

case "${1:-}" in
build | lint | test)
  if [ -n "${2:-}" ]; then
    docker run "${DOCKER_COMMON[@]}" "$IMAGE" "${1}" "${2}"
  else
    docker run "${DOCKER_COMMON[@]}" "$IMAGE" "${1}"
  fi
  ;;
bash)
  if [ "${2:-}" = "--no-mount" ]; then
    echo "Running bash without mounting workspace."
    docker run --rm -it --entrypoint /bin/bash -w /workspace "$IMAGE"
  else
    echo "Running bash with mounted workspace."
    docker run --rm -it --entrypoint /bin/bash "${DOCKER_COMMON[@]}" "$IMAGE"
  fi
  ;;
*)
  echo "Usage: $0 {build|lint|test|bash [--no-mount]}"
  exit 1
  ;;
esac
