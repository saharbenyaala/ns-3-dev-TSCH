# Makefile wrapper for waf

all:
	./waf --run lr-wpan-tsch
	grep -v "^[+]" lr-wpan-tsch.tr|wc -l

# free free to change this part to suit your requirements
configure:
	./waf configure --enable-examples --enable-tests

build:
	./waf build

install:
	./waf install

clean:
	./waf clean

distclean:
	./waf distclean
