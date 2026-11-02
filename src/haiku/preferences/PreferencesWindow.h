#ifndef UNIVERSAL_SCROLLER_PREFERENCES_WINDOW_H
#define UNIVERSAL_SCROLLER_PREFERENCES_WINDOW_H

#include "Configuration.h"

#include <Window.h>

#include <array>

class BCheckBox;
class BTextControl;

class PreferencesWindow : public BWindow {
public:
    PreferencesWindow();

    void MessageReceived(BMessage* message) override;
    bool QuitRequested() override;

private:
    void BuildInterface();
    void LoadControls();
    bool SaveControls();

    Configuration fConfiguration;
    BTextControl* fMinimumScroll = nullptr;
    std::array<BCheckBox*, Configuration::kButtonCombinationCount> fScrollCombinations {};
    std::array<BTextControl*, Configuration::kSpeedCount> fWheelX {};
    std::array<BTextControl*, Configuration::kSpeedCount> fWheelY {};
    BCheckBox* fUseWheelFactors = nullptr;
    std::array<BTextControl*, Configuration::kButtonCommandCount> fCommands {};
    std::array<BCheckBox*, Configuration::kButtonCommandCount> fSwallowClicks {};
    BCheckBox* fRestartInputServer = nullptr;
};

#endif
