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
                windows/event.cpp \
                windows/dllmain.c
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

# Up to here follow example DISTRHO plugin Makefile
# Now begin dpf-webui secret sauce
BASE_FLAGS += -Isrc -I$(DPF_CUSTOM_PATH)/distrho/src -DBIN_BASENAME=$(NAME)

# Platform-specific build flags
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

# Target for building DPF's graphics library comes first (see 'all' below)
dgl:
	make -C $(DPF_CUSTOM_PATH) dgl

# Reuse DISTRHO post-build scripts
# No symlinks on Windows/MinGW, just copy. It does not hurt even if cross-compiling.
TARGETS += utils

utils:
ifneq ($(WINDOWS),true)
	@ln -s $(DPF_CUSTOM_PATH)/utils .
else
	@cp -r $(DPF_CUSTOM_PATH)/utils .
endif

# Linux requires a helper binary
ifeq ($(LINUX),true)
TARGETS += lxhelper
HELPER_BIN = $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_helper

lxhelper: src/linux/helper.c src/linux/ipc.c
	@echo "Creating helper"
	$(SILENT)$(CC) $^ -o $(HELPER_BIN) -lX11 \
		`pkg-config --cflags --libs gtk+-3.0` \
		`pkg-config --cflags --libs webkit2gtk-4.0`
	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi

clean: clean_lxhelper

clean_lxhelper:
	rm -rf $(HELPER_BIN)
endif

# Mac requires Objective-C++ and creating a VST bundle
ifeq ($(MACOS),true)
TARGETS += macvst

macvst:
	@$(CURDIR)/utils/generate-vst-bundles.sh

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# Windows requires the WebView2 runtime library, currently hardcoded to 64-bit
ifeq ($(WINDOWS),true)
TARGETS += winlibs
WEBVIEW_DLL = lib/windows/WebView2/runtimes/win-x64/native/WebView2Loader.dll

winlibs:
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
endif

# Target for generating LV2 TTL files
# Currently broken on Windows
ifneq ($(WINDOWS),true)
ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

ifeq ($(CAN_GENERATE_TTL),true)
TARGETS += lv2ttl

lv2ttl: utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
endif
endif

# Target for copying web UI files comes last
TARGETS += resources

resources:
	@echo "Copying resource files"
	@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
ifeq ($(CAN_GENERATE_TTL),true)
	@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
endif
ifeq ($(LINUX),true)
	@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
endif
ifeq ($(MACOS),true)
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst/Contents/Resources
endif

all: dgl $(TARGETS)

# --------------------------------------------------------------
