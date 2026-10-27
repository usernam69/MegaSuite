#include "MegaHackWindow.hpp"
#include <algorithm>

namespace megasuite::ui {

    static MegaHackWindow* s_instance = nullptr;

    MegaHackWindow* MegaHackWindow::create() {
        auto ret = new MegaHackWindow();
        if (ret && ret->init()) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

    bool MegaHackWindow::init() {
        // Semi-transparent backdrop
        if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 140))) {
            return false;
        }

        setTouchEnabled(true);
        setKeypadEnabled(true);

        auto winSize = CCDirector::get()->getWinSize();

        // Container Window Node (scaled and centered)
        m_windowNode = CCNode::create();
        m_windowNode->setContentSize({theme::WindowWidth, theme::WindowHeight});
        m_windowNode->setAnchorPoint({0.5f, 0.5f});
        m_windowNode->setPosition({winSize.width / 2.0f, winSize.height / 2.0f});
        addChild(m_windowNode);

        // Window background (dark acrylic)
        m_windowBg = CCLayerColor::create(ccc4(24, 25, 33, 245), theme::WindowWidth, theme::WindowHeight);
        m_windowBg->setPosition({0, 0});
        m_windowNode->addChild(m_windowBg);

        // Subtle outer border
        auto border = CCLayerColor::create(ccc4(45, 48, 62, 255), theme::WindowWidth, 1.0f);
        border->setPosition({0, 0});
        m_windowNode->addChild(border);

        setupUI();
        setupTabs();
        loadCategory(Category::QoL);

        return true;
    }

    void MegaHackWindow::setupUI() {
        // Header
        auto header = MegaHeader::create(
            "MegaSuite", "v1.0.0",
            [this]() { hide(); },
            nullptr
        );
        header->setPosition({0, theme::WindowHeight - theme::HeaderHeight});
        m_windowNode->addChild(header);

        // Tab menu container
        auto tabBg = CCLayerColor::create(ccc4(20, 21, 28, 200), theme::WindowWidth, theme::TabBarHeight);
        tabBg->setPosition({0, theme::WindowHeight - theme::HeaderHeight - theme::TabBarHeight});
        m_windowNode->addChild(tabBg);

        m_tabMenu = CCMenu::create();
        m_tabMenu->setPosition({0, 0});
        tabBg->addChild(m_tabMenu);

        // Content Area
        m_contentArea = CCNode::create();
        m_contentArea->setContentSize({theme::ContentWidth, theme::ContentHeight});
        m_contentArea->setPosition({10.0f, 10.0f});
        m_windowNode->addChild(m_contentArea);
    }

    void MegaHackWindow::setupTabs() {
        struct TabDef {
            std::string name;
            Category cat;
        };

        std::vector<TabDef> tabs = {
            {"QoL Hacks", Category::QoL},
            {"Bugfixes",  Category::Bugfixes},
            {"DVD Mode",  Category::DVDMode},
            {"API Hub",   Category::APIs},
            {"Settings",  Category::Settings}
        };

        float startX = 50.0f;
        float spacing = 105.0f;

        for (size_t i = 0; i < tabs.size(); ++i) {
            auto btn = MegaTabButton::create(
                tabs[i].name, this, menu_selector(MegaHackWindow::onTabSelected)
            );
            btn->setTag(static_cast<int>(tabs[i].cat));
            btn->setPosition({startX + i * spacing, theme::TabBarHeight / 2.0f});
            m_tabMenu->addChild(btn);
            m_tabButtons.push_back(btn);
        }

        if (!m_tabButtons.empty()) {
            m_tabButtons[0]->setActive(true);
        }
    }

    void MegaHackWindow::onTabSelected(CCObject* sender) {
        auto btn = static_cast<MegaTabButton*>(sender);
        for (auto* t : m_tabButtons) {
            t->setActive(t == btn);
        }
        Category cat = static_cast<Category>(btn->getTag());
        loadCategory(cat);
    }

    void MegaHackWindow::loadCategory(Category cat) {
        m_currentCategory = cat;
        m_contentArea->removeAllChildrenWithCleanup(true);

        auto settings = ModuleManager::get()->getCategorySettings(cat);

        // 2-column grid layout
        float colWidth = 255.0f;
        float rowHeight = 44.0f;
        float padding = 8.0f;
        float startY = theme::ContentHeight - rowHeight - 4.0f;

        for (size_t i = 0; i < settings.size(); ++i) {
            const auto& s = settings[i];
            int col = i % 2;
            int row = i / 2;

            float posX = col * (colWidth + padding);
            float posY = startY - row * (rowHeight + padding);

            if (s.isBool) {
                auto toggle = MegaToggle::create(
                    s.id, s.name, s.description, s.boolVal,
                    [s](bool val) {
                        ModuleManager::get()->setBool(s.id, val);
                    }
                );
                toggle->setPosition({posX, posY});
                m_contentArea->addChild(toggle);
            } else {
                auto slider = MegaSlider::create(
                    s.id, s.name, s.minFloat, s.maxFloat, s.floatVal, s.format,
                    [s](float val) {
                        ModuleManager::get()->setFloat(s.id, val);
                    }
                );
                slider->setPosition({posX, posY});
                m_contentArea->addChild(slider);
            }
        }
    }

    void MegaHackWindow::registerWithTouchDispatcher() {
        CCDirector::sharedDirector()->getTouchDispatcher()->addTargetedDelegate(this, -600, true);
    }

    bool MegaHackWindow::ccTouchBegan(CCTouch* touch, CCEvent*) {
        if (!isVisible()) return false;

        auto pos = touch->getLocation();
        auto localPos = m_windowNode->convertToNodeSpace(pos);

        // Check if dragging via Header bar
        CCRect headerRect(0, theme::WindowHeight - theme::HeaderHeight, theme::WindowWidth, theme::HeaderHeight);
        if (headerRect.containsPoint(localPos)) {
            m_isDragging = true;
            m_dragOffset = m_windowNode->getPosition() - pos;
            return true;
        }

        // Swallows all clicks inside the window or backdrop
        CCRect windowRect(0, 0, theme::WindowWidth, theme::WindowHeight);
        if (!windowRect.containsPoint(localPos)) {
            // Click outside window closes it
            hide();
        }
        return true;
    }

    void MegaHackWindow::ccTouchMoved(CCTouch* touch, CCEvent*) {
        if (m_isDragging && m_windowNode) {
            auto pos = touch->getLocation();
            m_windowNode->setPosition(pos + m_dragOffset);
        }
    }

    void MegaHackWindow::ccTouchEnded(CCTouch*, CCEvent*) {
        m_isDragging = false;
    }

    void MegaHackWindow::show() {
        setVisible(true);
        setTouchEnabled(true);
        if (m_windowNode) {
            m_windowNode->setScale(0.85f);
            m_windowNode->stopAllActions();
            m_windowNode->runAction(CCEaseBackOut::create(CCScaleTo::create(0.2f, 1.0f)));
        }
        runAction(CCFadeIn::create(0.15f));
    }

    void MegaHackWindow::hide() {
        setTouchEnabled(false);
        if (m_windowNode) {
            m_windowNode->stopAllActions();
            m_windowNode->runAction(CCSequence::createWithTwoActions(
                CCEaseExponentialIn::create(CCScaleTo::create(0.15f, 0.85f)),
                CCCallFunc::create(this, callfunc_selector(CCNode::removeFromParent))
            ));
        } else {
            removeFromParent();
        }
        s_instance = nullptr;
    }

    void MegaHackWindow::toggle() {
        if (isOpen()) {
            s_instance->hide();
        } else {
            auto scene = CCScene::get();
            if (!scene) return;

            s_instance = MegaHackWindow::create();
            scene->addChild(s_instance, 100000);
            s_instance->show();
        }
    }

    bool MegaHackWindow::isOpen() {
        return s_instance != nullptr && s_instance->getParent() != nullptr && s_instance->isVisible();
    }

} // namespace megasuite::ui
