# Filename: Makefile
# Author:   oss@lucianoiam.com

EXAMPLE_BIN=example/bin/d_webgain

example: $(EXAMPLE_BIN)

$(EXAMPLE_BIN):
	@make -C example

clean:
	@make -C example clean

all: example
