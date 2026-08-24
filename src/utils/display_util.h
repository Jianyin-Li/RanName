#ifndef DISPLAY_UTIL_H
#define DISPLAY_UTIL_H

#include <string>

// Shared terminal display helpers: UTF-8 width handling that is aware of
// CJK / full-width characters (2 columns) and ANSI escape sequences (ignored).

namespace display {

inline unsigned int decodeUtf8(const std::string& s, size_t& i, int& adv) {
    unsigned char c = static_cast<unsigned char>(s[i]);
    if (c < 0x80) { adv = 1; return c; }
    if ((c & 0xE0) == 0xC0) {
        adv = 2;
        if (i + 1 >= s.size()) return c;
        return ((c & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu);
    }
    if ((c & 0xF0) == 0xE0) {
        adv = 3;
        if (i + 2 >= s.size()) return c;
        return ((c & 0x0Fu) << 12) |
               ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 6) |
               (static_cast<unsigned char>(s[i + 2]) & 0x3Fu);
    }
    if ((c & 0xF8) == 0xF0) {
        adv = 4;
        if (i + 3 >= s.size()) return c;
        return ((c & 0x07u) << 18) |
               ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 12) |
               ((static_cast<unsigned char>(s[i + 2]) & 0x3Fu) << 6) |
               (static_cast<unsigned char>(s[i + 3]) & 0x3Fu);
    }
    adv = 1;
    return c;
}

inline bool isWide(unsigned int cp) {
    return (cp >= 0x1100 && cp <= 0x115F) ||
           (cp >= 0x2E80 && cp <= 0x303E) ||
           (cp >= 0x3041 && cp <= 0x33FF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0xA000 && cp <= 0xA4CF) ||
           (cp >= 0xAC00 && cp <= 0xD7A3) ||
           (cp >= 0xF900 && cp <= 0xFAFF) ||
           (cp >= 0xFE10 && cp <= 0xFE19) ||
           (cp >= 0xFE30 && cp <= 0xFE52) ||
           (cp >= 0xFE54 && cp <= 0xFE66) ||
           (cp >= 0xFE68 && cp <= 0xFE6B) ||
           (cp >= 0xFF00 && cp <= 0xFF60) ||
           (cp >= 0xFFE0 && cp <= 0xFFE6);
}

inline int displayWidth(const std::string& text) {
    int w = 0;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == 0x1B) {  // ANSI escape, skip the whole sequence up to 'm'
            size_t j = text.find('m', i);
            if (j == std::string::npos) break;
            i = j + 1;
            continue;
        }
        int adv;
        unsigned int cp = decodeUtf8(text, i, adv);
        w += isWide(cp) ? 2 : 1;
        i += adv;
    }
    return w;
}

// Truncate text to a display width without breaking UTF-8 sequences.
// ANSI escape sequences are preserved but not counted toward the width.
inline std::string fitWidth(const std::string& text, int w) {
    std::string out;
    int cur = 0;
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if (c == 0x1B) {
            size_t j = text.find('m', i);
            if (j == std::string::npos) { out += text.substr(i); break; }
            out += text.substr(i, j - i + 1);
            i = j + 1;
            continue;
        }
        int adv;
        unsigned int cp = decodeUtf8(text, i, adv);
        int dw = isWide(cp) ? 2 : 1;
        if (cur + dw > w) break;
        out += text.substr(i, adv);
        cur += dw;
        i += adv;
    }
    return out;
}

inline std::string centerText(const std::string& text, int w) {
    int tw = displayWidth(text);
    if (tw >= w) return fitWidth(text, w);
    int left = (w - tw) / 2;
    return std::string(left, ' ') + text;
}

}  // namespace display

#endif  // DISPLAY_UTIL_H
