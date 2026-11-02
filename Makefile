CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Wpedantic -Werror

BUILD_DIRECTORY := build
TEST_BINARY := $(BUILD_DIRECTORY)/command_test

.PHONY: all clean test

all: test

$(TEST_BINARY): src/Command.cpp src/Command.h tests/CommandTest.cpp
	mkdir -p $(BUILD_DIRECTORY)
	$(CXX) $(CXXFLAGS) -Isrc src/Command.cpp tests/CommandTest.cpp -o $@

test: $(TEST_BINARY)
	$(TEST_BINARY)

clean:
	rm -f $(TEST_BINARY)
	rmdir $(BUILD_DIRECTORY) 2>/dev/null || true
