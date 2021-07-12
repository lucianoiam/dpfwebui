# Filename: Makefile
# Author:   oss@lucianoiam.com

examples:
	@make -C examples/webgain

clean:
	rm -rf bin
	rm -rf build

all: examples

.PHONY: examples
