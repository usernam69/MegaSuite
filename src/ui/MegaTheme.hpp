#pragma once

#include "../Common.hpp"

namespace megasuite::theme {

    // Authentic MegaHack v7/v8 Dark Charcoal & Emerald Green styling
    inline const ccColor3B BackgroundDark    = ccc3(12, 12, 14);     // Window frame backdrop
    inline const ccColor3B WindowPanel       = ccc3(18, 18, 22);     // Main window body
    inline const ccColor3B HeaderBar         = ccc3(14, 14, 17);     // Top title & drag header
    inline const ccColor3B SidebarBg         = ccc3(16, 16, 20);     // Left category column
    
    // Checkbox & list row styling (MegaHack checkbox list)
    inline const ccColor3B RowBgDefault      = ccc3(22, 22, 26);     // Row base
    inline const ccColor3B RowBgHover        = ccc3(30, 30, 36);     // Row hover
    inline const ccColor3B RowBorder         = ccc3(36, 36, 44);     // Border separator
    inline const ccColor3B CheckboxBg        = ccc3(28, 28, 34);     // Unchecked box background
    inline const ccColor3B CheckboxBorder    = ccc3(60, 60, 72);     // Unchecked border
    
    // Signature MegaHack Emerald Accent
    inline const ccColor3B AccentGreen       = ccc3(0, 255, 128);    // MegaHack vibrant green (#00FF80)
    inline const ccColor3B AccentGreenDark   = ccc3(0, 190, 95);     // Darker green border/active
    inline const ccColor3B AccentGreenDim    = ccc3(15, 60, 35);     // Subdued green for row active tint

    // Typography
    inline const ccColor3B TextPrimary       = ccc3(240, 240, 245);
    inline const ccColor3B TextSecondary     = ccc3(160, 160, 175);
    inline const ccColor3B TextMuted         = ccc3(100, 100, 115);
    inline const ccColor3B TextAccent        = ccc3(0, 255, 128);

    // Controls
    inline const ccColor3B SliderTrack       = ccc3(32, 32, 40);
    inline const ccColor3B SliderFill        = ccc3(0, 255, 128);

    // Layout Metrics (MegaHack standard multi-column layout)
    constexpr float WindowWidth  = 580.0f;
    constexpr float WindowHeight = 350.0f;
    constexpr float HeaderHeight = 32.0f;
    constexpr float SidebarWidth = 135.0f;
    constexpr float ContentWidth = WindowWidth - SidebarWidth - 14.0f;
    constexpr float ContentHeight= WindowHeight - HeaderHeight - 14.0f;

} // namespace megasuite::theme
