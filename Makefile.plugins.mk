# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Basic setup

PROJECT_VERSION ?= 1

WEBUI_ROOT_PATH := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
WEBUI_SRC_PATH ?= $(WEBUI_ROOT_PATH)/src

DPF_CUSTOM_PATH ?= $(WEBUI_ROOT_PATH)/dpf
DPF_CUSTOM_TARGET_DIR ?= bin
DPF_CUSTOM_BUILD_DIR ?= build

DPF_GIT_BRANCH ?= develop

# ------------------------------------------------------------------------------
# Prepare build environment

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
$(info Run MSYS as administrator for real symlink support)
MSYS_MINGW_SYMLINKS = :
else
MSYS_MINGW_SYMLINKS = export MSYS=winsymlinks:nativestrict
endif
endif

# TODO: Make this a recipe
ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
_ := $(shell git submodule update --init --recursive)
endif

# TODO: Make this a recipe
ifneq (,$(DPF_GIT_BRANCH))
ifeq (,$(findstring $(DPF_GIT_BRANCH),$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
_ := $(shell git -C $(DPF_CUSTOM_PATH) checkout $(DPF_GIT_BRANCH))
endif
endif

# TODO: Make this a recipe
ifeq ($(MACOS),true)
ifeq ($(shell grep -c FIXME_MacScaleFactor $(DPF_CUSTOM_PATH)/distrho/src/DistrhoUI.cpp),0)
$(info Patching DistrhoUI.cpp to workaround window size bug on macOS...)
_ := $(shell cd $(WEBUI_ROOT_PATH) && patch -u dpf/distrho/src/DistrhoUI.cpp -i src/DistrhoUI.cpp.patch)
endif
endif

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
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

include $(DPF_CUSTOM_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add build flags for web UI dependencies

BASE_FLAGS += -I. -I$(WEBUI_SRC_PATH) -I$(DPF_CUSTOM_PATH) -DBIN_BASENAME=$(NAME) \
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
# Define all web UI targets

# Target for DPF graphics library
TARGETS += $(DPF_CUSTOM_PATH)/build/libdgl.a

$(DPF_CUSTOM_PATH)/build/libdgl.a:
	make -C $(DPF_CUSTOM_PATH) dgl

# Target for generating LV2 TTL files
ifeq ($(CAN_GENERATE_TTL),true)
WEBUI_TARGET += lv2ttl

lv2ttl: $(DPF_CUSTOM_PATH)/utils/lv2_ttl_generator
	@$(DPF_CUSTOM_PATH)/utils/generate-ttl.sh

$(DPF_CUSTOM_PATH)/utils/lv2_ttl_generator:
	$(MAKE) -C $(DPF_CUSTOM_PATH)/utils/lv2-ttl-generator
endif

# Linux requires a helper binary
ifeq ($(LINUX),true)
LXHELPER_BIN = /tmp/$(NAME)_ui
WEBUI_TARGET += $(LXHELPER_BIN)

$(LXHELPER_BIN): $(WEBUI_SRC_PATH)/linux/helper.c $(WEBUI_SRC_PATH)/linux/extra/ipc.c
	@echo "Building helper..."
	$(SILENT)$(CC) $^ -Isrc -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
ifneq ($(filter jack,$(TARGETS)),)
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)
endif
ifneq (,$(findstring lv2,$(shell echo $(TARGETS)[@])))
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
endif
ifneq ($(filter dssi,$(TARGETS)),)
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi
endif
ifneq ($(filter vst,$(TARGETS)),)
	@cp -n $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)
endif
	@rm $(LXHELPER_BIN)

clean: clean_lxhelper

clean_lxhelper:
	rm -rf $(LXHELPER_BIN)
endif

# Mac requires compiling Objective-C++ and creating a VST bundle
ifeq ($(MACOS),true)
WEBUI_TARGET += macvst

macvst:
	@$(DPF_CUSTOM_PATH)/utils/generate-vst-bundles.sh

clean: clean_macvst

clean_macvst:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# Windows requires compiling resource files and linking to Edge WebView2
# The current Makefile is too lazy to support 32-bit but DLL is also available
ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH = ./lib/Microsoft.Web.WebView2
TARGETS += $(EDGE_WEBVIEW2_PATH)
WEBUI_TARGET += copywindll

ifeq (,$(shell which nuget 2>/dev/null))
ifneq ($(MSYS_MINGW),true)
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif

# The standalone Windows version requires a "bare" DLL instead of the assembly
copywindll:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
ifneq ($(filter jack,$(TARGETS)),)
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)
endif
ifneq (,$(findstring lv2,$(shell echo $(TARGETS)[@])))
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp $(WEBUI_SRC_PATH)/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
endif
ifneq ($(filter vst,$(TARGETS)),)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp $(WEBUI_SRC_PATH)/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
endif

clean: clean_windll

clean_windll:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader

ifeq ($(MSYS_MINGW),true)
$(EDGE_WEBVIEW2_PATH): /usr/bin/nuget.exe
else
$(EDGE_WEBVIEW2_PATH):
endif
	@echo Downloading Edge WebView2 SDK...
	@eval $(MSYS_MINGW_SYMLINKS)
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

# Always copy web UI files
WEBUI_TARGET += resources

QNAME = $(DPF_CUSTOM_TARGET_DIR)/$(NAME)

resources:
	@echo "Copying web UI resource files..."
	@(test -f $(QNAME) || test -f $(QNAME).exe || test -f $(QNAME)-vst.dll \
		&& mkdir -p $(QNAME)_res && cp -r ui/* $(QNAME)_res) || true
	@(test -d $(QNAME).lv2 \
		&& mkdir -p $(QNAME).lv2/$(NAME)_res && cp -r ui/* $(QNAME).lv2/$(NAME)_res) || true
	@(test -d $(QNAME)-dssi \
		&& mkdir -p $(QNAME)-dssi/$(NAME)_res && cp -r ui/* $(QNAME)-dssi/$(NAME)_res) || true
	@(test -d $(QNAME).vst \
		&& cp -r ui/* $(QNAME).vst/Contents/Resources) || true

clean: clean_resources

clean_resources:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_res
