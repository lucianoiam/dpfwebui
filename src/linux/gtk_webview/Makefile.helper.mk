# Filename: Makefile.helper.mk
# Author:   oss@lucianoiam.com

LXHELPER_FILES += gtk_webview/helper.c \
                  ipc.c

LXHELPER_FILES_PATH = $(LXHELPER_FILES:%=$(HIPHOP_SRC_PATH)/linux/%)

$(LXHELPER_BIN): $(LXHELPER_FILES_PATH)
	@echo "Building WebKitGTK helper..."
	@mkdir -p $(LXHELPER_BUILD_DIR)
	$(SILENT)$(CC) $^ -I. -I$(HIPHOP_SRC_PATH) -o $(LXHELPER_BIN) -lX11 \
		$(shell $(PKG_CONFIG) --cflags --libs gtk+-3.0) \
		$(shell $(PKG_CONFIG) --cflags --libs webkit2gtk-4.0)
