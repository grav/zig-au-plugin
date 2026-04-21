#!/bin/bash
set -e

echo "🛠️  Building development plugin..."
echo ""

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf zig-out

# Build in debug mode with logging enabled
echo "🔨 Building debug version..."
zig build -Ddebug-logging=true

# Install to system location
echo "📦 Installing to ~/Library/Audio/Plug-Ins/Components/..."
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/

# Re-sign the plugin
echo "🔏 Code signing plugin..."
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component

echo ""
echo "============================================"
echo "✅ Development build complete!"
echo "============================================"
echo "Location: ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component"
echo ""
echo "Features:"
echo "  • Debug logging enabled"
echo "  • Hot-reload ready"
echo "  • Code signed for macOS"
echo ""
echo "To test: uv run test_simple.py"
echo "To hot-reload: Set plugin.reload = True after rebuilding"
