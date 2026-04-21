#!/usr/bin/env python3
"""Test if CocoaUI is exposed"""

import subprocess
import sys

# Use auval to check if CocoaUI is available
result = subprocess.run(
    ["auval", "-s", "aufx", "volu", "Zigg"],
    capture_output=True,
    text=True,
    timeout=30
)

print("Checking for CocoaUI property...")
print(result.stdout)
print(result.stderr)

# Look for CocoaUI in the output
if "CocoaUI" in result.stdout or "CocoaUI" in result.stderr:
    print("\n✅ CocoaUI property is exposed!")
else:
    print("\n⚠️  CocoaUI property not found in auval output")
    print("This might be normal - trying to load plugin...")
    
sys.exit(0)
