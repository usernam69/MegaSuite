#pragma once

#include <Geode/Geode.hpp>

#include <string>
#include <string_view>

using namespace geode::prelude;

namespace ms {

// ---------------------------------------------------------------------------
// settings access
// ---------------------------------------------------------------------------

inline bool enabled(std::string_view key) {
    return Mod::get()->getSettingValue<bool>(key);
}

inline void setEnabled(std::string_view key, bool value) {
    Mod::get()->setSettingValue<bool>(key, value);
}

inline float getFloat(std::string_view key, float fallback) {
    float v = Mod::get()->getSettingValue<float>(key);
    if (v != v) { // NaN
        return fallback;
    }
    return v;
}

inline void setFloat(std::string_view key, float value) {
    Mod::get()->setSettingValue<float>(key, value);
}

inline float speedhackMult() {
    if (!enabled("speedhack-enabled")) {
        return 1.f;
    }
    float v = getFloat("speedhack-speed", 1.f);
    return v > 0.01f ? v : 1.f;
}

inline float dvdSpeedMult() {
    float v = getFloat("dvd-speed", 1.f);
    return v > 0.01f ? v : 1.f;
}

inline float uiScale() {
    float v = getFloat("ui-scale", 1.f);
    if (v < 0.5f) v = 0.5f;
    if (v > 1.6f) v = 1.6f;
    return v;
}

inline float uiOpacity() {
    float v = getFloat("ui-opacity", 0.94f);
    if (v < 0.f) v = 0.f;
    if (v > 1.f) v = 1.f;
    return v;
}

// ---------------------------------------------------------------------------
// MegaHack-ish theme
// ---------------------------------------------------------------------------

inline ccColor4B colDim()      { return ccc4(0, 0, 0, 170); }
inline ccColor4B colSidebar()  { return ccc4(20, 24, 22, 255); }
inline ccColor4B colContent()  { return ccc4(12, 14, 13, 255); }
inline ccColor4B colHeader()   { return ccc4(0, 110, 68, 255); }
inline ccColor4B colRowAlt()   { return ccc4(28, 33, 30, 120); }

inline ccColor3B colAccent()   { return ccc3(0, 240, 120); }
inline ccColor3B colText()     { return ccc3(228, 236, 231); }
inline ccColor3B colTextDim()  { return ccc3(130, 145, 137); }
inline ccColor3B colBgTint()   { return ccc3(16, 18, 17); }

// ---------------------------------------------------------------------------
// floating menu
// ---------------------------------------------------------------------------

void openMenu();
void closeMenu();
void toggleMenu();
bool isMenuOpen();

} // namespace ms
