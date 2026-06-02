# Khmer Font Rendering Strategy

To ensure complex scripts like Khmer (which rely heavily on ligatures, consonant/vowel stacking, and specialized text shaping) render perfectly without breaking, the application uses a custom **Advanced Subtitle Rendering** approach instead of relying on FFmpeg's built-in text filters (`drawtext` or `subtitles`).

## 1. Bypassing FFmpeg's Text Engine
FFmpeg's default text rendering often struggles with complex text layout (CTL) languages unless specifically compiled with advanced HarfBuzz/libass integrations, which can be highly platform-dependent. 

Instead of passing text strings directly to FFmpeg, the app uses Qt's robust `QPainter` and `QFont` engines (which natively utilize HarfBuzz) to draw the text onto transparent `QImage` frames. These perfectly-rendered images are then fed into FFmpeg using the `concat` demuxer as an image overlay sequence.

## 2. Strict Font Validation (`resolveKhmerFont`)
The `AdvancedSubtitleRenderer::resolveKhmerFont()` function ensures that the font being used actually knows how to shape Khmer characters:

- **Capability Checking:** It queries `QFontDatabase::writingSystems(family)` to verify if the requested font explicitly advertises support for `QFontDatabase::Khmer`.
- **Intelligent Fallbacks:** If the requested font lacks Khmer support, it queries the system for all fonts that support Khmer.
- **Preferred List:** It iterates through a hardcoded list of highly reliable Khmer fonts (e.g., `"Khmer UI"`, `"Khmer OS System"`, `"DaunPenh"`, `"MoolBoran"`, `"Hanuman"`, `"Nokora"`) to ensure the fallback font looks good, rather than picking a random system font that might have ugly glyphs.

## 3. High-Quality Rasterization
To ensure the complex curves and small diacritics of Khmer text are legible against any video background:
- `QFont::PreferAntialias` is forced on the font style strategy.
- `QPainter::Antialiasing` and `QPainter::TextAntialiasing` are enabled.
- A simulated text stroke (outline) is drawn by rendering the text in black offset by a few pixels in a circular radius, followed by the white text on top. This ensures legibility without needing complex shader effects.
