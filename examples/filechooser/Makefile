#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = d_filechooser

# --------------------------------------------------------------
# Project version, used for generating unique symbol names

HIPHOP_PROJECT_VERSION = 1

# --------------------------------------------------------------
# Web UI files location

HIPHOP_WEB_UI_PATH = ui

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
    FileChooserExamplePlugin.cpp

FILES_UI  = \
    FileChooserExampleUI.cpp

# --------------------------------------------------------------
# Do some magic

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR = ../../build/filechooser/1/2

include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(MACOS),true)
ifeq ($(HAVE_JACK),true)
TARGETS += jack
endif
endif

TARGETS += lv2_sep vst vst3

all: $(TARGETS) $(HIPHOP_TARGET)

# --------------------------------------------------------------
