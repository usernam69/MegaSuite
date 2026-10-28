#include "Common.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/ui/BasedButtonSprite.hpp>

using namespace geode::prelude;

// ---------------------------------------------------------------------------
// Main menu shortcut + TAB / ESC
// ---------------------------------------------------------------------------

class $modify(MSMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        CCNode* top = nullptr;
        if (cache && cache->spriteFrameByName("GJ_optionsBtn_001.png")) {
            top = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
        }
        if (!top) {
            top = CCLabelBMFont::create("MS", "bigFont.fnt");
        }

        auto spr = CircleButtonSprite::create(
            top, CircleBaseColor::Green, CircleBaseSize::Medium
        );
        if (!spr) {
            return true;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            spr, this, menu_selector(MSMenuLayer::onMegaSuite)
        );
        if (!btn) {
            return true;
        }
        btn->setID("megasuite-button"_spr);

        if (auto menu = this->getChildByID("bottom-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        else if (auto menu = this->getChildByID("main-menu")) {
            menu->addChild(btn);
            menu->updateLayout();
        }
        else {
            auto fallback = CCMenu::create();
            fallback->setPosition({30.f, 30.f});
            fallback->addChild(btn);
            this->addChild(fallback, 100);
        }

        return true;
    }

    void keyDown(enumKeyCodes key, double timestamp) {
        if (key == KEY_Tab) {
            ms::toggleMenu();
            return;
        }
        if (key == KEY_Escape && ms::isMenuOpen()) {
            ms::closeMenu();
            return;
        }
        MenuLayer::keyDown(key, timestamp);
    }

    void onMegaSuite(CCObject*) {
        ms::toggleMenu();
    }
};
