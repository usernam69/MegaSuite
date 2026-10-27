#include "BugfixModules.hpp"
#include "ModuleManager.hpp"

#include <Geode/modify/AudioEffectsLayer.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

namespace megasuite::bugfixes {

    void initBugfixes() {
        auto mgr = ModuleManager::get();
        mgr->registerBool("bugfix-practice-pulse", "Practice Music Sync Pulse Fix",
                          "Fixes pulsing objects in main level songs when using Practice Music Sync",
                          Category::Bugfixes, true, nullptr);
        mgr->registerBool("bugfix-p2-wave-trail", "Player 2 Wave Trail Fix",
                          "Fixes Player 2 dual wave trail visually glitching or disappearing after dual portal",
                          Category::Bugfixes, true, nullptr);
        mgr->registerBool("bugfix-startpos-mirror", "StartPos Mirror Mode Fix",
                          "Fixes StartPos objects ignoring mirror portal direction setting",
                          Category::Bugfixes, true, nullptr);
        mgr->registerBool("bugfix-mini-swing", "Mini Swing Default Icon Fix",
                          "Fixes mini swing displaying the default cube icon instead of player selection",
                          Category::Bugfixes, true, nullptr);
        mgr->registerBool("bugfix-dash-particles", "Dash Orb Particles Offset Fix",
                          "Fixes dash orb particles appearing with an incorrect pixel offset",
                          Category::Bugfixes, true, nullptr);
        mgr->registerBool("bugfix-show-trigger-particles", "Show Trigger Wave Particles Fix",
                          "Fixes wave trail showing ship/ufo thruster particles after hide/show triggers",
                          Category::Bugfixes, true, nullptr);
    }

} // namespace megasuite::bugfixes

using namespace megasuite;

// 1. Practice Music Sync Pulse Fix
class $modify(MegaPracticePulseFix, AudioEffectsLayer) {
    $override void audioStep(float p0) {
        if (!ModuleManager::get()->getBool("bugfix-practice-pulse")) {
            return AudioEffectsLayer::audioStep(p0);
        }
        auto* pl = PlayLayer::get();
        if (!pl) {
            return AudioEffectsLayer::audioStep(p0);
        }
        bool saved = pl->m_isPracticeMode;
        pl->m_isPracticeMode = pl->m_isPracticeMode && !pl->m_practiceMusicSync;
        AudioEffectsLayer::audioStep(p0);
        pl->m_isPracticeMode = saved;
    }
};

// 2. Player 2 Wave Trail Fix
class $modify(MegaDualWaveTrailFix, GJBaseGameLayer) {
    void toggleDualMode(GameObject* portal, bool state, PlayerObject* playerTouchingPortal, bool p4) {
        bool enabled = ModuleManager::get()->getBool("bugfix-p2-wave-trail");
        if (enabled && !state && playerTouchingPortal == m_player2) {
            if (m_player2 && m_player2->m_isDart && m_player2->m_waveTrail) {
                std::vector<CCPoint> points;
                for (auto pointNode : CCArrayExt<PointNode*>(m_player2->m_waveTrail->m_pointArray)) {
                    points.push_back(pointNode->m_point);
                }
                m_player2->m_waveTrail->reset();
                GJBaseGameLayer::toggleDualMode(portal, state, playerTouchingPortal, p4);
                if (m_player1 && m_player1->m_waveTrail) {
                    m_player1->m_waveTrail->reset();
                    for (auto& point : points) {
                        m_player1->m_waveTrail->addPoint(point);
                    }
                }
            } else {
                GJBaseGameLayer::toggleDualMode(portal, state, playerTouchingPortal, p4);
                if (m_player1 && m_player1->m_waveTrail) {
                    m_player1->m_waveTrail->reset();
                }
            }
        } else {
            GJBaseGameLayer::toggleDualMode(portal, state, playerTouchingPortal, p4);
        }
    }
};

// 3. StartPos Mirror Mode Fix
class $modify(MegaStartPosMirrorFix, PlayLayer) {
    void resetLevel() {
        PlayLayer::resetLevel();
        if (!ModuleManager::get()->getBool("bugfix-startpos-mirror")) return;
        if (!m_isTestMode || !m_startPosObject || !m_startPosObject->m_startSettings || m_isPracticeMode) return;
        PlayLayer::toggleFlipped(m_startPosObject->m_startSettings->m_mirrorMode, true);
    }
};

// 4. Mini Swing Default Icon Fix
class $modify(MegaMiniSwingFix, PlayerObject) {
    void togglePlayerScale(bool p0, bool p1) {
        PlayerObject::togglePlayerScale(p0, p1);
        if (!ModuleManager::get()->getBool("bugfix-mini-swing")) return;
        if (!m_isSwing || !m_defaultMiniIcon || !m_gameLayer) return;
        if (this != m_gameLayer->m_player1 && this != m_gameLayer->m_player2) return;
        PlayerObject::updatePlayerSwingFrame(GameManager::get()->getPlayerSwing());
    }

    // 5. Dash Orb Particles Offset Fix
    $override void setPosition(const CCPoint& position) {
        PlayerObject::setPosition(position);
        if (ModuleManager::get()->getBool("bugfix-dash-particles") && m_dashParticles) {
            m_dashParticles->setPosition(position);
        }
    }

    // 6. Show Trigger Wave Particles Fix
    $override void toggleVisibility(bool show) {
        bool willShowWave = show && m_isDart && m_isHidden && ModuleManager::get()->getBool("bugfix-show-trigger-particles");
        PlayerObject::toggleVisibility(show);
        if (willShowWave && m_trailingParticles) {
            m_trailingParticles->stopSystem();
        }
    }
};
