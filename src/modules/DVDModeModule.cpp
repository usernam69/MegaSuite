#include "DVDModeModule.hpp"
#include "ModuleManager.hpp"

#include <Geode/modify/FLAlertLayer.hpp>
#include <Geode/modify/HSVLiveOverlay.hpp>
#include <Geode/modify/LevelLeaderboard.hpp>
#include <Geode/modify/CCTouchDispatcher.hpp>

using namespace geode::prelude;

#define BOUNCE_ID "megasuite-bouncing"_spr
#define NO_BOUNCE_ID "megasuite-no-bounce"_spr
#define BG_REPLACER_ID "megasuite-bg-replacer"_spr
#define BOUNCE_PRESET_ID "megasuite-bounce-preset"_spr
#define area(ccSize) (ccSize.width * ccSize.height)

namespace megasuite::dvd {

    struct BouncePreset : CCObject {
        short m_xVec, m_yVec;
        CCPoint m_pos;
        BouncePreset(short xVec, short yVec, CCPoint pos): m_xVec(xVec), m_yVec(yVec), m_pos(pos) {
            this->autorelease();
        }
    };

    void findPage(const std::string& id, int zOrd, short xVec, short yVec, CCPoint pos);

    void initDVDMode() {
        auto mgr = ModuleManager::get();
        mgr->registerBool("dvd-enable", "DVD Screensaver Popups",
                          "Makes popups bounce across the screen edges like a DVD screensaver",
                          Category::DVDMode, false, nullptr);
        mgr->registerFloat("dvd-speed", "Bounce Velocity",
                           "Speed multiplier for the bouncing animation",
                           Category::DVDMode, 1.0f, 0.2f, 5.0f, "%.1fx", nullptr);
        mgr->registerBool("dvd-no-triggers", "Exclude Editor Triggers",
                          "Disables bouncing on trigger and color setup dialogs in the editor",
                          Category::DVDMode, true, nullptr);
        mgr->registerBool("dvd-heavy", "Weight Scaling",
                          "Larger popups move slightly slower while smaller dialogs move faster",
                          Category::DVDMode, true, nullptr);
    }

} // namespace megasuite::dvd

using namespace megasuite;
using namespace megasuite::dvd;

class $modify(MegaDVDAlertLayer, FLAlertLayer) {
    struct Fields {
        short xVector = 1;
        short yVector = 1;
        CCRect bouncingRect;
        float speed = 40.0f;
        bool actionCreated = false;
        bool fixDropDownAnimation = false;
    };

    inline void scheduleBounceStart(float wait = 0) {
        scheduleOnce(schedule_selector(MegaDVDAlertLayer::createBounceAction), wait);
    }

    void createBounceAction(float) {
        auto f = m_fields.self();
        if (!ModuleManager::get()->getBool("dvd-enable")) return;
        if (f->actionCreated) return;
        if (!getParent()) return;
        if (getUserObject(NO_BOUNCE_ID)) return;

        for (CCNode* node = this; node != nullptr; node = node->getParent()) {
            if (node->getUserObject(BOUNCE_ID)) return;
        }

        f->actionCreated = true;

        if (ModuleManager::get()->getBool("dvd-no-triggers")) {
            if (typeinfo_cast<SetupTriggerPopup*>(this)) return;
            if (typeinfo_cast<GJColorSetupLayer*>(this)) return;
            if (typeinfo_cast<HSVWidgetPopup*>(this)) return;
            if (typeinfo_cast<LevelSettingsLayer*>(this)) return;
            if (typeinfo_cast<ColorSelectLiveOverlay*>(this)) return;
            if (typeinfo_cast<HSVLiveOverlay*>(this)) return;
            if (getID() == "SetGroupIDLayer") return;
            if (getID() == "CustomizeObjectLayer") return;
        }

        if (!m_mainLayer) return;
        CCNode* bg = m_mainLayer->getChildByType<CCScale9Sprite>(0);
        if (!bg) bg = m_mainLayer->getChildByType<NineSlice>(0);
        if (!bg) bg = m_mainLayer->getChildByID(BG_REPLACER_ID);
        if (!bg) bg = m_mainLayer->getChildByType<CCSprite>(0);
        if (!bg) return;

        bool usePresetValues = false;
        if (auto obj = static_cast<BouncePreset*>(getUserObject(BOUNCE_PRESET_ID))) {
            f->xVector = obj->m_xVec;
            f->yVector = obj->m_yVec;
            m_mainLayer->setPosition(obj->m_pos);
            usePresetValues = true;
        }

        if (typeinfo_cast<CommunityCreditsPage*>(this)) {
            f->fixDropDownAnimation = true;
        }

        const auto winSz = CCDirector::get()->getWinSize();
        const auto bgBox = bg->boundingBox();

        if (bgBox.size.width >= winSz.width || bgBox.size.height >= winSz.height) return;

        const CCPoint shift = ccp(bgBox.getMidX(), bgBox.getMidY()) - (m_mainLayer->getContentSize() / 2);

        f->bouncingRect = CCRect(
            bgBox.size.width / 2 - shift.x,
            bgBox.size.height / 2 - shift.y,
            winSz.width - bgBox.size.width,
            winSz.height - bgBox.size.height
        );

        if (!usePresetValues) {
            int r = rand();
            f->xVector = (r & 4) ? 1 : -1;
            f->yVector = (r & 2) ? 1 : -1;
        }

        float speedMult = ModuleManager::get()->getFloat("dvd-speed");
        if (speedMult <= 0.05f) speedMult = 1.0f;

        if (ModuleManager::get()->getBool("dvd-heavy")) {
            f->speed = (66.0f - 0.0003f * area(bgBox.size)) * speedMult;
        } else {
            f->speed = speedMult * 40.0f;
        }

        for (auto* child : CCArrayExt<CCNode*>(getChildren())) {
            if (typeinfo_cast<FLAlertLayer*>(child)) {
                continue;
            }
            child->setUserObject(BOUNCE_ID, CCBool::create(true));
        }

        if (usePresetValues) {
            bounce();
        } else {
            runAction(CCSequence::createWithTwoActions(
                CCDelayTime::create(0.05f),
                CCCallFunc::create(this, callfunc_selector(MegaDVDAlertLayer::bounce))
            ));
        }
    }

    void bounce() {
        const auto f = m_fields.self();

        CCPoint pos;
        if (f->fixDropDownAnimation) {
            auto oldPoint = m_mainLayer->getPosition();
            m_mainLayer->setPosition({0, 0});
            pos = m_mainLayer->convertToWorldSpace(m_mainLayer->getContentSize() / 2);
            m_mainLayer->setPosition(oldPoint);
            f->fixDropDownAnimation = false;
        } else {
            pos = m_mainLayer->convertToWorldSpace(m_mainLayer->getContentSize() / 2);
        }

        if (pos.x > f->bouncingRect.getMaxX() - 0.5f || pos.x < f->bouncingRect.getMinX() + 0.5f) {
            f->xVector *= -1;
        }
        if (pos.y > f->bouncingRect.getMaxY() - 0.5f || pos.y < f->bouncingRect.getMinY() + 0.5f) {
            f->yVector *= -1;
        }

        const float xTarget = (f->xVector == 1) ? f->bouncingRect.getMaxX() : f->bouncingRect.getMinX();
        const float yTarget = (f->yVector == 1) ? f->bouncingRect.getMaxY() : f->bouncingRect.getMinY();
        const float xDist = std::abs(pos.x - xTarget);
        const float yDist = std::abs(pos.y - yTarget);
        const float dist = std::min(xDist, yDist);
        const float delay = (f->speed > 0) ? (dist / f->speed) : 1.0f;

        bool flag = true;
        for (auto* child : CCArrayExt<CCNode*>(getChildren())) {
            if (!child->getUserObject(BOUNCE_ID)) continue;
            if (flag) {
                child->runAction(CCSequence::createWithTwoActions(
                    CCMoveBy::create(delay, ccp(f->xVector, f->yVector) * dist),
                    CCCallFunc::create(this, callfunc_selector(MegaDVDAlertLayer::bounce))
                ));
                flag = false;
            } else {
                child->runAction(CCMoveBy::create(delay, ccp(f->xVector, f->yVector) * dist));
            }
        }
    }
};

class $modify(MegaDVDLevelLeaderboard, LevelLeaderboard) {
    void reloadLeaderboard(LevelLeaderboardType type, LevelLeaderboardMode mode) {
        short xVec = reinterpret_cast<MegaDVDAlertLayer*>(this)->m_fields->xVector;
        short yVec = reinterpret_cast<MegaDVDAlertLayer*>(this)->m_fields->yVector;
        auto pos = m_mainLayer ? m_mainLayer->getPosition() : CCPoint{0, 0};
        LevelLeaderboard::reloadLeaderboard(type, mode);
        if (auto newPopup = CCScene::get()->getChildByType<LevelLeaderboard>(0)) {
            newPopup->setUserObject(BOUNCE_PRESET_ID, new BouncePreset(xVec, yVec, pos));
            if (newPopup->m_mainLayer) {
                newPopup->m_mainLayer->setPosition(pos);
            }
        }
    }
};

class $modify(MegaDVDTouchDispatcher, CCTouchDispatcher) {
    void registerForcePrio(CCObject* p0, int p1) {
        CCTouchDispatcher::registerForcePrio(p0, p1);
        if (auto popup = typeinfo_cast<FLAlertLayer*>(p0)) {
            reinterpret_cast<MegaDVDAlertLayer*>(popup)->scheduleBounceStart();
        }
    }
};
