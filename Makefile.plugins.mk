# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Basic setup

PROJECT_VERSION ?= 1

WEBUI_ROOT_PATH := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
WEBUI_SRC_PATH  ?= $(WEBUI_ROOT_PATH)/src

DPF_PATH       ?= $(WEBUI_ROOT_PATH)/dpf
DPF_TARGET_DIR ?= bin
DPF_BUILD_DIR  ?= build

DPF_GIT_BRANCH ?= develop

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

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

ifeq ($(WINDOWS),true)
$(info FIXME - Windows DPF build for develop branch is broken as of 21.07.04)
DPF_GIT_BRANCH = e28b6770f6d85396d3cf887ecad8bc2d63313eb6
DPF_CUSTOM_PATH       = $(WEBUI_ROOT_PATH)/dpf
DPF_CUSTOM_TARGET_DIR = bin
DPF_CUSTOM_BUILD_DIR  = build
endif

# ------------------------------------------------------------------------------
# Prepare build

# TODO: Make this a recipe
ifeq (,$(wildcard $(DPF_PATH)/Makefile))
_ := $(shell git submodule update --init --recursive)
endif

# TODO: Make this a recipe
ifneq (,$(DPF_GIT_BRANCH))
ifeq (,$(findstring $(DPF_GIT_BRANCH),$(shell git -C $(DPF_PATH) branch --show-current)))
_ := $(shell git -C $(DPF_PATH) checkout $(DPF_GIT_BRANCH))
endif
endif

# TODO: Make this a recipe
ifeq ($(MACOS),true)
ifeq ($(shell grep -c FIXME_MacScaleFactor $(DPF_PATH)/distrho/src/DistrhoUI.cpp),0)
$(info Patching DistrhoUI.cpp to workaround window size bug on macOS...)
_ := $(shell cd $(WEBUI_ROOT_PATH) && patch -u dpf/distrho/src/DistrhoUI.cpp -i src/DistrhoUI.cpp.patch)
endif
endif

# ------------------------------------------------------------------------------
# Add web UI source

WEBUI_FILES_UI  = ProxyWebUI.cpp \
                  AbstractWebWidget.cpp \
                  ScriptValue.cpp
ifeq ($(LINUX),true)
WEBUI_FILES_UI += linux/ExternalGtkWebWidget.cpp \
                  linux/PlatformLinux.cpp \
                  linux/extra/ipc.c
endif
ifeq ($(MACOS),true)
WEBUI_FILES_UI += macos/CocoaWebWidget.mm \
                  macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
WEBUI_FILES_UI += windows/EdgeWebWidget.cpp \
                  windows/KeyboardRouter.cpp \
                  windows/PlatformWindows.cpp \
                  windows/extra/WebView2EventHandler.cpp \
                  windows/extra/WinApiStub.cpp \
                  windows/extra/cJSON.c \
                  windows/res/plugin.rc
endif

FILES_UI += $(WEBUI_FILES_UI:%=$(WEBUI_SRC_PATH)/%)

ifneq ($(WINDOWS),true)
UI_TYPE = cairo
else
UI_TYPE = opengl
endif

# ------------------------------------------------------------------------------
# Include DPF Makefile for plugins

include $(DPF_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add build flags for web UI dependencies

BASE_FLAGS += -I. -I$(WEBUI_SRC_PATH) -I$(DPF_PATH) -DBIN_BASENAME=$(NAME) \
              -DPROJECT_ID_HASH=$(shell echo $(NAME):$(PROJECT_VERSION) | shasum -a 256 | head -c 8)
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I$(EDGE_WEBVIEW2_PATH)/build/native/include
LINK_FLAGS += -L$(EDGE_WEBVIEW2_PATH)/build/native/x64 \
              -lole32 -lShlwapi -lMfplat -lksuser -lmfuuid -lwmcodecdspuuid -lWebView2Loader.dll \
              -static-libgcc -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread
endif

# ------------------------------------------------------------------------------
# DPF graphics library

TARGETS += $(DPF_PATH)/build/libdgl.a

$(DPF_PATH)/build/libdgl.a:
	make -C $(DPF_PATH) dgl

# ------------------------------------------------------------------------------
# LV2 manifest files

ifeq ($(CAN_GENERATE_TTL),true)
WEBUI_TARGET += lv2ttl

lv2ttl: $(DPF_PATH)/utils/lv2_ttl_generator
	@$(DPF_PATH)/utils/generate-ttl.sh

$(DPF_PATH)/utils/lv2_ttl_generator:
	$(MAKE) -C $(DPF_PATH)/utils/lv2-ttl-generator
endif

# ------------------------------------------------------------------------------
# Linux WebKitGTK helper binary

ifeq ($(LINUX),true)
LXHELPER_BIN = $(BUILD_DIR)/$(NAME)_ui
WEBUI_TARGET += $(LXHELPER_BIN)

$(LXHELPER_BIN): $(WEBUI_SRC_PATH)/linux/helper.c $(WEBUI_SRC_PATH)/linux/extra/ipc.c
	@echo "Building helper..."
	$(SILENT)$(CC) $^ -I$(WEBUI_SRC_PATH) -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
	@(test -f $(TARGET_DIR)/$(NAME) || test -f $(TARGET_DIR)/$(NAME)-vst.so \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR) \
		) || true
	@(test -d $(TARGET_DIR)/$(NAME).lv2 \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR)/$(NAME).lv2 \
		) || true
	@(test -d $(TARGET_DIR)/$(NAME)-dssi \
		&& cp $(LXHELPER_BIN) $(TARGET_DIR)/$(NAME)-dssi \
		) || true

clean: clean_lxhelper

clean_lxhelper:
	@rm -rf $(TARGET_DIR)/$(notdir $(LXHELPER_BIN))
	@rm -rf $(LXHELPER_BIN)
endif

# ------------------------------------------------------------------------------
# macOS VST bundle and Objective-C++ compilation

ifeq ($(MACOS),true)
WEBUI_TARGET += macvst

macvst:
	@$(DPF_PATH)/utils/generate-vst-bundles.sh

clean: clean_macvst

clean_macvst:
	@rm -rf $(TARGET_DIR)/$(NAME).vst

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# ------------------------------------------------------------------------------
# Windows requires compiling resource files and linking to Edge WebView2
# This Makefile version is too lazy to support 32-bit but DLL is also available
# The standalone JACK program requires a "bare" DLL instead of assembly

ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH = /opt/Microsoft.Web.WebView2
TARGETS += $(EDGE_WEBVIEW2_PATH)
WEBUI_TARGET += copywindll

ifeq (,$(shell which nuget 2>/dev/null))
ifneq ($(MSYS_MINGW),true)
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif

copywindll:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
	@(test -f $(TARGET_DIR)/$(NAME).exe \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR) \
		) || true
	@(test -f $(TARGET_DIR)/$(NAME).lv2 \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		&& cp $(WEBUI_SRC_PATH)/windows/res/WebView2Loader.manifest $(TARGET_DIR)/$(NAME).lv2/WebView2Loader \
		) || true
	@(test -f $(TARGET_DIR)/$(NAME)-vst.dll \
		&& mkdir -p $(TARGET_DIR)/WebView2Loader \
		&& cp $(WEBVIEW_DLL) $(TARGET_DIR)/WebView2Loader \
		&& cp $(WEBUI_SRC_PATH)/windows/res/WebView2Loader.manifest $(TARGET_DIR)/WebView2Loader \
		) || true

clean: clean_windll

clean_windll:
	@rm -rf $(TARGET_DIR)/WebView2Loader

ifeq ($(MSYS_MINGW),true)
$(EDGE_WEBVIEW2_PATH): /usr/bin/nuget.exe
else
$(EDGE_WEBVIEW2_PATH):
endif
	@echo Downloading Edge WebView2 SDK...
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory /opt
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet...
	@wget -P /usr/bin https://dist.nuget.org/win-x86-commandline/latest/nuget.exe

$(BUILD_DIR)/%.rc.o: %.rc
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@windres --input $< --output $@ --output-format=coff
endif

# ------------------------------------------------------------------------------
# Always copy UI files

WEBUI_TARGET += resources

resources:
	@echo "Copying web UI resource files..."
	@(test -f $(TARGET_DIR)/$(NAME) || test -f $(TARGET_DIR)/$(NAME).exe || test -f $(TARGET_DIR)/$(NAME)-vst.dll \
		&& mkdir -p $(TARGET_DIR)/$(NAME)_res \
		&& cp -r ui/* $(TARGET_DIR)/$(NAME)_res \
		) || true
	@(test -d $(TARGET_DIR)/$(NAME).lv2 \
		&& mkdir -p $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res \
		&& cp -r ui/* $(TARGET_DIR)/$(NAME).lv2/$(NAME)_res \
		) || true
	@(test -d $(TARGET_DIR)/$(NAME)-dssi \
		&& mkdir -p $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res \
		&& cp -r ui/* $(TARGET_DIR)/$(NAME)-dssi/$(NAME)_res \
		) || true
	@(test -d $(TARGET_DIR)/$(NAME).vst \
		&& cp -r ui/* $(TARGET_DIR)/$(NAME).vst/Contents/Resources \
		) || true

clean: clean_resources

clean_resources:
	@rm -rf $(TARGET_DIR)/$(NAME)_res
