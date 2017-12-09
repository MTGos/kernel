MPFR_VERSION=3.1.6
mpfr.tar.xz: mpfr-$(MPFR_VERSION).tar.xz
	mv $^ $@

mpfr-$(MPFR_VERSION).tar.xz:
	wget https://ftp.gnu.org/gnu/mpfr/$@

gcc/mpfr/: gcc/ mpfr-$(MPFR_VERSION)/
	mv mpfr-$(MPFR_VERSION)/ $@

mpfr-$(MPFR_VERSION)/: mpfr.tar.xz
	tar -xf $^

mpfr-patch:
	true

mpfr-build:
	true
