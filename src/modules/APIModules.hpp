#pragma once

#include "../Common.hpp"
#include <string>
#include <unordered_map>

namespace megasuite::api {

    void initAPIs();

    // Exported APIs for external mods or internal systems
    class LevelIDAPI {
    public:
        static int getCurrentLevelID();
        static std::string getCurrentLevelName();
        static std::string getCurrentAuthorName();
    };

    class NodeIDsAPI {
    public:
        static void dumpHierarchy(CCNode* root, int depth = 0);
        static CCNode* findByPattern(CCNode* root, const std::string& pattern);
    };

    class PersistenceAPI {
    public:
        static void setString(const std::string& key, const std::string& value);
        static std::string getString(const std::string& key, const std::string& def = "");

        static void setNumber(const std::string& key, double value);
        static double getNumber(const std::string& key, double def = 0.0);

        static void setBoolean(const std::string& key, bool value);
        static bool getBoolean(const std::string& key, bool def = false);
    };

} // namespace megasuite::api
