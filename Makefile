# match "module foobar() { // `make` me"
TARGETS=$(shell sed '/^module [A-Za-z0-9_-]*().*make..\?me.*$$/!d;s/module //;s/().*/.stl/' SCARA.scad)

all: ${TARGETS}
	echo ${TARGETS}

# auto-generated .scad files with .deps make make re-build always. keeping the
# scad files solves this problem. (explanations are welcome.)
.SECONDARY: $(shell echo "${TARGETS}" | sed 's/\.stl/.scad/g')

# explicit wildcard expansion suppresses errors when no files are found
include $(wildcard *.deps)

%.scad:
	echo -n 'use <SCARA.scad>\n$*();' > $@

%.stl: %.scad
	openscad -m make -o $@ -d $@.deps -D 'quality="production"' $<
