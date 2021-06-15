#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by lucianoiam

# Note this is not DPF's own Makefile.base.mk
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
# Note this is not DPF's own Makefile.plugins.mk
include Makefile.plugins.mk

all: $(DEP_TARGETS) $(TARGETS)

# --------------------------------------------------------------
