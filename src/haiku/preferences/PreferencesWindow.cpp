#include "PreferencesWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <GroupLayout.h>
#include <Message.h>
#include <String.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <View.h>

#include <cerrno>
#include <climits>
#include <cstdlib>
#include <cstdint>

namespace {

constexpr uint32 kSaveMessage = 'save';
constexpr uint32 kDefaultsMessage = 'dflt';

const char* const kCombinationLabels[Configuration::kButtonCombinationCount] = {
    "No buttons", "Primary", "Secondary", "Primary + secondary",
    "Tertiary", "Primary + tertiary", "Secondary + tertiary", "All buttons"
};

const char* const kCommandLabels[Configuration::kButtonCommandCount] = {
    "Primary down", "Primary + secondary inter-click", "Primary + tertiary inter-click",
    "Secondary down", "Secondary + primary inter-click", "Secondary + tertiary inter-click",
    "Tertiary down", "Tertiary + primary inter-click", "Tertiary + secondary inter-click"
};

bool ParseInt32(const char* text, std::int32_t& value)
{
    errno = 0;
    char* end = nullptr;
    const long parsed = std::strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < INT32_MIN || parsed > INT32_MAX) {
        return false;
    }
    value = static_cast<std::int32_t>(parsed);
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool ParseFloat(const char* text, float& value)
{
    errno = 0;
    char* end = nullptr;
    value = std::strtof(text, &end);
    return errno == 0 && end != text && *end == '\0';
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

PreferencesWindow::PreferencesWindow()
    : BWindow(BRect(100, 100, 720, 680), "UniversalScroller Preferences",
        B_TITLED_WINDOW, B_AUTO_UPDATE_SIZE_LIMITS | B_NOT_ZOOMABLE)
{
    fConfiguration.Load(Configuration::DefaultPath());
    BuildInterface();
    LoadControls();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void PreferencesWindow::BuildInterface()
{
    BTabView* tabs = new BTabView("settings");

    BView* scrolling = new BView("Scrolling", B_WILL_DRAW);
    scrolling->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    fMinimumScroll = new BTextControl("minimum", "Minimum movement squared:", "", nullptr);
    BLayoutBuilder::Group<>(scrolling, B_VERTICAL, B_USE_DEFAULT_SPACING)
        .SetInsets(B_USE_WINDOW_SPACING)
        .Add(fMinimumScroll)
        .Add(new BStringView(nullptr, "Scroll while these buttons are held:"));
    BGroupLayout* scrollingLayout = static_cast<BGroupLayout*>(scrolling->GetLayout());
    for (std::size_t index = 1; index < fScrollCombinations.size(); ++index) {
        fScrollCombinations[index] = new BCheckBox(nullptr, kCombinationLabels[index], nullptr);
        scrollingLayout->AddView(fScrollCombinations[index]);
    }
    tabs->AddTab(scrolling);

    BView* speed = new BView("Speed", B_WILL_DRAW);
    speed->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    fUseWheelFactors = new BCheckBox(nullptr, "Apply factors to mouse-wheel events", nullptr);
    BLayoutBuilder::Group<>(speed, B_VERTICAL, B_USE_DEFAULT_SPACING)
        .SetInsets(B_USE_WINDOW_SPACING)
        .Add(fUseWheelFactors);
    BGroupLayout* speedLayout = static_cast<BGroupLayout*>(speed->GetLayout());
    const char* speedNames[Configuration::kSpeedCount] = {"Normal", "Control-key fast"};
    for (std::size_t index = 0; index < Configuration::kSpeedCount; ++index) {
        speedLayout->AddView(new BStringView(nullptr, speedNames[index]));
        fWheelX[index] = new BTextControl(nullptr, "Horizontal factor:", "", nullptr);
        fWheelY[index] = new BTextControl(nullptr, "Vertical factor:", "", nullptr);
        speedLayout->AddView(fWheelX[index]);
        speedLayout->AddView(fWheelY[index]);
    }
    tabs->AddTab(speed);

    BView* commands = new BView("Commands", B_WILL_DRAW);
    commands->SetViewUIColor(B_PANEL_BACKGROUND_COLOR);
    BLayoutBuilder::Group<>(commands, B_VERTICAL, B_USE_DEFAULT_SPACING)
        .SetInsets(B_USE_WINDOW_SPACING);
    BGroupLayout* commandLayout = static_cast<BGroupLayout*>(commands->GetLayout());
    for (std::size_t index = 0; index < Configuration::kButtonCommandCount; ++index) {
        fCommands[index] = new BTextControl(nullptr, kCommandLabels[index], "", nullptr);
        fSwallowClicks[index] = new BCheckBox(nullptr, "Swallow physical click", nullptr);
        commandLayout->AddView(fCommands[index]);
        commandLayout->AddView(fSwallowClicks[index]);
    }
    tabs->AddTab(commands);

    BButton* defaults = new BButton(nullptr, "Defaults", new BMessage(kDefaultsMessage));
    BButton* save = new BButton(nullptr, "Save", new BMessage(kSaveMessage));
    fRestartInputServer = new BCheckBox(nullptr, "Restart input server after saving", nullptr);
    fRestartInputServer->SetValue(B_CONTROL_ON);
    save->MakeDefault(true);
    BLayoutBuilder::Group<>(this, B_VERTICAL, 0)
        .Add(tabs)
        .AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
            .SetInsets(B_USE_WINDOW_SPACING, 0)
            .Add(fRestartInputServer)
        .End()
        .AddGroup(B_HORIZONTAL, B_USE_DEFAULT_SPACING)
            .SetInsets(B_USE_WINDOW_SPACING)
            .Add(defaults)
            .AddGlue()
            .Add(save)
        .End();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void PreferencesWindow::LoadControls()
{
    BString value;
    value.SetToFormat("%" B_PRId32, fConfiguration.MinimumScroll());
    fMinimumScroll->SetText(value.String());
    for (std::size_t index = 1; index < fScrollCombinations.size(); ++index) {
        fScrollCombinations[index]->SetValue(fConfiguration.ScrollWhenMouseDown(index));
    }

    fUseWheelFactors->SetValue(fConfiguration.UseWheelFactors());
    for (std::size_t index = 0; index < Configuration::kSpeedCount; ++index) {
        value.SetToFormat("%.4g", fConfiguration.WheelFactorX(index));
        fWheelX[index]->SetText(value.String());
        value.SetToFormat("%.4g", fConfiguration.WheelFactorY(index));
        fWheelY[index]->SetText(value.String());
    }

    for (std::size_t index = 0; index < Configuration::kButtonCommandCount; ++index) {
        fCommands[index]->SetText(fConfiguration.ButtonCommand(index).Text().c_str());
        fSwallowClicks[index]->SetValue(fConfiguration.SwallowClick(index));
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool PreferencesWindow::SaveControls()
{
    Configuration updated = fConfiguration;
    std::int32_t minimumScroll = 0;
    if (!ParseInt32(fMinimumScroll->Text(), minimumScroll) || minimumScroll < 0) {
        (new BAlert("Invalid setting", "Minimum movement must be a non-negative whole number.", "OK"))->Go();
        return false;
    }
    updated.SetMinimumScroll(minimumScroll);

    for (std::size_t index = 1; index < fScrollCombinations.size(); ++index) {
        updated.SetScrollWhenMouseDown(index, fScrollCombinations[index]->Value() == B_CONTROL_ON);
    }
    updated.SetUseWheelFactors(fUseWheelFactors->Value() == B_CONTROL_ON);

    for (std::size_t index = 0; index < Configuration::kSpeedCount; ++index) {
        float factorX = 0.0F;
        float factorY = 0.0F;
        if (!ParseFloat(fWheelX[index]->Text(), factorX)
            || !ParseFloat(fWheelY[index]->Text(), factorY)) {
            (new BAlert("Invalid setting", "Wheel factors must be numbers.", "OK"))->Go();
            return false;
        }
        updated.SetWheelFactors(index, factorX, factorY);
    }

    for (std::size_t index = 0; index < Configuration::kButtonCommandCount; ++index) {
        updated.SetButtonCommand(index, fCommands[index]->Text());
        if (!updated.ButtonCommand(index).IsValid()) {
            BString message;
            message.SetToFormat("The command for ‘%s’ is invalid.", kCommandLabels[index]);
            (new BAlert("Invalid command", message.String(), "OK"))->Go();
            return false;
        }
        updated.SetSwallowClick(index, fSwallowClicks[index]->Value() == B_CONTROL_ON);
    }

    if (!updated.Save(Configuration::DefaultPath())) {
        (new BAlert("Save failed", "The UniversalScroller settings file could not be saved.", "OK"))->Go();
        return false;
    }
    fConfiguration = updated;
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void PreferencesWindow::MessageReceived(BMessage* message)
{
    switch (message->what) {
        case kSaveMessage:
            if (SaveControls()) {
                if (fRestartInputServer->Value() == B_CONTROL_ON) {
                    std::system("/system/servers/input_server -q");
                } else {
                    (new BAlert("Settings saved",
                        "Restart the input server when you are ready to reload the filter.", "OK"))->Go();
                }
            }
            break;
        case kDefaultsMessage:
            fConfiguration = Configuration();
            LoadControls();
            break;
        default:
            BWindow::MessageReceived(message);
            break;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool PreferencesWindow::QuitRequested()
{
    be_app->PostMessage(B_QUIT_REQUESTED);
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


