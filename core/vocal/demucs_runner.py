from pathlib import Path
import sys

import soundfile as sf


def install_soundfile_wav_writer():
    import demucs.audio as demucs_audio
    import demucs.separate as demucs_separate

    original_save_audio = demucs_audio.save_audio

    def save_audio(wav, path, samplerate, bitrate=320, clip="rescale",
                   bits_per_sample=16, as_float=False, preset=2):
        output_path = Path(path)
        if output_path.suffix.lower() != ".wav":
            return original_save_audio(
                wav,
                output_path,
                samplerate,
                bitrate=bitrate,
                clip=clip,
                bits_per_sample=bits_per_sample,
                as_float=as_float,
                preset=preset,
            )

        output_path.parent.mkdir(parents=True, exist_ok=True)
        clipped = demucs_audio.prevent_clip(wav, mode=clip).detach().cpu()
        if clipped.ndim == 1:
            audio = clipped.numpy()
        else:
            audio = clipped.transpose(0, 1).contiguous().numpy()

        if as_float:
            subtype = "FLOAT"
        elif bits_per_sample == 24:
            subtype = "PCM_24"
        else:
            subtype = "PCM_16"

        sf.write(str(output_path), audio, samplerate, subtype=subtype)

    demucs_audio.save_audio = save_audio
    demucs_separate.save_audio = save_audio
    return demucs_separate.main


def main():
    demucs_main = install_soundfile_wav_writer()
    return demucs_main()


if __name__ == "__main__":
    sys.exit(main())
