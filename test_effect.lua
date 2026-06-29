-- ==============================================================================
-- VideoStudio - CapCut-Equivalent Lua Scripting Test
-- Demonstrates effect parameter bindings, animation speed clocks, and LUT grading
-- ==============================================================================

print("============================================================")
print("  Starting VideoStudio Lua Effect Binding Test...")
print("  Engine: Lua 5.4 / Sol2 Binding Layer")
print("============================================================\n")

-- Define a sample CapCut effect template structure
EffectChain = {
    name = "Cyber Grid & Cinematic LUT",
    version = "1.0.0",
    engine_compatibility = "AmazingEngine/QtRHI",
    
    parameters = {
        speed_clock = 1.25,        -- Overlay animation clock multiplier
        blend_mode = "Screen",     -- Hardware GLSL shader blend mode
        opacity = 0.85,            -- Overlay transparency
        vector_asset = "assets/lottie/cyber_grid.json",
        lut_asset = "assets/luts/CapCut_TealAndOrange.cube"
    }
}

function process_frame_metadata(frame_index, timestamp_ms)
    -- Calculate dynamic effect time based on speed clock
    local effect_time = timestamp_ms * EffectChain.parameters.speed_clock
    
    print(string.format("[Frame %04d] Time: %6.2f ms | Speed Clock: %.2fx | Active Blend: %s | Opacity: %d%%",
        frame_index, effect_time, EffectChain.parameters.speed_clock, EffectChain.parameters.blend_mode, EffectChain.parameters.opacity * 100))
        
    return effect_time
end

-- Simulate processing the first 5 frames of video
print("Simulating real-time video frame processing across timeline:")
for i = 1, 5 do
    process_frame_metadata(i, i * 16.666) -- 60fps frame intervals
end

print("\n============================================================")
print("  Lua Effect Script Executed Successfully!")
print("============================================================")
