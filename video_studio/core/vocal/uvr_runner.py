import sys
import os
import argparse
from audio_separator.separator import Separator

def main():
    parser = argparse.ArgumentParser(description="Vocal Isolator using audio-separator")
    parser.add_argument("input_audio", help="Path to input audio file")
    parser.add_argument("-o", "--outdir", required=True, help="Output directory")
    args = parser.parse_args()

    input_file = args.input_audio
    output_dir = args.outdir

    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # Initialize the Separator
    # MDX-Net Inst_HQ_3 is one of the best models for isolating vocals and instruments
    separator = Separator(
        output_dir=output_dir,
        output_format="WAV"
    )

    # Load the MDX-Net model
    separator.load_model("UVR-MDX-NET-Inst_HQ_3.onnx")

    # Perform separation
    # output will be a tuple of (instrumental_filename, vocal_filename)
    results = separator.separate(input_file)
    
    if len(results) == 2:
        inst_out, voc_out = results
        # audio-separator generates filenames based on the input and model.
        # We need to rename them to what VocalIsolator.cpp expects: "vocals.wav" and "no_vocals.wav"
        
        inst_path = os.path.join(output_dir, inst_out)
        voc_path = os.path.join(output_dir, voc_out)
        
        final_inst = os.path.join(output_dir, "no_vocals.wav")
        final_voc = os.path.join(output_dir, "vocals.wav")
        
        # Replace the existing output if it exists
        if os.path.exists(final_inst):
            os.remove(final_inst)
        if os.path.exists(final_voc):
            os.remove(final_voc)
            
        os.rename(inst_path, final_inst)
        os.rename(voc_path, final_voc)
        
        print("Successfully isolated vocals to vocals.wav and no_vocals.wav")
    else:
        sys.stderr.write("Separation failed to produce two stems.\n")
        sys.exit(1)

if __name__ == "__main__":
    main()
