#include "UniversalScroller.h"

#include <InputServerFilter.h>

extern "C" _EXPORT BInputServerFilter* instantiate_input_filter();

BInputServerFilter* instantiate_input_filter()
{
    return new UniversalScroller();
}
//---------------------------------------------------------------------------------------------------------------------------------//


