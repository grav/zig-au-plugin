# Testing the Reload Button UI

The plugin includes a Cocoa UI with a "Reload Zig DSP" button for hot-reloading.

## How to Access the UI

The UI button will appear in DAWs that support Audio Unit custom UIs. The availability depends on your host:

### ✅ DAWs with AU UI Support
- **Logic Pro** - Click the plugin window controls to show custom UI
- **GarageBand** - Should show custom UI by default
- **Ableton Live** - Check "Edit" button in plugin window
- **Reaper** - Right-click plugin → "Show plug-in UI"

### ⚠️ Limited UI Support
- Some hosts only show generic parameter controls
- The `reload` parameter provides an alternative way to trigger reload

## Testing the Button

1. **Load the plugin** in a supported DAW
2. **Look for "Reload Zig DSP" button** in the plugin window
3. **Click the button** - you should see in Console.app:
   ```
   ========================================
   🔘 RELOAD BUTTON CLICKED!
   ========================================
   reloadDSP: Loading from bundle: ...
   reloadDSP: ✓ Loaded successfully
   ========================================
   ```

## Alternative: Use the Reload Parameter

If the UI button isn't accessible, use the `reload` parameter instead:

### From Python/Pedalboard:
```python
plugin.reload = True  # Triggers reloadDSP()
```

### From DAW Automation:
- Automate the "Reload" parameter
- Set it to 1.0 (or "On") to trigger reload

## Checking UI Availability

The plugin now successfully exposes its Cocoa UI. To verify:

```bash
# Check if plugin exposes CocoaUI
auval -v aufx volu Zigg 2>&1 | grep -i cocoa

# Expected output:
# Cocoa Views Available: 1
#   MyPluginUI
#     PASS
```

## Implementation Details

**src/wrapper.m:**
- File renamed from `.c` to `.m` for proper Objective-C compilation
- `MyPluginUI` class implements `AUCocoaUIBase` protocol
- Button target/action: `reloadClicked:` → `reloadDSP()`
- Property `kAudioUnitProperty_CocoaUI` returns bundle URL and UI class name

**build.zig:**
- Wrapper compiled as Objective-C (`.m` extension)
- Links Cocoa framework for UI support
- `-fblocks` flag for block support

## Troubleshooting

**UI button doesn't appear:**
1. Check if your DAW supports AU custom UIs
2. Try the `reload` parameter as alternative
3. Check Console.app for any loading errors

**Button doesn't work:**
1. Check Console.app for the "RELOAD BUTTON CLICKED!" message
2. If message doesn't appear, the click isn't reaching the callback
3. Verify plugin is signed: `codesign -dv ~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component`
