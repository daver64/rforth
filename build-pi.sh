#!/bin/bash
# Cross-compile RForth for Raspberry Pi Zero 2W

set -e

echo "Cross-compiling RForth for Raspberry Pi Zero 2W..."

# Check if cross-compiler is available
if ! command -v arm-linux-gnueabihf-gcc &> /dev/null; then
    echo "Error: ARM cross-compiler not found!"
    echo "Install with: sudo apt-get install gcc-arm-linux-gnueabihf g++-arm-linux-gnueabihf"
    exit 1
fi

# Create build directory for cross-compilation
mkdir -p build-pi

# Configure with toolchain file
cmake -B build-pi -DCMAKE_TOOLCHAIN_FILE=toolchain-pi-zero-2w.cmake

# Build
cmake --build build-pi

echo ""
echo "Cross-compilation complete!"
echo "Binary location: build-pi/bin/rforth"
echo ""
echo "To deploy to Raspberry Pi:"
echo "  scp bin/rforth pi@raspberrypi.local:~/"
echo "  scp -r examples pi@raspberrypi.local:~/"
echo ""
echo "On the Pi, run with:"
echo "  ./rforth examples/rpi/blink.f"
