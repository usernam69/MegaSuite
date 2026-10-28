#include "Common.hpp"

#include <Geode/ui/SliderNode.hpp>

#include <algorithm>
#include <vector>

using namespace geode::prelude;

namespace {

constexpr float WIN_W = 400.f;
constexpr float WIN_H = 260.f;
constexpr float HEADER_H = 26.f;

constexpr float CONTENT_W = 282.f;
constexpr float CONTENT_H = 222.f;
constexpr float ROW_W = 272.f;

struct RowDef {
    std::string key;
    std::string name;
    bool isFloat = false;
    float minV = 0.f;
    float maxV = 1.f;
    std::string suffix;
};

std::vector<std::string> tabNames() {
    return {"QoL", "DVD", "Bugfixes", "Config"};
}

std::vector<RowDef> rowsFor(int tab) {
    switch (tab) {
        case 0:
            return {
                {"speedhack-speed", "Speedhack Speed", true, 0.1f, 5.f, "x"},
                {"speedhack-enabled", "Speedhack"},
                {"noclip", "Noclip"},
                {"show-hitboxes", "Show Hitboxes"},
                {"instant-respawn", "Instant Respawn"},
                {"practice-music-sync", "Practice Music Sync"},
                {"startpos-switcher", "StartPos Switcher (Arrows)"},
            };
        case 1:
            return {
                {"dvd-speed", "Bounce Speed", true, 0.2f, 3.f, "x"},
                {"dvd-enable", "DVD Mode"},
                {"dvd-no-triggers", "Skip Trigger Popups"},
                {"dvd-heavy", "Heavy Popups"},
            };
        case 2:
            return {
                {"fix-practice-music-pulse", "Practice Music Pulse"},
                {"fix-dual-wave-trail", "Dual Wave Trail (P2)"},
                {"fix-dash-particles", "Dash Particles Offset"},
                {"fix-trigger-wave-particles", "Trigger Wave Particles"},
                {"fix-startpos-mirror", "StartPos Mirror Mode"},
                {"fix-mini-swing", "Mini Swing Frames"},
            };
        default:
            return {
                {"ui-scale", "Menu Scale", true, 0.7f, 1.4f, "x"},
                {"ui-opacity", "Background Opacity", true, 0.4f, 1.f, ""},
            };
    }
}

class MegaWindow;

MegaWindow* g_window = nullptr;

class MegaWindow : public CCLayerColor {
protected:
    CCNode* m_root = nullptr;
    CCNode* m_content = nullptr;
    CCScale9Sprite* m_bg = nullptr;
    CCMenu* m_mainMenu = nullptr;
    std::vector<CCLabelBMFont*> m_tabLabels;

    int m_tab = 0;
    std::vector<RowDef> m_rows;
    std::vector<CCSprite*> m_rowChecks;
    std::vector<CCLabelBMFont*> m_rowValues;

    bool m_dragging = false;
    CCPoint m_dragOffset = CCPointZero;

public:
    static MegaWindow* create() {
        auto ret = new MegaWindow();
        if (ret) {
            auto ws = CCDirector::get()->getWinSize();
            if (ret->initWithColor(ccc4(0, 0, 0, 170), ws.width, ws.height)) {
                ret->autorelease();
                ret->build();
                return ret;
            }
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onEnter() override {
        CCLayerColor::onEnter();
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
    }

    void onExit() override {
        this->setTouchEnabled(false);
        m_dragging = false;
        if (g_window == this) {
            g_window = nullptr;
        }
        CCLayerColor::onExit();
    }

    void build() {
        auto ws = this->getContentSize();

        m_root = CCNode::create();
        m_root->setContentSize({WIN_W, WIN_H});
        m_root->setAnchorPoint({0.5f, 0.5f});
        m_root->setPosition({ws.width / 2.f, ws.height / 2.f});
        m_root->setScale(ms::uiScale());
        this->addChild(m_root, 1);

        m_bg = CCScale9Sprite::create("GJ_square01.png");
        if (m_bg) {
            m_bg->setContentSize({WIN_W, WIN_H});
            m_bg->setAnchorPoint({0.f, 0.f});
            m_bg->setPosition({0.f, 0.f});
            m_bg->setColor(ms::colBgTint());
            m_root->addChild(m_bg, 0);
            this->applyOpacity();
        }

        auto sidebar = CCLayerColor::create(ms::colSidebar(), 96.f, WIN_H - 38.f);
        sidebar->setPosition({8.f, 8.f});
        m_root->addChild(sidebar, 2);

        m_content = CCLayerColor::create(ms::colContent(), CONTENT_W, CONTENT_H);
        m_content->setPosition({110.f, 8.f});
        m_root->addChild(m_content, 3);

        auto header = CCLayerColor::create(ms::colHeader(), WIN_W, HEADER_H);
        header->setPosition({0.f, WIN_H - HEADER_H});
        m_root->addChild(header, 5);

        auto title = CCLabelBMFont::create("MEGASUITE", "goldFont.fnt");
        title->setAnchorPoint({0.f, 0.5f});
        title->setPosition({10.f, WIN_H - HEADER_H / 2.f});
        title->setScale(0.55f);
        title->setColor(ms::colAccent());
        m_root->addChild(title, 6);

        auto sub = CCLabelBMFont::create("v1.0", "bigFont.fnt");
        sub->setAnchorPoint({1.f, 0.5f});
        sub->setPosition({WIN_W - 32.f, WIN_H - HEADER_H / 2.f});
        sub->setScale(0.35f);
        sub->setColor(ms::colTextDim());
        m_root->addChild(sub, 6);

        auto footer = CCLabelBMFont::create("TAB TO CLOSE", "bigFont.fnt");
        footer->setAnchorPoint({0.5f, 0.5f});
        footer->setPosition({56.f, 18.f});
        footer->setScale(0.28f);
        footer->setColor(ms::colTextDim());
        m_root->addChild(footer, 4);

        m_mainMenu = CCMenu::create();
        m_mainMenu->setPosition({0.f, 0.f});
        m_root->addChild(m_mainMenu, 20);

        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (!closeSpr) {
            closeSpr = CCSprite::create();
        }
        closeSpr->setScale(0.7f);
        auto closeBtn = CCMenuItemSpriteExtra::create(
            closeSpr, this, menu_selector(MegaWindow::onClose)
        );
        closeBtn->setPosition({WIN_W - 16.f, WIN_H - HEADER_H / 2.f});
        m_mainMenu->addChild(closeBtn);

        auto names = tabNames();
        for (int i = 0; i < (int)names.size(); i++) {
            auto holder = CCNode::create();
            holder->setContentSize({88.f, 22.f});

            auto lbl = CCLabelBMFont::create(names[i].c_str(), "bigFont.fnt");
            lbl->setAnchorPoint({0.f, 0.5f});
            lbl->setPosition({10.f, 11.f});
            lbl->setScale(0.45f);
            holder->addChild(lbl, 1);
            m_tabLabels.push_back(lbl);

            auto item = CCMenuItemSpriteExtra::create(
                holder, this, menu_selector(MegaWindow::onTab)
            );
            item->setTag(i);
            item->setPosition({56.f, 206.f - i * 26.f});
            m_mainMenu->addChild(item);
        }

        this->buildRows();
        this->updateTabColors();
    }

    void applyScale() {
        if (m_root) {
            m_root->setScale(ms::uiScale());
        }
    }

    void applyOpacity() {
        if (m_bg) {
            m_bg->setOpacity(static_cast<GLubyte>(255.f * ms::uiOpacity()));
        }
    }

    void updateTabColors() {
        for (int i = 0; i < (int)m_tabLabels.size(); i++) {
            m_tabLabels[i]->setColor(i == m_tab ? ms::colAccent() : ms::colTextDim());
        }
    }

    CCSprite* makeCheck(bool on) {
        auto spr = CCSprite::createWithSpriteFrameName(
            on ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png"
        );
        if (!spr) {
            spr = CCSprite::create();
        }
        spr->setScale(0.7f);
        return spr;
    }

    void setCheck(CCSprite* spr, bool on) {
        if (!spr) {
            return;
        }
        auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
        if (!cache) {
            return;
        }
        if (auto frame = cache->spriteFrameByName(
                on ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png")) {
            spr->setDisplayFrame(frame);
        }
    }

    void refreshRow(int i) {
        if (i < 0 || i >= (int)m_rows.size()) {
            return;
        }
        auto const& row = m_rows[i];
        if (!row.isFloat && i < (int)m_rowChecks.size()) {
            this->setCheck(m_rowChecks[i], ms::enabled(row.key));
        }
    }

    void buildRows() {
        if (!m_content) {
            return;
        }

        m_rows = rowsFor(m_tab);
        m_content->removeAllChildren();

        m_rowChecks.clear();
        m_rowValues.clear();

        auto menu = CCMenu::create();
        menu->setPosition({0.f, 0.f});
        m_content->addChild(menu, 2);

        float cursor = CONTENT_H - 8.f;
        const float cx = CONTENT_W / 2.f - 4.f;

        for (int i = 0; i < (int)m_rows.size(); i++) {
            auto const& row = m_rows[i];
            float h = row.isFloat ? 48.f : 26.f;
            float y = cursor - h;
            cursor = y;

            auto lbl = CCLabelBMFont::create(row.name.c_str(), "bigFont.fnt");
            lbl->setAnchorPoint({0.f, 0.5f});
            lbl->setScale(0.42f);
            lbl->setColor(ms::colText());
            lbl->setPosition({30.f, y + (row.isFloat ? h - 12.f : h / 2.f)});
            m_content->addChild(lbl, 1);

            if (!row.isFloat) {
                auto holder = CCNode::create();
                holder->setContentSize({ROW_W, h});

                auto check = this->makeCheck(ms::enabled(row.key));
                check->setPosition({13.f, h / 2.f});
                holder->addChild(check, 1);

                auto item = CCMenuItemSpriteExtra::create(
                    holder, this, menu_selector(MegaWindow::onRow)
                );
                item->setTag(i);
                item->setPosition({cx, y + h / 2.f});
                menu->addChild(item);

                m_rowChecks.push_back(check);
                m_rowValues.push_back(nullptr);
            } else {
                auto val = CCLabelBMFont::create("", "bigFont.fnt");
                val->setAnchorPoint({1.f, 0.5f});
                val->setPosition({CONTENT_W - 12.f, y + h - 12.f});
                val->setScale(0.4f);
                val->setColor(ms::colAccent());
                m_content->addChild(val, 1);
                m_rowValues.push_back(val);
                m_rowChecks.push_back(nullptr);

                float cur = ms::getFloat(row.key, row.minV);
                if (cur < row.minV) cur = row.minV;
                if (cur > row.maxV) cur = row.maxV;
                val->setString(fmt::format("{:.2f}{}", cur, row.suffix).c_str());

                auto slider = SliderNode::create([this, i](SliderNode*, float value) {
                    if (i >= (int)m_rows.size()) {
                        return;
                    }
                    auto const& r = m_rows[i];
                    ms::setFloat(r.key, value);
                    if (i < (int)m_rowValues.size() && m_rowValues[i]) {
                        m_rowValues[i]->setString(
                            fmt::format("{:.2f}{}", value, r.suffix).c_str()
                        );
                    }
                    if (r.key == "ui-scale") {
                        this->applyScale();
                    }
                    else if (r.key == "ui-opacity") {
                        this->applyOpacity();
                    }
                });
                if (slider) {
                    slider->setMin(row.minV);
                    slider->setMax(row.maxV);
                    slider->setValue(cur);
                    slider->setContentSize({240.f, 20.f});
                    slider->setPosition({cx, y + 13.f});
                    m_content->addChild(slider, 3);
                }
            }
        }
    }

    void onTab(CCObject* sender) {
        int idx = static_cast<CCNode*>(sender)->getTag();
        if (idx == m_tab) {
            return;
        }
        m_tab = idx;
        this->updateTabColors();
        this->buildRows();
    }

    void onRow(CCObject* sender) {
        int idx = static_cast<CCNode*>(sender)->getTag();
        if (idx < 0 || idx >= (int)m_rows.size()) {
            return;
        }
        auto const& row = m_rows[idx];
        if (row.isFloat) {
            return;
        }
        bool next = !ms::enabled(row.key);
        ms::setEnabled(row.key, next);
        this->refreshRow(idx);
    }

    void onClose(CCObject*) {
        ms::closeMenu();
    }

    // ------------------------------------------------------------- inputs
    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!m_root) {
            return true;
        }
        auto local = this->convertToNodeSpace(touch->getLocation());
        auto box = m_root->boundingBox();
        if (box.containsPoint(local)) {
            if (local.y > box.getMaxY() - HEADER_H * m_root->getScale()) {
                m_dragging = true;
                m_dragOffset = ccp(
                    local.x - m_root->getPositionX(),
                    local.y - m_root->getPositionY()
                );
            }
        }
        return true;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (!m_dragging || !m_root) {
            return;
        }
        auto local = this->convertToNodeSpace(touch->getLocation());
        auto ws = this->getContentSize();
        float scale = m_root->getScale();
        float halfW = (WIN_W * scale) / 2.f;
        float halfH = (WIN_H * scale) / 2.f;

        float x = local.x - m_dragOffset.x;
        float y = local.y - m_dragOffset.y;

        if (x < halfW) x = halfW;
        if (x > ws.width - halfW) x = ws.width - halfW;
        if (y < halfH) y = halfH;
        if (y > ws.height - halfH) y = ws.height - halfH;

        m_root->setPosition({x, y});
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        m_dragging = false;
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        m_dragging = false;
    }
};

} // namespace

namespace ms {

bool isMenuOpen() {
    return g_window != nullptr;
}

void openMenu() {
    if (g_window) {
        return;
    }
    auto scene = CCDirector::get()->getRunningScene();
    if (!scene) {
        return;
    }
    auto win = MegaWindow::create();
    if (!win) {
        return;
    }
    g_window = win;
    scene->addChild(win, 999999);
}

void closeMenu() {
    if (!g_window) {
        return;
    }

    auto win = g_window;
    g_window = nullptr;

    // the close button callback runs inside a menu item touch, deleting the
    // window right here would leave cocos2d with a dangling touch target
    win->retain();
    geode::queueInMainThread([win]() {
        if (win->getParent()) {
            win->removeFromParentAndCleanup(true);
        }
        win->release();
    });
}

void toggleMenu() {
    if (isMenuOpen()) {
        closeMenu();
    }
    else {
        openMenu();
    }
}

} // namespace ms
