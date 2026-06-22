from PIL import Image
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor

effect_dir = Path("c:/we_hunting/video_studio/assets/effect")
png_files = list(effect_dir.glob("*.png"))

def convert_to_real_png(file_path):
    try:
        with open(file_path, "rb") as f:
            header = f.read(4)
        if header != b'\x89PNG':
            # It's a WebP disguised as PNG, load it and save it back as PNG
            with Image.open(file_path) as img:
                img.load() # force loading the image data into memory
                img.save(file_path, format="PNG")
                print(f"Converted {file_path.name}")
    except Exception as e:
        print(f"Error converting {file_path.name}: {e}")

print(f"Checking {len(png_files)} files...")
with ThreadPoolExecutor(max_workers=8) as executor:
    executor.map(convert_to_real_png, png_files)

print("Conversion complete!")
