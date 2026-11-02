#include "Configuration.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
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


void TestDefaults()
{
    const Configuration configuration;
    Check(configuration.MinimumScroll() == 0, "default minimum scroll");
    Check(configuration.ButtonCommand(0).Text() == "PRIMARY_MOUSE_BUTTON", "default primary command");
    Check(!configuration.SwallowClick(0), "simple primary click is not swallowed");
    Check(configuration.SwallowClick(1), "primary inter-click is swallowed");
    Check(configuration.ScrollWhenMouseDown(3), "two-button scrolling enabled");
    Check(!configuration.ScrollWhenMouseDown(1), "single-button scrolling disabled");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestButtonDownIndices()
{
    Check(Configuration::ButtonDownIndex(0, 1) == 0, "primary transition");
    Check(Configuration::ButtonDownIndex(1, 5) == 2, "primary-tertiary transition");
    Check(Configuration::ButtonDownIndex(4, 6) == 8, "tertiary-secondary transition");
    Check(Configuration::ButtonDownIndex(3, 7) == -1, "unknown transition");
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestLegacyRoundTrip()
{
    const std::string path = "build/configuration-test.bin";
    Configuration saved;
    saved.SetMinimumScroll(42);
    saved.SetDoubleClickSpeed(1, 345678);
    saved.SetButtonCommand(2, "StyledEdit");
    saved.SetSwallowClick(2, false);
    saved.SetUseWheelFactors(false);
    saved.SetWheelFactors(1, 3.5F, 4.5F);
    saved.SetScrollWhenMouseDown(7, false);
    Check(saved.Save(path), "save legacy configuration");

    Configuration loaded;
    Check(loaded.Load(path), "load legacy configuration");
    Check(loaded.MinimumScroll() == 42, "round-trip minimum scroll");
    Check(loaded.DoubleClickSpeed(1) == 345678, "round-trip double-click speed");
    Check(loaded.ButtonCommand(2).Text() == "StyledEdit", "round-trip command");
    Check(!loaded.SwallowClick(2), "round-trip swallow setting");
    Check(!loaded.UseWheelFactors(), "round-trip wheel setting");
    Check(loaded.WheelFactorX(1) == 3.5F && loaded.WheelFactorY(1) == 4.5F,
        "round-trip wheel factors");
    Check(!loaded.ScrollWhenMouseDown(7), "round-trip scroll combination");
    std::remove(path.c_str());
}
//---------------------------------------------------------------------------------------------------------------------------------//


void TestTruncatedFileDoesNotChangeConfiguration()
{
    const std::string path = "build/truncated-configuration.bin";
    {
        std::ofstream stream(path, std::ios::binary);
        stream.write("bad", 3);
    }

    Configuration configuration;
    configuration.SetMinimumScroll(99);
    Check(!configuration.Load(path), "reject truncated configuration");
    Check(configuration.MinimumScroll() == 99, "failed load preserves current configuration");
    std::remove(path.c_str());
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

int main()
{
    TestDefaults();
    TestButtonDownIndices();
    TestLegacyRoundTrip();
    TestTruncatedFileDoesNotChangeConfiguration();

    std::cout << "All configuration tests passed.\n";
    return 0;
}
//---------------------------------------------------------------------------------------------------------------------------------//


