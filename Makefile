CC     = gcc
CPUTYPE= i686
CFLAGS = -O3 -march=$(CPUTYPE) -pipe -fforce-addr -W -Wall -pedantic
INCDIR = -I/usr/X11R6/include
DESTDIR= /usr/X11R6
LIBDIR = -L/usr/X11R6/lib
LIBS   = -lXpm -lX11 -lXext
SRCS   = wmpower/libacpi.c wmpower/libapm.c wmpower/libacpi.c wmpower/power_management.c lib_utils/lib_utils.c toshiba/toshiba_lib.c wmgeneral/wmgeneral.c toshiba/sci.c toshiba/hci.c
OBJS   = wmpower/libacpi.o wmpower/libapm.o wmpower/wmpower.o wmpower/power_management.o lib_utils/lib_utils.o toshiba/toshiba_lib.o wmgeneral/wmgeneral.o toshiba/sci.o toshiba/hci.o

.c.o: 
	$(CC) -D$(shell echo `uname -s`) $(CFLAGS) -c $< -o $*.o $(INCDIR)

all:	wmpower

wmpower.o: pixmaps/wmpower_master.xpm pixmaps/wmpower_mask.xbm wmpower/wmpower.h
wmpower: $(OBJS)
	$(CC) $(CFLAGS) -o wmpower/wmpower $^ $(INCDIR) $(LIBDIR) $(LIBS)

clean:
	for i in $(OBJS) ; do \
		rm -f $$i; \
	done
	rm -f *~
	rm -f wmpower/wmpower

install:: wmpower
	install -s -m 0755 wmpower/wmpower $(DESTDIR)/bin

uninstall:: wmpower
	rm -f $(DESTDIR)/bin/wmpower
