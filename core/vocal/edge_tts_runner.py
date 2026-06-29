import argparse
import asyncio
import json
import os
import re
import subprocess
import sys
import edge_tts


def parse_timestamp(ts_str):
    """Parse an SRT timestamp like '00:01:23,456' or '00:01:23.456' into seconds."""
    ts_str = ts_str.strip()
    h, m, s_ms = ts_str.split(':')
    s_ms = s_ms.replace('.', ',')
    if ',' in s_ms:
        s, ms = s_ms.split(',')
    else:
        s = s_ms
        ms = 0
    return int(h) * 3600 + int(m) * 60 + int(s) + int(ms) / 1000.0


def parse_srt(srt_file):
    with open(srt_file, 'r', encoding='utf-8', errors='ignore') as f:
        # Strip BOM if present
        content = f.read()
        if content.startswith('\ufeff'):
            content = content[1:]
        lines = content.replace('\r\n', '\n').split('\n')

    entries = []
    state = "EXPECT_INDEX"
    current = {}
    
    timestamp_re = re.compile(r'(\d{2}:\d{2}:\d{2}[,.]\d{3})\s*-->\s*(\d{2}:\d{2}:\d{2}[,.]\d{3})')
    index_re = re.compile(r'^\d+$')
    
    for line in lines:
        line = line.strip()
        
        if state == "EXPECT_INDEX":
            if not line:
                continue
            if index_re.match(line):
                current = {"index": line}
                state = "EXPECT_TIMESTAMP"
        
        elif state == "EXPECT_TIMESTAMP":
            if not line:
                continue
            match = timestamp_re.search(line)
            if match:
                current["start"] = parse_timestamp(match.group(1))
                current["end"] = parse_timestamp(match.group(2))
                current["duration"] = max(0.1, current["end"] - current["start"])
                current["text"] = ""
                state = "EXPECT_TEXT"
                
        elif state == "EXPECT_TEXT":
            if not line:
                if current.get("text"):
                    entries.append(current)
                    current = {}
                    state = "EXPECT_INDEX"
                # if no text yet, just skip the blank line
            else:
                if current.get("text"):
                    current["text"] += " "
                # Strip HTML tags
                clean_text = re.sub(r'<[^>]+>', '', line)
                clean_text = clean_text.replace('{', '').replace('}', '').strip()
                current["text"] += clean_text

    if state == "EXPECT_TEXT" and current.get("text"):
        entries.append(current)

    return entries


def get_audio_duration(filepath, ffmpeg_path="ffmpeg"):
    """Get the duration of an audio file using ffprobe/ffmpeg."""
    # Try ffprobe first (same directory as ffmpeg)
    ffprobe_path = os.path.join(os.path.dirname(ffmpeg_path), "ffprobe")
    if os.name == 'nt':
        ffprobe_path += ".exe"

    if not os.path.exists(ffprobe_path):
        ffprobe_path = "ffprobe"

    try:
        result = subprocess.run(
            [ffprobe_path, "-v", "error", "-show_entries", "format=duration",
             "-of", "default=noprint_wrappers=1:nokey=1", filepath],
            capture_output=True, text=True, timeout=10
        )
        if result.returncode == 0 and result.stdout.strip():
            return float(result.stdout.strip())
    except Exception:
        pass

    # Fallback: use ffmpeg to get duration
    try:
        result = subprocess.run(
            [ffmpeg_path, "-i", filepath, "-f", "null", "-"],
            capture_output=True, text=True, timeout=30
        )
        # Parse duration from ffmpeg stderr output
        match = re.search(r'Duration:\s*(\d+):(\d+):(\d+)\.(\d+)', result.stderr)
        if match:
            h, m, s, cs = match.groups()
            return int(h) * 3600 + int(m) * 60 + int(s) + int(cs) / 100.0
    except Exception:
        pass

    return None


def time_stretch_audio(input_path, output_path, target_duration, ffmpeg_path="ffmpeg"):
    """Use ffmpeg atempo filter to stretch/compress audio to target_duration."""
    actual_duration = get_audio_duration(input_path, ffmpeg_path)
    if actual_duration is None or actual_duration <= 0:
        sys.stderr.write(f"Could not determine duration of {input_path}, skipping stretch\n")
        return False

    ratio = actual_duration / target_duration

    # Cap the maximum speedup to 1.25x as requested by the user
    if ratio > 1.25:
        ratio = 1.25
        target_duration = actual_duration / ratio

    # Skip if ratio is too extreme (quality would be terrible)
    if ratio > 4.0 or ratio < 0.25:
        sys.stderr.write(
            f"Stretch ratio {ratio:.2f}x too extreme for {os.path.basename(input_path)}, "
            f"using natural speed\n"
        )
        return False

    # Skip if already close enough (within 5%)
    if 0.95 <= ratio <= 1.05:
        return False

    # Build atempo filter chain (atempo supports 0.5 to 100.0 per filter)
    # For values outside 0.5-2.0, we chain multiple filters
    filters = []
    remaining = ratio
    while remaining > 2.0:
        filters.append("atempo=2.0")
        remaining /= 2.0
    while remaining < 0.5:
        filters.append("atempo=0.5")
        remaining /= 0.5
    filters.append(f"atempo={remaining:.6f}")

    filter_str = ",".join(filters)

    try:
        result = subprocess.run(
            [ffmpeg_path, "-y", "-i", input_path,
             "-filter:a", filter_str,
             "-vn", output_path],
            capture_output=True, text=True, timeout=30
        )
        if result.returncode == 0 and os.path.exists(output_path) and os.path.getsize(output_path) > 0:
            return True
        else:
            sys.stderr.write(f"ffmpeg atempo failed: {result.stderr[-200:]}\n")
            return False
    except Exception as e:
        sys.stderr.write(f"ffmpeg stretch error: {e}\n")
        return False


async def generate_voice(srt_file, language, output_dir, ffmpeg_path="ffmpeg"):
    voice_map = {
        "Khmer": "km-KH-SreymomNeural",
        "English (Male)": "en-US-ChristopherNeural",
        "English (Female)": "en-US-JennyNeural"
    }
    voice = voice_map.get(language, "en-US-JennyNeural")

    entries = parse_srt(srt_file)
    metadata = []

    for i, entry in enumerate(entries):
        out_file = os.path.join(output_dir, f"voice_{i}.mp3")

        # Robust retry logic
        max_retries = 3
        success = False

        for attempt in range(max_retries):
            try:
                communicate = edge_tts.Communicate(entry["text"], voice, rate="+25%")
                await communicate.save(out_file)

                if os.path.exists(out_file) and os.path.getsize(out_file) > 0:
                    success = True
                    break
            except Exception as e:
                sys.stderr.write(f"Attempt {attempt+1} failed for voice_{i}.mp3: {str(e)}\n")

            if not success and attempt < max_retries - 1:
                await asyncio.sleep(2 * (attempt + 1))

        if not success:
            sys.stderr.write(f"Failed to generate voice_{i}.mp3 after {max_retries} attempts.\n")
            metadata.append({
                "path": out_file,
                "start": entry["start"],
                "duration": entry["duration"]
            })
            progress = int(((i + 1) / len(entries)) * 100)
            sys.stderr.write(f"{progress}%\n")
            sys.stderr.flush()
            continue

        # Time-stretch to match subtitle duration
        target_duration = entry["duration"]
        stretched_file = os.path.join(output_dir, f"voice_{i}_stretched.mp3")

        if time_stretch_audio(out_file, stretched_file, target_duration, ffmpeg_path):
            # Replace original with stretched version
            os.replace(stretched_file, out_file)

        # Get final duration for metadata
        final_duration = get_audio_duration(out_file, ffmpeg_path)
        if final_duration is None:
            final_duration = entry["duration"]

        metadata_item = {
            "path": out_file,
            "start": entry["start"],
            "duration": final_duration
        }   
        metadata.append(metadata_item)   
     
        progress = int(((i + 1) / len(entries)) * 100)
        sys.stderr.write(f"{progress}%\n")
        sys.stderr.flush()
        
        sys.stdout.write(json.dumps(metadata_item) + "\n")
        sys.stdout.flush()
    with open(os.path.join(output_dir, "metadata.json"), 'w', encoding='utf-8') as f:
        json.dump(metadata, f, indent=4)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--srt", required=True)
    parser.add_argument("--lang", required=True)
    parser.add_argument("--outdir", required=True)
    parser.add_argument("--ffmpeg", default="ffmpeg",
                        help="Path to ffmpeg executable for time-stretching")
    args = parser.parse_args()

    if not os.path.exists(args.outdir):
        os.makedirs(args.outdir)

    asyncio.run(generate_voice(args.srt, args.lang, args.outdir, args.ffmpeg))


if __name__ == "__main__":
    main()
