#include "PreferencesApplication.h"

#include "PreferencesWindow.h"

PreferencesApplication::PreferencesApplication()
    : BApplication("application/x-vnd.UniversalScroller-Preferences")
{
    (new PreferencesWindow())->Show();
}
//---------------------------------------------------------------------------------------------------------------------------------//


