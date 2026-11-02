CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror

BUILD_DIRECTORY := build
TEST_BINARIES := $(BUILD_DIRECTORY)/command_test $(BUILD_DIRECTORY)/configuration_test $(BUILD_DIRECTORY)/click_tracker_test

.PHONY: all clean install platform-clean run test uninstall

all:
	$(MAKE) -C src/filter stage
	$(MAKE) -C src/preferences stage

run: all
	build/UniversalScrollerPreferences

install: test
	$(MAKE) -C src/filter user-install
	$(MAKE) -C src/preferences user-install
	/system/servers/input_server -q

uninstall:
	$(MAKE) -C src/filter user-uninstall
	$(MAKE) -C src/preferences user-uninstall
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
	rm -f $(TEST_BINARIES) $(BUILD_DIRECTORY)/UniversalScroller $(BUILD_DIRECTORY)/UniversalScrollerPreferences $(BUILD_DIRECTORY)/configuration-test.bin $(BUILD_DIRECTORY)/invalid-configuration.bin $(BUILD_DIRECTORY)/invalid-loaded-configuration.bin $(BUILD_DIRECTORY)/preserved-configuration.bin $(BUILD_DIRECTORY)/truncated-configuration.bin
	rmdir $(BUILD_DIRECTORY) 2>/dev/null || true

ifeq ($(shell uname -s),Haiku)
clean: platform-clean
endif

platform-clean:
	$(MAKE) -C src/filter clean
	$(MAKE) -C src/preferences clean
	rm -f src/filter/_APP_
