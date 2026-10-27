#pragma once

#include <Geode/Geode.hpp>
#include <string>
#include <vector>
#include <functional>
#include <memory>

using namespace geode::prelude;

namespace megasuite {

    inline ccColor3B parseHexColor(const std::string& hexStr) {
        std::string hex = hexStr;
        if (!hex.empty() && hex[0] == '#') {
            hex = hex.substr(1);
        }
        if (hex.length() != 6) {
            return ccc3(0, 255, 127); // Default MegaHack neon green
        }
        try {
            int r = std::stoi(hex.substr(0, 2), nullptr, 16);
            int g = std::stoi(hex.substr(2, 2), nullptr, 16);
            int b = std::stoi(hex.substr(4, 2), nullptr, 16);
            return ccc3(static_cast<GLubyte>(r), static_cast<GLubyte>(g), static_cast<GLubyte>(b));
        } catch (...) {
            return ccc3(0, 255, 127);
        }
    }

    template<typename T>
    inline T getSetting(const std::string& key, T defaultValue) {
        auto mod = Mod::get();
        if (!mod) return defaultValue;
        return mod->getSettingValue<T>(key);
    }

    template<typename T>
    inline void setSetting(const std::string& key, T value) {
        auto mod = Mod::get();
        if (!mod) return;
        mod->setSettingValue<T>(key, value);
    }

} // namespace megasuite
