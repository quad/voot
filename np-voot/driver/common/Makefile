# Common driver library Makefile.
#
# $Id: Makefile,v 1.2 2002/06/12 09:33:51 quad Exp $

DRIVER_TARGET = libdriver-common

ifdef DEBUG
	DRIVER_DEFS += -DDEBUG=1
endif

# FIRST - Base interrupt and exception logic.
DRIVER_OBJS += exception.o exception-lowlevel.o system.o asic.o

# SECOND - Driver initializer.
DRIVER_OBJS += init.o

# Required utility functions.
DRIVER_OBJS += util.o printf.o

# Biosfont access and assertion.
DRIVER_OBJS += biosfont.o assert.o

# Network Layer (drivers and stack).
DRIVER_OBJS += rtl8139c.o bswap.o net.o biudp.o voot.o dumpio.o

# Gamedata interaction logic.
DRIVER_OBJS += gamedata.o

# TA VSync Heartbeat
DRIVER_OBJS += heartbeat.o

# SCIF Emulation layer, v2.
DRIVER_OBJS += scif_emu.o serial.o 

# Controller access.
DRIVER_OBJS += controller.o

# VMU access.
DRIVER_OBJS += vmu.o

# Framebuffer & PVR access.
DRIVER_OBJS += video.o

all: clean $(DRIVER_TARGET).a

include Makefile.driver
