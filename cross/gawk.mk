GAWK_VERSION=4.2.0
gawk.tar.xz: gawk-$(GAWK_VERSION).tar.xz
	mv $^ $@

gawk-$(GAWK_VERSION).tar.xz:
	wget https://ftp.gnu.org/gnu/gawk/$@

gawk/: gawk-$(GAWK_VERSION)/
	mv $^ $@

gawk-$(GAWK_VERSION)/: gawk.tar.xz
	tar -xf $^

gawk-patch:
	true

gawk-build: gawk-install
	rm -rf builddir

gawk-configure: gawk/
	mkdir builddir
	cd builddir; \
	../gawk/configure --prefix=$(CROSSPATH)

gawk-make: gawk-configure
	cd builddir; \
	$(MAKE)

gawk-install: gawk-make
	cd builddir; \
	$(MAKE) install
