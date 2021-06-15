# Filename: Makefile.base.mk
# Author:   oss@lucianoiam.com

# Allow placing DPF in a custom directory while including its Makefiles
DPF_CUSTOM_PATH = ./lib/DPF
DPF_CUSTOM_TARGET_DIR = ./bin
DPF_CUSTOM_BUILD_DIR = ./build

# Keep debug symbols and print full compiler output
SKIP_STRIPPING = true
VERBOSE = true

# Custom dpf-webui setting
USE_DPF_DEVELOP_BRANCH=true

WINNUGET_DOWNLOAD_URL=https://dist.nuget.org/win-x86-commandline/latest/nuget.exe

TARGET_MACHINE := $(shell gcc -dumpmachine)

ifneq (,$(findstring linux,$(TARGET_MACHINE)))
LINUX=true
endif

ifneq (,$(findstring apple,$(TARGET_MACHINE)))
MACOS=true
endif

ifneq (,$(findstring mingw,$(TARGET_MACHINE)))
WINDOWS=true
endif

ifneq (,$(findstring MINGW,$(MSYSTEM)))
MSYS_MINGW=true
endif

ifeq ($(MSYS_MINGW),true)
ifeq ($(shell cmd /c "net.exe session 1>NUL 2>NUL || exit /b 1"; echo $$?),1)
$(info MSYS is not running as administrator, ln -s will copy files instead of symlinking)
MSYS_MINGW_SYMLINKS=:
else
MSYS_MINGW_SYMLINKS=export MSYS=winsymlinks:nativestrict
endif
endif

ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif

ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
_:=$(shell git submodule update --init --recursive)
endif

ifeq ($(USE_DPF_DEVELOP_BRANCH),true)
ifeq (,$(findstring develop,$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
_:=$(shell git -C $(DPF_CUSTOM_PATH) checkout develop)
endif
endif
