#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = webgain

# --------------------------------------------------------------
# Project version for generating unique symbol names

DPF_WEBUI_PROJECT_VERSION = 1

# --------------------------------------------------------------
# Automatically inject dpf.js

DPF_WEBUI_INJECT_FRAMEWORK_JS = true

# --------------------------------------------------------------
# Enable Web UI by setting web files location

DPF_WEBUI_WEB_UI_PATH = ui

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
    WebGainExamplePlugin.cpp

FILES_UI  = \
    WebGainExampleUI.cpp

# --------------------------------------------------------------
# Do some magic

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR = ../../build/webgain/1/2

include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

TARGETS += lv2_sep clap vst3 vst

all: $(TARGETS) $(DPF_WEBUI_TARGET)

# --------------------------------------------------------------
