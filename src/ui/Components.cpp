#include "Components.hpp"
#include <sstream>
#include <iomanip>

namespace megasuite::ui {

    // ==========================================
    // MegaTabButton
    // ==========================================
    MegaTabButton* MegaTabButton::create(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        auto ret = new MegaTabButton();
        if (ret && ret->init(title, target, callback)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaTabButton::init(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback) {
        m_title = title;

        auto bg = CCNode::create();
        bg->setContentSize({90.0f, theme::TabBarHeight});

        m_label = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
        m_label->setScale(0.35f);
        m_label->setColor(theme::TextSecondary);
        m_label->setPosition({45.0f, theme::TabBarHeight / 2.0f + 2.0f});
        bg->addChild(m_label);

        m_indicator = CCLayerColor::create(ccc4(0, 255, 127, 255), 70.0f, 2.5f);
        m_indicator->setPosition({10.0f, 0.0f});
        m_indicator->setVisible(false);
        bg->addChild(m_indicator);

        if (!CCMenuItemSpriteExtra::init(bg, nullptr, target, callback)) {
            return false;
        }
        setContentSize(bg->getContentSize());
        return true;
    }

    void MegaTabButton::setActive(bool active) {
        m_active = active;
        if (m_label) {
            m_label->setColor(active ? theme::TextAccent : theme::TextSecondary);
            m_label->setScale(active ? 0.38f : 0.35f);
        }
        if (m_indicator) {
            m_indicator->setVisible(active);
        }
    }

    // ==========================================
    // MegaToggle
    // ==========================================
    MegaToggle* MegaToggle::create(const std::string& id, const std::string& name, const std::string& description,
                                  bool initialValue, std::function<void(bool)> onChanged) {
        auto ret = new MegaToggle();
        if (ret && ret->init(id, name, description, initialValue, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaToggle::init(const std::string& id, const std::string& name, const std::string& description,
                          bool initialValue, std::function<void(bool)> onChanged) {
        if (!CCNode::init()) return false;

        m_id = id;
        m_name = name;
        m_description = description;
        m_value = initialValue;
        m_onChanged = onChanged;

        setContentSize({250.0f, 38.0f});

        // Background card
        auto cardBg = CCLayerColor::create(ccc4(30, 32, 42, 220), 250.0f, 38.0f);
        cardBg->setPosition({0, 0});
        addChild(cardBg);

        // Labels
        m_nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        m_nameLabel->setScale(0.32f);
        m_nameLabel->setAnchorPoint({0.0f, 0.5f});
        m_nameLabel->setPosition({10.0f, 24.0f});
        m_nameLabel->setColor(m_value ? theme::TextPrimary : theme::TextSecondary);
        addChild(m_nameLabel);

        m_descLabel = CCLabelBMFont::create(description.c_str(), "chatFont.fnt");
        m_descLabel->setScale(0.45f);
        m_descLabel->setAnchorPoint({0.0f, 0.5f});
        m_descLabel->setPosition({10.0f, 10.0f});
        m_descLabel->setColor(theme::TextMuted);
        addChild(m_descLabel);

        // Toggle Switch visual
        auto switchContainer = CCNode::create();
        switchContainer->setContentSize({36.0f, 20.0f});

        m_switchBg = CCScale9Sprite::create("square02_001.png");
        m_switchBg->setContentSize({36.0f, 18.0f});
        m_switchBg->setPosition({18.0f, 10.0f});
        m_switchBg->setColor(m_value ? theme::AccentGreen : theme::ToggleTrackOff);
        switchContainer->addChild(m_switchBg);

        m_knob = CCSprite::createWithSpriteFrameName("sliderBtn_001.png");
        if (!m_knob) {
            m_knob = CCSprite::create("sliderThumb.png");
        }
        if (!m_knob) {
            m_knob = CCSprite::create();
        }
        m_knob->setScale(0.6f);
        m_knob->setPosition({m_value ? 26.0f : 10.0f, 10.0f});
        m_knob->setColor(m_value ? theme::ToggleKnobOn : theme::ToggleKnobOff);
        switchContainer->addChild(m_knob);

        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu);

        m_button = CCMenuItemSpriteExtra::create(
            switchContainer, nullptr, this, menu_selector(MegaToggle::onToggle)
        );
        m_button->setPosition({225.0f, 19.0f});
        menu->addChild(m_button);

        return true;
    }

    void MegaToggle::onToggle(CCObject*) {
        m_value = !m_value;
        updateVisuals(true);
        if (m_onChanged) {
            m_onChanged(m_value);
        }
    }

    void MegaToggle::updateVisuals(bool animated) {
        if (m_switchBg) {
            m_switchBg->setColor(m_value ? theme::AccentGreen : theme::ToggleTrackOff);
        }
        if (m_nameLabel) {
            m_nameLabel->setColor(m_value ? theme::TextPrimary : theme::TextSecondary);
        }
        if (m_knob) {
            float targetX = m_value ? 26.0f : 10.0f;
            m_knob->setColor(m_value ? theme::ToggleKnobOn : theme::ToggleKnobOff);
            if (animated) {
                m_knob->stopAllActions();
                m_knob->runAction(CCEaseExponentialOut::create(CCMoveTo::create(0.15f, {targetX, 10.0f})));
            } else {
                m_knob->setPosition({targetX, 10.0f});
            }
        }
    }

    void MegaToggle::setValue(bool val) {
        if (m_value != val) {
            m_value = val;
            updateVisuals(false);
        }
    }

    // ==========================================
    // MegaSlider
    // ==========================================
    MegaSlider* MegaSlider::create(const std::string& id, const std::string& name, float min, float max,
                                  float initialValue, const std::string& format, std::function<void(float)> onChanged) {
        auto ret = new MegaSlider();
        if (ret && ret->init(id, name, min, max, initialValue, format, onChanged)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaSlider::init(const std::string& id, const std::string& name, float min, float max,
                          float initialValue, const std::string& format, std::function<void(float)> onChanged) {
        if (!CCNode::init()) return false;

        m_id = id;
        m_name = name;
        m_min = min;
        m_max = max;
        m_value = initialValue;
        m_format = format;
        m_onChanged = onChanged;

        setContentSize({250.0f, 44.0f});

        auto cardBg = CCLayerColor::create(ccc4(30, 32, 42, 220), 250.0f, 44.0f);
        cardBg->setPosition({0, 0});
        addChild(cardBg);

        m_nameLabel = CCLabelBMFont::create(name.c_str(), "bigFont.fnt");
        m_nameLabel->setScale(0.32f);
        m_nameLabel->setAnchorPoint({0.0f, 0.5f});
        m_nameLabel->setPosition({10.0f, 32.0f});
        m_nameLabel->setColor(theme::TextPrimary);
        addChild(m_nameLabel);

        char buf[32];
        snprintf(buf, sizeof(buf), m_format.c_str(), m_value);
        m_valLabel = CCLabelBMFont::create(buf, "chatFont.fnt");
        m_valLabel->setScale(0.6f);
        m_valLabel->setAnchorPoint({1.0f, 0.5f});
        m_valLabel->setPosition({240.0f, 32.0f});
        m_valLabel->setColor(theme::TextAccent);
        addChild(m_valLabel);

        m_slider = Slider::create(this, menu_selector(MegaSlider::onSliderChanged), 0.6f);
        m_slider->setAnchorPoint({0.5f, 0.5f});
        m_slider->setPosition({125.0f, 14.0f});
        
        float progress = (m_max > m_min) ? (m_value - m_min) / (m_max - m_min) : 0.0f;
        m_slider->setValue(progress);
        addChild(m_slider);

        return true;
    }

    void MegaSlider::onSliderChanged(CCObject*) {
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

    void MegaSlider::setValue(float val) {
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
                                  std::function<void()> onClose, std::function<void(const std::string&)> onSearch) {
        auto ret = new MegaHeader();
        if (ret && ret->init(title, version, onClose, onSearch)) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaHeader::init(const std::string& title, const std::string& version,
                          std::function<void()> onClose, std::function<void(const std::string&)>) {
        if (!CCNode::init()) return false;

        setContentSize({theme::WindowWidth, theme::HeaderHeight});

        // Background
        auto bg = CCLayerColor::create(ccc4(20, 21, 28, 255), theme::WindowWidth, theme::HeaderHeight);
        bg->setPosition({0, 0});
        addChild(bg);

        // Green accent indicator bar on top
        auto topBar = CCLayerColor::create(ccc4(0, 255, 127, 255), theme::WindowWidth, 2.0f);
        topBar->setPosition({0, theme::HeaderHeight - 2.0f});
        addChild(topBar);

        // Title
        auto titleLabel = CCLabelBMFont::create(title.c_str(), "bigFont.fnt");
        titleLabel->setScale(0.5f);
        titleLabel->setAnchorPoint({0.0f, 0.5f});
        titleLabel->setPosition({14.0f, theme::HeaderHeight / 2.0f});
        titleLabel->setColor(theme::TextAccent);
        addChild(titleLabel);

        // Version badge
        auto verLabel = CCLabelBMFont::create(version.c_str(), "chatFont.fnt");
        verLabel->setScale(0.55f);
        verLabel->setAnchorPoint({0.0f, 0.5f});
        verLabel->setPosition({titleLabel->getPositionX() + titleLabel->getContentWidth() * 0.5f + 8.0f, theme::HeaderHeight / 2.0f - 1.0f});
        verLabel->setColor(theme::TextMuted);
        addChild(verLabel);

        // Menu for Close Button
        auto menu = CCMenu::create();
        menu->setPosition({0, 0});
        addChild(menu);

        auto closeSpr = CCSprite::createWithSpriteFrameName("GJ_closeBtn_001.png");
        if (closeSpr) {
            closeSpr->setScale(0.55f);
            auto closeBtn = CCMenuItemSpriteExtra::create(
                closeSpr, nullptr, this, nullptr
            );
            closeBtn->setPosition({theme::WindowWidth - 18.0f, theme::HeaderHeight / 2.0f});
            closeBtn->setUserObject("onClose", nullptr);
            // Lambda wrapper for close action
            menu->addChild(closeBtn);
        }

        return true;
    }

} // namespace megasuite::ui
