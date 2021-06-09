ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
_:=$(shell git submodule update --init --recursive)
endif

# Currently tracking the latest DPF updates, lock to master after stable release.
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

ifneq (,$(findstring MINGW,$(MSYSTEM)))
MSYS_MINGW=true
endif

ifeq ($(MSYS_MINGW),true)
ifeq ($(shell cmd /c "net.exe session 1>NUL 2>NUL || exit /b 1"; echo $$?),1)
#$(error Need to create symlinks, please run MSYS as administrator)
endif
endif

ifeq ($(WINDOWS),true)
  EDGE_WEBVIEW2_PATH=./lib/Microsoft.Web.WebView2
  ifeq (,$(wildcard $(EDGE_WEBVIEW2_PATH)))
    ifeq (,$(shell which nuget 2>/dev/null))
      ifeq ($(MSYS_MINGW),true)
        $(info Downloading NuGet...)
        _:=$(shell wget -P /usr/bin https://dist.nuget.org/win-x86-commandline/latest/nuget.exe)
      else
         $(error NuGet not found, try sudo apt install nuget or the equivalent for your distro.)
      endif
      $(info Downloading Edge WebView2 SDK...)
      _:=$(shell nuget install Microsoft.Web.WebView2 -OutputDirectory lib)
      _:=$(shell ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH))
    endif
  endif
endif

ifneq ($(MSYS_MINGW),true)
ifneq ($(CROSS_COMPILING),true)
CAN_GENERATE_TTL = true
else ifneq ($(EXE_WRAPPER),)
CAN_GENERATE_TTL = true
endif
endif
