# Filename: Makefile
# Author:   oss@lucianoiam.com

example:
	@make -C example

clean:
	@make -C example clean

all: example

.PHONY: example
