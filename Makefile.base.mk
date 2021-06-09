ifeq (,$(wildcard $(DPF_CUSTOM_PATH)))
_:=$(shell git clone --recurse-submodule -b develop https://github.com/DISTRHO/DPF.git lib/DPF)
endif

# This is needed until the DPF_CUSTOM_PATH support patch is merged into DPF
ifeq (,$(findstring develop,$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
_:=$(shell git -C $(DPF_CUSTOM_PATH) checkout develop)
endif

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

ifeq ($(WINDOWS),true)
ifeq (,$(wildcard ./lib/windows/Microsoft.Web.WebView2))
$(error WebView2 SDK not found, please have a look at BUILD.txt)
endif
endif

ifneq (,$(findstring MINGW,$(MSYSTEM)))
MSYS_MINGW=true
endif

ifeq ($(MSYS_MINGW),true)
ifeq ($(shell cmd /c "net.exe session 1>NUL 2>NUL || exit /b 1"; echo $$?),1)
#$(error Need to create symlinks, please run MSYS as administrator)
endif
endif

ifneq ($(MSYS_MINGW),true)
ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif
endif
