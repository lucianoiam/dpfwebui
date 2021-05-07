#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Web UI example by lucianoiam

DPF_CUSTOM_PATH = lib/dpf
DPF_CUSTOM_TARGET_DIR = bin
DPF_CUSTOM_BUILD_DIR = build

# Keep debug symbols (DPF Makefile.base.mk@148)
SKIP_STRIPPING = true
# For debug purposes
VERBOSE = true

# --------------------------------------------------------------
# DISTRHO project name, used for binaries

NAME = d_dpf_webui

# --------------------------------------------------------------
# Files to build

SRC_FILES_DSP = \
	WebPlugin.cpp

SRC_FILES_UI  = \
	WebUI.cpp


TARGET_MACHINE := $(shell gcc -dumpmachine)

ifneq ($(LINUX),true)
ifneq ($(MACOS),true)
ifneq ($(WINDOWS),true)

ifneq (,$(findstring linux,$(TARGET_MACHINE)))
LINUX=true
endif
ifneq (,$(findstring apple,$(TARGET_MACHINE)))
MACOS=true
endif
ifneq (,$(findstring mingw,$(TARGET_MACHINE)))
WINDOWS=true
endif

endif
endif
endif

ifeq ($(WINDOWS),true)
	SRC_FILES_UI += windows/WebView.cpp \
					windows/event.cpp
endif
ifeq ($(MACOS),true)
	SRC_FILES_UI += macos/WebView.mm
endif


FILES_DSP = $(SRC_FILES_DSP:%=src/%)
FILES_UI = $(SRC_FILES_UI:%=src/%)

# --------------------------------------------------------------
# Do some magic

ifneq ($(WINDOWS),true)
UI_TYPE = cairo
endif
include $(DPF_CUSTOM_PATH)/Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_CAIRO),true)
TARGETS += jack
endif
endif

ifneq ($(MACOS_OR_WINDOWS),true)
ifeq ($(HAVE_CAIRO),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif
endif

ifeq ($(HAVE_CAIRO),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += vst

BASE_FLAGS += -Isrc

ifeq ($(LINUX),true)
BASE_FLAGS += -lX11 \
				`pkg-config --cflags --libs gtk+-3.0` \
				`pkg-config --cflags --libs webkit2gtk-4.0`
endif

ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< -ObjC++ -c -o $@
endif


# Download MSYS2
# https://www.msys2.org/
# Follow website instructions to install the mingw-64 GCC

# Install the WebView2 SDK
# https://docs.microsoft.com/en-us/microsoft-edge/webview2/gettingstarted/win32
# Create Visual Studio Project
# Right click on solution
# Manage NuGet packages
# Install Microsoft.Web.WebView2
# Copy < solution dir >/packages/Microsoft.Web.WebView2.< version > -> lib/WebView2

# For now WebView2Loader.dll should be copied to the host .exe directory or c:/windows/system32
# There is no rpath equivalent for Windows, possible solution is to set search path
# during runtime
# https://stackoverflow.com/questions/3272383/linking-with-r-and-rpath-switches-on-windows

ifeq ($(WINDOWS),true)
BASE_FLAGS += -I./lib/windows/WebView2/build/native/include
LINK_FLAGS += -L./lib/windows/WebView2/build/native/x64 -lWebView2Loader.dll \
				-static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
endif

all: $(TARGETS)

# --------------------------------------------------------------
