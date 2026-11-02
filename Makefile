CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror

BUILD_DIRECTORY := build
TEST_BINARIES := $(BUILD_DIRECTORY)/command_test $(BUILD_DIRECTORY)/configuration_test

.PHONY: all clean test

all: test

$(BUILD_DIRECTORY)/command_test: src/Command.cpp src/Command.h tests/CommandTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/Command.cpp tests/CommandTest.cpp -o $@

$(BUILD_DIRECTORY)/configuration_test: src/Command.cpp src/Command.h src/Configuration.cpp src/Configuration.h tests/ConfigurationTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/Command.cpp src/Configuration.cpp tests/ConfigurationTest.cpp -o $@

test: $(TEST_BINARIES)
	$(BUILD_DIRECTORY)/command_test
	$(BUILD_DIRECTORY)/configuration_test

clean:
	rm -f $(TEST_BINARIES) $(BUILD_DIRECTORY)/configuration-test.bin $(BUILD_DIRECTORY)/truncated-configuration.bin
	rmdir $(BUILD_DIRECTORY) 2>/dev/null || true
