import onnxruntime as ort
import sys

def inspect_model(model_path):
    try:
        session = ort.InferenceSession(model_path)
        print(f"--- Input Nodes for {model_path} ---")
        for i in session.get_inputs():
            print(f"Name: {i.name}, Shape: {i.shape}, Type: {i.type}")
        
        print(f"\n--- Output Nodes for {model_path} ---")
        for o in session.get_outputs():
            print(f"Name: {o.name}, Shape: {o.shape}, Type: {o.type}")
    except Exception as e:
        print(f"Error inspecting model: {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        inspect_model(sys.argv[1])
    else:
        print("Please provide a model path.")
