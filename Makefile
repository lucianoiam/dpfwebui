# Filename: Makefile
# Author:   oss@lucianoiam.com

examples:
	@make -C examples/webgain

clean:
	@make -C examples/webgain clean

all: examples

.PHONY: examples
