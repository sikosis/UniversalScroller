#include "UniversalScroller.h"

#include <InterfaceDefs.h>
#include <List.h>
#include <Message.h>
#include <OS.h>
#include <View.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <memory>
#include <sstream>
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


} // namespace

UniversalScroller::UniversalScroller()
{
    fConfiguration.Load(Configuration::DefaultPath());
}
//---------------------------------------------------------------------------------------------------------------------------------//


status_t UniversalScroller::InitCheck()
{
    return B_OK;
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::Filter(BMessage* message, BList* outList)
{
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
                fPhysicalButtons = 0;
            }
            return B_DISPATCH_MESSAGE;
        }

        case B_MOUSE_DOWN:
            return HandleMouseDown(message, outList);

        case B_MOUSE_UP: {
            int32 buttons = 0;
            if (message->FindInt32("buttons", &buttons) == B_OK) {
                fPhysicalButtons = static_cast<std::uint32_t>(buttons);
            }
            return B_DISPATCH_MESSAGE;
        }

        case B_MOUSE_MOVED:
            return HandleMouseMoved(message);

        case B_MOUSE_WHEEL_CHANGED:
            return HandleMouseWheel(message);

        default:
            return B_DISPATCH_MESSAGE;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


filter_result UniversalScroller::HandleMouseDown(BMessage* message, BList* outList)
{
    int32 buttons = 0;
    message->FindPoint("where", &fButtonDownPosition);
    if (message->FindInt32("buttons", &buttons) != B_OK) {
        return B_DISPATCH_MESSAGE;
    }
    int32 modifiers = 0;
    if (message->FindInt32("modifiers", &modifiers) == B_OK) {
        fModifiers = static_cast<std::uint32_t>(modifiers);
    }

    const std::uint32_t currentButtons = static_cast<std::uint32_t>(buttons);
    const int commandIndex = Configuration::ButtonDownIndex(fPhysicalButtons, currentButtons);
    fPhysicalButtons = currentButtons;
    if ((fModifiers & B_OPTION_KEY) != 0 || commandIndex < 0) {
        return B_DISPATCH_MESSAGE;
    }

    const std::size_t index = static_cast<std::size_t>(commandIndex);
    const Command& command = fConfiguration.ButtonCommand(index);
    AddCommandMessage(command, *message, outList);

    if (command.Kind() == CommandKind::button) {
        const int32 mappedButtons = ButtonMask(command.Button());
        if (fConfiguration.SwallowClick(index)) {
            BMessage* mappedMessage = new BMessage(*message);
            mappedMessage->ReplaceInt32("buttons", mappedButtons);
            mappedMessage->ReplaceInt32("clicks", static_cast<int32>(command.ClickCount()));
            outList->AddItem(mappedMessage);
        } else {
            message->ReplaceInt32("buttons", mappedButtons);
            message->ReplaceInt32("clicks", static_cast<int32>(command.ClickCount()));
        }
    }

    return fConfiguration.SwallowClick(index) ? B_SKIP_MESSAGE : B_DISPATCH_MESSAGE;
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


void UniversalScroller::AddKeyMessages(const Command& command, const BMessage& source, BList* outList)
{
    std::vector<std::string> parts;
    std::istringstream parser(command.Text());
    std::string part;
    while (std::getline(parser, part, '_')) {
        parts.push_back(part);
    }
    if (parts.size() < 5 || parts[0] != "KEY") {
        return;
    }

    std::size_t index = 1;
    int32 modifiers = 0;
    while (index < parts.size()) {
        if (parts[index] == "SHIFT") {
            modifiers |= B_SHIFT_KEY;
        } else if (parts[index] == "OPTION") {
            modifiers |= B_OPTION_KEY;
        } else if (parts[index] == "CONTROL") {
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


