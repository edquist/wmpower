## Makefile.am for wmpower/src

SUBDIRS = dockapp power_management

INCLUDES = -Idockapp -Ipixmaps -Ipower_management

bin_PROGRAMS = wmpower
wmpower_SOURCES = wmpower.c

wmpower_LDADD = dockapp/libdockapp.a power_management/libpower.a
wmpower_LDFLAGS = -lXpm -lX11 -lXext -L/usr/X11R6/lib

EXTRA_DIST = \
		pixmaps/wmpower_mask.xbm \
		pixmaps/wmpower_master.xpm \
		pixmaps/wmpower_master_LowColor.xpm
