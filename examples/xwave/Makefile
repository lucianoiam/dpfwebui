#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = xwave

# --------------------------------------------------------------
# Project version for generating unique symbol names

DPF_WEBUI_PROJECT_VERSION = 1

# --------------------------------------------------------------
# Enable Web UI by setting web files location

DPF_WEBUI_WEB_UI_PATH = ui

# --------------------------------------------------------------
# Enable Web UI control from devices in the local network

DPF_WEBUI_NETWORK_UI = true

# --------------------------------------------------------------
# Build BSONVariant required by the binary protocol

DPF_WEBUI_SUPPORT_BSON = true

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
    XWaveExamplePlugin.cpp

FILES_UI  = \
    XWaveExampleUI.cpp

# --------------------------------------------------------------
# Do some magic

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR = ../../build/xwave/1/2

include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += lv2_sep clap vst3 vst

all: $(TARGETS) $(DPF_WEBUI_TARGET)

# --------------------------------------------------------------
