#include "Common.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace geode::prelude;

namespace {

// ---------------------------------------------------------------------------
// Layout metrics, authored against a 1920x1080 reference and scaled to the
// actual window so the menu keeps Eclipse's proportions on any resolution.
// ---------------------------------------------------------------------------
constexpr float REF_W      = 1920.f;
constexpr float COL_W      = 210.f;
constexpr float COL_GAP    = 6.f;
constexpr float HEAD_H     = 34.f;
constexpr float ROW_H      = 22.f;
constexpr float SEC_H      = 26.f;
constexpr float LEFT       = 8.f;
constexpr float DRAG_RANGE = 260.f; // pixels of drag for a full value sweep

inline ccColor4B panelCol()  { return ccc4(16, 16, 18, 236); }
inline ccColor4B sepCol()    { return ccc4(0, 0, 0, 135); }
inline ccColor4B headTop()   { return ccc4(200, 68, 255, 255); }
inline ccColor4B headBot()   { return ccc4(114, 22, 160, 255); }
inline ccColor4B secTop()    { return ccc4(172, 52, 228, 255); }
inline ccColor4B secBot()    { return ccc4(94, 18, 138, 255); }
inline ccColor4B boxBorder() { return ccc4(62, 62, 70, 255); }
inline ccColor4B boxFill()   { return ccc4(28, 28, 32, 255); }
inline ccColor4B accentCol() { return ccc4(0, 228, 96, 255); }
inline ccColor3B rowText()   { return ccc3(228, 228, 234); }
inline ccColor3B dimText()   { return ccc3(152, 152, 162); }
inline ccColor3B checkOff()  { return ccc3(196, 196, 204); }
inline ccColor3B checkOn()   { return ccc3(0, 228, 96); }

// ---------------------------------------------------------------------------
// Menu model
// ---------------------------------------------------------------------------

enum class Kind {
    Toggle,
    Float,
    Section
};

struct RowDef {
    Kind kind = Kind::Toggle;
    std::string key;
    std::string text;
    float minV = 0.f;
    float maxV = 1.f;
    std::string suffix;
};

struct ColumnDef {
    std::string title;
    std::vector<RowDef> rows;
};

RowDef mkToggle(char const* key, char const* text) {
    RowDef r;
    r.kind = Kind::Toggle;
    r.key = key;
    r.text = text;
    return r;
}

RowDef mkFloat(char const* key, char const* text, float lo, float hi, char const* suffix) {
    RowDef r;
    r.kind = Kind::Float;
    r.key = key;
    r.text = text;
    r.minV = lo;
    r.maxV = hi;
    r.suffix = suffix;
    return r;
}

std::vector<ColumnDef> buildColumns() {
    std::vector<ColumnDef> cols;

    cols.push_back({"Global", {
        mkFloat("speedhack-speed", "Speedhack Speed", 0.1f, 5.f, "x"),
        mkToggle("speedhack-enabled", "Speedhack"),
        mkToggle("noclip", "Noclip"),
        mkToggle("show-hitboxes", "Show Hitboxes"),
    }});

    cols.push_back({"Level", {
        mkToggle("instant-respawn", "Instant Respawn"),
        mkToggle("practice-music-sync", "Practice Music Sync"),
        mkToggle("startpos-switcher", "StartPos Switcher"),
    }});

    cols.push_back({"Fixes", {
        mkToggle("fix-practice-music-pulse", "Practice Music Pulse"),
        mkToggle("fix-dual-wave-trail", "Dual Wave Trail"),
        mkToggle("fix-dash-particles", "Dash Particles"),
        mkToggle("fix-trigger-wave-particles", "Trigger Wave Particles"),
        mkToggle("fix-startpos-mirror", "StartPos Mirror"),
        mkToggle("fix-mini-swing", "Mini Swing Frames"),
    }});

    cols.push_back({"Fun", {
        mkFloat("dvd-speed", "Bounce Speed", 0.2f, 3.f, "x"),
        mkToggle("dvd-enable", "DVD Mode"),
        mkToggle("dvd-no-triggers", "Skip Trigger Popups"),
        mkToggle("dvd-heavy", "Heavy Popups"),
    }});

    cols.push_back({"Interface", {
        mkFloat("ui-scale", "Menu Scale", 0.5f, 1.6f, "x"),
        mkFloat("ui-opacity", "Menu Opacity", 0.f, 1.f, ""),
    }});

    return cols;
}

// ---------------------------------------------------------------------------
// Window
// ---------------------------------------------------------------------------

class MegaWindow;

MegaWindow* g_window = nullptr;

class MegaWindow : public CCLayerColor {
public:
    struct RowUI {
        Kind kind = Kind::Toggle;
        std::string key;
        std::string suffix;
        float minV = 0.f;
        float maxV = 1.f;

        CCLayerColor* edgeBar = nullptr;
        CCSprite* check = nullptr;
        CCLabelBMFont* value = nullptr;
        CCLayerColor* knob = nullptr;

        // hit test rect, measured from the top-left of the screen trim
        float hitX = 0.f;
        float hitTop = 0.f;
        float hitW = 0.f;
        float hitH = 0.f;
    };

private:
    CCNode* m_root = nullptr;
    float m_scale = 1.f;
    float m_localH = 1080.f;
    std::vector<RowUI> m_rows;

    bool m_dragging = false;
    int m_dragRow = -1;
    float m_dragStartY = 0.f;
    float m_dragStartValue = 0.f;

public:
    static MegaWindow* create() {
        auto ret = new MegaWindow();
        if (ret) {
            auto ws = CCDirector::get()->getWinSize();
            if (ret->initWithColor(ccc4(0, 0, 0, 0), ws.width, ws.height)) {
                ret->autorelease();
                ret->build();
                return ret;
            }
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    void onEnter() override {
        // claim touches before any CCMenu underneath so a row click never
        // also presses a game button
        this->setTouchPriority(-999);
        this->setTouchEnabled(true);
        this->setKeypadEnabled(true);
        CCLayerColor::onEnter();
    }

    void onExit() override {
        this->setTouchEnabled(false);
        this->setKeypadEnabled(false);
        m_dragging = false;
        m_dragRow = -1;
        if (g_window == this) {
            g_window = nullptr;
        }
        CCLayerColor::onExit();
    }

    void keyBackClicked() override {
        ms::closeMenu();
    }

    // ------------------------------------------------------------- building

    void build() {
        auto ws = this->getContentSize();
        m_scale = (ws.width / REF_W) * ms::uiScale();
        if (m_scale <= 0.f) {
            m_scale = 1.f;
        }
        m_localH = ws.height / m_scale;

        m_root = CCNode::create();
        m_root->setAnchorPoint({0.f, 0.f});
        m_root->setPosition({0.f, 0.f});
        m_root->setScale(m_scale);
        this->addChild(m_root, 1);

        auto cols = buildColumns();

        for (size_t ci = 0; ci < cols.size(); ++ci) {
            float x = LEFT + static_cast<float>(ci) * (COL_W + COL_GAP);
            this->buildColumn(cols[ci], x);
        }
    }

    void buildColumn(ColumnDef const& def, float x) {
        float totalH = HEAD_H;
        for (auto const& r : def.rows) {
            totalH += (r.kind == Kind::Section) ? SEC_H : ROW_H;
        }

        auto col = CCNode::create();
        col->setContentSize({COL_W, totalH});
        col->setAnchorPoint({0.f, 1.f});
        col->setPosition({x, m_localH});
        m_root->addChild(col, 10);

        auto bg = CCLayerColor::create(panelCol(), COL_W, totalH);
        bg->setOpacity(static_cast<unsigned char>(236.f * ms::uiOpacity()));
        bg->setAnchorPoint({0.f, 0.f});
        bg->setPosition({0.f, 0.f});
        col->addChild(bg, 0);

        // header -----------------------------------------------------------
        auto head = CCLayerGradient::create(headTop(), headBot());
        if (head) {
            head->changeWidth(COL_W);
            head->changeHeight(HEAD_H);
            head->setAnchorPoint({0.f, 0.f});
            head->setPosition({0.f, totalH - HEAD_H});
            col->addChild(head, 2);
        }

        auto caret = CCLabelBMFont::create("v", "bigFont.fnt");
        if (caret) {
            caret->setAnchorPoint({0.5f, 0.5f});
            caret->setPosition({12.f, totalH - HEAD_H / 2.f});
            caret->setScale(0.26f);
            caret->setColor(ccc3(255, 255, 255));
            col->addChild(caret, 3);
        }

        auto title = CCLabelBMFont::create(def.title.c_str(), "bigFont.fnt");
        if (title) {
            title->setAnchorPoint({0.5f, 0.5f});
            title->setPosition({COL_W / 2.f, totalH - HEAD_H / 2.f});
            title->setScale(0.42f);
            title->setColor(ccc3(255, 255, 255));
            col->addChild(title, 3);
        }

        // rows -------------------------------------------------------------
        float off = HEAD_H;

        for (auto const& rd : def.rows) {
            if (rd.kind == Kind::Section) {
                auto bar = CCLayerGradient::create(secTop(), secBot());
                if (bar) {
                    bar->changeWidth(COL_W);
                    bar->changeHeight(SEC_H);
                    bar->setAnchorPoint({0.f, 0.f});
                    bar->setPosition({0.f, totalH - off - SEC_H});
                    col->addChild(bar, 2);
                }
                auto lbl = CCLabelBMFont::create(rd.text.c_str(), "bigFont.fnt");
                if (lbl) {
                    lbl->setAnchorPoint({0.f, 0.5f});
                    lbl->setPosition({8.f, totalH - off - SEC_H / 2.f});
                    lbl->setScale(0.34f);
                    lbl->setColor(ccc3(255, 255, 255));
                    col->addChild(lbl, 3);
                }
                off += SEC_H;
                continue;
            }

            float rowY = totalH - off - ROW_H;

            auto sep = CCLayerColor::create(sepCol(), COL_W, 1.f);
            sep->setAnchorPoint({0.f, 0.f});
            sep->setPosition({0.f, rowY});
            col->addChild(sep, 1);

            RowUI ui;
            ui.kind = rd.kind;
            ui.key = rd.key;
            ui.suffix = rd.suffix;
            ui.minV = rd.minV;
            ui.maxV = rd.maxV;
            ui.hitX = x;
            ui.hitTop = off;
            ui.hitW = COL_W;
            ui.hitH = ROW_H;

            auto lbl = CCLabelBMFont::create(rd.text.c_str(), "bigFont.fnt");
            if (lbl) {
                lbl->setAnchorPoint({0.f, 0.5f});
                lbl->setScale(0.36f);
                lbl->setColor(rowText());
                lbl->setPosition({30.f, rowY + ROW_H / 2.f});
                col->addChild(lbl, 4);
            }

            if (rd.kind == Kind::Toggle) {
                auto edge = CCLayerColor::create(accentCol(), 3.f, ROW_H);
                edge->setAnchorPoint({0.f, 0.f});
                edge->setPosition({0.f, rowY});
                edge->setOpacity(0);
                col->addChild(edge, 2);
                ui.edgeBar = edge;

                auto box = CCLayerColor::create(ccc4(58, 58, 64, 255), 15.f, 15.f);
                box->setAnchorPoint({0.f, 0.f});
                box->setPosition({9.f, rowY + (ROW_H - 15.f) / 2.f});
                col->addChild(box, 2);

                auto chk = CCSprite::createWithSpriteFrameName("GJ_checkOff_001.png");
                if (chk) {
                    auto cs = chk->getContentSize();
                    float k = (cs.width > 0.f) ? (15.f / cs.width) : 0.6f;
                    chk->setScale(k);
                    chk->setAnchorPoint({0.f, 0.f});
                    chk->setPosition({9.f, rowY + (ROW_H - 15.f) / 2.f});
                    chk->setColor(checkOff());
                    col->addChild(chk, 3);
                    ui.check = chk;
                }
            }
            else {
                auto outer = CCLayerColor::create(boxBorder(), 76.f, 17.f);
                outer->setAnchorPoint({0.f, 0.f});
                outer->setPosition({8.f, rowY + (ROW_H - 17.f) / 2.f});
                col->addChild(outer, 2);

                auto inner = CCLayerColor::create(boxFill(), 74.f, 15.f);
                inner->setAnchorPoint({0.f, 0.f});
                inner->setPosition({9.f, rowY + (ROW_H - 17.f) / 2.f + 1.f});
                col->addChild(inner, 3);

                auto val = CCLabelBMFont::create("", "bigFont.fnt");
                if (val) {
                    val->setAnchorPoint({0.5f, 0.5f});
                    val->setPosition({46.f, rowY + ROW_H / 2.f});
                    val->setScale(0.34f);
                    val->setColor(ccc3(255, 255, 255));
                    col->addChild(val, 4);
                    ui.value = val;
                }

                auto knob = CCLayerColor::create(accentCol(), 3.f, 4.f);
                if (knob) {
                    knob->setAnchorPoint({0.f, 0.f});
                    knob->setPosition({COL_W - 6.f, rowY + 4.f});
                    col->addChild(knob, 2);
                    ui.knob = knob;
                }

                // the label of a float row starts after the value box
                if (lbl) {
                    lbl->setPosition({92.f, rowY + ROW_H / 2.f});
                }
            }

            int index = static_cast<int>(m_rows.size());
            m_rows.push_back(ui);
            this->refreshRow(index);

            off += ROW_H;
        }
    }

    // ------------------------------------------------------------ updating

    void refreshRow(int i) {
        if (i < 0 || i >= static_cast<int>(m_rows.size())) {
            return;
        }
        auto& ui = m_rows[i];

        if (ui.kind == Kind::Toggle) {
            bool on = ms::enabled(ui.key);
            if (ui.edgeBar) {
                ui.edgeBar->setOpacity(on ? 255 : 0);
            }
            if (ui.check) {
                auto cache = CCSpriteFrameCache::sharedSpriteFrameCache();
                if (cache) {
                    if (auto frame = cache->spriteFrameByName(
                            on ? "GJ_checkOn_001.png" : "GJ_checkOff_001.png")) {
                        ui.check->setDisplayFrame(frame);
                    }
                }
                ui.check->setColor(on ? checkOn() : checkOff());
            }
        }
        else if (ui.kind == Kind::Float) {
            float v = ms::getFloat(ui.key, ui.minV);
            if (v < ui.minV) {
                v = ui.minV;
            }
            if (v > ui.maxV) {
                v = ui.maxV;
            }
            if (ui.value) {
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%.2f%s", v, ui.suffix.c_str());
                ui.value->setString(buf);
            }
            if (ui.knob) {
                float t = (ui.maxV > ui.minV) ? (v - ui.minV) / (ui.maxV - ui.minV) : 0.f;
                ui.knob->changeHeight(4.f + (ROW_H - 8.f) * t);
            }
        }
    }

    // -------------------------------------------------------------- inputs

    bool ccTouchBegan(CCTouch* touch, CCEvent* event) override {
        if (!m_root) {
            return false;
        }

        CCPoint local = m_root->convertToNodeSpace(touch->getLocation());
        float ty = m_localH - local.y;

        m_dragging = false;
        m_dragRow = -1;

        for (int i = static_cast<int>(m_rows.size()) - 1; i >= 0; --i) {
            auto const& ui = m_rows[i];
            if (local.x < ui.hitX || local.x > ui.hitX + ui.hitW) {
                continue;
            }
            if (ty < ui.hitTop || ty > ui.hitTop + ui.hitH) {
                continue;
            }

            if (ui.kind == Kind::Toggle) {
                ms::setEnabled(ui.key, !ms::enabled(ui.key));
                this->refreshRow(i);
                return true;
            }
            if (ui.kind == Kind::Float) {
                m_dragging = true;
                m_dragRow = i;
                m_dragStartY = local.y;
                m_dragStartValue = ms::getFloat(ui.key, ui.minV);
                return true;
            }
            return true;
        }

        // nothing hit: let the click fall through to the game
        return false;
    }

    void ccTouchMoved(CCTouch* touch, CCEvent* event) override {
        if (!m_dragging || m_dragRow < 0 || m_dragRow >= static_cast<int>(m_rows.size()) || !m_root) {
            return;
        }

        auto& ui = m_rows[m_dragRow];
        CCPoint local = m_root->convertToNodeSpace(touch->getLocation());

        float range = ui.maxV - ui.minV;
        float v = m_dragStartValue + (local.y - m_dragStartY) * (range / DRAG_RANGE);

        if (v < ui.minV) {
            v = ui.minV;
        }
        if (v > ui.maxV) {
            v = ui.maxV;
        }

        ms::setFloat(ui.key, v);
        this->refreshRow(m_dragRow);
    }

    void ccTouchEnded(CCTouch* touch, CCEvent* event) override {
        m_dragging = false;
        m_dragRow = -1;
    }

    void ccTouchCancelled(CCTouch* touch, CCEvent* event) override {
        m_dragging = false;
        m_dragRow = -1;
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

    // the close runs from inside a touch, deleting the window here would
    // leave cocos2d holding a dangling touch target
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
