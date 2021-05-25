#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Web UI example by lucianoiam

# Allow placing DPF in a random directory and include its Makefiles
# These variables are not supported by the official release as of 16 May 2021
DPF_CUSTOM_PATH = lib/dpf
DPF_CUSTOM_TARGET_DIR = ./bin
DPF_CUSTOM_BUILD_DIR = ./build

# Keep debug symbols (DPF Makefile.base.mk@148) and print full compiler output
SKIP_STRIPPING = true
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
ifeq ($(LINUX),true)
SRC_FILES_UI += linux/ExternalGtkWebViewUI.cpp \
                linux/ipc.c
endif
ifeq ($(MACOS),true)
SRC_FILES_UI += macos/CocoaWebViewUI.mm
endif
ifeq ($(WINDOWS),true)
SRC_FILES_UI += windows/EdgeWebViewUI.cpp \
                windows/event.cpp
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

# A helper binary is required on Linux
ifeq ($(LINUX),true)
TARGETS += helper
endif

BASE_FLAGS += -Isrc -I$(DPF_CUSTOM_PATH)/distrho/src -DBIN_NAME=$(NAME)

# Add platform-specific build flags
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I./lib/windows/WebView2/build/native/include
LINK_FLAGS += -L./lib/windows/WebView2/build/native/x64 -lShlwapi -lWebView2Loader.dll \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
endif

# Target for Linux helper
ifeq ($(LINUX),true)
HELPER_BIN = $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_helper

helper: src/linux/helper.c src/linux/ipc.c
	@echo "Creating helper"
	$(SILENT)$(CC) $^ -o $(HELPER_BIN) -lX11 \
		`pkg-config --cflags --libs gtk+-3.0` \
		`pkg-config --cflags --libs webkit2gtk-4.0`

clean: clean_helper

clean_helper:
	rm -rf $(HELPER_BIN)
endif

# Target for building Objective-C++ files and macOS VST bundle
ifeq ($(MACOS),true)
TARGETS += macvst

macvst:
	@$(CURDIR)/utils/generate-vst-bundles.sh

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# Target for generating LV2 TTL files
# Currently does not work on Windows because utils/ is a symlink
ifneq ($(WINDOWS),true)
ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif
endif

ifeq ($(CAN_GENERATE_TTL),true)
TARGETS += lv2ttl

lv2ttl: utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
endif

# Target for building DPF's graphics library
dgl:
	make -C $(DPF_CUSTOM_PATH) dgl

all: dgl $(TARGETS)

# --------------------------------------------------------------
