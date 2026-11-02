CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror

BUILD_DIRECTORY := build
TEST_BINARIES := $(BUILD_DIRECTORY)/command_test $(BUILD_DIRECTORY)/configuration_test $(BUILD_DIRECTORY)/click_tracker_test

.PHONY: all clean install platform-clean test uninstall

all:
	$(MAKE) -C src/haiku/filter
	$(MAKE) -C src/haiku/preferences

install: test
	$(MAKE) -C src/haiku/filter user-install
	$(MAKE) -C src/haiku/preferences user-install
	/system/servers/input_server -q

uninstall:
	$(MAKE) -C src/haiku/filter user-uninstall
	$(MAKE) -C src/haiku/preferences user-uninstall
	/system/servers/input_server -q

$(BUILD_DIRECTORY)/command_test: src/Command.cpp src/Command.h tests/CommandTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/Command.cpp tests/CommandTest.cpp -o $@

$(BUILD_DIRECTORY)/configuration_test: src/Command.cpp src/Command.h src/Configuration.cpp src/Configuration.h tests/ConfigurationTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/Command.cpp src/Configuration.cpp tests/ConfigurationTest.cpp -o $@

$(BUILD_DIRECTORY)/click_tracker_test: src/ClickTracker.cpp src/ClickTracker.h src/Command.h tests/ClickTrackerTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/ClickTracker.cpp tests/ClickTrackerTest.cpp -o $@

test: $(TEST_BINARIES)
	$(BUILD_DIRECTORY)/command_test
	$(BUILD_DIRECTORY)/configuration_test
	$(BUILD_DIRECTORY)/click_tracker_test

clean:
	rm -f $(TEST_BINARIES) $(BUILD_DIRECTORY)/configuration-test.bin $(BUILD_DIRECTORY)/invalid-configuration.bin $(BUILD_DIRECTORY)/invalid-loaded-configuration.bin $(BUILD_DIRECTORY)/preserved-configuration.bin $(BUILD_DIRECTORY)/truncated-configuration.bin
	rmdir $(BUILD_DIRECTORY) 2>/dev/null || true

ifeq ($(shell uname -s),Haiku)
clean: platform-clean
endif

platform-clean:
	$(MAKE) -C src/haiku/filter clean
	$(MAKE) -C src/haiku/preferences clean
	rm -f src/haiku/filter/_APP_
