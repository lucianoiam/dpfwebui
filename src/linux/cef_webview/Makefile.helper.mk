# Filename: Makefile.helper.mk
# Author:   oss@lucianoiam.com

# TODO

# Only 64-bit is currently supported

CEF_PATH = $(HIPHOP_LIB_PATH)/cef
CEF_PKG = cef_binary_93.1.12+ga8ffe4b+chromium-93.0.4577.82_linux64_minimal
CEF_PKG_FILE = $(CEF_PKG).tar.bz2
CEF_URL = https://cef-builds.spotifycdn.com/$(CEF_PKG_FILE)

$(LXHELPER_BIN): $(CEF_PATH)

$(CEF_PATH):
	@echo Downloading CEF...
	@wget -O /tmp/$(CEF_PKG_FILE) $(CEF_URL)
	@mkdir -p $(HIPHOP_LIB_PATH)
	@echo Decompressing CEF...
	@tar xjf /tmp/$(CEF_PKG_FILE) -C $(HIPHOP_LIB_PATH)
	@ln -s $(CEF_PKG) $(CEF_PATH)
	@rm /tmp/$(CEF_PKG_FILE)
