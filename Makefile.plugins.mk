# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Configuration defaults

# Location for binaries
DPF_TARGET_DIR ?= bin
# Location for object files
DPF_BUILD_DIR ?= build
# Enable built-in websockets server and load content over HTTP
DPF_WEBUI_NETWORK_UI ?= false
# (WIP) Enable HTTPS and secure WebSockets
DPF_WEBUI_NETWORK_SSL ?= false
# Build a type of Variant backed by libbson
DPF_WEBUI_SUPPORT_BSON ?= false
# Automatically inject dpf.js when loading content from file://
DPF_WEBUI_INJECT_FRAMEWORK_JS ?= false
# Web view implementation on Linux [ gtk | cef ]
DPF_WEBUI_LINUX_WEBVIEW ?= gtk
# Set to false for building current architecture only
DPF_WEBUI_MACOS_UNIVERSAL ?= false
# Support macOS down to High Sierra when WKWebView was introduced. This setting
# must be enabled when compiling on newer systems otherwise plugins will crash.
DPF_WEBUI_MACOS_OLD ?= false
# Build a rough and incomplete JACK application for development purposes
DPF_WEBUI_MACOS_DEV_STANDALONE ?= false
# Target directory for dpf.js and libraries relative to plugin ui directory
DPF_WEBUI_JS_LIB_TARGET_PATH ?=

ifeq ($(DPF_WEBUI_PROJECT_VERSION),)
$(error DPF_WEBUI_PROJECT_VERSION is not set)
endif

# ------------------------------------------------------------------------------
# Determine build environment

DPF_WEBUI_ROOT_PATH := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
DPF_WEBUI_INC_PATH  = $(DPF_WEBUI_ROOT_PATH)/webui
DPF_WEBUI_SRC_PATH  = $(DPF_WEBUI_ROOT_PATH)/webui/src
DPF_WEBUI_DEPS_PATH = $(DPF_WEBUI_ROOT_PATH)/deps
DPF_PATH         = $(DPF_WEBUI_ROOT_PATH)/dpf

ifneq ($(DPF_WEBUI_WEB_UI_PATH),)
WEB_UI = true
endif

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
# Run MSYS as administrator for real symlink support
MSYS_MINGW_SYMLINKS = :
else
MSYS_MINGW_SYMLINKS = export MSYS=winsymlinks:nativestrict
endif
endif

# ------------------------------------------------------------------------------
# Utility for determining plugin library path. Use the standard plugin resources
# path when available. User defined TARGETS variable becomes available only
# *after* inclusion of this Makefile hence the usage of the 'test' command.

TEST_LV2 = test -d $(DPF_TARGET_DIR)/$(NAME).lv2
TEST_CLAP_LINUX_OR_WINDOWS = test -f $(DPF_TARGET_DIR)/$(NAME).clap
TEST_CLAP_MACOS = test -d $(DPF_TARGET_DIR)/$(NAME).clap
TEST_VST3 = test -d $(DPF_TARGET_DIR)/$(NAME).vst3
TEST_VST2_LINUX = test -f $(DPF_TARGET_DIR)/$(NAME)-vst.so
TEST_VST2_MACOS = test -d $(DPF_TARGET_DIR)/$(NAME).vst
TEST_VST2_WINDOWS = test -f $(DPF_TARGET_DIR)/$(NAME)-vst.dll
TEST_JACK_LINUX_OR_MACOS = test -f $(DPF_TARGET_DIR)/$(NAME)
TEST_JACK_WINDOWS = test -f $(DPF_TARGET_DIR)/$(NAME).exe
TEST_NOBUNDLE = $(TEST_CLAP_LINUX_OR_WINDOWS) \
				|| $(TEST_VST2_WINDOWS) || $(TEST_VST2_LINUX) \
				|| $(TEST_JACK_LINUX_OR_MACOS) || $(TEST_JACK_WINDOWS)

LIB_DIR_LV2 = $(DPF_TARGET_DIR)/$(NAME).lv2/lib
LIB_DIR_CLAP_MACOS = $(DPF_TARGET_DIR)/$(NAME).clap/Contents/Resources
LIB_DIR_VST3 = $(DPF_TARGET_DIR)/$(NAME).vst3/Contents/Resources
LIB_DIR_VST2_MACOS = $(DPF_TARGET_DIR)/$(NAME).vst/Contents/Resources
LIB_DIR_NOBUNDLE = $(DPF_TARGET_DIR)/$(NAME)-lib

# ------------------------------------------------------------------------------
# Support some features missing from DPF like shared memory

DPF_WEBUI_FILES_DSP = PluginEx.cpp
DPF_WEBUI_FILES_UI  = UIEx.cpp

FILES_DSP += $(DPF_WEBUI_FILES_DSP:%=$(DPF_WEBUI_SRC_PATH)/dsp/%)

# ------------------------------------------------------------------------------
# Code shared by both UI and DSP
DPF_WEBUI_FILES_SHARED += JSONVariant.cpp \
				       thirdparty/cJSON.c
ifeq ($(DPF_WEBUI_SUPPORT_BSON),true)
DPF_WEBUI_FILES_SHARED += BSONVariant.cpp
endif

FILES_DSP += $(DPF_WEBUI_FILES_SHARED:%=$(DPF_WEBUI_SRC_PATH)/%)

# ------------------------------------------------------------------------------
# Optional support for web UI

ifeq ($(WEB_UI),true)
DPF_WEBUI_FILES_UI += WebUIBase.cpp \
				   WebViewBase.cpp \
				   WebViewUI.cpp
ifeq ($(DPF_WEBUI_NETWORK_UI),true)
DPF_WEBUI_FILES_UI += NetworkUI.cpp \
				   WebServer.cpp
endif
ifeq ($(LINUX),true)
DPF_WEBUI_FILES_UI += linux/LinuxWebViewUI.cpp \
				   linux/ChildProcessWebView.cpp \
				   linux/IpcChannel.cpp \
				   linux/ipc.c \
				   linux/scaling.c
endif
ifeq ($(MACOS),true)
DPF_WEBUI_FILES_UI += macos/MacWebViewUI.mm \
				   macos/CocoaWebView.mm
endif
ifeq ($(WINDOWS),true)
DPF_WEBUI_FILES_UI += windows/WindowsWebViewUI.cpp \
				   windows/EdgeWebView.cpp \
				   windows/WebView2EventHandler.cpp
endif
endif

FILES_UI += $(DPF_WEBUI_FILES_UI:%=$(DPF_WEBUI_SRC_PATH)/ui/%)
FILES_UI += $(DPF_WEBUI_FILES_SHARED:%=$(DPF_WEBUI_SRC_PATH)/%)

# ------------------------------------------------------------------------------
# Optional support for macOS universal binaries, keep this before DPF include.

ifeq ($(MACOS),true)
ifeq ($(DPF_WEBUI_MACOS_UNIVERSAL),true)
# Non CPU-specific optimization flags, see DPF Makefile.base.mk
NOOPT = true
MACOS_UNIVERSAL_FLAGS = -arch x86_64 -arch arm64
CFLAGS += $(MACOS_UNIVERSAL_FLAGS)
CXXFLAGS += $(MACOS_UNIVERSAL_FLAGS)
LDFLAGS += $(MACOS_UNIVERSAL_FLAGS)
endif
endif

# ------------------------------------------------------------------------------
# Include DPF Makefile for plugins

# These commands cannot belong to a target because this Makefile relies on DPF's
ifeq (,$(wildcard $(DPF_PATH)/Makefile))
_ := $(shell git submodule update --init --recursive)
endif

ifeq ($(WEB_UI),true)
UI_TYPE = external
endif

include $(DPF_PATH)/Makefile.plugins.mk

# ------------------------------------------------------------------------------
# Add shared build flags

BASE_FLAGS += -I$(DPF_WEBUI_INC_PATH) -I$(DPF_WEBUI_SRC_PATH) -I$(DPF_PATH) \
			  -DDPF_WEBUI_PLUGIN_BIN_BASENAME=$(NAME) \
			  -DDPF_WEBUI_PROJECT_ID_HASH=$(shell echo $(NAME):$(DPF_WEBUI_PROJECT_VERSION) \
				 | shasum -a 256 | head -c 8)
ifeq ($(LINUX),true)
BASE_FLAGS += -lrt
endif
ifeq ($(MACOS),true)
# Mute lots of warnings from DPF: 'vfork' is deprecated: Use posix_spawn or fork
BASE_FLAGS += -Wno-deprecated-declarations
ifeq ($(DPF_WEBUI_MACOS_OLD),true)
# Warning: ...was built for newer macOS version (11.0) than being linked (10.13)
BASE_FLAGS += -mmacosx-version-min=10.13
endif
endif

# ------------------------------------------------------------------------------
# Add build flags for web UI dependencies

ifeq ($(WEB_UI),true)
  ifeq ($(DPF_WEBUI_NETWORK_UI),true)
	BASE_FLAGS += -DDPF_WEBUI_NETWORK_UI -I$(LWS_PATH)/include -I$(LWS_BUILD_PATH)
	LINK_FLAGS += -L$(LWS_BUILD_PATH)/lib
	ifeq ($(DPF_WEBUI_INJECT_FRAMEWORK_JS),true)
	$(warning Network UI is enabled - disabling JavaScript framework injection)
	DPF_WEBUI_INJECT_FRAMEWORK_JS = false
	endif
	ifeq ($(DPF_WEBUI_NETWORK_SSL), true)
	BASE_FLAGS += -I$(MBEDTLS_PATH)/include -DDPF_WEBUI_NETWORK_SSL
	LINK_FLAGS += -L$(MBEDTLS_BUILD_PATH) -lmbedtls -lmbedcrypto -lmbedx509
	endif
	ifeq ($(WINDOWS),true)
	LINK_FLAGS += -lwebsockets_static -lWs2_32
	else
	LINK_FLAGS += -lwebsockets
	endif
	ifeq ($(LINUX),true)
	# Add -lcap after -lwebsockets
	LINK_FLAGS += -lcap
	endif
	ifeq ($(MACOS),true)
	# Some lws-http.h constants clash with MacOSX.sdk/usr/include/cups/http.h
	BASE_FLAGS += -D_CUPS_CUPS_H_ -D_CUPS_HTTP_H_ -D_CUPS_PPD_H_
	endif
  endif
  ifeq ($(DPF_WEBUI_INJECT_FRAMEWORK_JS),true)
  BASE_FLAGS += -DDPF_WEBUI_INJECT_FRAMEWORK_JS
  endif
  ifeq ($(DPF_WEBUI_SUPPORT_BSON), true)
  BASE_FLAGS += -DDPF_WEBUI_SUPPORT_BSON -I$(LIBBSON_PATH)/src/libbson/src \
				-I$(LIBBSON_PATH)/build/src/libbson/src
  LINK_FLAGS += -L$(LIBBSON_BUILD_PATH)/src/libbson -lbson-static-1.0
  ifeq ($(WINDOWS),true)
	LINK_FLAGS += -lWs2_32
	endif
  endif
  ifeq ($(DPF_WEBUI_PRINT_TRAFFIC),true)
  BASE_FLAGS += -DDPF_WEBUI_PRINT_TRAFFIC
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
	@echo "Building $(NAME)"

# ------------------------------------------------------------------------------
# Development - JACK-based application

ifeq ($(DPF_WEBUI_MACOS_DEV_STANDALONE),true)
ifeq ($(MACOS),true)
ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += jack
endif
endif
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Build DPF Graphics Library

ifneq ($(WEB_UI),true)
  LIBDGL_PATH = $(DPF_PATH)/build/libdgl.a
  TARGETS += $(LIBDGL_PATH)
  ifeq ($(DPF_WEBUI_MACOS_UNIVERSAL),true)
  DGL_MAKE_FLAGS = dgl NOOPT=true DGL_FLAGS="$(MACOS_UNIVERSAL_FLAGS)" \
				 DGL_LIBS="$(MACOS_UNIVERSAL_FLAGS)"
  endif

  $(LIBDGL_PATH):
		make -C $(DPF_PATH) $(DGL_MAKE_FLAGS)
endif

# ------------------------------------------------------------------------------
# Dependency - Clone and build Mbed TLS

ifeq ($(WEB_UI),true)
ifeq ($(DPF_WEBUI_NETWORK_UI),true)
ifeq ($(DPF_WEBUI_NETWORK_SSL), true)
MBEDTLS_GIT_URL = https://github.com/ARMmbed/mbedtls
MBEDTLS_GIT_TAG = v3.5.2
MBEDTLS_PATH = $(DPF_WEBUI_DEPS_PATH)/mbedtls
MBEDTLS_BUILD_PATH = ${MBEDTLS_PATH}/library
MBEDTLS_LIB_PATH = $(MBEDTLS_BUILD_PATH)/libmbedtls.a

ifeq ($(SKIP_STRIPPING),true)
MBEDTLS_MAKE_ARGS = DEBUG=1
endif

TARGETS += $(MBEDTLS_LIB_PATH)

$(MBEDTLS_LIB_PATH): $(MBEDTLS_PATH)
	@echo "Building Mbed TLS static library"
	@mkdir -p $(MBEDTLS_BUILD_PATH) && cd $(MBEDTLS_BUILD_PATH) && make

$(MBEDTLS_PATH):
	@mkdir -p $(DPF_WEBUI_DEPS_PATH)
	@git -C $(DPF_WEBUI_DEPS_PATH) clone --depth 1 --branch $(MBEDTLS_GIT_TAG) \
			$(MBEDTLS_GIT_URL)
endif
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Clone and build libwebsockets

ifeq ($(WEB_UI),true)
ifeq ($(DPF_WEBUI_NETWORK_UI),true)
LWS_GIT_URL = https://github.com/warmcat/libwebsockets
LWS_GIT_TAG = v4.3.3
LWS_PATH = $(DPF_WEBUI_DEPS_PATH)/libwebsockets
LWS_BUILD_PATH = ${LWS_PATH}/build
LWS_LIB_PATH = $(LWS_BUILD_PATH)/lib/libwebsockets.a

LWS_CMAKE_ARGS = -DLWS_WITH_SHARED=0 -DLWS_WITHOUT_TESTAPPS=1
ifeq ($(DPF_WEBUI_NETWORK_SSL),true)
LWS_CMAKE_ARGS += -DLWS_WITH_SSL=1 -DLWS_WITH_MBEDTLS=1 \
				  -DLWS_MBEDTLS_INCLUDE_DIRS=../../mbedtls/include
else
LWS_CMAKE_ARGS += -DLWS_WITH_SSL=0
endif

ifeq ($(WINDOWS),true)
LWS_LIB_PATH = $(LWS_BUILD_PATH)/lib/libwebsockets_static.a
LWS_CMAKE_ARGS += -G"MSYS Makefiles"
endif

TARGETS += $(LWS_LIB_PATH)

LWS_CMAKE_ENV = true
ifeq ($(LINUX),true)
LWS_CMAKE_ENV = export CFLAGS=-fPIC
endif
ifeq ($(MACOS),true)
ifeq ($(DPF_WEBUI_MACOS_UNIVERSAL),true)
LWS_CMAKE_ENV = export CMAKE_OSX_ARCHITECTURES="arm64;x86_64;"
endif
endif

$(LWS_LIB_PATH): $(LWS_PATH)
	@echo "Building libwebsockets static library"
	@mkdir -p $(LWS_BUILD_PATH) && cd $(LWS_BUILD_PATH) && $(LWS_CMAKE_ENV) \
		&& cmake .. $(LWS_CMAKE_ARGS) && cmake --build .

$(LWS_PATH):
	@mkdir -p $(DPF_WEBUI_DEPS_PATH)
	@git -C $(DPF_WEBUI_DEPS_PATH) clone --depth 1 --branch $(LWS_GIT_TAG) $(LWS_GIT_URL)
endif
endif

# ------------------------------------
# Dependency - Clone and build libbson

ifeq ($(DPF_WEBUI_SUPPORT_BSON), true)
LIBBSON_GIT_URL = https://github.com/mongodb/mongo-c-driver
LIBBSON_GIT_TAG = 1.26.0
LIBBSON_PATH = $(DPF_WEBUI_DEPS_PATH)/mongo-c-driver
LIBBSON_BUILD_PATH = ${LIBBSON_PATH}/build
LIBBSON_LIB_PATH = $(LIBBSON_BUILD_PATH)/src/libbson/libbson-static-1.0.a
LIBBSON_CMAKE_ARGS = -DENABLE_MONGOC=OFF -DENABLE_TESTS=OFF -DENABLE_EXAMPLES=OFF \
					 -DENABLE_BSON=ON -DENABLE_STATIC=ON -DCMAKE_BUILD_TYPE=Release

TARGETS += $(LIBBSON_LIB_PATH)

LIBBSON_CMAKE_ENV = true
ifeq ($(MACOS),true)
ifeq ($(DPF_WEBUI_MACOS_UNIVERSAL),true)
LIBBSON_CMAKE_ENV = export CMAKE_OSX_ARCHITECTURES="arm64;x86_64;"
endif
endif
ifeq ($(WINDOWS),true)
# build/calc_release_version.py fails on MSYS due to git returning error
# "fatal: Needed a single revision". Pass version to CMake.
LIBBSON_CMAKE_ARGS += -G"MSYS Makefiles" -DBUILD_VERSION=$(LIBBSON_GIT_TAG)
# Prevent "error: implicit declaration of function 'aligned_alloc' [-Werror=implicit-function-declaration]"
LIBBSON_CMAKE_ENV = export CFLAGS=-std=c99
endif

$(LIBBSON_LIB_PATH): $(LIBBSON_PATH)
	@echo "Building libbson static library"
	@mkdir -p $(LIBBSON_BUILD_PATH) && cd $(LIBBSON_BUILD_PATH) && $(LIBBSON_CMAKE_ENV) \
		&& cmake .. $(LIBBSON_CMAKE_ARGS) && cmake --build .

$(LIBBSON_PATH):
	@mkdir -p $(DPF_WEBUI_DEPS_PATH)
	@git -C $(DPF_WEBUI_DEPS_PATH) clone --depth 1 --branch $(LIBBSON_GIT_TAG) $(LIBBSON_GIT_URL)
endif

# ------------------------------------------------------------------------------
# Dependency - Built-in JavaScript library include and polyfills

ifeq ($(WEB_UI),true)
ifeq ($(DPF_WEBUI_INJECT_FRAMEWORK_JS),true)
FRAMEWORK_JS_PATH = $(DPF_WEBUI_SRC_PATH)/ui/dpf.js
DPF_JS_INCLUDE_PATH = $(FRAMEWORK_JS_PATH).inc

TARGETS += $(DPF_JS_INCLUDE_PATH)

$(DPF_JS_INCLUDE_PATH): $(FRAMEWORK_JS_PATH)
	@echo 'R"JS(' > $(DPF_JS_INCLUDE_PATH)
	@cat $(FRAMEWORK_JS_PATH) >> $(DPF_JS_INCLUDE_PATH)
	@echo ')JS"' >> $(DPF_JS_INCLUDE_PATH)
endif

ifeq ($(MACOS),true)
POLYFILL_JS_PATH = $(DPF_WEBUI_SRC_PATH)/ui/macos/polyfill.js
POLYFILL_JS_INCLUDE_PATH = $(POLYFILL_JS_PATH).inc

TARGETS += $(POLYFILL_JS_INCLUDE_PATH)

$(POLYFILL_JS_INCLUDE_PATH): $(POLYFILL_JS_PATH)
	@echo 'R"JS(' > $(POLYFILL_JS_INCLUDE_PATH)
	@cat $(POLYFILL_JS_PATH) >> $(POLYFILL_JS_INCLUDE_PATH)
	@echo ')JS"' >> $(POLYFILL_JS_INCLUDE_PATH)
endif
endif

# ------------------------------------------------------------------------------
# Dependency - Download Edge WebView2

ifeq ($(WEB_UI),true)
ifeq ($(WINDOWS),true)
ifeq ($(MSYS_MINGW),true)
NUGET_URL = https://dist.nuget.org/win-x86-commandline/latest/nuget.exe
NUGET_BIN = /usr/bin/nuget.exe

TARGETS += $(NUGET_BIN)

$(NUGET_BIN):
	@echo Downloading NuGet
	@wget -4 -P /usr/bin $(NUGET_URL)
else
ifeq (,$(shell which nuget 2>/dev/null))
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif

EDGE_WEBVIEW2_PATH = $(DPF_WEBUI_DEPS_PATH)/Microsoft.Web.WebView2

TARGETS += $(EDGE_WEBVIEW2_PATH)

$(EDGE_WEBVIEW2_PATH):
	@echo Downloading Edge WebView2 SDK
	@mkdir -p $(DPF_WEBUI_DEPS_PATH)
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory $(DPF_WEBUI_DEPS_PATH)
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)
endif
endif

# ------------------------------------------------------------------------------
# Linux only - Build webview helper binary

ifeq ($(WEB_UI),true)
ifeq ($(LINUX),true)

ifeq ($(DPF_WEBUI_LINUX_WEBVIEW),gtk)
BASE_FLAGS += -DDPF_WEBUI_LINUX_WEBVIEW_GTK
endif
ifeq ($(DPF_WEBUI_LINUX_WEBVIEW),cef)
BASE_FLAGS += -DDPF_WEBUI_LINUX_WEBVIEW_CEF
endif

# See Makefile.cef.mk and Makefile.gtk.mk
DPF_WEBUI_TARGET += lxhelper_bin

LXHELPER_NAME = ui-helper
LXHELPER_BUILD_PATH = $(BUILD_DIR)/helper

include $(DPF_WEBUI_SRC_PATH)/ui/linux/Makefile.$(DPF_WEBUI_LINUX_WEBVIEW).mk
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
DPF_WEBUI_TARGET += lxhelper_res

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
# Post build - Always copy web UI files

ifeq ($(WEB_UI),true)
DPF_WEBUI_TARGET += lib_ui_plugin
LIB_UI_DIR = ui
ifneq ($(DPF_WEBUI_INJECT_FRAMEWORK_JS),true)
DPF_WEBUI_TARGET += lib_ui_framework
LIB_JS_PATH = $(LIB_UI_DIR)/$(DPF_WEBUI_JS_LIB_TARGET_PATH)
LIB_JS_FILES = $(DPF_WEBUI_SRC_PATH)/ui/dpf.js
ifeq ($(DPF_WEBUI_SUPPORT_BSON),true)
LIB_JS_FILES += $(DPF_WEBUI_SRC_PATH)/thirdparty/bson.min.js
endif
endif
# https://unix.stackexchange.com/questions/178235/how-is-cp-f-different-from-cp-remove-destination
CP_JS_ARGS = -f
ifeq ($(LINUX),true)
CP_JS_ARGS += --remove-destination
endif

lib_ui_plugin:
	@echo "Copying plugin web UI files"
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2)/$(LIB_UI_DIR) \
		&& cp -r $(DPF_WEBUI_WEB_UI_PATH)/* $(LIB_DIR_LV2)/$(LIB_UI_DIR) \
		) || true
	@($(TEST_CLAP_MACOS) \
		&& mkdir -p $(LIB_DIR_CLAP_MACOS)/$(LIB_UI_DIR) \
		&& cp -r $(DPF_WEBUI_WEB_UI_PATH)/* $(LIB_DIR_CLAP_MACOS)/$(LIB_UI_DIR) \
		) || true
	@($(TEST_VST3) \
		&& mkdir -p $(LIB_DIR_VST3)/$(LIB_UI_DIR) \
		&& cp -r $(DPF_WEBUI_WEB_UI_PATH)/* $(LIB_DIR_VST3)/$(LIB_UI_DIR) \
		) || true
	@($(TEST_VST2_MACOS) \
		&& mkdir -p $(LIB_DIR_VST2_MACOS)/$(LIB_UI_DIR) \
		&& cp -r $(DPF_WEBUI_WEB_UI_PATH)/* $(LIB_DIR_VST2_MACOS)/$(LIB_UI_DIR) \
		) || true
	@($(TEST_NOBUNDLE) \
		&& mkdir -p $(LIB_DIR_NOBUNDLE)/$(LIB_UI_DIR) \
		&& cp -r $(DPF_WEBUI_WEB_UI_PATH)/* $(LIB_DIR_NOBUNDLE)/$(LIB_UI_DIR) \
		) || true

lib_ui_framework:
	@echo "Copying framework web UI files"
	@for LIB_JS_FILE in $(LIB_JS_FILES) ; do \
		($(TEST_LV2) \
			&& cp $(CP_JS_ARGS) $$LIB_JS_FILE $(LIB_DIR_LV2)/$(LIB_JS_PATH) \
			) || true ; \
		($(TEST_CLAP_MACOS) \
			&& cp $(CP_JS_ARGS) $$LIB_JS_FILE $(LIB_DIR_CLAP_MACOS)/$(LIB_JS_PATH) \
			) || true ; \
		($(TEST_VST3) \
			&& cp $(CP_JS_ARGS) $$LIB_JS_FILE $(LIB_DIR_VST3)/$(LIB_JS_PATH) \
			) || true ; \
		($(TEST_VST2_MACOS) \
			&& cp $(CP_JS_ARGS) $$LIB_JS_FILE $(LIB_DIR_VST2_MACOS)/$(LIB_JS_PATH) \
			) || true ; \
		($(TEST_NOBUNDLE) \
			&& cp $(CP_JS_ARGS) $$LIB_JS_FILE $(LIB_DIR_NOBUNDLE)/$(LIB_JS_PATH) \
			) || true ; \
	done

clean: clean_lib

clean_lib:
	@rm -rf $(LIB_DIR_NOBUNDLE)
endif

# ------------------------------------------------------------------------------
# Post build - Copy Windows Edge WebView2 DLL, currently only 64-bit is supported

ifeq ($(WEB_UI),true)
ifeq ($(WINDOWS),true)
DPF_WEBUI_TARGET += edge_dll
WEBVIEW_DLL = $(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll

edge_dll:
	@($(TEST_LV2) \
		&& mkdir -p $(LIB_DIR_LV2) \
		&& cp $(WEBVIEW_DLL) $(LIB_DIR_LV2) \
		) || true
	@($(TEST_VST3) \
		&& mkdir -p $(LIB_DIR_VST3) \
		&& cp $(WEBVIEW_DLL) $(LIB_DIR_VST3) \
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
DPF_WEBUI_TARGET += lv2ttl

lv2ttl: $(DPF_PATH)/utils/lv2_ttl_generator
	@# generate-ttl.sh expects hardcoded directory bin/
	@cd $(DPF_TARGET_DIR)/.. && $(abspath $(DPF_PATH))/utils/generate-ttl.sh

$(DPF_PATH)/utils/lv2_ttl_generator:
	$(MAKE) -C $(DPF_PATH)/utils/lv2-ttl-generator
endif
