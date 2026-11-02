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

all: default

DEVEL_DIRECTORY := $(shell findpaths -r makefile_engine B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine
