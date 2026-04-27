# /// script
# requires-python = ">=3.9"
# dependencies = [
#     "pedalboard",
#     "numpy",
# ]
# ///

import numpy as np
from pedalboard import load_plugin
import os

PLUGIN_IDENTIFIER = os.path.expanduser("~/Library/Audio/Plug-Ins/Components/MyZigPlugin.component")

print("Loading plugin...")
plugin = load_plugin(PLUGIN_IDENTIFIER)
print("✓ Loaded!")
print("Parameters:", list(plugin.parameters.keys()))

# Create simple audio
sample_rate = 44100
duration = 0.1  # Short test
t = np.linspace(0, duration, int(sample_rate * duration), False)
tone = np.sin(2 * np.pi * 440 * t)
audio_data = np.vstack((tone, tone)).astype(np.float32)

plugin.volume = 0.8
print("Processing audio...")
output = plugin(audio_data, sample_rate)
print("✓ Done!")
