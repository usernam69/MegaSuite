#include "Common.hpp"

#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/AudioEffectsLayer.hpp>
#include <Geode/modify/UILayer.hpp>
#include <Geode/modify/PauseLayer.hpp>
#include <Geode/modify/EditorUI.hpp>

#include <algorithm>
#include <vector>

using namespace geode::prelude;

namespace {

std::vector<StartPosObject*> g_startPositions;
int g_startPosIndex = 0;

void sortStartPositions() {
    std::sort(
        g_startPositions.begin(), g_startPositions.end(),
        [](StartPosObject* a, StartPosObject* b) {
            if (!a) return false;
            if (!b) return true;
            return a->getPositionX() < b->getPositionX();
        }
    );
}

} // namespace

// ---------------------------------------------------------------------------
// PlayLayer: noclip, instant respawn, hitboxes, practice music, startpos
// ---------------------------------------------------------------------------

class $modify(MSPlayLayer, PlayLayer) {
public:
    static MSPlayLayer* getMS() {
        return static_cast<MSPlayLayer*>(PlayLayer::get());
    }

    bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
        g_startPositions.clear();
        g_startPosIndex = 0;

        if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
            return false;
        }

        if (ms::enabled("practice-music-sync")) {
            m_practiceMusicSync = true;
        }

        return true;
    }

    void addObject(GameObject* object) {
        PlayLayer::addObject(object);

        if (object && object->m_objectID == 31) {
            g_startPositions.push_back(static_cast<StartPosObject*>(object));
        }
    }

    void destroyPlayer(PlayerObject* player, GameObject* object) {
        if (ms::enabled("noclip")) {
            return;
        }

        PlayLayer::destroyPlayer(player, object);

        if (!player || !ms::enabled("instant-respawn")) {
            return;
        }

        if (this->getActionByTag(0x10)) {
            this->stopActionByTag(0x10);

            auto seq = CCSequence::create(
                CCDelayTime::create(0.f),
                CCCallFunc::create(this, callfunc_selector(PlayLayer::delayedResetLevel)),
                nullptr
            );
            if (seq) {
                seq->setTag(0x10);
                this->runAction(seq);
            }
        }
    }

    void resetLevel() {
        PlayLayer::resetLevel();

        if (!ms::enabled("fix-startpos-mirror")) {
            return;
        }
        if (!m_isTestMode || !m_startPosObject || !m_startPosObject->m_startSettings || m_isPracticeMode) {
            return;
        }

        PlayLayer::toggleFlipped(m_startPosObject->m_startSettings->m_mirrorMode, true);
    }

    void togglePracticeMode(bool practiceMode) {
        PlayLayer::togglePracticeMode(practiceMode);

        if (ms::enabled("practice-music-sync")) {
            m_practiceMusicSync = true;
        }
    }

    void postUpdate(float dt) {
        PlayLayer::postUpdate(dt);

        bool want = ms::enabled("show-hitboxes");
        if (m_isDebugDrawEnabled != want) {
            this->toggleDebugDraw();
        }
    }

    void switchStartPos(int amount) {
        sortStartPositions();

        int total = static_cast<int>(g_startPositions.size());
        if (total == 0) {
            return;
        }

        int index = g_startPosIndex + amount;
        if (index < 0) {
            index = total;
        }
        if (index > total) {
            index = 0;
        }

        g_startPosIndex = index;
        m_currentCheckpoint = nullptr;

        StartPosObject* target = (index == 0) ? nullptr : g_startPositions[index - 1];
        this->setStartPosObject(target);

        if (this->isRunning()) {
            if (m_isPracticeMode) {
                this->resetLevelFromStart();
            }
            PlayLayer::resetLevel();
            this->startMusic();
        }
    }
};

// ---------------------------------------------------------------------------
// StartPos switcher keybinds
// ---------------------------------------------------------------------------

class $modify(MSUILayer, UILayer) {
    void handleKeypress(enumKeyCodes key, bool down, double timestamp) {
        if (down) {
            if (key == KEY_Tab) {
                ms::toggleMenu();
                return;
            }
            if (key == KEY_Escape && ms::isMenuOpen()) {
                ms::closeMenu();
                return;
            }
        }

        if (down && ms::enabled("startpos-switcher")) {
            if (auto pl = MSPlayLayer::getMS()) {
                if (!pl->m_levelEndAnimationStarted) {
                    if (key == KEY_Left) {
                        pl->switchStartPos(-1);
                    }
                    else if (key == KEY_Right) {
                        pl->switchStartPos(1);
                    }
                }
            }
        }

        UILayer::handleKeypress(key, down, timestamp);
    }
};

// ---------------------------------------------------------------------------
// TAB / ESC in the pause menu and the editor
// ---------------------------------------------------------------------------

class $modify(MSPauseLayer, PauseLayer) {
    void keyDown(enumKeyCodes key, double timestamp) {
        if (key == KEY_Tab) {
            ms::toggleMenu();
            return;
        }
        if (key == KEY_Escape && ms::isMenuOpen()) {
            ms::closeMenu();
            return;
        }
        PauseLayer::keyDown(key, timestamp);
    }
};

class $modify(MSEditorUI, EditorUI) {
    void keyDown(enumKeyCodes key, double timestamp) {
        if (key == KEY_Tab) {
            ms::toggleMenu();
            return;
        }
        if (key == KEY_Escape && ms::isMenuOpen()) {
            ms::closeMenu();
            return;
        }
        EditorUI::keyDown(key, timestamp);
    }
};

// ---------------------------------------------------------------------------
// PlayerObject bugfixes
// ---------------------------------------------------------------------------

class $modify(MSPlayerObject, PlayerObject) {
    void setPosition(CCPoint const& position) {
        PlayerObject::setPosition(position);

        // 2.2 moved ship / ufo click particles to the trailing particles
        // position; restore the 2.113 behaviour
        if (ms::enabled("fix-dash-particles") && m_dashParticles) {
            m_dashParticles->setPosition(position);
        }
    }

    void toggleVisibility(bool show) {
        bool willShowWave =
            show && m_isDart && m_isHidden && ms::enabled("fix-trigger-wave-particles");

        PlayerObject::toggleVisibility(show);

        // toggleVisibility hits the "always has trail" branch and calls
        // m_trailingParticles->resetSystem(), which removes trigger-spawned
        // wave particles
        if (willShowWave && m_trailingParticles) {
            m_trailingParticles->stopSystem();
        }
    }

    void togglePlayerScale(bool enable, bool noEffects) {
        PlayerObject::togglePlayerScale(enable, noEffects);

        if (!ms::enabled("fix-mini-swing")) {
            return;
        }
        if (!m_isSwing || !m_defaultMiniIcon || !m_gameLayer) {
            return;
        }
        if (this != m_gameLayer->m_player1 && this != m_gameLayer->m_player2) {
            return;
        }

        this->updatePlayerSwingFrame(GameManager::get()->getPlayerSwing());
    }
};

// ---------------------------------------------------------------------------
// GJBaseGameLayer: speedhack + dual wave trail fix
// ---------------------------------------------------------------------------

class $modify(MSBaseGameLayer, GJBaseGameLayer) {
    void update(float dt) {
        GJBaseGameLayer::update(dt * ms::speedhackMult());
    }

    void toggleDualMode(GameObject* portal, bool dual, PlayerObject* player, bool noEffects) {
        bool fix = ms::enabled("fix-dual-wave-trail");

        if (!fix || dual || player != m_player2 || !m_player1 || !m_player2) {
            GJBaseGameLayer::toggleDualMode(portal, dual, player, noEffects);
            return;
        }

        if (!m_player2->m_isDart || !m_player2->m_waveTrail || !m_player1->m_waveTrail) {
            GJBaseGameLayer::toggleDualMode(portal, dual, player, noEffects);
            if (m_player1->m_waveTrail) {
                m_player1->m_waveTrail->reset();
            }
            return;
        }

        std::vector<CCPoint> points;
        for (auto pointNode : CCArrayExt<PointNode*>(m_player2->m_waveTrail->m_pointArray)) {
            points.push_back(pointNode->m_point);
        }

        // the game fades both trails out, blank the player 2 trail so it is
        // invisible, then restore the points on the player 1 trail
        m_player2->m_waveTrail->reset();
        GJBaseGameLayer::toggleDualMode(portal, dual, player, noEffects);

        m_player1->m_waveTrail->reset();
        for (auto& point : points) {
            m_player1->m_waveTrail->addPoint(point);
        }
    }
};

// ---------------------------------------------------------------------------
// Practice music pulse fix
// ---------------------------------------------------------------------------

class $modify(MSAudioEffectsLayer, AudioEffectsLayer) {
    void audioStep(float dt) {
        if (!ms::enabled("fix-practice-music-pulse")) {
            return AudioEffectsLayer::audioStep(dt);
        }

        auto pl = PlayLayer::get();
        if (!pl) {
            return AudioEffectsLayer::audioStep(dt);
        }

        bool saved = pl->m_isPracticeMode;
        pl->m_isPracticeMode = pl->m_isPracticeMode && !pl->m_practiceMusicSync;

        AudioEffectsLayer::audioStep(dt);

        pl->m_isPracticeMode = saved;
    }
};
