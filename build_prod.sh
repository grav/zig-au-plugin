#!/bin/bash
set -e

echo "🚀 Building production plugin..."
echo ""

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf zig-out

# Build in release mode without debug logging
echo "🔨 Building optimized release..."
zig build -Doptimize=ReleaseFast

# Install to system location
echo "📦 Installing to ~/Library/Audio/Plug-Ins/Components/..."
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/

# Re-sign the plugin
echo "🔏 Code signing plugin..."
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component

echo ""
echo "============================================"
echo "✅ Production build complete!"
echo "============================================"
echo "Location: ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component"
echo ""
echo "Features:"
echo "  • Optimized for performance (ReleaseFast)"
echo "  • Debug logging disabled"
echo "  • Code signed for macOS"
echo ""
echo "To test: uv run test_simple.py"
