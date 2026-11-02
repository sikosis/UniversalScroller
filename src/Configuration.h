#ifndef UNIVERSAL_SCROLLER_CONFIGURATION_H
#define UNIVERSAL_SCROLLER_CONFIGURATION_H

#include "Command.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

class Configuration {
public:
    static constexpr std::size_t kButtonCommandCount = 9;
    static constexpr std::size_t kButtonCombinationCount = 8;
    static constexpr std::size_t kSpeedCount = 2;

    Configuration();

    static std::string DefaultPath();
    static int ButtonDownIndex(std::uint32_t previousButtons, std::uint32_t currentButtons);

    bool Load(const std::string& path);
    bool Save(const std::string& path) const;

    std::int32_t MinimumScroll() const;
    void SetMinimumScroll(std::int32_t minimumScroll);
    std::int32_t DoubleClickSpeed(std::size_t button) const;
    void SetDoubleClickSpeed(std::size_t button, std::int32_t speed);
    const Command& ButtonCommand(std::size_t index) const;
    void SetButtonCommand(std::size_t index, const std::string& text);
    bool SwallowClick(std::size_t index) const;
    void SetSwallowClick(std::size_t index, bool swallow);
    float WheelFactorX(std::size_t speed) const;
    float WheelFactorY(std::size_t speed) const;
    void SetWheelFactors(std::size_t speed, float x, float y);
    bool UseWheelFactors() const;
    void SetUseWheelFactors(bool useWheelFactors);
    bool ScrollWhenMouseDown(std::size_t buttons) const;
    void SetScrollWhenMouseDown(std::size_t buttons, bool scroll);

private:
    bool WriteFile(const std::string& path) const;

    std::int32_t fMinimumScroll = 0;
    std::array<std::int32_t, 3> fDoubleClickSpeed {250000, 250000, 250000};
    std::array<Command, kButtonCommandCount> fButtonCommands;
    std::array<bool, kButtonCommandCount> fSwallowClicks;
    std::array<float, kSpeedCount> fWheelFactorsX {1.0F, 10.0F};
    std::array<float, kSpeedCount> fWheelFactorsY {1.0F, 10.0F};
    bool fUseWheelFactors = true;
    std::array<bool, kButtonCombinationCount> fScrollWhenMouseDown;
};

#endif
