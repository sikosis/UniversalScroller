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
    Check(Command("KEY_1_65_1_65_A").Kind() == CommandKind::key, "key command");
    Check(Command("key_shift_1_65_1_65_A").Kind() == CommandKind::key,
        "case-insensitive modified key command");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestExecutableFallbacks()
{
    Check(Command("StyledEdit").Kind() == CommandKind::executable, "application command");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestInvalidReservedCommands()
{
    Check(!Command("").IsValid(), "empty command is invalid");
    Check(!Command("KEY_").IsValid(), "empty key command is invalid");
    Check(!Command("KEY_A").IsValid(), "incomplete key command is invalid");
    Check(!Command("KEY_1_65_4_65_66_67_68_A").IsValid(), "oversized key byte sequence is invalid");
    Check(!Command("KEY_1_65_1_999_A").IsValid(), "out-of-range key byte is invalid");
    Check(!Command("SYSMSG_UNKNOWN").IsValid(), "unknown system command is invalid");
    Check(!Command("PRIMARY_MOUSE_BUTTON0").IsValid(), "zero click count is invalid");
    Check(!Command("PRIMARY_MOUSE_BUTTON_TWO").IsValid(), "invalid click suffix is invalid");
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

int main()
{
    TestMouseButtons();
    TestSimpleCommands();
    TestExecutableFallbacks();
    TestInvalidReservedCommands();

    std::cout << "All command tests passed.\n";
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


