# XXX

all: gooSysExecTest

gooSysExecTest: ../../goo/examples/sysexec/main.c
	g++ -x c -I ../../goo/inc/ $< -lgoo-dbg -L. -Wl,-rpath,. -g -o $@ -Wall
