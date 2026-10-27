#include "Components.hpp"

namespace megasuite::ui {

    // ==========================================
    // MegaSidebarItem
    // ==========================================
    MegaSidebarItem* MegaSidebarItem::create(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        auto ret = new MegaSidebarItem();
        if (ret && ret->init(title, target, callback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaSidebarItem::init(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        m_title = title;

        auto container = CCNode::create();
        container->setContentSize({theme::SidebarWidth - 8.0f, 32.0f});

        m_bgHighlight = CCLayerColor::create(ccc4(26, 26, 32, 255), theme::SidebarWidth - 8.0f, 32.0f);
        m_bgHighlight->setPosition({0, 0});
        m_bgHighlight->setVisible(false);
        container->addChild(m_bgHighlight);

        // Green left border stripe for active category (classic MegaHack sidebar)
        m_indicator = CCLayerColor::create(ccc4(0, 255, 128, 255), 3.0f, 32.0f);
        m_indicator->setPosition({0.0f, 0.0f});
        m_indicator->setVisible(false);
        container->addChild(m_indicator);

        m_label = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
        m_label->setScale(0.33f);
        m_label->setAnchorPoint({0.0f, 0.5f});
        m_label->setPosition({12.0f, 16.0f});
        m_label->setColor(theme::TextSecondary);
        container->addChild(m_label);

        if (!CCMenuItemSpriteExtra::init(container, nullptr, target, callback)) {
            return false;
        }
        setContentSize(container->getContentSize());
        return true;
    }

    void MegaSidebarItem::setActive(bool active) {
        m_active = active;
        if (m_label) {
            m_label->setColor(active ? theme::TextAccent : theme::TextSecondary);
        }
        if (m_indicator) {
            m_indicator->setVisible(active);
        }
        if (m_bgHighlight) {
            m_bgHighlight->setVisible(active);
        }
    }

    // ==========================================
    // MegaCheckboxRow (MegaHack style square checkbox row)
    // ==========================================
    MegaCheckboxRow* MegaCheckboxRow::create(const std::string& id, const std::string& name, const std::string& description,
                                            bool initialValue, std::function<void(bool)> onChanged) {
        auto ret = new MegaCheckboxRow();
        if (ret && ret->init(id, name, description, initialValue, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaCheckboxRow::init(const std::string& id, const std::string& name, const std::string& description,
                                bool initialValue, std::function<void(bool)> onChanged) {
        if (!CCNode::init()) return false;

        m_id = id;
        m_name = name;
        m_description = description;
        m_checked = initialValue;
        m_onChanged = onChanged;

        constexpr float rowW = 202.0f;
        constexpr float rowH = 34.0f;
        setContentSize({rowW, rowH});

        // Row background
        m_rowBg = CCLayerColor::create(ccc4(22, 22, 27, 240), rowW, rowH);
        m_rowBg->setPosition({0, 0});
        addChild(m_rowBg);

        // Thin bottom border separator
        auto bottomLine = CCLayerColor::create(ccc4(34, 34, 42, 255), rowW, 1.0f);
        bottomLine->setPosition({0, 0});
        addChild(bottomLine);

        // Checkbox square (16x16 px)
        constexpr float boxSize = 16.0f;
        constexpr float boxX = 8.0f;
        constexpr float boxY = (rowH - boxSize) / 2.0f;

        m_boxBorder = CCLayerColor::create(ccc4(65, 65, 78, 255), boxSize, boxSize);
        m_boxBorder->setPosition({boxX, boxY});
        addChild(m_boxBorder);

        m_boxBg = CCLayerColor::create(ccc4(28, 28, 34, 255), boxSize - 2.0f, boxSize - 2.0f);
        m_boxBg->setPosition({boxX + 1.0f, boxY + 1.0f});
        addChild(m_boxBg);

        // Green checkmark icon / tick
        m_checkmark = CCSprite::createWithSpriteFrameName("GJ_completesIcon_001.png");
        if (!m_checkmark) {
            m_checkmark = CCSprite::create();
        }
        m_checkmark->setScale(0.42f);
        m_checkmark->setColor(theme::AccentGreen);
        m_checkmark->setPosition({boxX + boxSize / 2.0f, boxY + boxSize / 2.0f});
        m_checkmark->setVisible(m_checked);
        addChild(m_checkmark);

        // Module Name
        m_nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        m_nameLabel->setScale(0.28f);
        m_nameLabel->setAnchorPoint({0.0f, 0.5f});
        m_nameLabel->setPosition({boxX + boxSize + 8.0f, rowH / 2.0f + 1.0f});
        m_nameLabel->setColor(m_checked ? theme::TextPrimary : theme::TextSecondary);
        addChild(m_nameLabel);

        // Clickable transparent menu item covering whole row
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu);

        auto hitSpr = CCNode::create();
        hitSpr->setContentSize({rowW, rowH});
        m_hitButton = CCMenuItemSpriteExtra::create(
            hitSpr, nullptr, this, menu_selector(MegaCheckboxRow::onToggle)
        );
        m_hitButton->setPosition({rowW / 2.0f, rowH / 2.0f});
        menu->addChild(m_hitButton);

        updateVisuals();
        return true;
    }

    void MegaCheckboxRow::onToggle(CCObject*) {
        m_checked = !m_checked;
        updateVisuals();
        if (m_onChanged) {
            m_onChanged(m_checked);
        }
    }

    void MegaCheckboxRow::updateVisuals() {
        if (m_checkmark) {
            m_checkmark->setVisible(m_checked);
        }
        if (m_boxBorder) {
            m_boxBorder->setColor(m_checked ? theme::AccentGreen : ccc3(65, 65, 78));
        }
        if (m_boxBg) {
            m_boxBg->setColor(m_checked ? theme::AccentGreenDim : ccc3(28, 28, 34));
        }
        if (m_nameLabel) {
            m_nameLabel->setColor(m_checked ? theme::TextPrimary : theme::TextMuted);
        }
        if (m_rowBg) {
            m_rowBg->setColor(m_checked ? ccc3(26, 29, 34) : ccc3(20, 20, 25));
        }
    }

    void MegaCheckboxRow::setValue(bool val) {
        if (m_checked != val) {
            m_checked = val;
            updateVisuals();
        }
    }

    // ==========================================
    // MegaSliderRow
    // ==========================================
    MegaSliderRow* MegaSliderRow::create(const std::string& id, const std::string& name, float min, float max,
                                        float initialValue, const std::string& format, std::function<void(float)> onChanged) {
        auto ret = new MegaSliderRow();
        if (ret && ret->init(id, name, min, max, initialValue, format, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaSliderRow::init(const std::string& id, const std::string& name, float min, float max,
                             float initialValue, const std::string& format, std::function<void(float)> onChanged) {
        if (!CCNode::init()) return false;

        m_id = id;
        m_name = name;
        m_min = min;
        m_max = max;
        m_value = initialValue;
        m_format = format;
        m_onChanged = onChanged;

        constexpr float rowW = 202.0f;
        constexpr float rowH = 42.0f;
        setContentSize({rowW, rowH});

        auto rowBg = CCLayerColor::create(ccc3(22, 22, 27), rowW, rowH);
        rowBg->setPosition({0, 0});
        addChild(rowBg);

        auto bottomLine = CCLayerColor::create(ccc3(34, 34, 42), rowW, 1.0f);
        bottomLine->setPosition({0, 0});
        addChild(bottomLine);

        m_nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        m_nameLabel->setScale(0.27f);
        m_nameLabel->setAnchorPoint({0.0f, 0.5f});
        m_nameLabel->setPosition({8.0f, 31.0f});
        m_nameLabel->setColor(theme::TextPrimary);
        addChild(m_nameLabel);

        char buf[32];
        snprintf(buf, sizeof(buf), m_format.c_str(), m_value);
        m_valLabel = CCLabelBMFont::create(buf, "chatFont.fnt");
        m_valLabel->setScale(0.55f);
        m_valLabel->setAnchorPoint({1.0f, 0.5f});
        m_valLabel->setPosition({rowW - 8.0f, 31.0f});
        m_valLabel->setColor(theme::TextAccent);
        addChild(m_valLabel);

        m_slider = Slider::create(this, menu_selector(MegaSliderRow::onSliderChanged), 0.52f);
        m_slider->setAnchorPoint({0.5f, 0.5f});
        m_slider->setPosition({rowW / 2.0f, 13.0f});

        float progress = (m_max > m_min) ? (m_value - m_min) / (m_max - m_min) : 0.0f;
        m_slider->setValue(progress);
        addChild(m_slider);

        return true;
    }

    void MegaSliderRow::onSliderChanged(CCObject*) {
        if (!m_slider) return;
        float progress = m_slider->getValue();
        m_value = m_min + progress * (m_max - m_min);

        if (m_valLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), m_format.c_str(), m_value);
            m_valLabel->setString(buf);
        }

        if (m_onChanged) {
            m_onChanged(m_value);
        }
    }

    void MegaSliderRow::setValue(float val) {
        m_value = val;
        if (m_slider && (m_max > m_min)) {
            m_slider->setValue((m_value - m_min) / (m_max - m_min));
        }
        if (m_valLabel) {
            char buf[32];
            snprintf(buf, sizeof(buf), m_format.c_str(), m_value);
            m_valLabel->setString(buf);
        }
    }

    // ==========================================
    // MegaHeader
    // ==========================================
    MegaHeader* MegaHeader::create(const std::string& title, const std::string& version,
                                  std::function<void()> onClose) {
        auto ret = new MegaHeader();
        if (ret && ret->init(title, version, onClose)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaHeader::init(const std::string& title, const std::string& version,
                          std::function<void()> onClose) {
        if (!CCNode::init()) return false;

        setContentSize({theme::WindowWidth, theme::HeaderHeight});

        auto bg = CCLayerColor::create(ccc4(14, 14, 17, 255), theme::WindowWidth, theme::HeaderHeight);
        bg->setPosition({0, 0});
        addChild(bg);

        // Top green accent bar
        auto topBar = CCLayerColor::create(ccc4(0, 255, 128, 255), theme::WindowWidth, 2.0f);
        topBar->setPosition({0, theme::HeaderHeight - 2.0f});
        addChild(topBar);

        // MegaHack styled title
        auto titleLabel = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
        titleLabel->setScale(0.42f);
        titleLabel->setAnchorPoint({0.0f, 0.5f});
        titleLabel->setPosition({12.0f, theme::HeaderHeight / 2.0f});
        titleLabel->setColor(theme::TextAccent);
        addChild(titleLabel);

        auto verLabel = CCLabelBMFont::create(version.c_str(), "chatFont.fnt");
        verLabel->setScale(0.5f);
        verLabel->setAnchorPoint({0.0f, 0.5f});
        verLabel->setPosition({titleLabel->getPositionX() + titleLabel->getContentWidth() * 0.42f + 8.0f, theme::HeaderHeight / 2.0f - 1.0f});
        verLabel->setColor(theme::TextMuted);
        addChild(verLabel);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu);

        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (closeSpr) {
            closeSpr->setScale(0.5f);
            auto closeBtn = CCMenuItemSpriteExtra::create(
                closeSpr, nullptr, this, nullptr
            );
            closeBtn->setPosition({theme::WindowWidth - 16.0f, theme::HeaderHeight / 2.0f});
            menu->addChild(closeBtn);
        }

        return true;
    }

} // namespace megasuite::ui
