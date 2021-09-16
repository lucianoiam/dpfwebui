# Filename: Makefile.helper.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Helper binary

LXHELPER_SRC += gtk_webview/helper.c \
				ipc.c

LXHELPER_OBJ = $(LXHELPER_SRC:%=$(LXHELPER_BUILD_DIR)/%.o)

LXHELPER_CPPFLAGS = -I. -I$(HIPHOP_SRC_PATH) \
					$(shell $(PKG_CONFIG) --cflags gtk+-3.0 webkit2gtk-4.0)
LXHELPER_LDFLAGS = -lpthread -lX11 \
				   $(shell $(PKG_CONFIG) --libs gtk+-3.0 webkit2gtk-4.0)

lxhelper_bin: $(LXHELPER_BUILD_DIR)/$(LXHELPER_NAME)

$(LXHELPER_BUILD_DIR)/$(LXHELPER_NAME): $(LXHELPER_OBJ)
	@echo "Compiling $<"
	@$(CXX) $^ -o $@ $(LXHELPER_LDFLAGS)

$(LXHELPER_BUILD_DIR)/%.c.o: $(HIPHOP_SRC_PATH)/linux/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@$(CC) $(LXHELPER_CPPFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# List of helper files

LXHELPER_FILES = $(LXHELPER_BUILD_DIR)/$(LXHELPER_NAME)
