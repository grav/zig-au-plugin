# Zig AU Plugin

A minimal Audio Unit (AU) plugin written in Zig, demonstrating how to create a working AU v2 effect plugin with parameter support.

## Features

- ✅ AU v2 compliant audio effect plugin
- ✅ Single volume parameter (linear gain, 0.0 - 1.0)
- ✅ Stereo (2-channel) audio processing
- ✅ Passes full `auval` validation
- ✅ Render notify callback support
- ✅ Parameter scheduling support

## Requirements

- macOS (tested on Apple Silicon)
- Zig compiler (0.16.0 or compatible)
- Xcode Command Line Tools (for AudioToolbox framework)

## Project Structure

```
.
├── build.zig              # Build configuration
├── src/
│   ├── main.zig          # Audio processing logic (Zig)
│   ├── wrapper.c         # AU plugin implementation (C)
│   └── c.h               # C headers
└── README.md
```

## Building

Set your Zig compiler path and build the plugin:

```bash
~/bin/zig-aarch64-macos-0.16.0/zig build
```

The build output will be in `zig-out/MyZigPlugin.component/`

## Installation

Copy the built plugin to your system's Audio Unit plugin directory:

```bash
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components
```

Clear the Audio Unit cache to register the plugin:

```bash
killall -9 AudioComponentRegistrar
```

## Testing

### Validate with auval

Run Apple's official Audio Unit validation tool:

```bash
auval -v aufx volu Zigg
```

Expected output should end with:
```
--------------------------------------------------
AU VALIDATION SUCCEEDED.
--------------------------------------------------
```

### Plugin Identification

- **Type**: `aufx` (Audio Effect)
- **Subtype**: `volu` (Volume)
- **Manufacturer**: `Zigg`
- **Bundle ID**: `com.example.zigplugin`

### Quick Test

Just check if the plugin loads without full validation:

```bash
auval -s aufx volu Zigg
```

## Development

### Build & Install (One-liner)

```bash
~/bin/zig-aarch64-macos-0.16.0/zig build && \
cp -r zig-out/MyZigPlugin.component ~/Library/Audio/Plug-Ins/Components && \
killall -9 AudioComponentRegistrar
```

### Modify the Audio Processing

Edit `src/main.zig` to change how audio is processed:

```zig
export fn processAudio(in_buffer: [*]const f32, out_buffer: [*]f32, frames: usize, volume: f32) void {
    var i: usize = 0;
    while (i < frames) : (i += 1) {
        out_buffer[i] = in_buffer[i] * volume;  // Apply volume
    }
}
```

### Modify Plugin Metadata

Edit `build.zig` to change plugin information in the `Info.plist`:

- **Name**: Change `<string>Demo: ZigPlugin</string>`
- **Type**: Change `<string>aufx</string>` (aufx = effect, aumu = instrument, etc.)
- **Subtype**: Change `<string>volu</string>` (4-character code)
- **Manufacturer**: Change `<string>Zigg</string>` (4-character code)
- **Bundle ID**: Change `<string>com.example.zigplugin</string>`

### Add Parameters

Parameters are defined in `src/wrapper.c`. The current implementation has one parameter:

- **Parameter ID 0**: Volume (Linear Gain, 0.0-1.0, default 0.5)

To add more parameters, modify:
1. `MyPluginState` struct to store parameter values
2. `AUGetParameter` / `AUSetParameter` to handle new parameter IDs
3. `kAudioUnitProperty_ParameterList` to return all parameter IDs
4. `kAudioUnitProperty_ParameterInfo` to describe each parameter

## Technical Details

### Architecture

- **Audio Processing**: Pure Zig code (`src/main.zig`)
- **AU Interface**: C wrapper (`src/wrapper.c`) implementing AU v2 API
- **Build System**: Zig build system with C interop

### Supported Features

- [x] Parameter get/set (Global scope only)
- [x] Parameter scheduling (immediate & ramped)
- [x] Render notify callbacks (pre/post-render)
- [x] Stream format negotiation
- [x] Property listeners
- [x] Preset management (basic)
- [x] Class info (state save/restore)

### Limitations

- Stereo only (2 in, 2 out)
- No UI (headless plugin)
- Parameters only on Global scope
- Single preset
- No latency compensation
- No tail time

## Troubleshooting

### Plugin doesn't appear in DAW

1. Clear the AU cache: `killall -9 AudioComponentRegistrar`
2. Check installation path: `ls ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component`
3. Verify bundle structure: `ls ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component/Contents/`

### Validation fails

Run `auval -v aufx volu Zigg` and check the specific error. Common issues:
- **Open times timeout**: Wait longer, first load can be slow due to code signing
- **Parameter errors**: Check scope/element validation in `AUGetParameter`/`AUSetParameter`
- **Render errors**: Verify audio buffer handling in `AURender`

### Build errors

- Ensure Zig compiler path is correct
- Check that Xcode Command Line Tools are installed: `xcode-select --install`
- Verify AudioToolbox framework is available: `ls /System/Library/Frameworks/AudioToolbox.framework`

## License

This is example code for educational purposes.

## References

- [Audio Unit Programming Guide](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitProgrammingGuide/)
- [Zig Language Reference](https://ziglang.org/documentation/master/)
