#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Web UI by lucianoiam
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = d_webgain

# --------------------------------------------------------------
# Project version, used for generating unique symbol names

APX_PROJECT_VERSION = 1

# --------------------------------------------------------------
# Web UI files location

APX_WEB_UI_PATH = ui

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
    WebGainExamplePlugin.cpp

FILES_UI  = \
    WebGainExampleUI.cpp

# --------------------------------------------------------------
# Do some magic

DPF_BUILD_DIR = build

include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifneq ($(WINDOWS),true)
ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += jack
endif
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

all: $(TARGETS) $(APX_TARGET)

# --------------------------------------------------------------
