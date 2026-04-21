# Debug Logging Configuration

The plugin now supports conditional debug logging that can be controlled at build time.

## Build Modes

### 1. Development (Default)
```bash
zig build
```
- Debug logging **enabled** by default
- Slower, unoptimized code
- Best for development and debugging

### 2. Production (Release)
```bash
zig build -Doptimize=ReleaseFast
```
- Debug logging **disabled** by default
- Fast, optimized code
- Best for distribution

### 3. Custom Control
You can explicitly enable/disable logging regardless of optimization level:

```bash
# Force enable logging in release build
zig build -Doptimize=ReleaseFast -Ddebug-logging=true

# Force disable logging in debug build
zig build -Ddebug-logging=false
```

## Installation

After building, always copy and re-sign the plugin:

```bash
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components/
codesign --force --deep --sign - ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component
```

⚠️ **Important**: The code signing step is required to avoid "Invalid Page" crashes on macOS.

## Testing Build Modes

Run the test script to verify all build configurations:

```bash
./test_build_modes.sh
```

This will test:
- ✅ Debug build has logging
- ✅ Release build has no logging
- ✅ Explicit flags work correctly

## How It Works

The build system passes a compile-time option to the Zig code:

**build.zig:**
```zig
const enable_debug_logging = b.option(bool, "debug-logging", "Enable debug logging") 
    orelse (optimize == .Debug);
```

**main.zig:**
```zig
if (build_options.enable_debug_logging) {
    std.debug.print("volume: {d}\n", .{volume});
}
```

The condition is evaluated at compile time, so there's zero runtime overhead when disabled.
