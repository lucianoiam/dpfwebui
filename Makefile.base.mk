# Created by lucianoiam
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
$(info For real symlink support re-run MSYS as administrator)
MSYS_MINGW_SYMLINKS=:
else
MSYS_MINGW_SYMLINKS=export MSYS=winsymlinks:nativestrict
endif
endif

ifneq ($(MSYS_MINGW),true)
ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif
endif

ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
_:=$(shell git submodule update --init --recursive)
endif

ifeq ($(USE_DPF_DEVELOP_BRANCH),true)
ifeq (,$(findstring develop,$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
_:=$(shell git -C $(DPF_CUSTOM_PATH) checkout develop)
endif
endif
