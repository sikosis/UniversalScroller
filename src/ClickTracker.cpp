#include "ClickTracker.h"

#include <algorithm>
#include <stdexcept>

namespace {

constexpr std::uint32_t kMaximumSyntheticClicks = 32;

} // namespace

std::vector<ClickEvent> ClickTracker::BuildSequence(MouseButton button,
    std::uint32_t requestedClicks, std::int64_t timestamp, std::int64_t maximumInterval)
{
    std::vector<ClickEvent> events;
    if (requestedClicks == 0) {
        return events;
    }
    requestedClicks = std::min(requestedClicks, kMaximumSyntheticClicks);

    const std::size_t index = ButtonIndex(button);
    if (timestamp < fLastClickTime[index]
        || timestamp - fLastClickTime[index] > maximumInterval) {
        fAccumulatedClicks[index] = 0;
    }
    fLastClickTime[index] = timestamp;

    events.reserve(requestedClicks * 2 - 1);
    for (std::uint32_t click = 0; click < requestedClicks; ++click) {
        ++fAccumulatedClicks[index];
        events.push_back({true, button, fAccumulatedClicks[index]});
        if (click + 1 < requestedClicks) {
            events.push_back({false, button, fAccumulatedClicks[index]});
        }
    }
    return events;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void ClickTracker::Reset()
{
    fLastClickTime.fill(0);
    fAccumulatedClicks.fill(0);
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::size_t ClickTracker::ButtonIndex(MouseButton button)
{
    switch (button) {
        case MouseButton::primary:
            return 0;
        case MouseButton::secondary:
            return 1;
        case MouseButton::tertiary:
            return 2;
        case MouseButton::none:
            throw std::invalid_argument("a click sequence requires a mouse button");
    }
    throw std::invalid_argument("unknown mouse button");
}
//---------------------------------------------------------------------------------------------------------------------------------//


