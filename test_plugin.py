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

    print("\nProcessing audio...")
    # Run the audio through your AU
    effected_audio = plugin(audio_data, sample_rate)

    print(f"Saving output to {OUTPUT_FILE}...")
    with AudioFile(OUTPUT_FILE, 'w', sample_rate, effected_audio.shape[0]) as f:
        f.write(effected_audio)

    print("Done! 🎉")

if __name__ == "__main__":
    main()
