#ifndef UNIVERSAL_SCROLLER_PREFERENCES_WINDOW_H
#define UNIVERSAL_SCROLLER_PREFERENCES_WINDOW_H

#include "Configuration.h"

#include <Window.h>

#include <array>

class BCheckBox;
class BPopUpMenu;
class BStringView;
class BTextControl;

class PreferencesWindow : public BWindow {
public:
    PreferencesWindow();

    void MessageReceived(BMessage* message) override;
    bool QuitRequested() override;

private:
    void BuildInterface();
    void LoadControls();
    void LoadSelectedAction();
    void StoreSelectedAction();
    void SetStatus(const char* status);
    bool SaveControls();

    Configuration fConfiguration;
    BTextControl* fMinimumScroll = nullptr;
    std::array<BCheckBox*, Configuration::kButtonCombinationCount> fScrollCombinations {};
    std::array<BTextControl*, Configuration::kSpeedCount> fWheelX {};
    std::array<BTextControl*, Configuration::kSpeedCount> fWheelY {};
    BCheckBox* fUseWheelFactors = nullptr;
    BPopUpMenu* fActionMenu = nullptr;
    BTextControl* fCommand = nullptr;
    BCheckBox* fSwallowClick = nullptr;
    std::size_t fSelectedAction = 0;
    BStringView* fStatus = nullptr;
};

#endif
