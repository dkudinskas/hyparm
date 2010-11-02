# Shell to use
SHELL = /bin/sh

# variable definitions
BUILDDIR = build/
export BUILDDIR
TESTDIR = test/

# Default Target
all:
	cd $(BUILDDIR); make;

.PHONY: backup
backup:
	@echo "Making a backup..."
	@tar -cvf hypervisorBackup.tar *
	@cp -v hypervisorBackup.tar ~/Dropbox/
	@echo " done."


.PHONY: clean
clean:
	cd $(BUILDDIR); make clean;
	cd $(TESTDIR); make clean;
