#include "Command.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {

void Check(bool condition, const std::string& description)
{
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        std::exit(1);
    }
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestMouseButtons()
{
    const Command primary("PRIMARY_MOUSE_BUTTON");
    Check(primary.Kind() == CommandKind::button, "primary command kind");
    Check(primary.Button() == MouseButton::primary, "primary button");
    Check(primary.ClickCount() == 1, "primary click count");

    const Command doubleSecondary("secondary_mouse_button_double");
    Check(doubleSecondary.Button() == MouseButton::secondary, "case-insensitive secondary button");
    Check(doubleSecondary.ClickCount() == 2, "double-click count");

    const Command repeatedTertiary("TERTIARY_MOUSE_BUTTON_DOUBLE3");
    Check(repeatedTertiary.Button() == MouseButton::tertiary, "repeated tertiary button");
    Check(repeatedTertiary.ClickCount() == 6, "double-click multiplier");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestSimpleCommands()
{
    Check(Command("sysmsg_cut").Kind() == CommandKind::cut, "cut command");
    Check(Command("SYSMSG_COPY").Kind() == CommandKind::copy, "copy command");
    Check(Command("SYSMSG_PASTE").Kind() == CommandKind::paste, "paste command");
    Check(Command("KEY_A").Kind() == CommandKind::key, "key command");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestExecutableFallbacks()
{
    Check(Command("").Kind() == CommandKind::executable, "empty command fallback");
    Check(Command("KEY_").Kind() == CommandKind::executable, "empty key fallback");
    Check(Command("PRIMARY_MOUSE_BUTTON0").Kind() == CommandKind::executable,
        "zero click count fallback");
    Check(Command("PRIMARY_MOUSE_BUTTON_TWO").Kind() == CommandKind::executable,
        "invalid click suffix fallback");
    Check(Command("StyledEdit").Kind() == CommandKind::executable, "application command");
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

int main()
{
    TestMouseButtons();
    TestSimpleCommands();
    TestExecutableFallbacks();

    std::cout << "All command tests passed.\n";
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


