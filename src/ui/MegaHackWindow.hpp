#pragma once

#include "Components.hpp"
#include "../modules/ModuleManager.hpp"
#include <vector>

namespace megasuite::ui {

    class MegaHackWindow : public CCLayerColor {
    protected:
        CCNode* m_windowNode = nullptr;
        CCLayerColor* m_windowBg = nullptr;
        CCNode* m_contentArea = nullptr;
        CCMenu* m_sidebarMenu = nullptr;
        std::vector<MegaSidebarItem*> m_sidebarButtons;
        Category m_currentCategory = Category::QoL;
        bool m_isDragging = false;
        CCPoint m_dragOffset;

        bool init();
        void setupUI();
        void setupSidebar();
        void loadCategory(Category cat);
        void onCategorySelected(CCObject* sender);
        void onClose(CCObject* sender);

        // Touch handling
        bool ccTouchBegan(CCTouch* touch, CCEvent* event) override;
        void ccTouchMoved(CCTouch* touch, CCEvent* event) override;
        void ccTouchEnded(CCTouch* touch, CCEvent* event) override;
        void registerWithTouchDispatcher() override;

    public:
        static MegaHackWindow* create();
        static void toggle();
        static bool isOpen();

        void show();
        void hide();
    };

} // namespace megasuite::ui
