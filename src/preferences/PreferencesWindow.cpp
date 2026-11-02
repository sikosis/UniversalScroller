#include "PreferencesWindow.h"

#include <Alert.h>
#include <Application.h>
#include <Button.h>
#include <CheckBox.h>
#include <InterfaceDefs.h>
#include <LayoutBuilder.h>
#include <GroupLayout.h>
#include <MenuField.h>
#include <MenuItem.h>
#include <Message.h>
#include <PopUpMenu.h>
#include <String.h>
#include <StringView.h>
#include <TabView.h>
#include <TextControl.h>
#include <View.h>

#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstdint>

namespace {

constexpr uint32 kSaveMessage = 'save';
constexpr uint32 kDefaultsMessage = 'dflt';
constexpr uint32 kSelectActionMessage = 'slac';
constexpr uint32 kPrimaryMessage = 'prmy';
constexpr uint32 kPrimaryDoubleMessage = 'prmd';
constexpr uint32 kSecondaryMessage = 'scnd';
constexpr uint32 kSecondaryDoubleMessage = 'scdd';
constexpr uint32 kTertiaryMessage = 'trty';
constexpr uint32 kTertiaryDoubleMessage = 'trtd';
constexpr uint32 kCutMessage = 'pcut';
constexpr uint32 kCopyMessage = 'pcpy';
constexpr uint32 kPasteMessage = 'ppst';

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
    return errno == 0 && end != text && *end == '\0' && std::isfinite(value);
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
    fActionMenu = new BPopUpMenu("action");
    fActionMenu->SetLabelFromMarked(true);
    fActionMenu->SetRadioMode(true);
    for (std::size_t index = 0; index < Configuration::kButtonCommandCount; ++index) {
        BMessage* selection = new BMessage(kSelectActionMessage);
        selection->AddInt32("index", static_cast<int32>(index));
        fActionMenu->AddItem(new BMenuItem(kCommandLabels[index], selection));
    }
    BMenuField* actionField = new BMenuField(nullptr, "When:", fActionMenu);
    fSwallowClick = new BCheckBox(nullptr, "Swallow the physical click", nullptr);
    fCommand = new BTextControl(nullptr, "Command:", "", nullptr);

    BButton* primary = new BButton(nullptr, "Primary", new BMessage(kPrimaryMessage));
    BButton* primaryDouble = new BButton(nullptr, "Primary double", new BMessage(kPrimaryDoubleMessage));
    BButton* secondary = new BButton(nullptr, "Secondary", new BMessage(kSecondaryMessage));
    BButton* secondaryDouble = new BButton(nullptr, "Secondary double", new BMessage(kSecondaryDoubleMessage));
    BButton* tertiary = new BButton(nullptr, "Tertiary", new BMessage(kTertiaryMessage));
    BButton* tertiaryDouble = new BButton(nullptr, "Tertiary double", new BMessage(kTertiaryDoubleMessage));
    BButton* cut = new BButton(nullptr, "Cut", new BMessage(kCutMessage));
    BButton* copy = new BButton(nullptr, "Copy", new BMessage(kCopyMessage));
    BButton* paste = new BButton(nullptr, "Paste", new BMessage(kPasteMessage));

    BLayoutBuilder::Group<>(commands, B_VERTICAL, B_USE_DEFAULT_SPACING)
        .SetInsets(B_USE_WINDOW_SPACING)
        .Add(actionField)
        .Add(fSwallowClick)
        .Add(fCommand)
        .Add(new BStringView(nullptr, "Presets"))
        .AddGrid(B_USE_DEFAULT_SPACING, B_USE_DEFAULT_SPACING)
            .Add(primary, 0, 0)
            .Add(secondary, 1, 0)
            .Add(tertiary, 2, 0)
            .Add(primaryDouble, 0, 1)
            .Add(secondaryDouble, 1, 1)
            .Add(tertiaryDouble, 2, 1)
            .Add(cut, 0, 2)
            .Add(copy, 1, 2)
            .Add(paste, 2, 2)
        .End()
        .AddGlue();
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

    BMenuItem* selectedItem = fActionMenu->ItemAt(static_cast<int32>(fSelectedAction));
    if (selectedItem != nullptr) {
        selectedItem->SetMarked(true);
    }
    LoadSelectedAction();
}
//---------------------------------------------------------------------------------------------------------------------------------//


void PreferencesWindow::LoadSelectedAction()
{
    fCommand->SetText(fConfiguration.ButtonCommand(fSelectedAction).Text().c_str());
    fSwallowClick->SetValue(fConfiguration.SwallowClick(fSelectedAction));
}
//---------------------------------------------------------------------------------------------------------------------------------//


void PreferencesWindow::StoreSelectedAction()
{
    fConfiguration.SetButtonCommand(fSelectedAction, fCommand->Text());
    fConfiguration.SetSwallowClick(fSelectedAction, fSwallowClick->Value() == B_CONTROL_ON);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool PreferencesWindow::SaveControls()
{
    StoreSelectedAction();
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
        if (!updated.ButtonCommand(index).IsValid()) {
            BString message;
            message.SetToFormat("The command for ‘%s’ is invalid.", kCommandLabels[index]);
            (new BAlert("Invalid command", message.String(), "OK"))->Go();
            return false;
        }
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
            fSelectedAction = 0;
            LoadControls();
            break;
        case kSelectActionMessage: {
            int32 index = 0;
            if (message->FindInt32("index", &index) == B_OK && index >= 0
                && index < static_cast<int32>(Configuration::kButtonCommandCount)) {
                StoreSelectedAction();
                fSelectedAction = static_cast<std::size_t>(index);
                LoadSelectedAction();
            }
            break;
        }
        case kPrimaryMessage:
            fCommand->SetText("PRIMARY_MOUSE_BUTTON");
            break;
        case kPrimaryDoubleMessage:
            fCommand->SetText("PRIMARY_MOUSE_BUTTON_DOUBLE");
            break;
        case kSecondaryMessage:
            fCommand->SetText("SECONDARY_MOUSE_BUTTON");
            break;
        case kSecondaryDoubleMessage:
            fCommand->SetText("SECONDARY_MOUSE_BUTTON_DOUBLE");
            break;
        case kTertiaryMessage:
            fCommand->SetText("TERTIARY_MOUSE_BUTTON");
            break;
        case kTertiaryDoubleMessage:
            fCommand->SetText("TERTIARY_MOUSE_BUTTON_DOUBLE");
            break;
        case kCutMessage:
            fCommand->SetText("SYSMSG_CUT");
            break;
        case kCopyMessage:
            fCommand->SetText("SYSMSG_COPY");
            break;
        case kPasteMessage:
            fCommand->SetText("SYSMSG_PASTE");
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


