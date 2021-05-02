#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Web UI example by lucianoiam

DPF_CUSTOM_PATH = lib/dpf
DPF_CUSTOM_TARGET_DIR = bin
DPF_CUSTOM_BUILD_DIR = build

# Keep debug symbols (DPF Makefile.base.mk@148)
SKIP_STRIPPING = true

# --------------------------------------------------------------
# DISTRHO project name, used for binaries

NAME = d_dpf_webui

# --------------------------------------------------------------
# Files to build

SRC_FILES_DSP = \
	WebPlugin.cpp

SRC_FILES_UI  = \
	WebUI.cpp

FILES_DSP = $(SRC_FILES_DSP:%=src/%)
FILES_UI = $(SRC_FILES_UI:%=src/%)

# --------------------------------------------------------------
# Do some magic

UI_TYPE = cairo
include $(DPF_CUSTOM_PATH)/Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_CAIRO),true)
TARGETS += jack
endif
endif

ifneq ($(MACOS_OR_WINDOWS),true)
ifeq ($(HAVE_CAIRO),true)
ifeq ($(HAVE_LIBLO),true)
TARGETS += dssi
endif
endif
endif

ifeq ($(HAVE_CAIRO),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += vst

BASE_FLAGS += -Isrc -lX11 \
				`pkg-config --cflags --libs gtk+-3.0` \
				`pkg-config --cflags --libs webkit2gtk-4.0`

all: $(TARGETS)

# --------------------------------------------------------------
