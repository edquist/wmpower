## Makefile.am for wmpower/src

INCLUDES =	-Iacpi -Iapm -Idell -Itoshiba -Ilib_utils

noinst_LIBRARIES = libpower.a

libpower_a_SOURCES = power_management.c \
								power_management.h \
								acpi/libacpi.c \
								acpi/libacpi.h \
								apm/libapm.c \
								apm/libapm.h \
								dell/dell_lib.c \
								dell/dell_lib.h \
								toshiba/toshiba_lib.c \
								toshiba/toshiba_lib.h \
								toshiba/hci.c \
								toshiba/hci.h \
								toshiba/sci.c \
								toshiba/sci.h \
								lib_utils/lib_utils.c \
								lib_utils/lib_utils.h
