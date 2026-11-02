#include "Command.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

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


bool ParseUnsigned(std::string_view text, unsigned int& value)
{
    if (text.empty()) {
        return false;
    }
    value = 0;
    for (unsigned char character : text) {
        if (!std::isdigit(character)) {
            return false;
        }
        const unsigned int digit = character - '0';
        if (value > (std::numeric_limits<unsigned int>::max() - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::vector<std::string_view> Split(std::string_view text)
{
    std::vector<std::string_view> parts;
    while (true) {
        const std::size_t separator = text.find('_');
        parts.push_back(text.substr(0, separator));
        if (separator == std::string_view::npos) {
            break;
        }
        text.remove_prefix(separator + 1);
    }
    return parts;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool IsValidKeyCommand(std::string_view text)
{
    std::vector<std::string_view> parts = Split(text.substr(4));
    std::size_t index = 0;
    while (index < parts.size()
        && (EqualsIgnoringCase(parts[index], "SHIFT")
            || EqualsIgnoringCase(parts[index], "OPTION")
            || EqualsIgnoringCase(parts[index], "CONTROL"))) {
        ++index;
    }
    if (parts.size() - index < 4) {
        return false;
    }

    unsigned int key = 0;
    unsigned int rawCharacter = 0;
    unsigned int byteCount = 0;
    if (!ParseUnsigned(parts[index++], key)
        || !ParseUnsigned(parts[index++], rawCharacter)
        || !ParseUnsigned(parts[index++], byteCount)
        || byteCount > 3
        || parts.size() - index < byteCount + 1) {
        return false;
    }
    for (unsigned int byte = 0; byte < byteCount; ++byte) {
        unsigned int value = 0;
        if (!ParseUnsigned(parts[index++], value) || value > 255) {
            return false;
        }
    }
    return !parts[index].empty();
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


bool Command::IsValid() const
{
    return fKind != CommandKind::invalid;
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
    } else if (StartsWithIgnoringCase(fText, "KEY_")) {
        fKind = IsValidKeyCommand(fText) ? CommandKind::key : CommandKind::invalid;
    } else if (fText.empty()
        || StartsWithIgnoringCase(fText, "SYSMSG_")
        || StartsWithIgnoringCase(fText, "PRIMARY_MOUSE_BUTTON")
        || StartsWithIgnoringCase(fText, "SECONDARY_MOUSE_BUTTON")
        || StartsWithIgnoringCase(fText, "TERTIARY_MOUSE_BUTTON")) {
        fKind = CommandKind::invalid;
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


