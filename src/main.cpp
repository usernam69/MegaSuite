#include "Common.hpp"
#include "modules/ModuleManager.hpp"
#include "modules/QOLModules.hpp"
#include "modules/DVDModeModule.hpp"
#include "modules/BugfixModules.hpp"
#include "modules/APIModules.hpp"
#include "ui/MegaHackWindow.hpp"

#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/CCKeyboardDispatcher.hpp>

using namespace geode::prelude;
using namespace megasuite;
using namespace megasuite::ui;

// Hook Keyboard Dispatcher to toggle the MegaHack menu via Tab key
class $modify(MegaKeyboardDispatcher, CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool isKeyRepeat) {
        if (down && !isKeyRepeat) {
            // Default keybind: Tab (enumKeyCodes::KEY_Tab is 0x09)
            if (key == enumKeyCodes::KEY_Tab) {
                MegaHackWindow::toggle();
                return true;
            }
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, isKeyRepeat);
    }
};

// Hook MenuLayer to add a quick-launch MegaHack button on the main title screen
class $modify(MegaMenuLayer, MenuLayer) {
    bool init() {
        if (!MenuLayer::init()) return false;

        auto bottomMenu = getChildByID("bottom-menu");
        if (!bottomMenu) {
            bottomMenu = getChildByType<CCMenu>(0);
        }

        if (bottomMenu) {
            auto spr = CircleButtonSprite::createWithSpriteFrameName(
                "geode.loader/geode-logo.png",
                0.85f,
                CircleBaseColor::Green,
                CircleBaseSize::Medium
            );
            if (!spr) {
                spr = CircleButtonSprite::create(
                    CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png"),
                    CircleBaseColor::Green,
                    CircleBaseSize::Medium
                );
            }

            auto btn = CCMenuItemSpriteExtra::create(
                spr,
                this,
                menu_selector(MegaMenuLayer::onMegaHackBtn)
            );
            btn->setID("megasuite-button"_spr);
            bottomMenu->addChild(btn);
            bottomMenu->updateLayout();
        }

        return true;
    }

    void onMegaHackBtn(CCObject*) {
        MegaHackWindow::toggle();
    }
};

$on_mod(Loaded) {
    log::info("MegaSuite initializing...");

    // Initialize all modular components
    megasuite::qol::initQOLModules();
    megasuite::dvd::initDVDMode();
    megasuite::bugfixes::initBugfixes();
    megasuite::api::initAPIs();

    // Register General Settings
    auto mgr = ModuleManager::get();
    mgr->registerFloat("ui-scale", "Menu Scale",
                       "Scale multiplier of the MegaHack UI window",
                       Category::Settings, 1.0f, 0.6f, 1.5f, "%.2fx", nullptr);
    mgr->registerFloat("ui-opacity", "Window Opacity",
                       "Acrylic glass background opacity",
                       Category::Settings, 0.94f, 0.4f, 1.0f, "%.0f%%", nullptr);

    log::info("MegaSuite successfully loaded with QOLMod, DVD Mode, Bugfixes, and Developer APIs!");
}
