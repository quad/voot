# VR Customization and 5.60 VR Test driver Makefile
#
# $Id: Makefile,v 1.3 2002/06/23 03:22:52 quad Exp $

DRIVER_COMMON_DIR = ../common
DRIVER_TARGET = cust-plus
DRIVER_OBJS = $(DRIVER_COMMON_DIR)/crt0.o module.o customize.o
DRIVER_INCS = -I$(DRIVER_COMMON_DIR)
DRIVER_DEFS = -DDEBUG

all: clean libdriver-common.a $(DRIVER_TARGET).bin

libdriver-common.a:
	make -C $(DRIVER_COMMON_DIR)

include $(DRIVER_COMMON_DIR)/Makefile.driver
