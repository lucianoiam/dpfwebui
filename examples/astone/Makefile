#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#

# --------------------------------------------------------------
# Project name, used for binaries

NAME = d_astone

# --------------------------------------------------------------
# Project version, used for generating unique symbol names

HIPHOP_PROJECT_VERSION = 1

# --------------------------------------------------------------
# AssemblyScript DSP files location

HIPHOP_AS_DSP_PATH = dsp

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
    AsToneExamplePlugin.cpp

FILES_UI  = \
    AsToneExampleUI.cpp \
    Blendish.cpp

# --------------------------------------------------------------
# Do some magic

DPF_TARGET_DIR = ../../bin
DPF_BUILD_DIR = ../../build/astone/1/2

include ../../Makefile.plugins.mk

# --------------------------------------------------------------
# Enable all possible plugin types

ifeq ($(HAVE_JACK),true)
ifeq ($(HAVE_OPENGL),true)
TARGETS += jack
endif
endif

ifeq ($(HAVE_OPENGL),true)
TARGETS += lv2_sep
else
TARGETS += lv2_dsp
endif

TARGETS += vst vst3

all: $(TARGETS) $(HIPHOP_TARGET)

# --------------------------------------------------------------
