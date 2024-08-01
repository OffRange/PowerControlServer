#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Variables
REPO_URL="https://github.com/OffRange/PowerControlServer.git"
BUILD_DIR="build"
INSTALL_PREFIX="/usr/local"

# Functions
print_usage() {
    echo "Usage: $0 [-p INSTALL_PREFIX] [-b BUILD_DIR] [-h]"
    echo "  -p INSTALL_PREFIX : Installation directory (default: /usr/local)"
    echo "  -b BUILD_DIR      : Build directory (default: build)"
    echo "  -h                : Print this help message"
}

# Parse command-line arguments
while getopts "p:b:h" opt; do
    case ${opt} in
    p)
        INSTALL_PREFIX=${OPTARG}
        ;;
    b)
        BUILD_DIR=${OPTARG}
        ;;
    h)
        print_usage
        exit 0
        ;;
    \?)
        echo "Invalid option: -$OPTARG" >&2
        print_usage
        exit 1
        ;;
    :)
        echo "Option -$OPTARG requires an argument." >&2
        print_usage
        exit 1
        ;;
    esac
done

# Download the source code
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Cloning repository from ${REPO_URL}..."
    git clone "${REPO_URL}" "${BUILD_DIR}"
else
    echo "Using existing build directory: ${BUILD_DIR}"
fi

# Navigate to the build directory
cd "${BUILD_DIR}"

# Create a build directory if it doesn't exist
mkdir -p build && cd build

# Generate the build system with CMake
echo "Running CMake..."
cmake -DCMAKE_INSTALL_PREFIX="${INSTALL_PREFIX}" ..

# Build the project
echo "Building the project..."
make -j$(nproc)

# Install the project
echo "Installing the project..."
sudo make install

# Clean up (optional)
echo "Cleaning up..."
cd ../..
rm -rf "${BUILD_DIR}"

echo "Installation completed successfully!"
