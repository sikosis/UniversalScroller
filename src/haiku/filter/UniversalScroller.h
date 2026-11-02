#ifndef UNIVERSAL_SCROLLER_FILTER_H
#define UNIVERSAL_SCROLLER_FILTER_H

#include "Configuration.h"

#include <InputServerFilter.h>
#include <Point.h>
#include <SupportDefs.h>

class BList;
class BMessage;

class UniversalScroller : public BInputServerFilter {
public:
    UniversalScroller();

    status_t InitCheck() override;
    filter_result Filter(BMessage* message, BList* outList) override;

private:
    filter_result HandleMouseDown(BMessage* message, BList* outList);
    filter_result HandleMouseMoved(BMessage* message);
    filter_result HandleMouseWheel(BMessage* message);
    void AddCommandMessage(const Command& command, const BMessage& source, BList* outList);
    void AddKeyMessages(const Command& command, const BMessage& source, BList* outList);
    void LaunchCommand(const std::string& command);

    Configuration fConfiguration;
    std::uint32_t fPhysicalButtons = 0;
    std::uint32_t fModifiers = 0;
    BPoint fButtonDownPosition;
};

#endif
