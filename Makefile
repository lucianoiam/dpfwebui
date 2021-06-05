#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Web UI example by lucianoiam

# Allow placing DPF in a random directory and include its Makefiles
# These variables are not supported by the official release as of May '21
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
    WebUI.cpp \
    base/BaseWebView.cpp

# Note this is not DPF's Makefile.base.mk
include Makefile.base.mk

# Add platform-specific source files
ifeq ($(LINUX),true)
SRC_FILES_UI += arch/linux/ExternalGtkWebView.cpp \
                arch/linux/PlatformLinux.cpp \
                arch/linux/ipc.c
endif
ifeq ($(MACOS),true)
SRC_FILES_UI += arch/macos/CocoaWebView.mm \
                arch/macos/PlatformMac.cpp
endif
ifeq ($(WINDOWS),true)
SRC_FILES_UI += arch/windows/EdgeWebView.cpp \
                arch/windows/WebView2EventHandler.cpp \
                arch/windows/WinApiStub.cpp \
                arch/windows/PlatformWindows.cpp \
                arch/windows/res/plugin.rc
endif

FILES_DSP = $(SRC_FILES_DSP:%=src/%)
FILES_UI = $(SRC_FILES_UI:%=src/%)

# --------------------------------------------------------------
# Do some magic
ifneq ($(WINDOWS),true)
# FIXME: Use OpenGL on all platforms, seems there are no window resize events with cairo backend?
#UI_TYPE = cairo
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

# --------------------------------------------------------------
# Up to here follow example DISTRHO plugin Makefile, now begin dpf-webui secret sauce
BASE_FLAGS += -Isrc -I$(DPF_CUSTOM_PATH) -DBIN_BASENAME=$(NAME)

# Platform-specific build flags
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I./lib/windows/WebView2/build/native/include
LINK_FLAGS += -L./lib/windows/WebView2/build/native/x64 -lole32 -lShlwapi -lWebView2Loader.dll \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread -Wl,-Bdynamic
endif

# Reuse DISTRHO post-build scripts
ifneq ($(WINDOWS),true)
TARGETS += utils

utils:
ifneq ($(MSYS_MINGW),true)
	@MSYS=winsymlinks:nativestrict
endif
	@ln -s $(DPF_CUSTOM_PATH)/utils .
endif

# Linux requires a helper binary
ifeq ($(LINUX),true)
TARGETS += lxhelper
HELPER_BIN = $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_helper

lxhelper: src/arch/linux/helper.c src/arch/linux/ipc.c
	@echo "Creating helper"
	$(SILENT)$(CC) $^ -Isrc -o $(HELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi

clean: clean_lxhelper

clean_lxhelper:
	rm -rf $(HELPER_BIN)
endif

# Mac requires compiling Objective-C++ and creating a VST bundle
ifeq ($(MACOS),true)
TARGETS += macvst

macvst:
	@$(CURDIR)/utils/generate-vst-bundles.sh

clean: clean_macvst

clean_macvst:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# Windows requires compiling resource files and linking to WebView2, currently only 64-bit
ifeq ($(WINDOWS),true)
TARGETS += winlibs
WEBVIEW_DLL = lib/windows/WebView2/runtimes/win-x64/native/WebView2Loader.dll

winlibs:
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader

clean: clean_winlibs

clean_winlibs:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader

$(BUILD_DIR)/%.rc.o: %.rc
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@windres --input $< --output $@ --output-format=coff
endif

# Target for generating LV2 TTL files
ifeq ($(CAN_GENERATE_TTL),true)
TARGETS += lv2ttl

lv2ttl: utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
endif

# Target for copying web UI files comes last
TARGETS += resources

resources:
	@echo "Copying resource files"
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
ifeq ($(LINUX),true)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
endif
ifeq ($(MACOS),true)
	@cp -r res/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst/Contents/Resources
endif

clean: clean_resources

clean_resources:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources

# Target for building DPF's graphics library goes firstmost
dgl:
	make -C $(DPF_CUSTOM_PATH) dgl

all: dgl $(TARGETS)

# --------------------------------------------------------------
