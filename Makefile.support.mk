# Filename: Makefile.support.mk
# Author:   oss@lucianoiam.com

BASE_FLAGS += -Isrc -I$(DPF_CUSTOM_PATH) -DBIN_BASENAME=$(NAME) \
              -DPROJECT_ID_HASH=$(shell echo $(NAME):$(PROJECT_VERSION) | shasum -a 256 | head -c 8)

# Platform-specific build flags
ifeq ($(LINUX),true)
LINK_FLAGS += -lpthread -ldl
endif
ifeq ($(MACOS),true)
LINK_FLAGS += -framework WebKit 
endif
ifeq ($(WINDOWS),true)
BASE_FLAGS += -I$(EDGE_WEBVIEW2_PATH)/build/native/include
LINK_FLAGS += -L$(EDGE_WEBVIEW2_PATH)/build/native/x64 \
              -lole32 -lShlwapi -lWebView2Loader.dll -static-libgcc \
              -static-libstdc++ -Wl,-Bstatic -lstdc++ -lpthread
endif

# Target for DPF graphics library
DEP_TARGETS += $(DPF_CUSTOM_PATH)/build/libdgl.a

$(DPF_CUSTOM_PATH)/build/libdgl.a:
	make -C $(DPF_CUSTOM_PATH) dgl

# Reuse DPF post-build scripts
TARGETS += ./utils

./utils:
	@eval $(MSYS_MINGW_SYMLINKS)
	@ln -s $(DPF_CUSTOM_PATH)/utils .

# Target for generating LV2 TTL files
ifeq ($(CAN_GENERATE_TTL),true)
TARGETS += lv2ttl

lv2ttl: utils/lv2_ttl_generator
	@$(CURDIR)/utils/generate-ttl.sh

utils/lv2_ttl_generator:
	$(MAKE) -C utils/lv2-ttl-generator
endif

# Linux requires a helper binary
ifeq ($(LINUX),true)
LXHELPER_BIN = /tmp/$(NAME)_ui
TARGETS += $(LXHELPER_BIN)

$(LXHELPER_BIN): src/arch/linux/helper.c src/arch/linux/extra/ipc.c
	@echo "Building helper..."
	$(SILENT)$(CC) $^ -Isrc -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
ifneq ($(filter jack,$(TARGETS)),)
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)
endif
ifneq (,$(findstring lv2,$(shell echo $(TARGETS)[@])))
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2
endif
ifneq ($(filter dssi,$(TARGETS)),)
	@cp $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi
endif
ifneq ($(filter vst,$(TARGETS)),)
	@cp -n $(LXHELPER_BIN) $(DPF_CUSTOM_TARGET_DIR)
endif
	@rm $(LXHELPER_BIN)

clean: clean_lxhelper

clean_lxhelper:
	rm -rf $(LXHELPER_BIN)
endif

# Mac requires compiling Objective-C++ and creating a VST bundle
ifeq ($(MACOS),true)
TARGETS += macvst

macvst:
	@$(CURDIR)/utils/generate-vst-bundles.sh

clean: clean_macvst

clean_macvst:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst

$(BUILD_DIR)/%.mm.o: %.mm
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	$(SILENT)$(CXX) $< $(BUILD_CXX_FLAGS) -ObjC++ -c -o $@
endif

# Windows requires compiling resource files and linking to Edge WebView2
# The current Makefile is too lazy to support 32-bit but DLL is also available
ifeq ($(WINDOWS),true)
EDGE_WEBVIEW2_PATH=./lib/Microsoft.Web.WebView2
DEP_TARGETS += $(EDGE_WEBVIEW2_PATH)
TARGETS += copywindll

ifeq (,$(shell which nuget 2>/dev/null))
ifneq ($(MSYS_MINGW),true)
$(error NuGet not found, try sudo apt install nuget or the equivalent for your distro)
endif
endif

# The standalone Windows version requires a "bare" DLL instead of the assembly
copywindll:
	@$(eval WEBVIEW_DLL=$(EDGE_WEBVIEW2_PATH)/runtimes/win-x64/native/WebView2Loader.dll)
ifneq ($(filter jack,$(TARGETS)),)
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)
endif
ifneq (,$(findstring lv2,$(shell echo $(TARGETS)[@])))
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/WebView2Loader
endif
ifneq ($(filter vst,$(TARGETS)),)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp $(WEBVIEW_DLL) $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
	@cp src/arch/windows/res/WebView2Loader.manifest $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader
endif

clean: clean_windll

clean_windll:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/WebView2Loader

ifeq ($(MSYS_MINGW),true)
$(EDGE_WEBVIEW2_PATH): /usr/bin/nuget.exe
else
$(EDGE_WEBVIEW2_PATH):
endif
	@echo Downloading Edge WebView2 SDK...
	@eval $(MSYS_MINGW_SYMLINKS)
	@nuget install Microsoft.Web.WebView2 -OutputDirectory lib
	@ln -rs $(EDGE_WEBVIEW2_PATH).* $(EDGE_WEBVIEW2_PATH)

/usr/bin/nuget.exe:
	@echo Downloading NuGet...
	@wget -P /usr/bin $(WINNUGET_DOWNLOAD_URL)

$(BUILD_DIR)/%.rc.o: %.rc
	-@mkdir -p "$(shell dirname $(BUILD_DIR)/$<)"
	@echo "Compiling $<"
	@windres --input $< --output $@ --output-format=coff
endif

# Always copy web UI files
TARGETS += resources

resources:
	@echo "Copying web UI resource files..."
ifneq ($(filter jack,$(TARGETS)),)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	@cp -r src/ui/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
endif
ifneq (,$(findstring lv2,$(shell echo $(TARGETS)[@])))
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
	@cp -r src/ui/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).lv2/$(NAME)_resources
endif
ifneq ($(filter dssi,$(TARGETS)),)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
	@cp -r src/ui/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)-dssi/$(NAME)_resources
endif
ifneq ($(filter vst,$(TARGETS)),)
ifeq ($(MACOS),true)
	@cp -r src/ui/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME).vst/Contents/Resources
endif
ifeq ($(WINDOWS),true)
	-@mkdir -p $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
	@cp -rn src/ui/* $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
endif
endif

clean: clean_resources

clean_resources:
	@rm -rf $(DPF_CUSTOM_TARGET_DIR)/$(NAME)_resources
