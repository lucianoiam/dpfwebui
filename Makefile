# Filename: Makefile
# Author:   oss@lucianoiam.com

examples:
	@make -C examples/webgain
	@make -C examples/zcomp
	@make -C examples/xwave

clean:
	@make clean -C examples/webgain
	@make clean -C examples/zcomp
	@make clean -C examples/xwave
	rm -rf build/*

all: examples

.PHONY: examples
