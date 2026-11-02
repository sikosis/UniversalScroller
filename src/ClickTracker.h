#ifndef UNIVERSAL_SCROLLER_CLICK_TRACKER_H
#define UNIVERSAL_SCROLLER_CLICK_TRACKER_H

#include "Command.h"

#include <array>
#include <cstdint>
#include <vector>

struct ClickEvent {
    bool down;
    MouseButton button;
    std::uint32_t clickCount;
};

class ClickTracker {
public:
    std::vector<ClickEvent> BuildSequence(MouseButton button, std::uint32_t requestedClicks,
        std::int64_t timestamp, std::int64_t maximumInterval);
    void Reset();

private:
    static std::size_t ButtonIndex(MouseButton button);

    std::array<std::int64_t, 3> fLastClickTime {};
    std::array<std::uint32_t, 3> fAccumulatedClicks {};
};

#endif
