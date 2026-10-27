# MegaSuite: Unified Geode Mod

A Geometry Dash Geode mod combining **QOLMod**, **DVD Mode**, **Bugfixes**, and **APIs** in the iconic style of **MegaHack**.

## Project Structure

```
├── mod.json                     # Geode mod manifest
├── CMakeLists.txt               # CMake build script
├── about.md                     # In-game Geode mod description
├── changelog.md                 # Version release notes
├── src/
│   ├── main.cpp                 # Mod entry point & keybind hooks
│   ├── Common.hpp               # Utilities & color parsing
│   ├── ui/
│   │   ├── MegaTheme.hpp        # MegaHack styling metrics & neon palette
│   │   ├── Components.hpp       # Toggles, Sliders, Tabs, Header
│   │   ├── Components.cpp       # Component implementations
│   │   ├── MegaHackWindow.hpp   # Floating UI window header
│   │   └── MegaHackWindow.cpp   # Window overlay, drag & touch handling
│   └── modules/
│       ├── ModuleManager.hpp    # Central setting & category manager
│       ├── ModuleManager.cpp    # Setting dispatch & registration
│       ├── QOLModules.hpp       # Speedhack, StartPos, Hitboxes, Noclip
│       ├── QOLModules.cpp       # QOL implementations & hooks
│       ├── DVDModeModule.hpp    # Bouncing popup physics header
│       ├── DVDModeModule.cpp    # Vector reflection & screen collision
│       ├── BugfixModules.hpp    # Bugfix headers
│       ├── BugfixModules.cpp    # 6 core GD 2.2 bugfixes
│       ├── APIModules.hpp       # Level ID, Node IDs & Persistence
│       └── APIModules.cpp       # API implementations & hooks
└── references/                  # Reference source trees
```

## How to Build

1. Ensure the [Geode CLI](https://geode-sdk.org/) and a C++23 capable compiler (MSVC on Windows, Clang on macOS) are installed:
   ```bash
   geode sdk install
   ```

2. Configure and build the `.geode` package:
   ```bash
   geode build
   ```

3. Or install directly to your Geometry Dash directory:
   ```bash
   geode build --install
   ```

## Controls

- Toggle Menu: <kbd>Tab</kbd>
- Alternatively click the green **MegaSuite** button in the main menu bottom bar.
