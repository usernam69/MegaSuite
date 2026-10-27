#include "QOLModules.hpp"
#include "ModuleManager.hpp"

#include <Geode/modify/CCScheduler.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/FMODAudioEngine.hpp>

using namespace geode::prelude;

namespace megasuite::qol {

    static float s_currentSpeed = 1.0f;

    void updateAudioSpeed(float speed) {
        if (!FMODAudioEngine::sharedEngine()) return;
        // Adjust master channel pitch/frequency if speedhack audio sync is enabled
        auto fmod = FMODAudioEngine::sharedEngine();
        if (fmod->m_backgroundMusicChannel) {
            fmod->m_backgroundMusicChannel->setPitch(speed);
        }
    }

    void initQOLModules() {
        auto mgr = ModuleManager::get();

        // Speedhack
        mgr->registerBool("speedhack-enabled", "Speedhack",
                          "Alters the game update tick rate for practice or fast-forwarding",
                          Category::QoL, false, [](bool val) {
                              if (!val) {
                                  s_currentSpeed = 1.0f;
                                  updateAudioSpeed(1.0f);
                              }
                          });
        mgr->registerFloat("speedhack-speed", "Speed Multiplier",
                           "Game speed factor (e.g., 0.5x, 1.5x, 2.0x)",
                           Category::QoL, 1.0f, 0.1f, 5.0f, "%.2fx", [](float val) {
                               if (ModuleManager::get()->getBool("speedhack-enabled")) {
                                   s_currentSpeed = val;
                                   if (ModuleManager::get()->getBool("speedhack-audio")) {
                                       updateAudioSpeed(val);
                                   }
                               }
                           });
        mgr->registerBool("speedhack-audio", "Speedhack Audio Pitch",
                          "Pitch the level background music to match speedhack speed",
                          Category::QoL, false, [](bool val) {
                              if (val && ModuleManager::get()->getBool("speedhack-enabled")) {
                                  updateAudioSpeed(s_currentSpeed);
                              } else {
                                  updateAudioSpeed(1.0f);
                              }
                          });

        // StartPos Switcher
        mgr->registerBool("startpos-switcher", "StartPos Switcher",
                          "Cycle between start positions in levels using left and right arrows",
                          Category::QoL, true, nullptr);

        // Show Hitboxes
        mgr->registerBool("show-hitboxes", "Show Hitboxes",
                          "Display object collision boxes and player boundaries in real-time",
                          Category::QoL, false, nullptr);

        // Practice Music Sync
        mgr->registerBool("practice-music-sync", "Practice Music Sync",
                          "Keep original level song playing in sync during practice attempts",
                          Category::QoL, true, nullptr);

        // Instant Respawn
        mgr->registerBool("instant-respawn", "Instant Respawn",
                          "Skips the death particle animation delay and immediately resets",
                          Category::QoL, false, nullptr);

        // Noclip
        mgr->registerBool("noclip", "Noclip",
                          "Prevents dying upon collision with spikes, obstacles, or walls",
                          Category::QoL, false, nullptr);
    }

} // namespace megasuite::qol

using namespace megasuite;

// 1. Speedhack hook
class $modify(MegaScheduler, CCScheduler) {
    void update(float dt) {
        if (ModuleManager::get()->getBool("speedhack-enabled")) {
            float speed = ModuleManager::get()->getFloat("speedhack-speed");
            if (speed > 0.01f && speed < 20.0f) {
                dt *= speed;
            }
        }
        CCScheduler::update(dt);
    }
};

// 2. PlayLayer hooks (Hitboxes, Instant Respawn, StartPos, Practice Music)
class $modify(MegaPlayLayer, PlayLayer) {
    struct Fields {
        std::vector<StartPosObject*> m_startPositions;
        int m_currentIndex = -1;
    };

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) return false;

        // Auto Practice Music Sync
        if (ModuleManager::get()->getBool("practice-music-sync")) {
            m_practiceMusicSync = true;
        }

        // Hitboxes debug display
        if (ModuleManager::get()->getBool("show-hitboxes")) {
            m_debugDraw = true;
        }

        return true;
    }

    void resetLevel() {
        PlayLayer::resetLevel();
        if (ModuleManager::get()->getBool("show-hitboxes")) {
            m_debugDraw = true;
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        // Noclip check
        if (ModuleManager::get()->getBool("noclip")) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);

        // Instant Respawn
        if (ModuleManager::get()->getBool("instant-respawn")) {
            resetLevel();
        }
    }
};
