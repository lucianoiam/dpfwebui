# Filename: Makefile.gtk.mk
# Author:   oss@lucianoiam.com

# ------------------------------------------------------------------------------
# Build helper binary

LXHELPER_SRC = gtk_helper.c \
			   ipc.c \
			   scaling.c

LXHELPER_OBJ = $(LXHELPER_SRC:%=$(LXHELPER_BUILD_PATH)/%.o)

LXHELPER_CPPFLAGS = -I. -I$(DPF_WEBUI_INC_PATH) \
					$(shell $(PKG_CONFIG) --cflags gtk+-3.0 webkit2gtk-4.0)
LXHELPER_LDFLAGS = -lpthread -lX11 \
				   $(shell $(PKG_CONFIG) --libs gtk+-3.0 webkit2gtk-4.0)

lxhelper_bin: $(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME)

$(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME): $(LXHELPER_OBJ)
	@echo "Compiling $<"
	@$(CXX) $^ -o $@ $(LXHELPER_LDFLAGS)

$(LXHELPER_BUILD_PATH)/%.c.o: $(DPF_WEBUI_SRC_PATH)/ui/linux/%.c
	@mkdir -p $(dir $@)
	@echo "Compiling $<"
	@$(CC) $(LXHELPER_CPPFLAGS) -c $< -o $@

# ------------------------------------------------------------------------------
# Only copy the monolithic GTK helper binary

LXHELPER_FILES = $(LXHELPER_BUILD_PATH)/$(LXHELPER_NAME)
