conf-objs := conf.o zconf.tab.o
nconf-objs := nconf.o zconf.tab.o nconf.gui.o

.PHONY: config menuconfig defconfig nconfig clean distclean

config: nconfig
menuconfig: nconfig

defconfig: conf
	./$< Kconfig --defconfig

nconfig: nconf
	./$< Kconfig

conf: conf.o zconf.tab.o
	gcc -o $@ $^

nconf:	nconf.o nconf.gui.o zconf.tab.o
	gcc -o $@ $^ -lmenu -lpanel

conf.o: conf.c expr.h lkc.h lkc_proto.h
nconf.o: nconf.c expr.h lkc.h lkc_proto.h
nconf.gui.o: nconf.gui.c nconf.h
zconf.tab.o: zconf.tab.c zconf.lex.c zconf.hash.c
 
clean:
	rm conf nconf $(conf-objs) $(nconf-objs) > /dev/null 2>&1 || :

distclean:
	rm .config > /dev/null 2>&1 || :
