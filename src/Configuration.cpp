#include "Configuration.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <sys/stat.h>
#include <type_traits>
#include <unistd.h>
#include <utility>

namespace {

constexpr std::uint32_t kConfigurationVersion = 1;
constexpr std::size_t kLegacyCommandLength = 250;

template<typename Value>
bool ReadValue(std::istream& stream, Value& value)
{
    static_assert(std::is_trivially_copyable<Value>::value, "configuration values must be byte-copyable");
    stream.read(reinterpret_cast<char*>(&value), sizeof(value));
    return stream.good();
}
//---------------------------------------------------------------------------------------------------------------------------------//


template<typename Value>
bool WriteValue(std::ostream& stream, const Value& value)
{
    static_assert(std::is_trivially_copyable<Value>::value, "configuration values must be byte-copyable");
    stream.write(reinterpret_cast<const char*>(&value), sizeof(value));
    return stream.good();
}
//---------------------------------------------------------------------------------------------------------------------------------//


} // namespace

Configuration::Configuration()
    : fButtonCommands {
        Command("PRIMARY_MOUSE_BUTTON"), Command("beep"), Command("beep"),
        Command("SECONDARY_MOUSE_BUTTON"), Command("beep"), Command("beep"),
        Command("TERTIARY_MOUSE_BUTTON"), Command("SYSMSG_COPY"), Command("SYSMSG_PASTE")
    },
    fSwallowClicks {false, true, true, false, true, true, false, true, true},
    fScrollWhenMouseDown {false, false, false, true, false, true, true, true}
{
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::string Configuration::DefaultPath()
{
    const char* home = std::getenv("HOME");
    if (home != nullptr && home[0] != '\0') {
        return std::string(home) + "/.universalscroller";
    }
    return "/boot/home/.universalscroller";
}
//---------------------------------------------------------------------------------------------------------------------------------//


int Configuration::ButtonDownIndex(std::uint32_t previousButtons, std::uint32_t currentButtons)
{
    constexpr std::array<std::array<std::uint32_t, 2>, kButtonCommandCount> transitions {{
        {{0, 1}}, {{1, 3}}, {{1, 5}},
        {{0, 2}}, {{2, 3}}, {{2, 6}},
        {{0, 4}}, {{4, 5}}, {{4, 6}}
    }};

    for (std::size_t index = 0; index < transitions.size(); ++index) {
        if (transitions[index][0] == previousButtons && transitions[index][1] == currentButtons) {
            return static_cast<int>(index);
        }
    }
    return -1;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::Load(const std::string& path)
{
    std::ifstream stream(path, std::ios::binary);
    if (!stream.is_open()) {
        return false;
    }

    Configuration loaded;
    std::uint32_t version = 0;
    if (!ReadValue(stream, version) || version != kConfigurationVersion
        || !ReadValue(stream, loaded.fMinimumScroll)) {
        return false;
    }

    for (std::size_t index = 0; index < 2; ++index) {
        if (!ReadValue(stream, loaded.fDoubleClickSpeed[index])) {
            return false;
        }
    }

    for (std::size_t index = 0; index < loaded.fButtonCommands.size(); ++index) {
        std::array<char, kLegacyCommandLength> commandBytes {};
        stream.read(commandBytes.data(), commandBytes.size());
        if (!stream.good()) {
            return false;
        }
        const auto terminator = std::find(commandBytes.begin(), commandBytes.end(), '\0');
        loaded.fButtonCommands[index] = Command(std::string(commandBytes.begin(), terminator));
    }

    for (std::size_t index = 0; index < loaded.fSwallowClicks.size(); ++index) {
        std::uint8_t value = 0;
        if (!ReadValue(stream, value)) {
            return false;
        }
        loaded.fSwallowClicks[index] = value != 0;
    }

    std::uint8_t useWheelFactors = 0;
    if (!ReadValue(stream, useWheelFactors)) {
        return false;
    }
    loaded.fUseWheelFactors = useWheelFactors != 0;

    if (!ReadValue(stream, loaded.fWheelFactorsX[0])
        || !ReadValue(stream, loaded.fWheelFactorsY[0])
        || !ReadValue(stream, loaded.fWheelFactorsX[1])
        || !ReadValue(stream, loaded.fWheelFactorsY[1])) {
        return false;
    }

    for (std::size_t index = 0; index < loaded.fScrollWhenMouseDown.size(); ++index) {
        std::uint8_t value = 0;
        if (!ReadValue(stream, value)) {
            return false;
        }
        loaded.fScrollWhenMouseDown[index] = value != 0;
    }

    if (!loaded.IsValid()) {
        return false;
    }
    *this = std::move(loaded);
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::Save(const std::string& path) const
{
    if (!IsValid()) {
        return false;
    }
    const std::string temporaryPath = path + ".tmp." + std::to_string(getpid());
    if (!WriteFile(temporaryPath)) {
        std::remove(temporaryPath.c_str());
        return false;
    }
    if (chmod(temporaryPath.c_str(), S_IRUSR | S_IWUSR) != 0
        || std::rename(temporaryPath.c_str(), path.c_str()) != 0) {
        std::remove(temporaryPath.c_str());
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::IsValid() const
{
    if (fMinimumScroll < 0) {
        return false;
    }
    for (std::int32_t speed : fDoubleClickSpeed) {
        if (speed <= 0) {
            return false;
        }
    }
    for (const Command& command : fButtonCommands) {
        if (!command.IsValid()) {
            return false;
        }
    }
    for (std::size_t index = 0; index < kSpeedCount; ++index) {
        if (!std::isfinite(fWheelFactorsX[index]) || !std::isfinite(fWheelFactorsY[index])) {
            return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::WriteFile(const std::string& path) const
{
    std::ofstream stream(path, std::ios::binary | std::ios::trunc);
    if (!stream.is_open() || !WriteValue(stream, kConfigurationVersion)
        || !WriteValue(stream, fMinimumScroll)) {
        return false;
    }

    for (std::size_t index = 0; index < 2; ++index) {
        if (!WriteValue(stream, fDoubleClickSpeed[index])) {
            return false;
        }
    }

    for (const Command& command : fButtonCommands) {
        std::array<char, kLegacyCommandLength> commandBytes {};
        const std::size_t length = std::min(command.Text().size(), commandBytes.size() - 1);
        std::memcpy(commandBytes.data(), command.Text().data(), length);
        stream.write(commandBytes.data(), commandBytes.size());
        if (!stream.good()) {
            return false;
        }
    }

    for (bool swallowClick : fSwallowClicks) {
        const std::uint8_t value = swallowClick ? 1 : 0;
        if (!WriteValue(stream, value)) {
            return false;
        }
    }

    const std::uint8_t useWheelFactors = fUseWheelFactors ? 1 : 0;
    if (!WriteValue(stream, useWheelFactors)
        || !WriteValue(stream, fWheelFactorsX[0])
        || !WriteValue(stream, fWheelFactorsY[0])
        || !WriteValue(stream, fWheelFactorsX[1])
        || !WriteValue(stream, fWheelFactorsY[1])) {
        return false;
    }

    for (bool scroll : fScrollWhenMouseDown) {
        const std::uint8_t value = scroll ? 1 : 0;
        if (!WriteValue(stream, value)) {
            return false;
        }
    }
    stream.flush();
    if (!stream.good()) {
        return false;
    }
    stream.close();
    return stream.good();
}
//---------------------------------------------------------------------------------------------------------------------------------//


std::int32_t Configuration::MinimumScroll() const
{
    return fMinimumScroll;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetMinimumScroll(std::int32_t minimumScroll)
{
    fMinimumScroll = minimumScroll;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::UseWheelFactors() const
{
    return fUseWheelFactors;
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetUseWheelFactors(bool useWheelFactors)
{
    fUseWheelFactors = useWheelFactors;
}
//---------------------------------------------------------------------------------------------------------------------------------//



std::int32_t Configuration::DoubleClickSpeed(std::size_t button) const
{
    return fDoubleClickSpeed.at(button);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetDoubleClickSpeed(std::size_t button, std::int32_t speed)
{
    fDoubleClickSpeed.at(button) = speed;
}
//---------------------------------------------------------------------------------------------------------------------------------//


const Command& Configuration::ButtonCommand(std::size_t index) const
{
    return fButtonCommands.at(index);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetButtonCommand(std::size_t index, const std::string& text)
{
    fButtonCommands.at(index) = Command(text);
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::SwallowClick(std::size_t index) const
{
    return fSwallowClicks.at(index);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetSwallowClick(std::size_t index, bool swallow)
{
    fSwallowClicks.at(index) = swallow;
}
//---------------------------------------------------------------------------------------------------------------------------------//


float Configuration::WheelFactorX(std::size_t speed) const
{
    return fWheelFactorsX.at(speed);
}
//---------------------------------------------------------------------------------------------------------------------------------//


float Configuration::WheelFactorY(std::size_t speed) const
{
    return fWheelFactorsY.at(speed);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetWheelFactors(std::size_t speed, float x, float y)
{
    fWheelFactorsX.at(speed) = x;
    fWheelFactorsY.at(speed) = y;
}
//---------------------------------------------------------------------------------------------------------------------------------//


bool Configuration::ScrollWhenMouseDown(std::size_t buttons) const
{
    return fScrollWhenMouseDown.at(buttons);
}
//---------------------------------------------------------------------------------------------------------------------------------//


void Configuration::SetScrollWhenMouseDown(std::size_t buttons, bool scroll)
{
    fScrollWhenMouseDown.at(buttons) = scroll;
}
//---------------------------------------------------------------------------------------------------------------------------------//


