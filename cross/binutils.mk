BINUTILS_VERSION=2.30
binutils.tar.bz2: binutils-$(BINUTILS_VERSION).tar.bz2
	mv $^ $@

binutils-$(BINUTILS_VERSION).tar.bz2:
	wget https://ftp.gnu.org/gnu/binutils/$@

binutils/: binutils-$(BINUTILS_VERSION)/
	mv $^ $@

binutils-$(BINUTILS_VERSION)/: binutils.tar.bz2
	tar -xf $^

binutils-patch:
	true

binutils-build: binutils-install
	rm -rf builddir

binutils-configure:
	mkdir builddir
	cd builddir; \
	../binutils/configure --prefix=$(CROSSPATH) --target=$(TARGET) --with-sysroot --disable-nls --disable-werror

binutils-make: binutils-configure
	$(MAKE) -C builddir

binutils-install: binutils-make
	$(MAKE) -C builddir install
