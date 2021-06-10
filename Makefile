#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# WebUI graphics by lucianoiam

# Allow placing DPF in a custom directory while including its Makefiles
DPF_CUSTOM_PATH = ./lib/DPF
DPF_CUSTOM_TARGET_DIR = ./bin
DPF_CUSTOM_BUILD_DIR = ./build

# Keep debug symbols and print full compiler output
SKIP_STRIPPING = true
VERBOSE = true

# Note this is not DPF's own Makefile.base.mk
USE_DPF_DEVELOP_BRANCH=true
include Makefile.base.mk

# --------------------------------------------------------------
# DISTRHO project name, used for binaries

NAME = d_dpf_webui

# --------------------------------------------------------------
# Files to build

SRC_FILES_DSP = \
    WebExamplePlugin.cpp

SRC_FILES_UI  = \
    WebExampleUI.cpp \
    base/WebUI.cpp \
    base/BaseWebView.cpp \
    base/ScriptValue.cpp

# Add platform-specific source files
ifeq ($(LINUX),true)
SRC_FILES_UI += arch/linux/ExternalGtkWebView.cpp \
                arch/linux/PlatformLinux.cpp \
                arch/linux/extra/ipc.c
endif
ifeq ($(MACOS),true)
SRC_FILES_UI += arch/macos/CocoaWebView.mm \
                arch/macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
SRC_FILES_UI += arch/windows/EdgeWebView.cpp \
                arch/windows/PlatformWindows.cpp \
                arch/windows/extra/WebView2EventHandler.cpp \
                arch/windows/extra/WinApiStub.cpp \
                arch/windows/extra/cJSON.c \
                arch/windows/res/plugin.rc
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
#TARGETS += dssi
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
# Up to here follow almost intact example DPF plugin Makefile
# Now add dpf-webui's own magic
BASE_FLAGS += -Isrc -I$(DPF_CUSTOM_PATH) -DBIN_BASENAME=$(NAME)

# Platform-specific build flags
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I$(EDGE_WEBVIEW2_PATH)/build/native/include
LINK_FLAGS += -L$(EDGE_WEBVIEW2_PATH)/build/native/x64 \
              -lole32 -lShlwapi -lWebView2Loader.dll -static-libgcc \
              -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread
endif

# Switch to DPF develop if specified
ifeq ($(USE_DPF_DEVELOP_BRANCH),true)
ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
ifeq (,$(findstring develop,$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
_:=$(shell git -C $(DPF_CUSTOM_PATH) checkout develop)
endif
endif
endif

# Target for DPF graphics library
DEP_TARGETS += dgl

dgl: $(DPF_CUSTOM_PATH)/build/libdgl.a

$(DPF_CUSTOM_PATH)/build/libdgl.a:
	make -C $(DPF_CUSTOM_PATH) dgl

# Reuse DPF post-build scripts
ifneq ($(WINDOWS),true)
TARGETS += utils

utils:
	@$(MSYS_MINGW_SYMLINKS)
	@ln -s $(DPF_CUSTOM_PATH)/utils .
endif

# Target for generating LV2 TTL files
ifeq ($(CAN_GENERATE_TTL),true)
TARGETS += lv2ttl

lv2ttl: utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
endif

# Linux requires a helper binary
ifeq ($(LINUX),true)
TARGETS += lxhelper
HELPER_BIN = $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_ui

lxhelper: src/arch/linux/helper.c src/arch/linux/extra/ipc.c
	@echo "Creating helper"
	$(SILENT)$(CC) $^ -Isrc -o $(HELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
#	@cp $(HELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi

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

# Windows requires compiling resource files and linking to WebView2
# The current Makefile is too lazy to support 32-bit but DLL is also available
ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH=./lib/Microsoft.Web.WebView2
DEP_TARGETS += $(EDGE_WEBVIEW2_PATH)
TARGETS += copywindll

ifeq (,$(shell which nuget 2>/dev/null))
ifneq ($(MSYS_MINGW),true)
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif

copywindll:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader

clean: clean_windll

clean_windll:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader

ifeq ($(MSYS_MINGW),true)
$(EDGE_WEBVIEW2_PATH): /usr/bin/nuget.exe
else
$(EDGE_WEBVIEW2_PATH):
endif
	@echo Downloading Edge WebView2 SDK...
	@$(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory lib
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet...
	@wget -P /usr/bin https://dist.nuget.org/win-x86-commandline/latest/nuget.exe

$(BUILD_DIR)/%.rc.o: %.rc
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@windres --input $< --output $@ --output-format=coff
endif

# Target for copying web UI files appended last
TARGETS += resources

resources:
	@echo "Copying web resource files"
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	@cp -r src/web/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
	@cp -r src/web/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
#ifeq ($(LINUX),true)
#	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
#	@cp -r src/web/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
#endif
ifeq ($(MACOS),true)
	@cp -r src/web/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst/Contents/Resources
endif

clean: clean_resources

clean_resources:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources

all: $(DEP_TARGETS) $(TARGETS)

# --------------------------------------------------------------
