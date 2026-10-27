#include "ModuleManager.hpp"

namespace megasuite {

    ModuleManager::ModuleManager() {}

    ModuleManager* ModuleManager::get() {
        static ModuleManager instance;
        return &instance;
    }

    void ModuleManager::registerBool(const std::string& id, const std::string& name, const std::string& description,
                                    Category cat, bool defaultVal, std::function<void(bool)> onChange) {
        ModSetting s;
        s.id = id;
        s.name = name;
        s.description = description;
        s.category = cat;
        s.isBool = true;
        s.boolVal = getSetting<bool>(id, defaultVal);
        s.onBoolChanged = onChange;

        m_indexMap[id] = m_settings.size();
        m_settings.push_back(s);

        if (onChange) {
            onChange(s.boolVal);
        }
    }

    void ModuleManager::registerFloat(const std::string& id, const std::string& name, const std::string& description,
                                     Category cat, float defaultVal, float min, float max, const std::string& format,
                                     std::function<void(float)> onChange) {
        ModSetting s;
        s.id = id;
        s.name = name;
        s.description = description;
        s.category = cat;
        s.isBool = false;
        s.floatVal = getSetting<double>(id, static_cast<double>(defaultVal));
        s.minFloat = min;
        s.maxFloat = max;
        s.format = format;
        s.onFloatChanged = onChange;

        m_indexMap[id] = m_settings.size();
        m_settings.push_back(s);

        if (onChange) {
            onChange(s.floatVal);
        }
    }

    std::vector<ModSetting> ModuleManager::getCategorySettings(Category cat) const {
        std::vector<ModSetting> out;
        for (const auto& s : m_settings) {
            if (s.category == cat) {
                out.push_back(s);
            }
        }
        return out;
    }

    bool ModuleManager::getBool(const std::string& id) const {
        auto it = m_indexMap.find(id);
        if (it != m_indexMap.end()) {
            return m_settings[it->second].boolVal;
        }
        return false;
    }

    void ModuleManager::setBool(const std::string& id, bool val) {
        auto it = m_indexMap.find(id);
        if (it != m_indexMap.end()) {
            m_settings[it->second].boolVal = val;
            setSetting<bool>(id, val);
            if (m_settings[it->second].onBoolChanged) {
                m_settings[it->second].onBoolChanged(val);
            }
        }
    }

    float ModuleManager::getFloat(const std::string& id) const {
        auto it = m_indexMap.find(id);
        if (it != m_indexMap.end()) {
            return m_settings[it->second].floatVal;
        }
        return 0.0f;
    }

    void ModuleManager::setFloat(const std::string& id, float val) {
        auto it = m_indexMap.find(id);
        if (it != m_indexMap.end()) {
            m_settings[it->second].floatVal = val;
            setSetting<double>(id, static_cast<double>(val));
            if (m_settings[it->second].onFloatChanged) {
                m_settings[it->second].onFloatChanged(val);
            }
        }
    }

} // namespace megasuite
