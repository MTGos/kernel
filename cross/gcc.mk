GCC_VERSION=7.2.0
GCC_MAJOR_VERSION=7.2.0
gcc.tar.xz: gcc-$(GCC_VERSION).tar.xz
	mv $^ $@

gcc-$(GCC_VERSION).tar.xz:
	wget https://ftp.gnu.org/gnu/gcc/gcc-$(GCC_MAJOR_VERSION)/$@

gcc/: gcc-$(GCC_VERSION)/
	mv $^ $@

gcc-$(GCC_VERSION)/: gcc.tar.xz
	tar -xf $^

gcc-patch:
	true #Todo: Replace this with the sed that enables libstdc++

gcc-build: gcc-install
	rm -rf builddir

gcc-configure:
	mkdir builddir
	cd builddir; \
	../gcc/configure --prefix=$(CROSSPATH) --target=$(TARGET) --disable-nls --enable-languages=c,c++ --without-headers

gcc-make: gcc-configure
	$(MAKE) -C builddir all-gcc
	$(MAKE) -C builddir all-target-libgcc

gcc-install: gcc-make
	$(MAKE) -C builddir install-gcc
	$(MAKE) -C builddir  install-target-libgcc
