INCDIR   =  -I/usr/X11R6/include
DESTDIR  =  /usr/X11R6
LIBDIR   =  -L/usr/X11R6/lib
LIBS     =  -lXpm -lX11 -lXext
SRCS     =  wmpower/libacpi.c wmpower/libapm.c wmpower/libacpi.c wmpower/power_management.c lib_utils/lib_utils.c toshiba/toshiba_lib.c wmgeneral/wmgeneral.c toshiba/sci.c toshiba/hci.c
OBJS     =  wmpower/libacpi.o wmpower/libapm.o wmpower/wmpower.o wmpower/power_management.o lib_utils/lib_utils.o toshiba/toshiba_lib.o wmgeneral/wmgeneral.o toshiba/sci.o toshiba/hci.o
USER     =  $(shell whoami)

#What is the C compiler?
ifndef CC
	CC = gcc	
endif

#Is it gcc?
ifeq "$(findstring gcc,$(shell $(CC) --version))" "gcc"
  #Is it's version >= 3?	
	ifeq "$(findstring 3.,$(shell $(CC) -dumpversion))" "3."
	  #We can set processor specific optimizations
  	PROCESSOR=$(shell cat /proc/cpuinfo)
		#Is our CPU a Celeron?	
	  ifeq "$(findstring Celeron,$(PROCESSOR))" "Celeron"
		  CPUFLAG = -march=pentium2
		endif
		ifeq "$(findstring Celeron (Coppermine),$(PROCESSOR))" "Celeron (Coppermine)"	
	 		CPUFLAG = -march=pentium3
		endif	  
		#Is our CPU a Pentium?
		ifeq "$(findstring Pentium,$(PROCESSOR))" "Pentium"
			CPUFLAG = -march=pentium
		endif	
		#Is our CPU a Pentium II?
		ifeq "$(findstring Pentium II,$(PROCESSOR))" "Pentium II"
			CPUFLAG = -march=pentium2
		endif	
		#Is our CPU a Pentium III?
		ifeq "$(findstring Pentium III,$(PROCESSOR))" "Pentium III"
			CPUFLAG = -march=pentium3
		endif	
		ifeq "$(findstring Intel(R) Pentium(R) 4,$(PROCESSOR))" "Intel(R) Pentium(R) 4"
			CPUFLAG = -march=pentium4
		endif
		ifeq "$(findstring AMD Athlon(tm),$(PROCESSOR))" "AMD Athlon(tm)"
		  CPUFLAG = -march=athlon
		endif
		ifeq "$(findstring AMD Athlon(tm) MP,$(PROCESSOR))" "AMD Athlon(tm) MP"
		  CPUFLAG = -march=athlon-mp
		endif
		ifeq "$(findstring AMD Athlon(tm) XP,$(PROCESSOR))" "AMD Athlon(tm) XP"
		  CPUFLAG = -march=athlon-xp
		endif		
		#Is our CPU an Ultra Sparc?
		ifeq "$(findstring UltraSparc,$(PROCESSOR))" "UltraSparc"
			CPUFLAG = -mcpu=ultrasparc
		endif	
	endif	
	ifndef PROCESSOR	
	  #Alas, we can set only generic arch optimizations
		PROCESSOR=$(shell uname -m)
		#Is our CPU an x86?
		ifeq "$(findstring 86,$(PROCESSOR))" "86"
			CPUFLAG = -march=$(PROCESSOR)
		endif	
	endif
endif	
CFLAGS = $(CPUFLAG) -O3 -pipe -fforce-addr -W -Wall -pedantic

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

ifeq "$(USER)" "root"	
install:: wmpower
	@echo Performing root install
	@echo	
	install -s -m 0755 wmpower/wmpower $(DESTDIR)/bin
	@echo
	@echo All done!
uninstall:: wmpower
	rm -f $(DESTDIR)/bin/wmpower
else
ifdef HOME
install:: wmpower
	@echo Performing install for user $(USER)
	@echo
	install -d -m 0700 $(HOME)/bin
	install -s -m 0700 wmpower/wmpower $(HOME)/bin
	@echo
	@echo All done!
uninstall:: wmpower
	rm -f $(HOME)/bin/wmpower
	-rmdir $(HOME)/bin
else
install::	wmpower
	@echo Cannot install: please set enviroment
	@echo variable HOME to your home directory.
uninstall::	wmpower
	@echo Cannot uninstall: please set enviroment
	@echo variable HOME to your home directory.
endif  
endif	
