#pragma once

#include "MegaTheme.hpp"
#include <functional>

namespace megasuite::ui {

    class MegaTabButton : public CCMenuItemSpriteExtra {
    protected:
        std::string m_title;
        bool m_active = false;
        CCLabelBMFont* m_label = nullptr;
        CCLayerColor* m_indicator = nullptr;

        bool init(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback);

    public:
        static MegaTabButton* create(const std::string& title, CCObject* target, cocos2d::SEL_MenuHandler callback);
        void setActive(bool active);
        bool isActive() const { return m_active; }
        const std::string& getTitle() const { return m_title; }
    };

    class MegaToggle : public CCNode {
    protected:
        std::string m_id;
        std::string m_name;
        std::string m_description;
        bool m_value = false;
        std::function<void(bool)> m_onChanged;

        CCLabelBMFont* m_nameLabel = nullptr;
        CCLabelBMFont* m_descLabel = nullptr;
        CCScale9Sprite* m_switchBg = nullptr;
        CCSprite* m_knob = nullptr;
        CCMenuItemSpriteExtra* m_button = nullptr;

        bool init(const std::string& id, const std::string& name, const std::string& description,
                  bool initialValue, std::function<void(bool)> onChanged);

        void onToggle(CCObject* sender);
        void updateVisuals(bool animated = true);

    public:
        static MegaToggle* create(const std::string& id, const std::string& name, const std::string& description,
                                  bool initialValue, std::function<void(bool)> onChanged);
        
        void setValue(bool val);
        bool getValue() const { return m_value; }
        const std::string& getId() const { return m_id; }
        const std::string& getName() const { return m_name; }
    };

    class MegaSlider : public CCNode {
    protected:
        std::string m_id;
        std::string m_name;
        float m_min = 0.0f;
        float m_max = 1.0f;
        float m_value = 0.0f;
        std::string m_format = "%.2f";
        std::function<void(float)> m_onChanged;

        CCLabelBMFont* m_nameLabel = nullptr;
        CCLabelBMFont* m_valLabel = nullptr;
        Slider* m_slider = nullptr;

        bool init(const std::string& id, const std::string& name, float min, float max,
                  float initialValue, const std::string& format, std::function<void(float)> onChanged);

        void onSliderChanged(CCObject* sender);

    public:
        static MegaSlider* create(const std::string& id, const std::string& name, float min, float max,
                                  float initialValue, const std::string& format, std::function<void(float)> onChanged);

        void setValue(float val);
        float getValue() const { return m_value; }
        const std::string& getId() const { return m_id; }
    };

    class MegaHeader : public CCNode {
    protected:
        bool init(const std::string& title, const std::string& version,
                  std::function<void()> onClose, std::function<void(const std::string&)> onSearch);

    public:
        static MegaHeader* create(const std::string& title, const std::string& version,
                                  std::function<void()> onClose, std::function<void(const std::string&)> onSearch);
    };

} // namespace megasuite::ui
