## Compile Makefile for Kernel Module ##
EXTRA_CFLAGS +=
APP_EXTRA_FLAGS:= -O2 -ansi -pedantic
KERNEL_SRC:= /lib/modules/$(shell uname -r)/build
SUBDIR= $(PWD)
GCC:=gcc
RM:=rm

.PHONY : clean

all: clean modules app

obj-m += mp1_final.o
mp1_final-objs := linklist.o Work_Queue.o mp1.o

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(SUBDIR) modules

app: my_factorial.c my_factorial.h
	$(GCC) -o my_factorial my_factorial.c

clean:
	$(RM) -f my_factorial *~ *.ko *.o *.mod.c Module.symvers modules.order
