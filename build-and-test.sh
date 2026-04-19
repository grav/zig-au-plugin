#!/bin/bash

set -e  # Exit on error

echo "==================================================="
echo "Building Zig AU Plugin..."
echo "==================================================="

# Build the plugin
~/bin/zig-aarch64-macos-0.16.0/zig build

echo ""
echo "==================================================="
echo "Installing plugin..."
echo "==================================================="

# Copy to plugin directory
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components

echo "✓ Installed to ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component"

echo ""
echo "==================================================="
echo "Clearing AU cache..."
echo "==================================================="

# Clear Audio Unit cache
killall -9 AudioComponentRegistrar 2>/dev/null || true

echo "✓ AU cache cleared"

# Give the system a moment to settle
sleep 2

echo ""
echo "==================================================="
echo "Running auval..."
echo "==================================================="
echo ""

# Run validation
auval -v aufx volu Zigg

echo ""
echo "==================================================="
echo "Done!"
echo "==================================================="
