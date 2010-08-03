all: build-tests

build-tests: \
	build-frameAllocator-test \
	build-pageTable-test

run-all-tests:\
	 run-frameAllocator-test \
	run-pageTable-test

.PHONY: clean
clean:	clean-frameAllocator-test \
	clean-pageTable-test \
	clean-common 

clean-frameAllocator-test:
	cd frameAllocator; make clean;

build-frameAllocator-test:
	cd frameAllocator; make;

run-frameAllocator-test:
	cd frameAllocator; make run-test;

build-pageTable-test:
	cd pageTable; make;

run-pageTable-test:
	cd pageTable; make run-test;

clean-pageTable-test:
	cd pageTable; make clean;

clean-common:
	cd common; make clean;
