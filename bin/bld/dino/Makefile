EXEC = cuckoo.out

OBJECTS = \
	main.o \

include ../../../Makefile.common

include $(MAKER_ROOT)/Makefile.dino

# Private aux library: it exists to exclude some sources from Mementos passes
LIBCUCKOO_ROOT = ../../../lib

CFLAGS += \
	-I$(LIBCUCKOO_ROOT)/src/include \
	-I$(LIBMSP_ROOT)/src/include \
	-I$(LIBMSPMATH_ROOT)/src/include \
	-I$(LIBMSPBUILTINS_ROOT)/src/include \
	-I$(LIBIO_ROOT)/src/include \

LLVM_LIBS += \
	$(LIBCUCKOO_ROOT)/bld/clang/libcuckoo.a.bc \
	$(LIBMSPMATH_ROOT)/bld/clang/libmspmath.a.bc \
	$(LIBMSP_ROOT)/bld/clang/libmsp.a.bc \

LFLAGS += \
	-L$(LIBMSPBUILTINS_ROOT)/bld/gcc \

LIBS += \
	-lmspbuiltins \

VPATH = ../../src
