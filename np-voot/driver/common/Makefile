# Common driver library Makefile.
#
# $Id: Makefile,v 1.14 2002/11/14 20:56:02 quad Exp $

DRIVER_TARGET = libdriver-common
DRIVER_CLEAN = lwip.a-clean
DRIVER_INCS = -I./lwip/
LIBS = -llwip

# FIRST - Base interrupt and exception logic.
DRIVER_OBJS += exception.o exception-lowlevel.o system.o asic.o ubc.o

# SECOND - Driver initializer.
DRIVER_OBJS += init.o

# Utility functions.
DRIVER_OBJS += util.o printf.o searchmem.o video.o serial.o bswap.o

# Biosfont access and assertion.
DRIVER_OBJS += biosfont.o biosfont-lowlevel.o assert.o

# Network layer.
DRIVER_OBJS += rtl8139c.o dumpio.o

# Gamedata interaction logic.
DRIVER_OBJS += gamedata.o

# VOOT internal accessors.
DRIVER_OBJS += malloc.o controller.o vmu.o timer.o

# VOOT animation/render cycle patch.
DRIVER_OBJS += anim.o

all: $(DRIVER_TARGET).a lwip.a

# lwIP TCP/IP stack.
lwip.a:
	make -f Makefile.lwip

lwip.a-clean:
	make -f Makefile.lwip clean

include Makefile.driver
