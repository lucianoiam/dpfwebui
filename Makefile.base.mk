# Sanity checks
ifeq (,$(wildcard $(DPF_CUSTOM_PATH)/Makefile))
$(error DPF not found, make sure it was cloned as a submodule)
endif

# This is needed until the DPF_CUSTOM_PATH support patch is merged into DPF
ifneq (,$(findstring master,$(shell git -C $(DPF_CUSTOM_PATH) branch --show-current)))
$(error DPF branch is master, make sure its current branch is develop)
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
ifeq (,$(wildcard ./lib/windows/WebView2))
$(error WebView2 SDK not found, please have a look at BUILD.txt)
endif
endif
