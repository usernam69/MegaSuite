#include "Common.hpp"

#include <Geode/modify/FLAlertLayer.hpp>

#include <algorithm>
#include <cmath>
#include <cstdlib>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// DVD Mode - every popup bounces around the screen forever
// ---------------------------------------------------------------------------

class $modify(MSDVDLayer, FLAlertLayer) {
    struct Fields {
        short xVec = 1;
        short yVec = 1;
        CCRect bounds;
        float speed = 40.f;
        bool started = false;
    };

    bool isTriggerPopup() {
        if (typeinfo_cast<SetupTriggerPopup*>(this)) return true;
        if (typeinfo_cast<GJColorSetupLayer*>(this)) return true;
        if (typeinfo_cast<HSVWidgetPopup*>(this)) return true;
        if (typeinfo_cast<LevelSettingsLayer*>(this)) return true;
        if (typeinfo_cast<ColorSelectLiveOverlay*>(this)) return true;
        if (typeinfo_cast<HSVLiveOverlay*>(this)) return true;
        return false;
    }

    void show() {
        FLAlertLayer::show();

        if (!ms::enabled("dvd-enable")) {
            return;
        }

        auto f = m_fields.self();
        if (f->started) {
            return;
        }
        if (!m_mainLayer) {
            return;
        }
        if (ms::enabled("dvd-no-triggers") && this->isTriggerPopup()) {
            return;
        }

        auto bg = m_mainLayer->getChildByType<CCScale9Sprite>(0);
        if (!bg) {
            return;
        }

        auto winSize = CCDirector::get()->getWinSize();
        auto bgBox = bg->boundingBox();

        if (bgBox.size.width >= winSize.width || bgBox.size.height >= winSize.height) {
            return;
        }

        CCPoint shift = ccp(bgBox.getMidX(), bgBox.getMidY())
            - (m_mainLayer->getContentSize() / 2.f);

        f->bounds = CCRect(
            bgBox.size.width / 2.f - shift.x,
            bgBox.size.height / 2.f - shift.y,
            winSize.width - bgBox.size.width,
            winSize.height - bgBox.size.height
        );

        int r = rand();
        f->xVec = (r & 4) ? 1 : -1;
        f->yVec = (r & 2) ? 1 : -1;

        float mult = ms::dvdSpeedMult();
        float speed = 40.f * mult;
        if (ms::enabled("dvd-heavy")) {
            speed = (66.f - 0.0003f * (bgBox.size.width * bgBox.size.height)) * mult;
        }
        if (speed < 8.f) {
            speed = 8.f;
        }
        f->speed = speed;
        f->started = true;

        this->scheduleOnce(schedule_selector(MSDVDLayer::beginBounce), 0.2f);
    }

    void beginBounce(float) {
        this->bounce();
    }

    void bounce() {
        auto f = m_fields.self();
        if (!f->started || !m_mainLayer) {
            return;
        }

        CCPoint pos = m_mainLayer->convertToWorldSpace(
            m_mainLayer->getContentSize() / 2.f
        );

        if (pos.x > f->bounds.getMaxX() - 0.5f || pos.x < f->bounds.getMinX() + 0.5f) {
            f->xVec *= -1;
        }
        if (pos.y > f->bounds.getMaxY() - 0.5f || pos.y < f->bounds.getMinY() + 0.5f) {
            f->yVec *= -1;
        }

        float xTarget = (f->xVec == 1) ? f->bounds.getMaxX() : f->bounds.getMinX();
        float yTarget = (f->yVec == 1) ? f->bounds.getMaxY() : f->bounds.getMinY();

        float dist = std::min(
            std::fabs(pos.x - xTarget),
            std::fabs(pos.y - yTarget)
        );
        if (dist < 1.f) {
            dist = 1.f;
        }

        float delay = dist / f->speed;
        if (delay < 0.02f) {
            delay = 0.02f;
        }

        m_mainLayer->runAction(CCSequence::createWithTwoActions(
            CCMoveBy::create(delay, ccp((float)f->xVec, (float)f->yVec) * dist),
            CCCallFunc::create(this, callfunc_selector(MSDVDLayer::bounce))
        ));
    }
};
