#include "APIModules.hpp"
#include "ModuleManager.hpp"

#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/modify/EditLevelLayer.hpp>

using namespace geode::prelude;

namespace megasuite::api {

    static int s_currentLevelID = 0;
    static std::string s_currentLevelName = "";
    static std::string s_currentAuthorName = "";
    static std::unordered_map<std::string, std::string> s_persistentStrings;
    static std::unordered_map<std::string, double> s_persistentNumbers;
    static std::unordered_map<std::string, bool> s_persistentBools;

    void initAPIs() {
        auto mgr = ModuleManager::get();
        mgr->registerBool("api-level-id", "Level ID API",
                          "Exposes active level metadata and ID tracking for other mods and overlays",
                          Category::APIs, true, nullptr);
        mgr->registerBool("api-node-ids", "Node IDs Inspector",
                          "Auto-tags and identifies UI nodes across GD menus for easy hooking",
                          Category::APIs, true, nullptr);
        mgr->registerBool("api-persistence", "Unified Persistence API",
                          "Fast, in-memory & file-backed key-value storage engine for cross-mod data",
                          Category::APIs, true, nullptr);
    }

    int LevelIDAPI::getCurrentLevelID() {
        return s_currentLevelID;
    }

    std::string LevelIDAPI::getCurrentLevelName() {
        return s_currentLevelName;
    }

    std::string LevelIDAPI::getCurrentAuthorName() {
        return s_currentAuthorName;
    }

    void NodeIDsAPI::dumpHierarchy(CCNode* root, int depth) {
        if (!root) return;
        std::string indent(depth * 2, ' ');
        std::string id = root->getID();
        std::string tag = id.empty() ? "(unnamed)" : id;
        log::debug("{}- Node: {} [type: {}]", indent, tag, typeid(*root).name());

        for (auto* child : CCArrayExt<CCNode*>(root->getChildren())) {
            dumpHierarchy(child, depth + 1);
        }
    }

    CCNode* NodeIDsAPI::findByPattern(CCNode* root, const std::string& pattern) {
        if (!root) return nullptr;
        if (root->getID().find(pattern) != std::string::npos) {
            return root;
        }
        for (auto* child : CCArrayExt<CCNode*>(root->getChildren())) {
            auto res = findByPattern(child, pattern);
            if (res) return res;
        }
        return nullptr;
    }

    void PersistenceAPI::setString(const std::string& key, const std::string& value) {
        s_persistentStrings[key] = value;
        Mod::get()->setSavedValue(key, value);
    }

    std::string PersistenceAPI::getString(const std::string& key, const std::string& def) {
        auto it = s_persistentStrings.find(key);
        if (it != s_persistentStrings.end()) return it->second;
        return Mod::get()->getSavedValue<std::string>(key, def);
    }

    void PersistenceAPI::setNumber(const std::string& key, double value) {
        s_persistentNumbers[key] = value;
        Mod::get()->setSavedValue(key, value);
    }

    double PersistenceAPI::getNumber(const std::string& key, double def) {
        auto it = s_persistentNumbers.find(key);
        if (it != s_persistentNumbers.end()) return it->second;
        return Mod::get()->getSavedValue<double>(key, def);
    }

    void PersistenceAPI::setBoolean(const std::string& key, bool value) {
        s_persistentBools[key] = value;
        Mod::get()->setSavedValue(key, value);
    }

    bool PersistenceAPI::getBoolean(const std::string& key, bool def) {
        auto it = s_persistentBools.find(key);
        if (it != s_persistentBools.end()) return it->second;
        return Mod::get()->getSavedValue<bool>(key, def);
    }

} // namespace megasuite::api

using namespace megasuite;
using namespace megasuite::api;

class $modify(MegaLevelInfoLayer, LevelInfoLayer) {
    bool init(GJGameLevel* level, bool challenge) {
        if (!LevelInfoLayer::init(level, challenge)) return false;

        if (level && ModuleManager::get()->getBool("api-level-id")) {
            s_currentLevelID = level->m_levelID;
            s_currentLevelName = level->m_levelName;
            s_currentAuthorName = level->m_creatorName;

            log::info("MegaSuite API: Registered Level {} ('{}') by {}",
                      s_currentLevelID, s_currentLevelName, s_currentAuthorName);
        }
        return true;
    }
};

class $modify(MegaEditLevelLayer, EditLevelLayer) {
    bool init(GJGameLevel* level) {
        if (!EditLevelLayer::init(level)) return false;

        if (level && ModuleManager::get()->getBool("api-level-id")) {
            s_currentLevelID = level->m_levelID;
            s_currentLevelName = level->m_levelName;
            s_currentAuthorName = level->m_creatorName;
        }
        return true;
    }
};
