CROSSPATH = $(HOME)/opt
export PATH := $(CROSSPATH)/bin:$(PATH)
pre: | clean $(CROSSPATH)/ fetch extract patch prereqs

clean:
	rm -rf $(CROSSPATH)
	rm -rf gawk binutils gcc *.tar.* builddir

$(CROSSPATH)/:
	mkdir -p $(CROSSPATH)

include gawk.mk
include gcc.mk
include binutils.mk
include mpc.mk
include mpfr.mk
include gmp.mk
include isl.mk
include cloog.mk

fetch: gawk.tar.xz gcc.tar.xz binutils.tar.bz2 mpc.tar.gz mpfr.tar.xz gmp.tar.xz isl.tar.xz cloog.tar.gz

extract: gawk/ binutils/ gcc/mpc/ gcc/mpfr/ gcc/gmp/ gcc/isl/ gcc/cloog/

patch: gawk-patch gcc-patch binutils-patch mpc-patch mpfr-patch gmp-patch isl-patch cloog-patch

prereqs: gawk-build

build:
	TARGET=$(TARGET) $(MAKE) -e binutils-build
	TARGET=$(TARGET) $(MAKE) -e gcc-build
