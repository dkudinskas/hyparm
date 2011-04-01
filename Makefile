# Shell to use
SHELL = /bin/sh

# variable definitions
BUILDDIR = build/
export BUILDDIR

# Default Target
all:
	cd $(BUILDDIR); make;

.PHONY: clean
clean:
	cd $(BUILDDIR); make clean;
