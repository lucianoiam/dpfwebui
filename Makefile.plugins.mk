# Filename: Makefile.plugins.mk
# Author:   oss@lucianoiam.com

# Allow placing DPF in a custom directory while including its Makefiles
DPF_CUSTOM_PATH = ./lib/DPF
DPF_CUSTOM_TARGET_DIR = ./bin
DPF_CUSTOM_BUILD_DIR = ./build

# Note this is not the DPF version of Makefile.base.mk
DPF_GIT_BRANCH=develop
include Makefile.base.mk

SRC_FILES_UI += base/ProxyWebUI.cpp \
                base/BaseWebWidget.cpp \
                base/ScriptValue.cpp

# Add ProxyWebUI files
ifeq ($(LINUX),true)
SRC_FILES_UI += arch/linux/ExternalGtkWebWidget.cpp \
                arch/linux/PlatformLinux.cpp \
                arch/linux/extra/ipc.c
endif
ifeq ($(MACOS),true)
SRC_FILES_UI += arch/macos/CocoaWebWidget.mm \
                arch/macos/PlatformMac.mm
endif
ifeq ($(WINDOWS),true)
SRC_FILES_UI += arch/windows/EdgeWebWidget.cpp \
                arch/windows/PlatformWindows.cpp \
                arch/windows/extra/WebView2EventHandler.cpp \
                arch/windows/extra/WinApiStub.cpp \
                arch/windows/extra/cJSON.c \
                arch/windows/res/plugin.rc
endif

FILES_DSP = $(SRC_FILES_DSP:%=src/%)
FILES_UI = $(SRC_FILES_UI:%=src/%)

ifneq ($(WINDOWS),true)
UI_TYPE = cairo
else
UI_TYPE = opengl
endif

# Now the magic
include $(DPF_CUSTOM_PATH)/Makefile.plugins.mk
