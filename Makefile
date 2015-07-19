# match "module foobar() { // `make` me"
TARGETS=$(shell awk '/^module [A-Za-z0-9_-]*().*make/ { sub(/\(\)/, "", $$2); print $$2 ".stl" }' SCARA.scad)

all: ${TARGETS}

# auto-generated .scad files with .deps make make re-build always. keeping the
# scad files solves this problem. (explanations are welcome.)
.SECONDARY: $(shell echo "${TARGETS}" | sed 's/\.stl/.scad/g')

# explicit wildcard expansion suppresses errors when no files are found
include $(wildcard *.deps)

%.scad:
	printf 'use <SCARA.scad>\n$*();' > $@

%.stl: %.scad
	/Applications/OpenSCAD.app/Contents/MacOS/OpenSCAD -m make -o $@ -d $@.deps -D 'quality="production"' $<
