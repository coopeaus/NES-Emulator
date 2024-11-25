# Ubuntu LTS Image
FROM ubuntu:22.04

# No interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    clang \
    clang-tidy \
    clang-format \
    cmake \
    git \
    ca-certificates \
    libsdl2-2.0-0 \
    libsdl2-dev \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory and copy the current directory contents
WORKDIR /usr/src/app
COPY . /usr/src/app

# Remove any existing build directory and start a fresh build to generate compile_commands.json
RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_CXX_COMPILER=clang++ ..

# Expose the working directory as a volume to persist output files
VOLUME ["/usr/src/app"]

# Run the lint.sh script when the container starts
# CMD ["bash", "-c", "find src/ -name '*.cpp' -exec clang-tidy -p build -header-filter='.*' {} \\;"]
CMD ["bash", "-c", "find src/ -name '*.cpp' -exec clang-format -i {} \\; && find src/ -name '*.cpp' -exec clang-tidy -p build -header-filter='.*' {} \\;"]
