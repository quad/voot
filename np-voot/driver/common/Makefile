# Common driver library Makefile.
#
# $Id: Makefile,v 1.10 2002/11/04 18:42:38 quad Exp $

DRIVER_TARGET = libdriver-common
DRIVER_CLEAN = lwip.a-clean
LIBS = -llwip

# FIRST - Base interrupt and exception logic.
DRIVER_OBJS += exception.o exception-lowlevel.o system.o asic.o ubc.o

# SECOND - Driver initializer.
DRIVER_OBJS += init.o

# Utility functions.
DRIVER_OBJS += util.o printf.o searchmem.o video.o serial.o bswap.o

# Biosfont access and assertion.
DRIVER_OBJS += biosfont.o biosfont-lowlevel.o assert.o

# Network Layer (drivers and stack).
#DRIVER_OBJS += net.o ether.o rtl8139c.o biudp.o voot.o dumpio.o

# Gamedata interaction logic.
DRIVER_OBJS += gamedata.o

# TA VSync Heartbeat.
DRIVER_OBJS += heartbeat.o

# VOOT internal accessors.
DRIVER_OBJS += malloc.o controller.o vmu.o

# VOOT animation/render cycle patch.
DRIVER_OBJS += anim.o

all: $(DRIVER_TARGET).a lwip.a

# lwIP TCP/IP stack.
lwip.a:
	make -f Makefile.lwip

lwip.a-clean:
	make -f Makefile.lwip clean

include Makefile.driver
