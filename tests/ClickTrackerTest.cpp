#include "ClickTracker.h"

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


void TestDoubleClickSequence()
{
    ClickTracker tracker;
    const std::vector<ClickEvent> events = tracker.BuildSequence(
        MouseButton::primary, 2, 1000000, 250000);
    Check(events.size() == 3, "double click emits down, up, down");
    Check(events[0].down && events[0].clickCount == 1, "first click down");
    Check(!events[1].down && events[1].clickCount == 1, "intermediate click up");
    Check(events[2].down && events[2].clickCount == 2, "second click down");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestAccumulationWithinInterval()
{
    ClickTracker tracker;
    tracker.BuildSequence(MouseButton::secondary, 1, 1000000, 250000);
    const std::vector<ClickEvent> events = tracker.BuildSequence(
        MouseButton::secondary, 1, 1200000, 250000);
    Check(events.size() == 1 && events[0].clickCount == 2,
        "nearby click continues accumulation");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestResetAfterInterval()
{
    ClickTracker tracker;
    tracker.BuildSequence(MouseButton::tertiary, 1, 1000000, 250000);
    const std::vector<ClickEvent> events = tracker.BuildSequence(
        MouseButton::tertiary, 1, 1300000, 250000);
    Check(events.size() == 1 && events[0].clickCount == 1,
        "late click starts a new sequence");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestButtonsAccumulateIndependently()
{
    ClickTracker tracker;
    tracker.BuildSequence(MouseButton::primary, 1, 1000000, 250000);
    const std::vector<ClickEvent> events = tracker.BuildSequence(
        MouseButton::secondary, 1, 1100000, 250000);
    Check(events[0].clickCount == 1, "buttons have independent accumulators");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestSequenceLengthIsBounded()
{
    ClickTracker tracker;
    const std::vector<ClickEvent> events = tracker.BuildSequence(
        MouseButton::primary, 1000000, 1000000, 250000);
    Check(events.size() == 63, "synthetic click sequence is bounded");
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

int main()
{
    TestDoubleClickSequence();
    TestAccumulationWithinInterval();
    TestResetAfterInterval();
    TestButtonsAccumulateIndependently();
    TestSequenceLengthIsBounded();

    std::cout << "All click tracker tests passed.\n";
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


