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

# Note this is not DPF's Makefile.base.mk
include Makefile.base.mk

# Add platform-specific source files
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

# Setting cairo for Windows disables the UI, is that expected?
include $(DPF_CUSTOM_PATH)/Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += jack
endif
endif

ifneq ($(MACOS_OR_WINDOWS),true)
ifeq ($(HAVE_OPENGL),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif
endif

ifeq ($(HAVE_OPENGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += vst

BASE_FLAGS += -Isrc

# Add platform-specific build flags
ifeq ($(LINUX),true)
BASE_FLAGS += -lX11 \
               `pkg-config --cflags --libs gtk+-3.0` \
               `pkg-config --cflags --libs webkit2gtk-4.0`
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I./lib/windows/WebView2/build/native/include
LINK_FLAGS += -L./lib/windows/WebView2/build/native/x64 -lWebView2Loader.dll \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
endif

# Target for building Objective-C++ files, only apply to macOS
$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< -ObjC++ -c -o $@

# Target for building DPF's graphics library
$(DPF_CUSTOM_PATH)/build/libdgl-opengl.a:
	make -C $(DPF_CUSTOM_PATH) dgl

all: $(TARGETS)

# --------------------------------------------------------------
