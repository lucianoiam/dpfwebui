#!/usr/bin/make -f
# CEF boilerplate
# Created by lucianoiam on 2021.16.01

# CEF docs at https://bitbucket.org/chromiumembedded/cef/wiki/Tutorial
# --------------------------------------------------------------------------------

# FIXME - CEF_PATH must be an absolute path
ifeq ($(strip $(CEF_PATH)),)
$(error Environment variable CEF_PATH is not properly set)
endif

CEF_BIN_PATH = $(CEF_PATH)/Release
CEF_RES_PATH = $(CEF_PATH)/Resources

ifeq ($(strip $(CEF_BOILERPLATE_PATH)),)
CEF_BOILERPLATE_PATH = .
endif

all: simple

CEF_CXX = c++
CEF_CPPFLAGS = -I$(CEF_PATH) -std=c++14 -DOVERRIDE=
CEF_CXXFLAGS = -DCEF_USE_SANDBOX -DNDEBUG -DWRAPPING_CEF_SHARED -D_FILE_OFFSET_BITS=64 -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -O3 -DNDEBUG -fno-strict-aliasing -fPIC -fstack-protector -funwind-tables -fvisibility=hidden --param=ssp-buffer-size=4 -pipe -pthread -Wall -Werror -Wno-missing-field-initializers -Wno-unused-parameter -Wno-error=comment -Wno-comment -m64 -march=x86-64 -fno-exceptions -fno-rtti -fno-threadsafe-statics -fvisibility-inlines-hidden -std=gnu++11 -Wsign-compare -Wno-undefined-var-template -Wno-literal-suffix -Wno-narrowing -Wno-attributes -O2 -fdata-sections -ffunction-sections -fno-ident -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=2
CEF_LDFLAGS = -lcef_dll_wrapper -L$(CEF_BUILD_DIR) -lcef -L$(CEF_BIN_PATH) -lX11 -O3 -DNDEBUG -rdynamic -fPIC -pthread -Wl,--disable-new-dtags -Wl,--fatal-warnings -Wl,-rpath,. -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -m64 -Wl,-O1 -Wl,--as-needed -Wl,--gc-sections

CEF_BUILD_DIR = build
CEF_TARGET_DIR = bin

include $(CEF_BOILERPLATE_PATH)/Makefile.deps.mk

directories: $(CEF_BUILD_DIR) $(CEF_TARGET_DIR)

$(CEF_BUILD_DIR):
	mkdir $(CEF_BUILD_DIR)

$(CEF_TARGET_DIR):
	mkdir $(CEF_TARGET_DIR)

clean:
	rm -rf $(CEF_TARGET_DIR)
	rm -rf $(CEF_BUILD_DIR)

.PHONY: clean

# Simple application
# --------------------------------------------------------------------------------

simple: CXX = $(CEF_CXX)
simple: CPPFLAGS = $(CEF_CPPFLAGS)
simple: CXXFLAGS = $(CEF_CXXFLAGS)
simple: LDFLAGS = $(CEF_LDFLAGS) 

simple: directories libcef_dll_wrapper link_cef_bin link_cef_res $(CEF_TARGET_DIR)/simple

FILES_SIMPLE = \
	simple_app.cc \
	simple_handler.cc \
	simple_handler_linux.cc \
	cefsimple_linux.cc

OBJ_SIMPLE = $(FILES_SIMPLE:%.cc=$(CEF_BUILD_DIR)/%.o)

$(CEF_TARGET_DIR)/simple: $(OBJ_SIMPLE) 
	$(CXX) $(OBJ_SIMPLE) -o $@ $(LDFLAGS)

$(CEF_BUILD_DIR)/%.o: %.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -c $< -o $@
