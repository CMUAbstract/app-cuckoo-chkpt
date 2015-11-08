EXEC = cuckoo.out

OBJECTS = \
	main.o \

include ../../../Makefile.common
include $(MAKER_ROOT)/Makefile.gcc

# Private aux library: it exists to exclude some sources from Mementos passes
LIBCUCKOO_ROOT = ../../../lib

CFLAGS += \
	-I$(LIBCUCKOO_ROOT)/src/include \
	-I$(LIBMSP_ROOT)/src/include \
	-I$(LIBMSPMATH_ROOT)/src/include \
	-I$(LIBMSPBUILTINS_ROOT)/src/include \
	-I$(LIBIO_ROOT)/src/include \

LFLAGS += \
	-L$(LIBCUCKOO_ROOT)/bld/gcc \
	-L$(LIBMSPMATH_ROOT)/bld/gcc \

LIBS += \
	-lcuckoo \
	-lmspmath \

VPATH = ../../src
