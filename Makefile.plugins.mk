# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Basic setup

HIPHOP_ROOT_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
HIPHOP_SRC_PATH  ?= $(HIPHOP_ROOT_PATH)/src
HIPHOP_LIB_PATH  ?= $(HIPHOP_ROOT_PATH)/lib

DPF_PATH         ?= $(HIPHOP_ROOT_PATH)/dpf
DPF_TARGET_DIR   ?= bin
DPF_BUILD_DIR    ?= build
DPF_GIT_BRANCH   ?= develop

ifneq ($(HIPHOP_AS_DSP_PATH),)
AS_DSP = true
HIPHOP_ENABLE_WASI ?= true
endif
ifneq ($(HIPHOP_WEB_UI_PATH),)
WEB_UI = true
endif

# ------------------------------------------------------------------------------
# Check for mandatory variables

ifeq ($(HIPHOP_PROJECT_VERSION),)
$(error HIPHOP_PROJECT_VERSION is not set)
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

NPM_ENV = true

# ------------------------------------------------------------------------------
# Add optional support for AssemblyScript DSP

ifeq ($(AS_DSP),true)
HIPHOP_FILES_DSP  = WasmHostPlugin.cpp \
                    WasmEngine.cpp \
                    Platform.cpp
ifeq ($(LINUX),true)
HIPHOP_FILES_DSP += linux/PlatformLinux.cpp
endif
ifeq ($(MACOS),true)
HIPHOP_FILES_DSP += macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
HIPHOP_FILES_DSP += windows/PlatformWindows.cpp \
                    windows/extra/WinApiStub.cpp 
endif

FILES_DSP += $(HIPHOP_FILES_DSP:%=$(HIPHOP_SRC_PATH)/%)
endif

# ------------------------------------------------------------------------------
# Add optional support for web UI

ifeq ($(WEB_UI),true)
HIPHOP_FILES_UI  = WebHostUI.cpp \
                   AbstractWebView.cpp \
                   JsValue.cpp \
                   Platform.cpp
ifeq ($(LINUX),true)
HIPHOP_FILES_UI += linux/ExternalGtkWebView.cpp \
                   linux/PlatformLinux.cpp \
                   linux/extra/ipc.c
endif
ifeq ($(MACOS),true)
HIPHOP_FILES_UI += macos/CocoaWebView.mm \
                   macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
HIPHOP_FILES_UI += windows/EdgeWebView.cpp \
                   windows/KeyboardRouter.cpp \
                   windows/PlatformWindows.cpp \
                   windows/extra/WebView2EventHandler.cpp \
                   windows/extra/WinApiStub.cpp \
                   windows/extra/cJSON.c \
                   windows/resources/plugin.rc
endif

FILES_UI += $(HIPHOP_FILES_UI:%=$(HIPHOP_SRC_PATH)/%)
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

ifneq ($(WINDOWS),true)
UI_TYPE = cairo
else
UI_TYPE = opengl
endif

include $(DPF_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add shared build flags
BASE_FLAGS += -I$(HIPHOP_SRC_PATH) -I$(DPF_PATH) -DBIN_BASENAME=$(NAME) \
              -DHIPHOP_PROJECT_ID_HASH=$(shell echo $(NAME):$(HIPHOP_PROJECT_VERSION) \
              	| shasum -a 256 | head -c 8)

# ------------------------------------------------------------------------------
# Add build flags for AssemblyScript DSP dependencies

ifeq ($(AS_DSP),true)
BASE_FLAGS += -I$(WASMER_PATH)/include
ifeq ($(HIPHOP_ENABLE_WASI),true)
BASE_FLAGS += -DHIPHOP_ENABLE_WASI
endif
LINK_FLAGS += -L$(WASMER_PATH)/lib -lwasmer
ifeq ($(MACOS),true)
LINK_FLAGS += -framework AppKit 
endif
ifeq ($(WINDOWS),true)
LINK_FLAGS += -Wl,-Bstatic -lWs2_32 -lBcrypt -lUserenv
endif
endif

# ------------------------------------------------------------------------------
# Add build flags for web UI dependencies

ifeq ($(WEB_UI),true)
ifeq ($(HIPHOP_PRINT_TRAFFIC),true)
BASE_FLAGS += -DHIPHOP_PRINT_TRAFFIC
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
endif

# ------------------------------------------------------------------------------
# Print some info

TARGETS += info

info:
	@echo "Hip-Hop : $(HIPHOP_ROOT_PATH)"
	@echo "DPF     : $(DPF_PATH) @ $(DPF_GIT_BRANCH)"
	@echo "Build   : $(DPF_BUILD_DIR)"
	@echo "Target  : $(DPF_TARGET_DIR)"

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
	@cd $(HIPHOP_ROOT_PATH) && patch -u dpf/distrho/src/DistrhoUI.cpp -i DistrhoUI.cpp.patch
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Download Wasmer

ifeq ($(AS_DSP),true)
WASMER_PATH = $(HIPHOP_LIB_PATH)/wasmer

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
WASMER_URL = https://github.com/lucianoiam/hiphop/files/6795372/wasmer-mingw-amd64.tar.gz
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
# Dependency - Download Node.js for MinGW

ifeq ($(AS_DSP),true)
ifeq ($(MSYS_MINGW),true)
NPM_ENV = export PATH=$$PATH:/opt/node && export NODE_SKIP_PLATFORM_CHECK=1
ifeq (,$(wildcard /opt/node))
NPM_FILENAME = node-v16.5.0-win-x64.zip
NPM_URL = https://nodejs.org/dist/latest/$(NPM_FILENAME)

TARGETS += /opt/node/npm

/opt/node/npm:
	@echo Downloading Node.js ...
	@wget -P /tmp $(NPM_URL)
	@unzip -o /tmp/$(NPM_FILENAME) -d /opt
	@mv /opt/$(basename $(NPM_FILENAME)) /opt/node
	@rm /tmp/$(NPM_FILENAME)
endif
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Link framework AS files to user project, to be manually called

AS_ASSEMBLY_PATH = $(HIPHOP_AS_DSP_PATH)/assembly

frameworkas:
	@test -f $(AS_ASSEMBLY_PATH)/index.ts \
		|| ln -s $(abspath $(HIPHOP_SRC_PATH)/dsp/index.ts) $(AS_ASSEMBLY_PATH)
	@test -f $(AS_ASSEMBLY_PATH)/distrho-plugin.ts \
		|| ln -s $(abspath $(HIPHOP_SRC_PATH)/dsp/distrho-plugin.ts) $(AS_ASSEMBLY_PATH)

# ------------------------------------------------------------------------------
# Dependency - Download Edge WebView2

ifeq ($(WEB_UI),true)
ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH = $(HIPHOP_LIB_PATH)/Microsoft.Web.WebView2

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
	@mkdir -p $(HIPHOP_LIB_PATH)
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory $(HIPHOP_LIB_PATH)
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet...
	@wget -P /usr/bin $(NUGET_URL)
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Built-in JavaScript library include

ifeq ($(WEB_UI),true)
UI_JS_INCLUDE_PATH = $(HIPHOP_SRC_PATH)/ui/distrho-ui.js.include

TARGETS += $(UI_JS_INCLUDE_PATH)

$(UI_JS_INCLUDE_PATH):
	@echo 'R"UI_JS(' > $(UI_JS_INCLUDE_PATH)
	@cat $(HIPHOP_SRC_PATH)/ui/distrho-ui.js >> $(UI_JS_INCLUDE_PATH)
	@echo ')UI_JS"' >> $(UI_JS_INCLUDE_PATH)
endif

# ------------------------------------------------------------------------------
# Linux only - Build WebKitGTK helper binary

ifeq ($(WEB_UI),true)
ifeq ($(LINUX),true)
LXHELPER_BIN = $(BUILD_DIR)/$(NAME)_ui
HIPHOP_TARGET += $(LXHELPER_BIN)

$(LXHELPER_BIN): $(HIPHOP_SRC_PATH)/linux/helper.c $(HIPHOP_SRC_PATH)/linux/extra/ipc.c
	@echo "Building helper..."
	$(SILENT)$(CC) $^ -I$(HIPHOP_SRC_PATH) -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
endif
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
# User defined TARGETS are only available *after* inclusion of this Makefile

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

ifeq ($(WEB_UI),true)
ifeq ($(LINUX),true)
HIPHOP_TARGET += lxhelper

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
endif

# ------------------------------------------------------------------------------
# Post build - Create macOS VST bundle

ifeq ($(MACOS),true)
HIPHOP_TARGET += macvst

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

ifeq ($(WEB_UI),true)
ifeq ($(WINDOWS),true)
HIPHOP_TARGET += edgelib

edgelib:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
	@($(TEST_WINDOWS_JACK) \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR) \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(HIPHOP_SRC_PATH)/windows/resources/WebView2Loader.manifest $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		) || true
	@($(TEST_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/WebView2Loader \
		&& cp $(HIPHOP_SRC_PATH)/windows/resources/WebView2Loader.manifest $(TARGET_DIR)/WebView2Loader \
		) || true

clean: clean_edgelib

clean_edgelib:
	@rm -rf $(TARGET_DIR)/WebView2Loader
endif
endif

# ------------------------------------------------------------------------------
# Post build - Always copy lib files

ifneq ($(AS_DSP),)
HIPHOP_TARGET += libdsp

WASM_SRC_PATH = $(HIPHOP_AS_DSP_PATH)/build/optimized.wasm
WASM_DST_PATH = dsp/plugin.wasm

libdsp:
	@echo "Building AssemblyScript project..."
	@# npm --prefix fails on MinGW due to paths mixing \ and /
	@test -d $(HIPHOP_AS_DSP_PATH)/node_modules \
		|| (cd $(HIPHOP_AS_DSP_PATH) && $(NPM_ENV) && npm install)
	@cd $(HIPHOP_AS_DSP_PATH) && $(NPM_ENV) && npm run asbuild
	@echo "Copying WebAssembly DSP binary..."
	@($(TEST_JACK_OR_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)_lib/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME)_lib/$(WASM_DST_PATH) \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/$(NAME)_lib/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME).lv2/$(NAME)_lib/$(WASM_DST_PATH) \
		) || true
	@($(TEST_DSSI) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_lib/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_lib/$(WASM_DST_PATH) \
		) || true
	@($(TEST_MAC_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).vst/Contents/Resources/dsp \
		&& cp -r $(WASM_SRC_PATH) $(TARGET_DIR)/$(NAME).vst/Contents/Resources/$(WASM_DST_PATH) \
		) || true
endif

ifeq ($(WEB_UI),true)
HIPHOP_TARGET += libui

libui:
	@echo "Copying web UI files..."
	@($(TEST_JACK_OR_WINDOWS_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)_lib/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME)_lib/ui \
		) || true
	@($(TEST_LV2) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/$(NAME)_lib/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME).lv2/$(NAME)_lib/ui \
		) || true
	@($(TEST_DSSI) \
		&& mkdir -p $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_lib/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_lib/ui \
		) || true
	@($(TEST_MAC_VST) \
		&& mkdir -p $(TARGET_DIR)/$(NAME).vst/Contents/Resources/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(TARGET_DIR)/$(NAME).vst/Contents/Resources/ui \
		) || true

clean: clean_lib

clean_lib:
	@rm -rf $(TARGET_DIR)/$(NAME)_lib
endif

# ------------------------------------------------------------------------------
# Post build - Create LV2 manifest files

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

ifeq ($(CAN_GENERATE_TTL),true)
HIPHOP_TARGET += lv2ttl

lv2ttl: $(DPF_PATH)/utils/lv2_ttl_generator
	@# TODO - generate-ttl.sh expects hardcoded directory bin/
	@cd $(DPF_TARGET_DIR)/.. && $(abspath $(DPF_PATH))/utils/generate-ttl.sh

$(DPF_PATH)/utils/lv2_ttl_generator:
	$(MAKE) -C $(DPF_PATH)/utils/lv2-ttl-generator
endif
