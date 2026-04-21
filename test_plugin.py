# /// script
# requires-python = ">=3.9"
# dependencies = [
#     "pedalboard",
#     "numpy",
# ]
# ///

import sys
import numpy as np
from pedalboard import load_plugin
from pedalboard.io import AudioFile

# --- CONFIGURATION ---
# Change this to match the name of your AU plugin.
# You can usually find the name in the output of `auval -a`.
# Format is usually "AU:YourPluginName" or the path to the .component file.
import os
PLUGIN_IDENTIFIER = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component")
OUTPUT_FILE = "output.wav"
# ---------------------

def main():
    print("Generating a 1-second 440Hz sine wave test tone...")
    sample_rate = 44100
    duration = 1.0
    t = np.linspace(0, duration, int(sample_rate * duration), False)
    
    # Create a stereo audio buffer (2 channels, samples)
    tone = np.sin(2 * np.pi * 440 * t)
    audio_data = np.vstack((tone, tone)).astype(np.float32)

    print(f"Loading plugin: {PLUGIN_IDENTIFIER}")
    try:
        # Load the Audio Unit
        plugin = load_plugin(PLUGIN_IDENTIFIER)
    except Exception as e:
        print(f"\n❌ Failed to load plugin '{PLUGIN_IDENTIFIER}'.")
        print("Make sure your AU is built, copied to ~/Library/Audio/Plug-Ins/Components/, and validated by auval.")
        print(f"Error details: {e}")
        sys.exit(1)

    # Print available parameters so you can see exactly what pedalboard calls them
    print("\n✅ Plugin loaded successfully!")
    print("\nAvailable parameters exposed by your AU:")
    for param_name in plugin.parameters.keys():
        print(f"  - {param_name}")

    # Set the volume parameter!
    # Note: pedalboard maps AU parameters to python properties dynamically. 
    # If your parameter is named "Volume" or "volume", you can usually just do `plugin.volume`.
    # Update the property name below to match what was printed in the list above.
    try:
        # We'll try common names
        if hasattr(plugin, 'volume'):
            plugin.volume = 0.7
            print(f"\nChanged 'volume' parameter to: {plugin.volume}")
        elif hasattr(plugin, 'gain'):
            plugin.gain = 0.5
            print(f"\nChanged 'gain' parameter to: {plugin.gain}")
        else:
            print("\n⚠️ Note: Didn't automatically find a 'volume' or 'gain' parameter to adjust.")
            print("Look at the parameter list above and modify the script (e.g. plugin.your_param_name = 0.5)")
    except Exception as e:
        print(f"Error setting parameter: {e}")

    print("\nProcessing audio (first pass)...")
    # Run the audio through your AU
    effected_audio = plugin(audio_data, sample_rate)

    # Test hot reload functionality
    print("\n" + "="*60)
    print("Testing hot-reload functionality...")
    print("="*60)
    
    # Modify the Zig source code to add more obvious processing
    print("\n📝 Modifying Zig source code...")
    import subprocess
    zig_source = "/Users/grav/repo/zig-au-plugin/src/main.zig"
    with open(zig_source, 'r') as f:
        original_code = f.read()
    
    # Modify the code to multiply by 2 in addition to the volume
    modified_code = original_code.replace(
        'out_buffer[i] = (last + in_buffer[i])/2 * volume;',
        'out_buffer[i] = (last + in_buffer[i])/2 * volume * 2.0; // MODIFIED!'
    )
    
    with open(zig_source, 'w') as f:
        f.write(modified_code)
    
    print("✓ Code modified (doubled output)")
    
    # Rebuild the Zig plugin
    print("\n🔨 Rebuilding Zig plugin...")
    result = subprocess.run(
        ["zig", "build"],
        cwd="/Users/grav/repo/zig-au-plugin",
        capture_output=True,
        text=True
    )
    if result.returncode != 0:
        print(f"Build failed: {result.stderr}")
        # Restore original code
        with open(zig_source, 'w') as f:
            f.write(original_code)
        sys.exit(1)
    
    print("✓ Build completed")
    
    # Copy the rebuilt dylib to the installed location
    print("\n📦 Installing updated dylib...")
    result = subprocess.run(
        ["cp", "-r", "zig-out/MyZigPlugin.component", 
         os.path.expanduser("~/Library/Audio/Plug-Ins/Components/")],
        cwd="/Users/grav/repo/zig-au-plugin",
        capture_output=True,
        text=True
    )
    if result.returncode != 0:
        print(f"Install failed: {result.stderr}")
    else:
        print("✓ Dylib installed")
    
    # Trigger reload via the reload parameter
    print("\n🔄 Triggering hot reload...")
    if hasattr(plugin, 'reload'):
        plugin.reload = True
        print("✓ Reload triggered via 'reload' parameter")
    else:
        print("⚠️ No 'reload' parameter found, but DSP should be reloaded on next Initialize")
    
    # Process again with modified code
    print("\n📊 Processing audio again with modified DSP...")
    effected_audio_modified = plugin(audio_data, sample_rate)
    
    # Restore original code
    print("\n🔙 Restoring original code...")
    with open(zig_source, 'w') as f:
        f.write(original_code)
    print("✓ Code restored")
    
    # Rebuild to restore and reinstall
    print("🔨 Rebuilding with original code...")
    subprocess.run(["zig", "build"], cwd="/Users/grav/repo/zig-au-plugin", 
                   capture_output=True, text=True)
    subprocess.run(
        ["cp", "-r", "zig-out/MyZigPlugin.component",
         os.path.expanduser("~/Library/Audio/Plug-Ins/Components/")],
        cwd="/Users/grav/repo/zig-au-plugin",
        capture_output=True, text=True
    )
    print("✓ Original version restored and installed")

    print(f"\nSaving output to {OUTPUT_FILE}...")
    with AudioFile(OUTPUT_FILE, 'w', sample_rate, effected_audio_modified.shape[0]) as f:
        f.write(effected_audio_modified)

    print("\n" + "="*60)
    print("Done! 🎉")
    print("="*60)
    print(f"Output saved to {OUTPUT_FILE}")
    print("\nThe hot-reload test modified the DSP code, reloaded it,")
    print("and processed audio with the new code - all without restarting!")


if __name__ == "__main__":
    main()
