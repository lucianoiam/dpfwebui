# Filename: Makefile.gtk.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Build helper binary

LXHELPER_SRC += gtk_helper.c \
				ipc.c

LXHELPER_OBJ = $(LXHELPER_SRC:%=$(LXHELPER_BUILD_DIR)/%.o)

LXHELPER_CPPFLAGS = -I. -I$(HIPHOP_SRC_PATH) \
					$(shell $(PKG_CONFIG) --cflags gtk+-3.0 webkit2gtk-4.0)

ifneq ($(HIPHOP_PLUGIN_MAX_WIDTH),)                                             
ifneq ($(HIPHOP_PLUGIN_MAX_HEIGHT),) 
LXHELPER_CPPFLAGS += -DHIPHOP_PLUGIN_MAX_WIDTH=$(HIPHOP_PLUGIN_MAX_WIDTH)
LXHELPER_CPPFLAGS += -DHIPHOP_PLUGIN_MAX_HEIGHT=$(HIPHOP_PLUGIN_MAX_HEIGHT) 
endif
endif

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
# Only copy the monolithic GTK helper binary

LXHELPER_FILES = $(LXHELPER_BUILD_DIR)/$(LXHELPER_NAME)
