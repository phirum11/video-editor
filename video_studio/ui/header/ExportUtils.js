
function formatSize(seconds, resolution, exportVideo) {
    if (!seconds || seconds <= 0) return "0 MB";
    
    // Rough estimation (MB per second)
    let mbPerSecond = 0.024; // Default audio-only (approx 192kbps)
    
    if (exportVideo !== false) {
        if (resolution === "4K") {
            mbPerSecond = 5.625; // ~45 Mbps
        } else if (resolution === "1080P") {
            mbPerSecond = 1.5; // ~12 Mbps
        } else if (resolution === "720P") {
            mbPerSecond = 0.625; // ~5 Mbps
        } else if (resolution === "480P") {
            mbPerSecond = 0.3125; // ~2.5 Mbps
        } else if (resolution === "360P") {
            mbPerSecond = 0.125; // ~1 Mbps
        }
    }
    
    const mb = seconds * mbPerSecond;
    if (mb > 1024) return (mb / 1024).toFixed(1) + " GB";
    return mb.toFixed(1) + " MB";
}

function formatDuration(seconds) {
    if (!seconds || seconds <= 0) return "00:00:00";
    const h = Math.floor(seconds / 3600);
    const m = Math.floor((seconds % 3600) / 60);
    const s = Math.floor(seconds % 60);
    return (h < 10 ? "0" : "") + h + ":" + (m < 10 ? "0" : "") + m + ":" + (s < 10 ? "0" : "") + s;
}
