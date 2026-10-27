#include "MegaHackWindow.hpp"

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
        if (!CCLayerColor::initWithColor(ccc4(0, 0, 0, 150))) {
            return false;
        }

        setTouchEnabled(true);
        setKeypadEnabled(true);

        auto winSize = CCDirector::get()->getWinSize();

        // Main floating MegaHack container
        m_windowNode = CCNode::create();
        m_windowNode->setContentSize({theme::WindowWidth, theme::WindowHeight});
        m_windowNode->setAnchorPoint({0.5f, 0.5f});
        m_windowNode->setPosition({winSize.width / 2.0f, winSize.height / 2.0f});
        addChild(m_windowNode);

        // Window background (Dark charcoal)
        m_windowBg = CCLayerColor::create(ccc4(18, 18, 22, 250), theme::WindowWidth, theme::WindowHeight);
        m_windowBg->setPosition({0, 0});
        m_windowNode->addChild(m_windowBg);

        // Outer crisp border
        auto border = CCLayerColor::create(ccc4(40, 40, 50, 255), theme::WindowWidth, 1.0f);
        border->setPosition({0, 0});
        m_windowNode->addChild(border);

        setupUI();
        setupSidebar();
        loadCategory(Category::QoL);

        return true;
    }

    void MegaHackWindow::setupUI() {
        // Drag header bar
        auto header = MegaHeader::create(
            "MegaSuite", "v1.0.0",
            [this]() { hide(); }
        );
        header->setPosition({0, theme::WindowHeight - theme::HeaderHeight});
        m_windowNode->addChild(header);

        // Left sidebar column
        auto sideBg = CCLayerColor::create(ccc4(15, 15, 18, 255), theme::SidebarWidth, theme::WindowHeight - theme::HeaderHeight);
        sideBg->setPosition({0, 0});
        m_windowNode->addChild(sideBg);

        // Vertical divider between sidebar and content
        auto divider = CCLayerColor::create(ccc4(35, 35, 42, 255), 1.0f, theme::WindowHeight - theme::HeaderHeight);
        divider->setPosition({theme::SidebarWidth, 0});
        m_windowNode->addChild(divider);

        m_sidebarMenu = CCMenu::create();
        m_sidebarMenu->setPosition({0, 0});
        sideBg->addChild(m_sidebarMenu);

        // Content Area (Multi-column checkbox grid)
        m_contentArea = CCNode::create();
        m_contentArea->setContentSize({theme::ContentWidth, theme::ContentHeight});
        m_contentArea->setPosition({theme::SidebarWidth + 8.0f, 6.0f});
        m_windowNode->addChild(m_contentArea);
    }

    void MegaHackWindow::setupSidebar() {
        struct CatDef {
            std::string name;
            Category cat;
        };

        std::vector<CatDef> categories = {
            {"Bypass & QoL", Category::QoL},
            {"Bugfixes",     Category::Bugfixes},
            {"DVD Mode",     Category::DVDMode},
            {"Developer API",Category::APIs},
            {"Settings",     Category::Settings}
        };

        float startY = (theme::WindowHeight - theme::HeaderHeight) - 24.0f;
        float spacing = 36.0f;

        for (size_t i = 0; i < categories.size(); ++i) {
            auto item = MegaSidebarItem::create(
                categories[i].name, this, menu_selector(MegaHackWindow::onCategorySelected)
            );
            item->setTag(static_cast<int>(categories[i].cat));
            item->setPosition({(theme::SidebarWidth - 8.0f) / 2.0f + 4.0f, startY - i * spacing});
            m_sidebarMenu->addChild(item);
            m_sidebarButtons.push_back(item);
        }

        if (!m_sidebarButtons.empty()) {
            m_sidebarButtons[0]->setActive(true);
        }
    }

    void MegaHackWindow::onCategorySelected(CCObject* sender) {
        auto item = static_cast<MegaSidebarItem*>(sender);
        for (auto* btn : m_sidebarButtons) {
            btn->setActive(btn == item);
        }
        Category cat = static_cast<Category>(item->getTag());
        loadCategory(cat);
    }

    void MegaHackWindow::loadCategory(Category cat) {
        m_currentCategory = cat;
        m_contentArea->removeAllChildrenWithCleanup(true);

        auto settings = ModuleManager::get()->getCategorySettings(cat);

        // Authentic MegaHack 2-column checkbox/toggle list
        float colWidth = 204.0f;
        float rowHeight = 36.0f;
        float paddingX = 8.0f;
        float paddingY = 4.0f;
        float startY = theme::ContentHeight - rowHeight;

        for (size_t i = 0; i < settings.size(); ++i) {
            const auto& s = settings[i];
            int col = i % 2;
            int row = i / 2;

            float posX = col * (colWidth + paddingX);
            float posY = startY - row * (rowHeight + paddingY);

            if (s.isBool) {
                auto checkbox = MegaCheckboxRow::create(
                    s.id, s.name, s.description, s.boolVal,
                    [s](bool val) {
                        ModuleManager::get()->setBool(s.id, val);
                    }
                );
                checkbox->setPosition({posX, posY});
                m_contentArea->addChild(checkbox);
            } else {
                auto slider = MegaSliderRow::create(
                    s.id, s.name, s.minFloat, s.maxFloat, s.floatVal, s.format,
                    [s](float val) {
                        ModuleManager::get()->setFloat(s.id, val);
                    }
                );
                slider->setPosition({posX, posY - 6.0f});
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

        // Click outside closes window
        CCRect windowRect(0, 0, theme::WindowWidth, theme::WindowHeight);
        if (!windowRect.containsPoint(localPos)) {
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
            m_windowNode->setScale(0.9f);
            m_windowNode->stopAllActions();
            m_windowNode->runAction(CCEaseBackOut::create(CCScaleTo::create(0.18f, 1.0f)));
        }
        runAction(CCFadeIn::create(0.15f));
    }

    void MegaHackWindow::hide() {
        setTouchEnabled(false);
        if (m_windowNode) {
            m_windowNode->stopAllActions();
            m_windowNode->runAction(CCSequence::createWithTwoActions(
                CCEaseExponentialIn::create(CCScaleTo::create(0.12f, 0.9f)),
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
