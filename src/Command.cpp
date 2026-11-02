#include "Command.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string_view>
#include <utility>

namespace {

bool EqualsIgnoringCase(std::string_view left, std::string_view right)
{
    if (left.size() != right.size()) {
        return false;
    }

    return std::equal(left.begin(), left.end(), right.begin(),
        [](unsigned char leftCharacter, unsigned char rightCharacter) {
            return std::toupper(leftCharacter) == std::toupper(rightCharacter);
        });
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool StartsWithIgnoringCase(std::string_view text, std::string_view prefix)
{
    return text.size() >= prefix.size()
        && EqualsIgnoringCase(text.substr(0, prefix.size()), prefix);
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

Command::Command(std::string text)
    : fText(std::move(text))
{
    Parse();
}
//---------------------------------------------------------------------------------------------------------------------------------//


CommandKind Command::Kind() const
{
    return fKind;
}
//---------------------------------------------------------------------------------------------------------------------------------//


MouseButton Command::Button() const
{
    return fButton;
}
//---------------------------------------------------------------------------------------------------------------------------------//


unsigned int Command::ClickCount() const
{
    return fClickCount;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const std::string& Command::Text() const
{
    return fText;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Command::Parse()
{
    if (ParseButton("PRIMARY_MOUSE_BUTTON_DOUBLE", MouseButton::primary, 2)
        || ParseButton("SECONDARY_MOUSE_BUTTON_DOUBLE", MouseButton::secondary, 2)
        || ParseButton("TERTIARY_MOUSE_BUTTON_DOUBLE", MouseButton::tertiary, 2)
        || ParseButton("PRIMARY_MOUSE_BUTTON", MouseButton::primary, 1)
        || ParseButton("SECONDARY_MOUSE_BUTTON", MouseButton::secondary, 1)
        || ParseButton("TERTIARY_MOUSE_BUTTON", MouseButton::tertiary, 1)) {
        return;
    }

    if (EqualsIgnoringCase(fText, "SYSMSG_CUT")) {
        fKind = CommandKind::cut;
    } else if (EqualsIgnoringCase(fText, "SYSMSG_COPY")) {
        fKind = CommandKind::copy;
    } else if (EqualsIgnoringCase(fText, "SYSMSG_PASTE")) {
        fKind = CommandKind::paste;
    } else if (StartsWithIgnoringCase(fText, "KEY_") && fText.size() > 4) {
        fKind = CommandKind::key;
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Command::ParseButton(const char* name, MouseButton button, unsigned int clickCount)
{
    const std::string_view commandName(name);
    if (!StartsWithIgnoringCase(fText, commandName)) {
        return false;
    }

    const std::string_view suffix = std::string_view(fText).substr(commandName.size());
    unsigned int multiplier = 1;
    if (!suffix.empty()) {
        multiplier = 0;
        for (unsigned char character : suffix) {
            if (!std::isdigit(character)) {
                return false;
            }

            const unsigned int digit = character - '0';
            if (multiplier > (std::numeric_limits<unsigned int>::max() - digit) / 10) {
                return false;
            }
            multiplier = multiplier * 10 + digit;
        }

        if (multiplier == 0) {
            return false;
        }
    }

    if (clickCount > std::numeric_limits<unsigned int>::max() / multiplier) {
        return false;
    }

    fKind = CommandKind::button;
    fButton = button;
    fClickCount = clickCount * multiplier;
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


