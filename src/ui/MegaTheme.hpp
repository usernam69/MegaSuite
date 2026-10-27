#pragma once

#include "../Common.hpp"

namespace megasuite::theme {

    // Classic MegaHack dark palette & neon green accents
    inline const ccColor3B BackgroundDark    = ccc3(16, 17, 22);
    inline const ccColor3B WindowPanel       = ccc3(24, 25, 33);
    inline const ccColor3B HeaderBar         = ccc3(20, 21, 28);
    inline const ccColor3B CardBackground    = ccc3(30, 32, 42);
    inline const ccColor3B CardBorder        = ccc3(45, 48, 62);
    
    inline const ccColor3B AccentGreen       = ccc3(0, 255, 127); // #00FF7F
    inline const ccColor3B AccentGreenDark   = ccc3(0, 180, 90);
    inline const ccColor3B AccentGlow        = ccc3(80, 255, 170);

    inline const ccColor3B ToggleTrackOff    = ccc3(40, 42, 54);
    inline const ccColor3B ToggleKnobOff     = ccc3(120, 125, 140);
    inline const ccColor3B ToggleKnobOn      = ccc3(255, 255, 255);

    inline const ccColor3B TextPrimary       = ccc3(248, 250, 252);
    inline const ccColor3B TextSecondary     = ccc3(148, 163, 184);
    inline const ccColor3B TextMuted         = ccc3(100, 110, 125);
    inline const ccColor3B TextAccent        = ccc3(0, 255, 127);

    inline const ccColor3B SliderTrack       = ccc3(38, 40, 52);
    inline const ccColor3B SliderThumb       = ccc3(0, 255, 127);

    // Layout Metrics
    constexpr float WindowWidth  = 540.0f;
    constexpr float WindowHeight = 330.0f;
    constexpr float HeaderHeight = 36.0f;
    constexpr float TabBarHeight = 30.0f;
    constexpr float ContentWidth = WindowWidth - 20.0f;
    constexpr float ContentHeight= WindowHeight - HeaderHeight - TabBarHeight - 14.0f;

} // namespace megasuite::theme
