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

ifeq ($(HIPHOP_PROJECT_VERSION),)
$(error HIPHOP_PROJECT_VERSION is not set)
endif

ifneq ($(HIPHOP_AS_DSP_PATH),)
AS_DSP = true
HIPHOP_ENABLE_WASI ?= true
endif

ifneq ($(HIPHOP_WEB_UI_PATH),)
WEB_UI = true
endif

NPM_OPT_SET_PATH = true

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
# Utility for determining plugin library path. Use the standard plugin resources
# path when available. User defined TARGETS variable becomes available only
# *after* inclusion of this Makefile hence the usage of the 'test' command.

TEST_LV2 = test -d $(TARGET_DIR)/$(NAME).lv2
TEST_VST3 = test -d $(TARGET_DIR)/$(NAME).vst3
TEST_VST2_LINUX = test -f $(TARGET_DIR)/$(NAME)-vst.so
TEST_VST2_MACOS = test -d $(TARGET_DIR)/$(NAME).vst
TEST_VST2_WINDOWS = test -f $(TARGET_DIR)/$(NAME)-vst.dll
TEST_JACK_LINUX_OR_MACOS = test -f $(TARGET_DIR)/$(NAME)
TEST_JACK_WINDOWS = test -f $(TARGET_DIR)/$(NAME).exe
TEST_NOBUNDLE = $(TEST_VST2_WINDOWS) || $(TEST_VST2_LINUX) \
                || $(TEST_JACK_LINUX_OR_MACOS) || $(TEST_JACK_WINDOWS)

LIB_DIR_LV2 = $(TARGET_DIR)/$(NAME).lv2/lib
LIB_DIR_VST3 = $(TARGET_DIR)/$(NAME).vst3/Contents/Resources
LIB_DIR_VST2_MACOS = $(TARGET_DIR)/$(NAME).vst/Contents/Resources
LIB_DIR_NOBUNDLE = $(TARGET_DIR)/$(NAME)-lib

# ------------------------------------------------------------------------------
# Add optional support for AssemblyScript DSP

ifeq ($(AS_DSP),true)
HIPHOP_FILES_DSP  = WasmHostPlugin.cpp \
                    WasmEngine.cpp
ifeq ($(LINUX),true)
HIPHOP_FILES_DSP += linux/LinuxPath.cpp
endif
ifeq ($(MACOS),true)
HIPHOP_FILES_DSP += macos/MacPath.mm
endif
ifeq ($(WINDOWS),true)
HIPHOP_FILES_DSP += windows/WindowsPath.cpp
endif

FILES_DSP += $(HIPHOP_FILES_DSP:%=$(HIPHOP_SRC_PATH)/%)
endif

# ------------------------------------------------------------------------------
# Add optional support for web UI

ifeq ($(WEB_UI),true)
HIPHOP_FILES_UI  = AbstractWebHostUI.cpp \
                   AbstractWebView.cpp \
                   JsValue.cpp
ifeq ($(LINUX),true)
HIPHOP_FILES_UI += linux/LinuxPath.cpp \
                   linux/LinuxWebHostUI.cpp \
                   linux/ChildProcessWebView.cpp \
                   linux/IpcChannel.cpp \
                   linux/ipc.c
endif
ifeq ($(MACOS),true)
HIPHOP_FILES_UI += macos/MacPath.mm \
                   macos/MacWebHostUI.mm \
                   macos/CocoaWebView.mm
endif
ifeq ($(WINDOWS),true)
HIPHOP_FILES_UI += windows/WindowsPath.cpp \
                   windows/WindowsWebHostUI.cpp \
                   windows/EdgeWebView.cpp \
                   windows/WebView2EventHandler.cpp \
                   windows/cJSON.c
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

ifeq ($(WEB_UI),true)
UI_TYPE = external
endif

include $(DPF_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add shared build flags

BASE_FLAGS += -I$(HIPHOP_SRC_PATH) -I$(DPF_PATH) -DPLUGIN_BIN_BASENAME=$(NAME) \
              -DHIPHOP_PROJECT_ID_HASH=$(shell echo $(NAME):$(HIPHOP_PROJECT_VERSION) \
              	| shasum -a 256 | head -c 8)
ifeq ($(MACOS),true)
# This is needed otherwise expect crashes on older macOS when compiling on newer
# systems. Minimum supported target is High Sierra when WKWebView was introduced.
BASE_FLAGS += -mmacosx-version-min=10.13
endif

# ------------------------------------------------------------------------------
# Add build flags for AssemblyScript DSP dependencies

ifeq ($(AS_DSP),true)
BASE_FLAGS += -I$(WASMER_PATH)/include
ifeq ($(HIPHOP_ENABLE_WASI),true)
BASE_FLAGS += -DHIPHOP_ENABLE_WASI
endif
LINK_FLAGS += -L$(WASMER_PATH)/lib -lwasmer
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework AppKit 
endif
ifeq ($(WINDOWS),true)
LINK_FLAGS += -Wl,-Bstatic -lWs2_32 -lBcrypt -lUserenv -lShlwapi
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
BASE_FLAGS += -I$(EDGE_WEBVIEW2_PATH)/build/native/include -Wno-unknown-pragmas
LINK_FLAGS += -L$(EDGE_WEBVIEW2_PATH)/build/native/x64 \
              -lole32 -lShlwapi -lMfplat -lksuser -lmfuuid -lwmcodecdspuuid \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic \
              -lstdc++ -lpthread
endif
endif

# ------------------------------------------------------------------------------
# Print some info

TARGETS += info

info:
	@echo "hiphop : $(HIPHOP_ROOT_PATH)"
	@echo "DPF    : $(DPF_PATH) @ $(DPF_GIT_BRANCH)"
	@echo "Build  : $(DPF_BUILD_DIR)"
	@echo "Target : $(DPF_TARGET_DIR)"

# ------------------------------------------------------------------------------
# Dependency - Build DPF Graphics Library

ifneq ($(WEB_UI),true)
TARGETS += $(DPF_PATH)/build/libdgl.a

$(DPF_PATH)/build/libdgl.a:
	make -C $(DPF_PATH) dgl
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
# Wasmer official Windows binary distribution requires MSVC, download a custom build for MinGW
WASMER_PKG_FILE = wasmer-mingw-amd64.tar.gz
WASMER_URL = https://github.com/lucianoiam/hiphop/files/6795372/wasmer-mingw-amd64.tar.gz
endif

# https://stackoverflow.com/questions/37038472/osx-how-to-statically-link-a-library-and-dynamically-link-the-standard-library
$(WASMER_PATH):
	@wget -4 -O /tmp/$(WASMER_PKG_FILE) $(WASMER_URL)
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
NPM_OPT_SET_PATH = export PATH=$$PATH:/opt/node && export NODE_SKIP_PLATFORM_CHECK=1
ifeq (,$(wildcard /opt/node))
NPM_VERSION = 16.6.0
NPM_FILENAME = node-v$(NPM_VERSION)-win-x64.zip
NPM_URL = https://nodejs.org/dist/v$(NPM_VERSION)/$(NPM_FILENAME)

TARGETS += /opt/node/npm

/opt/node/npm:
	@echo Downloading Node.js
	@wget -4 -P /tmp $(NPM_URL)
	@unzip -o /tmp/$(NPM_FILENAME) -d /opt
	@mv /opt/$(basename $(NPM_FILENAME)) /opt/node
	@rm /tmp/$(NPM_FILENAME)
endif
endif
endif

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
	@echo Downloading Edge WebView2 SDK
	@mkdir -p $(HIPHOP_LIB_PATH)
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory $(HIPHOP_LIB_PATH)
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet
	@wget -4 -P /usr/bin $(NUGET_URL)
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Built-in JavaScript library include and polyfills

ifeq ($(WEB_UI),true)
UI_JS_PATH = $(HIPHOP_SRC_PATH)/ui/distrho-ui.js
UI_JS_INCLUDE_PATH = $(UI_JS_PATH).inc

TARGETS += $(UI_JS_INCLUDE_PATH)

$(UI_JS_INCLUDE_PATH): $(UI_JS_PATH)
	@echo 'R"JS(' > $(UI_JS_INCLUDE_PATH)
	@cat $(UI_JS_PATH) >> $(UI_JS_INCLUDE_PATH)
	@echo ')JS"' >> $(UI_JS_INCLUDE_PATH)

ifeq ($(MACOS),true)
POLYFILL_JS_PATH = $(HIPHOP_SRC_PATH)/ui/event-target-polyfill.js
POLYFILL_JS_INCLUDE_PATH = $(POLYFILL_JS_PATH).inc

TARGETS += $(POLYFILL_JS_INCLUDE_PATH)

$(POLYFILL_JS_INCLUDE_PATH): $(POLYFILL_JS_PATH)
	@echo 'R"JS(' > $(POLYFILL_JS_INCLUDE_PATH)
	@cat $(POLYFILL_JS_PATH) >> $(POLYFILL_JS_INCLUDE_PATH)
	@echo ')JS"' >> $(POLYFILL_JS_INCLUDE_PATH)

endif
endif

# ------------------------------------------------------------------------------
# Linux only - Build webview helper binary

ifeq ($(WEB_UI),true)
ifeq ($(LINUX),true)
LXWEBVIEW_TYPE ?= gtk

ifeq ($(LXWEBVIEW_TYPE),gtk)
BASE_FLAGS += -DLXWEBVIEW_GTK
endif
ifeq ($(LXWEBVIEW_TYPE),cef)
BASE_FLAGS += -DLXWEBVIEW_CEF
endif

HIPHOP_TARGET += lxhelper_bin

LXHELPER_NAME = ui-helper
LXHELPER_BUILD_DIR = $(BUILD_DIR)/helper

include $(HIPHOP_SRC_PATH)/linux/Makefile.$(LXWEBVIEW_TYPE).mk
endif
endif

# ------------------------------------------------------------------------------
# Mac only - Build Objective-C++ files

ifeq ($(MACOS),true)
$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
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
# Post build - Copy Linux helper files

ifeq ($(WEB_UI),true)
ifeq ($(LINUX),true)
HIPHOP_TARGET += lxhelper_res

lxhelper_res:
	@echo "Copying UI helper files"
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2) \
		&& cp -ru $(LXHELPER_FILES) $(LIB_DIR_LV2) \
		) || true
	@($(TEST_VST3) \
		&& mkdir -p $(LIB_DIR_VST3) \
		&& cp -ru $(LXHELPER_FILES) $(LIB_DIR_VST3) \
		) || true
	@($(TEST_NOBUNDLE) \
		&& mkdir -p $(LIB_DIR_NOBUNDLE) \
		&& cp -ru $(LXHELPER_FILES) $(LIB_DIR_NOBUNDLE) \
		) || true
endif
endif

# ------------------------------------------------------------------------------
# Post build - Compile AssemblyScript project

ifneq ($(AS_DSP),)
ifneq ($(HIPHOP_AS_SKIP_FRAMEWORK_FILES),true)
HIPHOP_TARGET += framework_as

AS_ASSEMBLY_PATH = $(HIPHOP_AS_DSP_PATH)/assembly

framework_as:
	@test -f $(AS_ASSEMBLY_PATH)/index.ts \
		|| ln -s $(abspath $(HIPHOP_SRC_PATH)/dsp/index.ts) $(AS_ASSEMBLY_PATH)
	@test -f $(AS_ASSEMBLY_PATH)/distrho-plugin.ts \
		|| ln -s $(abspath $(HIPHOP_SRC_PATH)/dsp/distrho-plugin.ts) $(AS_ASSEMBLY_PATH)
endif

WASM_SRC_PATH = $(HIPHOP_AS_DSP_PATH)/build/optimized.wasm
WASM_MODULE = main.wasm

HIPHOP_TARGET += $(WASM_SRC_PATH)

$(WASM_SRC_PATH): $(AS_ASSEMBLY_PATH)/plugin.ts
	@echo "Building AssemblyScript project"
	@# npm --prefix fails on MinGW due to paths mixing \ and /
	@test -d $(HIPHOP_AS_DSP_PATH)/node_modules \
		|| (cd $(HIPHOP_AS_DSP_PATH) && $(NPM_OPT_SET_PATH) && npm install)
	@cd $(HIPHOP_AS_DSP_PATH) && $(NPM_OPT_SET_PATH) && npm run asbuild
endif

# ------------------------------------------------------------------------------
# Post build - Always copy AssemblyScript DSP binary

ifneq ($(AS_DSP),)
HIPHOP_TARGET += lib_dsp

lib_dsp:
	@echo "Copying WebAssembly DSP binary"
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2)/dsp \
		&& cp -r $(WASM_SRC_PATH) $(LIB_DIR_LV2)/dsp/$(WASM_MODULE) \
		) || true
	@($(TEST_VST3) \
		&& mkdir -p $(LIB_DIR_VST3)/dsp \
		&& cp -r $(WASM_SRC_PATH) $(LIB_DIR_VST3)/dsp/$(WASM_MODULE) \
		) || true
	@($(TEST_VST2_MACOS) \
		&& mkdir -p $(LIB_DIR_VST2_MACOS)/dsp \
		&& cp -r $(WASM_SRC_PATH) $(LIB_DIR_VST2_MACOS)/dsp/$(WASM_MODULE) \
		) || true
	@($(TEST_NOBUNDLE) \
		&& mkdir -p $(LIB_DIR_NOBUNDLE)/dsp \
		&& cp -r $(WASM_SRC_PATH) $(LIB_DIR_NOBUNDLE)/dsp/$(WASM_MODULE) \
		) || true
endif

# ------------------------------------------------------------------------------
# Post build - Always copy web UI files

ifeq ($(WEB_UI),true)
HIPHOP_TARGET += lib_ui

lib_ui:
	@echo "Copying web UI files"
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2)/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(LIB_DIR_LV2)/ui \
		) || true
	@($(TEST_VST3) \
		&& mkdir -p $(LIB_DIR_VST3)/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(LIB_DIR_VST3)/ui \
		) || true
	@($(TEST_VST2_MACOS) \
		&& mkdir -p $(LIB_DIR_VST2_MACOS)/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(LIB_DIR_VST2_MACOS)/ui \
		) || true
	@($(TEST_NOBUNDLE) \
		&& mkdir -p $(LIB_DIR_NOBUNDLE)/ui \
		&& cp -r $(HIPHOP_WEB_UI_PATH)/* $(LIB_DIR_NOBUNDLE)/ui \
		) || true

clean: clean_lib

clean_lib:
	@rm -rf $(LIB_DIR_NOBUNDLE)
endif

# ------------------------------------------------------------------------------
# Post build - Copy Windows Edge WebView2 DLL, currently only 64-bit is supported

ifeq ($(WEB_UI),true)
ifeq ($(WINDOWS),true)
HIPHOP_TARGET += edge_lib
WEBVIEW_DLL = $(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll

edge_lib:
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2) \
		&& cp $(WEBVIEW_DLL) $(LIB_DIR_LV2) \
		) || true
	@($(TEST_NOBUNDLE) \
		&& mkdir -p $(LIB_DIR_NOBUNDLE) \
		&& cp $(WEBVIEW_DLL) $(LIB_DIR_NOBUNDLE) \
		) || true
endif
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
