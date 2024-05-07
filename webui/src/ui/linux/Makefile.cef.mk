# Filename: Makefile.cef.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Currently only the 64-bit version of CEF is supported though the library is
# also available on 32-bit platforms. https://bitbucket.org/chromiumembedded/cef/

CEF_DISTRO = cef_binary_107.1.12+g65b79a6+chromium-107.0.5304.122_linux64_minimal
CEF_DISTRO_FILE = $(CEF_DISTRO).tar.bz2
CEF_PATH = $(DPF_WEBUI_DEPS_PATH)/cef
CEF_BUILD_PATH = $(CEF_PATH)/build
CEF_BIN_PATH = $(CEF_PATH)/Release
CEF_RES_PATH = $(CEF_PATH)/Resources
CEF_URL = https://cef-builds.spotifycdn.com/$(CEF_DISTRO_FILE)

# ------------------------------------------------------------------------------
# CEF distribution download

$(CEF_PATH):
	@echo Downloading CEF
	@wget -4 -O /tmp/$(CEF_DISTRO_FILE) $(CEF_URL)
	@mkdir -p $(DPF_WEBUI_DEPS_PATH)
	@echo Decompressing CEF, this takes a long while!...
	@tar xjf /tmp/$(CEF_DISTRO_FILE) -C $(DPF_WEBUI_DEPS_PATH)
	@ln -s $(CEF_DISTRO) $(CEF_PATH)
	@rm /tmp/$(CEF_DISTRO_FILE)

# ------------------------------------------------------------------------------
# CEF C++ wrapper static library

CEF_WRAPPER_LIB = $(CEF_BUILD_PATH)/libcef_dll_wrapper/libcef_dll_wrapper.a

$(CEF_WRAPPER_LIB):
	@echo Building CEF C++ wrapper library
	@mkdir -p $(CEF_BUILD_PATH) \
		&& cd $(CEF_BUILD_PATH) \
		&& cmake -DCMAKE_DISABLE_FIND_PACKAGE_Doxygen=TRUE .. \
		&& make

# ------------------------------------------------------------------------------
# Build helper binary

LXHELPER_SRC = CefHelper.cpp \
			   IpcChannel.cpp \
			   ipc.c \
			   scaling.c \
			   ../../../../dpf/distrho/extra/sofd/libsofd.c

LXHELPER_OBJ = $(LXHELPER_SRC:%=$(LXHELPER_BUILD_PATH)/ui/linux/%.o)

LXHELPER_CFLAGS = -I$(DPF_WEBUI_INC_PATH) -I$(DPF_PATH) -I$(CEF_PATH) -DHAVE_X11 \
                  -DDPF_WEBUI_SKIP_DPF -DDPF_WEBUI_PLUGIN_BIN_BASENAME=$(NAME)

# Copied from the cefsimple example
LXHELPER_LDFLAGS = -ldl -lXi -lcef_dll_wrapper -L$(CEF_BUILD_PATH)/libcef_dll_wrapper \
				   -lcef -L$(CEF_BIN_PATH) -lX11 -O3 -DNDEBUG -rdynamic -fPIC \
				   -pthread -Wl,--disable-new-dtags -Wl,--fatal-warnings \
				   -Wl,-rpath,. -Wl,-z,noexecstack -Wl,-z,now -Wl,-z,relro -m64 \
				   -Wl,-O1 -Wl,--as-needed -Wl,--gc-sections

lxhelper_bin: $(CEF_PATH) $(CEF_WRAPPER_LIB) $(CEF_WRAPPER_LIB) \
			  $(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME)

$(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME): $(LXHELPER_OBJ)
	@echo "Compiling $<"
	@$(CXX) $^ -o $@ $(LXHELPER_LDFLAGS)

$(LXHELPER_BUILD_PATH)/%.cpp.o: $(DPF_WEBUI_SRC_PATH)/%.cpp
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@$(CXX) $(LXHELPER_CFLAGS) -c $< -o $@

$(LXHELPER_BUILD_PATH)/%.c.o: $(DPF_WEBUI_SRC_PATH)/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@$(CC) $(LXHELPER_CFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# List of helper binaries and resource files

CEF_FILES_BIN = \
	chrome-sandbox \
	libEGL.so \
	libGLESv2.so \
	libcef.so \
	snapshot_blob.bin \
	v8_context_snapshot.bin

CEF_FILES_RES = \
	resources.pak \
	chrome_100_percent.pak \
	chrome_200_percent.pak \
	icudtl.dat \
	locales

LXHELPER_FILES =  $(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME)
LXHELPER_FILES += $(CEF_FILES_BIN:%=$(CEF_BIN_PATH)/%)
LXHELPER_FILES += $(CEF_FILES_RES:%=$(CEF_RES_PATH)/%)
