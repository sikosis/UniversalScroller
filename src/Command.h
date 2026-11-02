#ifndef UNIVERSAL_SCROLLER_COMMAND_H
#define UNIVERSAL_SCROLLER_COMMAND_H

#include <cstdint>
#include <string>

enum class CommandKind {
    invalid,
    button,
    key,
    cut,
    copy,
    paste,
    executable
};

enum class MouseButton : std::int8_t {
    none = -1,
    primary,
    secondary,
    tertiary
};

class Command {
public:
    explicit Command(std::string text);

    CommandKind Kind() const;
    MouseButton Button() const;
    unsigned int ClickCount() const;
    const std::string& Text() const;
    bool IsValid() const;

private:
    void Parse();
    bool ParseButton(const char* name, MouseButton button, unsigned int clickCount);

    std::string fText;
    CommandKind fKind = CommandKind::executable;
    MouseButton fButton = MouseButton::none;
    unsigned int fClickCount = 0;
};

#endif
