import os
import subprocess
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

effect_dir = Path("c:/we_hunting/video_studio/assets/effect")
ffmpeg_path = r"c:\we_hunting\video_studio\build_mingw\ffmpeg.exe"

def convert_webp(webp_file):
    png_file = webp_file.with_suffix(".png")
    # Only convert if the PNG doesn't exist already
    if not png_file.exists():
        cmd = [
            ffmpeg_path,
            "-v", "error",
            "-i", str(webp_file),
            "-y", str(png_file)
        ]
        try:
            subprocess.run(cmd, check=True)
            webp_file.unlink() # delete the original webp
            print(f"Converted {webp_file.name}")
        except Exception as e:
            print(f"Error converting {webp_file.name}: {e}")

webp_files = list(effect_dir.glob("*.webp"))
print(f"Found {len(webp_files)} webp files to convert...")

# Use ThreadPoolExecutor to speed up batch conversion
with ThreadPoolExecutor(max_workers=8) as executor:
    executor.map(convert_webp, webp_files)

print("Conversion complete!")
