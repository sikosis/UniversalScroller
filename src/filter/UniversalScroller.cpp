#include "UniversalScroller.h"

#include <InterfaceDefs.h>
#include <List.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <sstream>
#include <sys/stat.h>
#include <utility>
#include <vector>

namespace {

constexpr std::uint32_t kMouseButtonMask = 7;

std::int32_t ButtonMask(MouseButton button)
{
    switch (button) {
        case MouseButton::primary:
            return B_PRIMARY_MOUSE_BUTTON;
        case MouseButton::secondary:
            return B_SECONDARY_MOUSE_BUTTON;
        case MouseButton::tertiary:
            return B_TERTIARY_MOUSE_BUTTON;
        case MouseButton::none:
            return 0;
    }
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


int32 LaunchCommandThread(void* data)
{
    std::unique_ptr<std::string> command(static_cast<std::string*>(data));
    return std::system(command->c_str());
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::size_t ButtonIndex(MouseButton button)
{
    switch (button) {
        case MouseButton::primary:
            return 0;
        case MouseButton::secondary:
            return 1;
        case MouseButton::tertiary:
            return 2;
        case MouseButton::none:
            return 0;
    }
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string Uppercase(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char character) {
            return static_cast<char>(std::toupper(character));
        });
    return value;
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

UniversalScroller::UniversalScroller()
{
    fConfiguration.Load(Configuration::DefaultPath());
    struct stat status {};
    if (stat(Configuration::DefaultPath().c_str(), &status) == 0) {
        fConfigurationNode = static_cast<std::uint64_t>(status.st_ino);
        fConfigurationModifiedTime = static_cast<std::int64_t>(status.st_mtime);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t UniversalScroller::InitCheck()
{
    return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::Filter(BMessage* message, BList* outList)
{
    ReloadConfigurationIfNeeded();
    if (message == nullptr || outList == nullptr) {
        return B_DISPATCH_MESSAGE;
    }

    switch (message->what) {
        case B_MODIFIERS_CHANGED: {
            int32 modifiers = 0;
            if (message->FindInt32("modifiers", &modifiers) == B_OK) {
                fModifiers = static_cast<std::uint32_t>(modifiers);
            }
            if ((fModifiers & B_OPTION_KEY) != 0) {
                ReleaseVirtualButtons(outList);
                fPhysicalButtons = 0;
                fClickTracker.Reset();
            }
            return B_DISPATCH_MESSAGE;
        }

        case B_MOUSE_DOWN:
            return HandleMouseDown(message, outList);

        case B_MOUSE_UP:
            return HandleMouseUp(message);

        case B_MOUSE_MOVED:
            return HandleMouseMoved(message);

        case B_MOUSE_WHEEL_CHANGED:
            return HandleMouseWheel(message);

        default:
            return B_DISPATCH_MESSAGE;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::ReloadConfigurationIfNeeded()
{
    const std::int64_t now = system_time();
    if (now < fNextConfigurationCheck) {
        return;
    }
    fNextConfigurationCheck = now + 1000000;

    struct stat status {};
    const std::string path = Configuration::DefaultPath();
    if (stat(path.c_str(), &status) != 0) {
        return;
    }
    const std::uint64_t node = static_cast<std::uint64_t>(status.st_ino);
    const std::int64_t modifiedTime = static_cast<std::int64_t>(status.st_mtime);
    if (node == fConfigurationNode && modifiedTime == fConfigurationModifiedTime) {
        return;
    }

    Configuration updated;
    if (updated.Load(path)) {
        fConfiguration = std::move(updated);
        fConfigurationNode = node;
        fConfigurationModifiedTime = modifiedTime;
        fClickTracker.Reset();
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::HandleMouseDown(BMessage* message, BList* outList)
{
    int32 buttons = 0;
    message->FindPoint("where", &fButtonDownPosition);
    fPointerPosition = fButtonDownPosition;
    if (message->FindInt32("buttons", &buttons) != B_OK) {
        return B_DISPATCH_MESSAGE;
    }
    int32 modifiers = 0;
    if (message->FindInt32("modifiers", &modifiers) == B_OK) {
        fModifiers = static_cast<std::uint32_t>(modifiers);
    }

    const std::uint32_t previousButtons = fPhysicalButtons;
    const std::uint32_t currentButtons = static_cast<std::uint32_t>(buttons);
    const int commandIndex = Configuration::ButtonDownIndex(previousButtons, currentButtons);
    fPhysicalButtons = currentButtons;
    if ((fModifiers & B_OPTION_KEY) != 0 || commandIndex < 0) {
        fVirtualButtons = currentButtons;
        return B_DISPATCH_MESSAGE;
    }

    const std::size_t index = static_cast<std::size_t>(commandIndex);
    const Command& command = fConfiguration.ButtonCommand(index);
    if (command.Kind() == CommandKind::button) {
        const int32 mappedButtons = ButtonMask(command.Button());
        const std::uint32_t newlyPressedButtons = currentButtons & ~previousButtons;
        const bool canDispatchDirectly = command.ClickCount() == 1
            && static_cast<std::uint32_t>(mappedButtons) == newlyPressedButtons
            && !fConfiguration.SwallowClick(index);
        if (canDispatchDirectly) {
            fVirtualButtons = previousButtons | static_cast<std::uint32_t>(mappedButtons);
            message->ReplaceInt32("buttons", static_cast<int32>(fVirtualButtons));
            return B_DISPATCH_MESSAGE;
        }

        AddButtonMessages(command, *message, previousButtons, outList);
    } else {
        AddCommandMessage(command, *message, outList);
    }

    return fConfiguration.SwallowClick(index) ? B_SKIP_MESSAGE : B_DISPATCH_MESSAGE;
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::HandleMouseUp(BMessage* message)
{
    int32 buttons = 0;
    if (message->FindInt32("buttons", &buttons) != B_OK) {
        return B_DISPATCH_MESSAGE;
    }
    message->FindPoint("where", &fPointerPosition);

    fPhysicalButtons = static_cast<std::uint32_t>(buttons);
    if (fVirtualButtons != fPhysicalButtons) {
        message->ReplaceInt32("buttons", buttons);
        fVirtualButtons = fPhysicalButtons;
    }
    return B_DISPATCH_MESSAGE;
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::HandleMouseMoved(BMessage* message)
{
    if ((fModifiers & B_OPTION_KEY) != 0) {
        return B_DISPATCH_MESSAGE;
    }

    int32 buttons = 0;
    BPoint position;
    if (message->FindInt32("buttons", &buttons) != B_OK
        || message->FindPoint("where", &position) != B_OK) {
        return B_DISPATCH_MESSAGE;
    }
    fPointerPosition = position;

    const std::size_t combination = static_cast<std::uint32_t>(buttons) & kMouseButtonMask;
    if (!fConfiguration.ScrollWhenMouseDown(combination)) {
        return B_DISPATCH_MESSAGE;
    }

    const std::size_t speed = (fModifiers & B_CONTROL_KEY) != 0 ? 1 : 0;
    float deltaX = fConfiguration.WheelFactorX(speed) * (position.x - fButtonDownPosition.x);
    float deltaY = fConfiguration.WheelFactorY(speed) * (position.y - fButtonDownPosition.y);
    const float minimum = static_cast<float>(fConfiguration.MinimumScroll());
    if (deltaX * deltaX <= minimum && deltaY * deltaY <= minimum) {
        return B_SKIP_MESSAGE;
    }

    if ((fModifiers & B_SHIFT_KEY) != 0) {
        std::swap(deltaX, deltaY);
    }

    message->what = B_MOUSE_WHEEL_CHANGED;
    message->RemoveName("be:wheel_delta_x");
    message->RemoveName("be:wheel_delta_y");
    message->AddFloat("be:wheel_delta_x", deltaX);
    message->AddFloat("be:wheel_delta_y", deltaY);
    fButtonDownPosition = position;
    return B_DISPATCH_MESSAGE;
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::HandleMouseWheel(BMessage* message)
{
    if ((fModifiers & B_OPTION_KEY) != 0) {
        return B_DISPATCH_MESSAGE;
    }

    float deltaX = 0.0F;
    float deltaY = 0.0F;
    message->FindFloat("be:wheel_delta_x", &deltaX);
    message->FindFloat("be:wheel_delta_y", &deltaY);
    if ((fModifiers & B_SHIFT_KEY) != 0) {
        std::swap(deltaX, deltaY);
    }
    if (fConfiguration.UseWheelFactors()) {
        const std::size_t speed = (fModifiers & B_CONTROL_KEY) != 0 ? 1 : 0;
        deltaX *= fConfiguration.WheelFactorX(speed);
        deltaY *= fConfiguration.WheelFactorY(speed);
    }
    message->ReplaceFloat("be:wheel_delta_x", deltaX);
    message->ReplaceFloat("be:wheel_delta_y", deltaY);
    return B_DISPATCH_MESSAGE;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::AddCommandMessage(const Command& command, const BMessage& source, BList* outList)
{
    uint32 what = 0;
    switch (command.Kind()) {
        case CommandKind::invalid:
            return;
        case CommandKind::cut:
            what = 'CCUT';
            break;
        case CommandKind::copy:
            what = 'COPY';
            break;
        case CommandKind::paste:
            what = 'PSTE';
            break;
        case CommandKind::executable:
            LaunchCommand(command.Text());
            return;
        case CommandKind::key:
            AddKeyMessages(command, source, outList);
            return;
        case CommandKind::button:
            return;
    }

    BMessage* commandMessage = new BMessage(what);
    int64 when = system_time() - 5;
    source.FindInt64("when", &when);
    commandMessage->AddInt64("when", when);
    outList->AddItem(commandMessage);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::AddButtonMessages(const Command& command, const BMessage& source,
    std::uint32_t baseButtons, BList* outList)
{
    int64 when = system_time();
    source.FindInt64("when", &when);
    const std::size_t buttonIndex = ButtonIndex(command.Button());
    const std::vector<ClickEvent> events = fClickTracker.BuildSequence(command.Button(),
        command.ClickCount(), when, fConfiguration.DoubleClickSpeed(buttonIndex));
    const std::uint32_t mappedButton = static_cast<std::uint32_t>(ButtonMask(command.Button()));

    for (const ClickEvent& event : events) {
        BMessage* clickMessage = new BMessage(source);
        clickMessage->what = event.down ? B_MOUSE_DOWN : B_MOUSE_UP;
        const std::uint32_t buttons = event.down ? baseButtons | mappedButton : baseButtons;
        clickMessage->ReplaceInt32("buttons", static_cast<int32>(buttons));
        clickMessage->ReplaceInt32("clicks", static_cast<int32>(event.clickCount));
        clickMessage->ReplaceInt64("when", when++);
        outList->AddItem(clickMessage);
    }
    fVirtualButtons = baseButtons | mappedButton;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::AddKeyMessages(const Command& command, const BMessage& source, BList* outList)
{
    std::vector<std::string> parts;
    std::istringstream parser(command.Text());
    std::string part;
    while (std::getline(parser, part, '_')) {
        parts.push_back(part);
    }
    if (parts.size() < 5 || Uppercase(parts[0]) != "KEY") {
        return;
    }

    std::size_t index = 1;
    int32 modifiers = 0;
    while (index < parts.size()) {
        const std::string modifier = Uppercase(parts[index]);
        if (modifier == "SHIFT") {
            modifiers |= B_SHIFT_KEY;
        } else if (modifier == "OPTION") {
            modifiers |= B_OPTION_KEY;
        } else if (modifier == "CONTROL") {
            modifiers |= B_CONTROL_KEY;
        } else {
            break;
        }
        ++index;
    }
    if (parts.size() - index < 4) {
        return;
    }

    const int32 key = static_cast<int32>(std::strtol(parts[index++].c_str(), nullptr, 10));
    const int32 rawCharacter = static_cast<int32>(std::strtol(parts[index++].c_str(), nullptr, 10));
    std::size_t byteCount = static_cast<std::size_t>(std::strtoul(parts[index++].c_str(), nullptr, 10));
    byteCount = std::min<std::size_t>(byteCount, 3);
    if (parts.size() - index < byteCount + 1) {
        return;
    }

    std::vector<int8> bytes;
    for (std::size_t byteIndex = 0; byteIndex < byteCount; ++byteIndex) {
        bytes.push_back(static_cast<int8>(std::strtol(parts[index++].c_str(), nullptr, 10)));
    }
    std::string characters = parts[index++];
    while (index < parts.size()) {
        characters += "_" + parts[index++];
    }

    int32 physicalModifiers = 0;
    source.FindInt32("modifiers", &physicalModifiers);
    const int64 when = system_time();

    BMessage* modifiersDown = new BMessage(B_MODIFIERS_CHANGED);
    modifiersDown->AddInt64("when", when);
    modifiersDown->AddInt32("modifiers", modifiers);
    modifiersDown->AddInt32("be:old_modifiers", physicalModifiers);
    outList->AddItem(modifiersDown);

    for (uint32 what : {B_KEY_DOWN, B_KEY_UP}) {
        BMessage* keyMessage = new BMessage(what);
        keyMessage->AddInt64("when", when);
        keyMessage->AddInt32("modifiers", modifiers);
        keyMessage->AddInt32("key", key);
        keyMessage->AddInt32("raw_char", rawCharacter);
        for (int8 byte : bytes) {
            keyMessage->AddInt8("byte", byte);
        }
        keyMessage->AddString("bytes", characters.c_str());
        keyMessage->AddInt8("states", 0);
        outList->AddItem(keyMessage);
    }

    BMessage* modifiersUp = new BMessage(B_MODIFIERS_CHANGED);
    modifiersUp->AddInt64("when", when);
    modifiersUp->AddInt32("modifiers", physicalModifiers);
    modifiersUp->AddInt32("be:old_modifiers", modifiers);
    outList->AddItem(modifiersUp);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::LaunchCommand(const std::string& command)
{
    std::unique_ptr<std::string> copy(new std::string(command));
    const thread_id thread = spawn_thread(LaunchCommandThread, "UniversalScroller command",
        B_LOW_PRIORITY, copy.get());
    if (thread >= B_OK) {
        copy.release();
        resume_thread(thread);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


void UniversalScroller::ReleaseVirtualButtons(BList* outList)
{
    if (fVirtualButtons == 0) {
        return;
    }

    BMessage* release = new BMessage(B_MOUSE_UP);
    release->AddInt64("when", system_time());
    release->AddPoint("where", fPointerPosition);
    release->AddInt32("modifiers", static_cast<int32>(fModifiers));
    release->AddInt32("buttons", 0);
    outList->AddItem(release);
    fVirtualButtons = 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


