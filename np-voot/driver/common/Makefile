# Common driver library Makefile.
#
# $Id: Makefile,v 1.4 2002/06/20 10:20:04 quad Exp $

DRIVER_TARGET = libdriver-common

# FIRST - Base interrupt and exception logic.
DRIVER_OBJS += exception.o exception-lowlevel.o system.o asic.o

# SECOND - Driver initializer.
DRIVER_OBJS += init.o

# Utility functions.
DRIVER_OBJS += util.o printf.o searchmem.o video.o serial.o

# Biosfont access and assertion.
DRIVER_OBJS += biosfont.o assert.o

# Network Layer (drivers and stack).
DRIVER_OBJS += rtl8139c.o bswap.o net.o biudp.o voot.o dumpio.o

# Gamedata interaction logic.
DRIVER_OBJS += gamedata.o

# TA VSync Heartbeat
DRIVER_OBJS += heartbeat.o

# VOOT internal accessors
DRIVER_OBJS += malloc.o controller.o vmu.o

all: $(DRIVER_TARGET).a

include Makefile.driver
