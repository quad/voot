# VR Customization and 5.60 VR Test driver Makefile
#
# $Id: Makefile,v 1.11 2002/12/17 11:31:58 quad Exp $

DRIVER_COMMON_DIR = ../common
DRIVER_TARGET = debug
DRIVER_OBJS = $(DRIVER_COMMON_DIR)/crt0.o module.o scixb_emu.o
DRIVER_INCS = -I$(DRIVER_COMMON_DIR)
DRIVER_DEFS = -DDEBUG
DRIVER_LIBS = -llwip

all: clean libdriver-common.a $(DRIVER_TARGET).bin

libdriver-common.a:
	make -C $(DRIVER_COMMON_DIR)

include $(DRIVER_COMMON_DIR)/Makefile.driver
