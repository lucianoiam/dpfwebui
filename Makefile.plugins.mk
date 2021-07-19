# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Basic setup

APX_ROOT_PATH  := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
APX_SRC_PATH   ?= $(APX_ROOT_PATH)/src
APX_LIB_PATH   ?= $(APX_ROOT_PATH)/lib

DPF_PATH       ?= $(APX_ROOT_PATH)/dpf
DPF_TARGET_DIR ?= bin
DPF_BUILD_DIR  ?= build
DPF_GIT_BRANCH ?= develop

ifneq ($(APX_AS_DSP_PATH),)
AS_DSP = true
endif

# ------------------------------------------------------------------------------
# Check for mandatory variables

ifeq ($(APX_PROJECT_VERSION),)
$(error APX_PROJECT_VERSION is not set)
endif
ifeq ($(APX_WEB_UI_PATH),)
$(error APX_WEB_UI_PATH is not set)
endif

# ------------------------------------------------------------------------------
# Determine build environment

TARGET_MACHINE := $(shell gcc -dumpmachine)

ifneq (,$(findstring linux,$(TARGET_MACHINE)))
LINUX = true
endif
ifneq (,$(findstring apple,$(TARGET_MACHINE)))
MACOS = true
endif
ifneq (,$(findstring mingw,$(TARGET_MACHINE)))
WINDOWS = true
endif
ifneq (,$(findstring MINGW,$(MSYSTEM)))
MSYS_MINGW = true
endif

ifeq ($(MSYS_MINGW),true)
ifeq ($(shell cmd /c "net.exe session 1>NUL 2>NUL || exit /b 1"; echo $$?),1)
#$(info Run MSYS as administrator for real symlink support)
MSYS_MINGW_SYMLINKS = :
else
MSYS_MINGW_SYMLINKS = export MSYS=winsymlinks:nativestrict
endif
endif

# ------------------------------------------------------------------------------
# Add optional support for AssemblyScript based DSP

ifeq ($(AS_DSP),true)
APX_FILES_DSP  = WasmHostPlugin.cpp \
                 Platform.cpp
ifeq ($(LINUX),true)
APX_FILES_DSP += linux/PlatformLinux.cpp
endif
ifeq ($(MACOS),true)
APX_FILES_DSP += macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
APX_FILES_DSP += windows/PlatformWindows.cpp \
                 windows/extra/WinApiStub.cpp 
endif

FILES_DSP += $(APX_FILES_DSP:%=$(APX_SRC_PATH)/%)
endif

# ------------------------------------------------------------------------------
# Add web-based UI source

APX_FILES_UI  = WebHostUI.cpp \
                AbstractWebWidget.cpp \
                ScriptValue.cpp \
                Platform.cpp
ifeq ($(LINUX),true)
APX_FILES_UI += linux/ExternalGtkWebWidget.cpp \
                linux/PlatformLinux.cpp \
                linux/extra/ipc.c
endif
ifeq ($(MACOS),true)
APX_FILES_UI += macos/CocoaWebWidget.mm \
                macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
APX_FILES_UI += windows/EdgeWebWidget.cpp \
                windows/KeyboardRouter.cpp \
                windows/PlatformWindows.cpp \
                windows/extra/WebView2EventHandler.cpp \
                windows/extra/WinApiStub.cpp \
                windows/extra/cJSON.c \
                windows/res/plugin.rc
endif

FILES_UI += $(APX_FILES_UI:%=$(APX_SRC_PATH)/%)

ifneq ($(WINDOWS),true)
UI_TYPE = cairo
else
UI_TYPE = opengl
endif

# ------------------------------------------------------------------------------
# Include DPF Makefile for plugins

ifeq (,$(wildcard $(DPF_PATH)/Makefile))
_ := $(shell git submodule update --init --recursive)
endif

ifneq (,$(DPF_GIT_BRANCH))
ifeq (,$(findstring $(DPF_GIT_BRANCH),$(shell git -C $(DPF_PATH) branch --show-current)))
_ := $(shell git -C $(DPF_PATH) checkout $(DPF_GIT_BRANCH))
endif
endif

include $(DPF_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add build flags for AssemblyScript DSP dependencies

ifeq ($(AS_DSP),true)
BASE_FLAGS += -I$(WASMER_PATH)/include
LINK_FLAGS += -L$(WASMER_PATH)/lib -lwasmer
ifeq ($(MACOS),true)
LINK_FLAGS += -framework AppKit 
endif
ifeq ($(WINDOWS),true)
LINK_FLAGS += -Wl,-Bstatic -lWs2_32 -lBcrypt -lUserenv
endif
endif

# ------------------------------------------------------------------------------
# Add build flags for web-based UI dependencies

BASE_FLAGS += -I$(APX_SRC_PATH) -I$(DPF_PATH) -DBIN_BASENAME=$(NAME) \
              -DAPX_PROJECT_ID_HASH=$(shell echo $(NAME):$(APX_PROJECT_VERSION) \
              	| shasum -a 256 | head -c 8)
ifeq ($(APX_PRINT_TRAFFIC),true)
BASE_FLAGS += -DAPX_PRINT_TRAFFIC
endif
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I$(EDGE_WEBVIEW2_PATH)/build/native/include
LINK_FLAGS += -L$(EDGE_WEBVIEW2_PATH)/build/native/x64 \
              -lole32 -lShlwapi -lMfplat -lksuser -lmfuuid -lwmcodecdspuuid \
              -lWebView2Loader.dll -static-libgcc -static-libstdc++ -Wl,-Bstatic \
              -lstdc++ -lpthread
endif

# ------------------------------------------------------------------------------
# Print some info

TARGETS += info

info:
	@echo "Apices : $(APX_ROOT_PATH)"
	@echo "DPF    : $(DPF_PATH) @ $(DPF_GIT_BRANCH)"
	@echo "Build  : $(DPF_BUILD_DIR)"
	@echo "Target : $(DPF_TARGET_DIR)"

# ------------------------------------------------------------------------------
# Dependency - Build DPF Graphics Library

TARGETS += $(DPF_PATH)/build/libdgl.a

$(DPF_PATH)/build/libdgl.a:
	make -C $(DPF_PATH) dgl

TARGETS += MACSIZEBUG

MACSIZEBUG:
ifeq ($(MACOS),true)
ifeq ($(shell grep -c FIXME_MacScaleFactor $(DPF_PATH)/distrho/src/DistrhoUI.cpp),0)
	@echo Patching DistrhoUI.cpp to workaround window size bug on macOS...
	@cd $(APX_ROOT_PATH) && patch -u dpf/distrho/src/DistrhoUI.cpp -i DistrhoUI.cpp.patch
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Download Wasmer

ifeq ($(AS_DSP),true)
WASMER_PATH = $(APX_LIB_PATH)/wasmer

TARGETS += $(WASMER_PATH)

ifeq ($(LINUX_OR_MACOS),true)
ifeq ($(LINUX),true)
WASMER_PKG_FILE = wasmer-linux-amd64.tar.gz
endif
ifeq ($(MACOS),true)
WASMER_PKG_FILE = wasmer-darwin-amd64.tar.gz
endif
WASMER_URL = https://github.com/wasmerio/wasmer/releases/download/2.0.0/$(WASMER_PKG_FILE)
endif
ifeq ($(WINDOWS),true)
# Wasmer official binary distribution requires MSVC 
WASMER_PKG_FILE = wasmer-mingw-amd64.tar.gz
WASMER_URL = https://github.com/lucianoiam/apices/files/6795372/wasmer-mingw-amd64.tar.gz
endif

# https://stackoverflow.com/questions/37038472/osx-how-to-statically-link-a-library-and-dynamically-link-the-standard-library
$(WASMER_PATH):
	@wget -O /tmp/$(WASMER_PKG_FILE) $(WASMER_URL)
	@mkdir -p $(WASMER_PATH)
	@tar xzf /tmp/$(WASMER_PKG_FILE) -C $(WASMER_PATH)
ifeq ($(LINUX),true)
	@mv $(WASMER_PATH)/lib/libwasmer.so $(WASMER_PATH)/lib/libwasmer.so.ignore
endif
ifeq ($(MACOS),true)
	@mv $(WASMER_PATH)/lib/libwasmer.dylib $(WASMER_PATH)/lib/libwasmer.dylib.ignore
endif
	@rm /tmp/$(WASMER_PKG_FILE)
endif

# ------------------------------------------------------------------------------
# Dependency - Download Edge WebView2

ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH = $(APX_LIB_PATH)/Microsoft.Web.WebView2

TARGETS += $(EDGE_WEBVIEW2_PATH)

NUGET_URL = https://dist.nuget.org/win-x86-commandline/latest/nuget.exe

ifeq ($(MSYS_MINGW),true)
$(EDGE_WEBVIEW2_PATH): /usr/bin/nuget.exe
else
$(EDGE_WEBVIEW2_PATH):
ifeq (,$(shell which nuget 2>/dev/null))
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif
	@echo Downloading Edge WebView2 SDK...
	@mkdir -p $(APX_LIB_PATH)
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory $(APX_LIB_PATH)
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet...
	@wget -P /usr/bin $(NUGET_URL)
endif

# ------------------------------------------------------------------------------
# Linux only - Build WebKitGTK helper binary

ifeq ($(LINUX),true)
LXHELPER_BIN = $(BUILD_DIR)/$(NAME)_ui
APX_TARGET += $(LXHELPER_BIN)

$(LXHELPER_BIN): $(APX_SRC_PATH)/linux/helper.c $(APX_SRC_PATH)/linux/extra/ipc.c
	@echo "Building helper..."
	$(SILENT)$(CC) $^ -I$(APX_SRC_PATH) -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
endif

# ------------------------------------------------------------------------------
# Mac only - Build Objective-C++ files

ifeq ($(MACOS),true)
$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# ------------------------------------------------------------------------------
# Windows only - Build resource files

ifeq ($(WINDOWS),true)
$(BUILD_DIR)/%.rc.o: %.rc
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@windres --input $< --output $@ --output-format=coff
endif

# ------------------------------------------------------------------------------
# Post build - Determine built targets
# User defined $(TARGETS) are only available *after* inclusion of this Makefile

TEST_LV2 = test -d $(TARGET_DIR)/$(NAME).lv2
TEST_DSSI = test -d $(TARGET_DIR)/$(NAME)-dssi
TEST_LINUX_OR_MACOS_JACK = test -f $(TARGET_DIR)/$(NAME)
TEST_LINUX_VST = test -f $(TARGET_DIR)/$(NAME)-vst.so
TEST_MAC_VST = test -d $(TARGET_DIR)/$(NAME).vst
TEST_WINDOWS_JACK = test -f $(TARGET_DIR)/$(NAME).exe
TEST_WINDOWS_VST = test -f $(TARGET_DIR)/$(NAME)-vst.dll
TEST_JACK_OR_WINDOWS_VST = $(TEST_LINUX_OR_MACOS_JACK) || $(TEST_WINDOWS_JACK) \
							|| $(TEST_WINDOWS_VST) 

# ------------------------------------------------------------------------------
# Post build - Copy Linux helper

ifeq ($(LINUX),true)
APX_TARGET += lxhelper

lxhelper:
	@($(TEST_LINUX_OR_MACOS_JACK) || $(TEST_LINUX_VST) \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR) \
		) || true
	@($(TEST_LV2) \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR)/$(NAME).lv2 \
		) || true
	@($(TEST_DSSI) \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR)/$(NAME)-dssi \
		) || true

clean: clean_lxhelper

clean_lxhelper:
	@rm -rf $(TARGET_DIR)/$(notdir $(LXHELPER_BIN))
	@rm -rf $(LXHELPER_BIN)
endif

# ------------------------------------------------------------------------------
# Post build - Create macOS VST bundle

ifeq ($(MACOS),true)
APX_TARGET += macvst

macvst:
	@# TODO - generate-vst-bundles.sh expects hardcoded directory bin/
	@cd $(DPF_TARGET_DIR)/.. && $(abspath $(DPF_PATH))/utils/generate-vst-bundles.sh

clean: clean_macvst

clean_macvst:
	@rm -rf $(TARGET_DIR)/$(NAME).vst
endif

# ------------------------------------------------------------------------------
# Post build - Copy Windows Edge WebView2 DLL
# This Makefile version is too lazy to support 32-bit but DLL is also available.
# The "bare" DLL is enough for the standalone JACK target, no need for assembly.

ifeq ($(WINDOWS),true)
APX_TARGET += edgelib

edgelib:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
	@($(TEST_WINDOWS_JACK) \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR) \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(APX_SRC_PATH)/windows/res/WebView2Loader.manifest $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		) || true
	@($(TEST_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/WebView2Loader \
		&& cp $(APX_SRC_PATH)/windows/res/WebView2Loader.manifest $(TARGET_DIR)/WebView2Loader \
		) || true

clean: clean_edgelib

clean_edgelib:
	@rm -rf $(TARGET_DIR)/WebView2Loader
endif

# ------------------------------------------------------------------------------
# Post build - Always copy resource files

ifneq ($(AS_DSP),)
APX_TARGET += resourcesdsp

WASM_SRC_PATH = $(APX_AS_DSP_PATH)/build/optimized.wasm
WASM_DST_PATH = dsp/main.wasm

resourcesdsp:
	@echo "Building AssemblyScript project..."
	@npm --prefix $(APX_AS_DSP_PATH) run asbuild
	@echo "Copying WebAssembly DSP binary..."
	@($(TEST_JACK_OR_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)_res/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME)_res/$(WASM_DST_PATH) \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res/$(WASM_DST_PATH) \
		) || true
	@($(TEST_DSSI) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res/$(WASM_DST_PATH) \
		) || true
	@($(TEST_MAC_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).vst/Contents/Resources/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME).vst/Contents/Resources/$(WASM_DST_PATH) \
		) || true
endif

APX_TARGET += resourcesui

resourcesui:
	@echo "Copying web UI resource files..."
	@($(TEST_JACK_OR_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)_res/ui \
		&& cp -r $(APX_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME)_res/ui \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res/ui \
		&& cp -r $(APX_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res/ui \
		) || true
	@($(TEST_DSSI) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res/ui \
		&& cp -r $(APX_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res/ui \
		) || true
	@($(TEST_MAC_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).vst/Contents/Resources/ui \
		&& cp -r $(APX_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME).vst/Contents/Resources/ui \
		) || true

clean: clean_resources

clean_resources:
	@rm -rf $(TARGET_DIR)/$(NAME)_res

# ------------------------------------------------------------------------------
# Post build - Create LV2 manifest files

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

ifeq ($(CAN_GENERATE_TTL),true)
APX_TARGET += lv2ttl

lv2ttl: $(DPF_PATH)/utils/lv2_ttl_generator
	@# TODO - generate-ttl.sh expects hardcoded directory bin/
	@cd $(DPF_TARGET_DIR)/.. && $(abspath $(DPF_PATH))/utils/generate-ttl.sh

$(DPF_PATH)/utils/lv2_ttl_generator:
	$(MAKE) -C $(DPF_PATH)/utils/lv2-ttl-generator
endif
