#!/bin/bash
set -e

echo "Testing debug logging build configurations..."
echo ""

# Test 1: Default debug build (should have logging)
echo "1️⃣  Testing default debug build..."
zig build
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component > /dev/null 2>&1

if uv run test_simple.py 2>&1 | grep -q "volume:"; then
    echo "✓ Debug build has logging (as expected)"
else
    echo "✗ Debug build missing logging (unexpected!)"
    exit 1
fi

# Test 2: Release build (should NOT have logging)
echo ""
echo "2️⃣  Testing release build..."
zig build -Doptimize=ReleaseFast
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component > /dev/null 2>&1

if uv run test_simple.py 2>&1 | grep -q "volume:"; then
    echo "✗ Release build has logging (unexpected!)"
    exit 1
else
    echo "✓ Release build has NO logging (as expected)"
fi

# Test 3: Explicit debug logging flag
echo ""
echo "3️⃣  Testing explicit -Ddebug-logging=true..."
zig build -Doptimize=ReleaseFast -Ddebug-logging=true
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component > /dev/null 2>&1

if uv run test_simple.py 2>&1 | grep -q "volume:"; then
    echo "✓ Explicit flag enables logging (as expected)"
else
    echo "✗ Explicit flag didn't enable logging (unexpected!)"
    exit 1
fi

# Test 4: Explicit disabled logging
echo ""
echo "4️⃣  Testing explicit -Ddebug-logging=false..."
zig build -Ddebug-logging=false
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component > /dev/null 2>&1

if uv run test_simple.py 2>&1 | grep -q "volume:"; then
    echo "✗ Explicit false still has logging (unexpected!)"
    exit 1
else
    echo "✓ Explicit false disables logging (as expected)"
fi

echo ""
echo "============================================"
echo "✅ All build configuration tests passed!"
echo "============================================"
