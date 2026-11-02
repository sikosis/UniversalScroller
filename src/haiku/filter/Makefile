NAME = UniversalScroller
TYPE = SHARED
SRCS = UniversalScroller.cpp FilterEntry.cpp ../../Command.cpp ../../Configuration.cpp
RSRCS =
LIBS = _APP_ be
LOCAL_INCLUDE_PATHS = ../.. 
OPTIMIZE = FULL
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17
LINKER_FLAGS =
INSTALL_DIRECTORY = /boot/home/config/non-packaged/add-ons/input_server/filters

all: _APP_ default

DEVEL_DIRECTORY := $(shell findpaths -r makefile_engine B_FIND_PATH_DEVELOP_DIRECTORY)
include $(DEVEL_DIRECTORY)/etc/makefile-engine

clean:
	rm -f _APP_

$(TARGET): _APP_

_APP_: /system/servers/input_server
	ln -s $< $@

install: default
	mkdir -p $(INSTALL_DIRECTORY)
	cp $(TARGET) $(INSTALL_DIRECTORY)/UniversalScroller

uninstall:
	rm -f $(INSTALL_DIRECTORY)/UniversalScroller
