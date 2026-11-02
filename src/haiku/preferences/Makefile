NAME = UniversalScrollerPreferences
TYPE = APP
SRCS = main.cpp PreferencesApplication.cpp PreferencesWindow.cpp ../../Command.cpp ../../Configuration.cpp
RSRCS =
LIBS = be $(STDCPPLIBS)
LOCAL_INCLUDE_PATHS = ../..
OPTIMIZE = FULL
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17
LINKER_FLAGS =
INSTALL_DIRECTORY = /boot/home/config/non-packaged/apps/UniversalScroller

all: default

DEVEL_DIRECTORY := $(shell findpaths -r makefile_engine B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

.PHONY: user-install user-uninstall

user-install: default
	mkdir -p $(INSTALL_DIRECTORY)
	cp $(TARGET) $(INSTALL_DIRECTORY)/UniversalScrollerPreferences

user-uninstall:
	rm -f $(INSTALL_DIRECTORY)/UniversalScrollerPreferences
	rmdir $(INSTALL_DIRECTORY) 2>/dev/null || true
