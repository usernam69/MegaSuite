#pragma once

#include "../Common.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace megasuite {

    enum class Category {
        QoL,
        DVDMode,
        Bugfixes,
        APIs,
        Settings
    };

    struct ModSetting {
        std::string id;
        std::string name;
        std::string description;
        Category category;
        bool isBool = true;
        bool boolVal = false;
        float floatVal = 0.0f;
        float minFloat = 0.0f;
        float maxFloat = 1.0f;
        std::string format = "%.2f";
        std::function<void(bool)> onBoolChanged;
        std::function<void(float)> onFloatChanged;
    };

    class ModuleManager {
    protected:
        std::vector<ModSetting> m_settings;
        std::unordered_map<std::string, size_t> m_indexMap;

        ModuleManager();

    public:
        static ModuleManager* get();

        void initDefaults();
        void registerBool(const std::string& id, const std::string& name, const std::string& description,
                          Category cat, bool defaultVal, std::function<void(bool)> onChange);
        void registerFloat(const std::string& id, const std::string& name, const std::string& description,
                           Category cat, float defaultVal, float min, float max, const std::string& format,
                           std::function<void(float)> onChange);

        std::vector<ModSetting> getCategorySettings(Category cat) const;
        bool getBool(const std::string& id) const;
        void setBool(const std::string& id, bool val);
        float getFloat(const std::string& id) const;
        void setFloat(const std::string& id, float val);
    };

} // namespace megasuite
