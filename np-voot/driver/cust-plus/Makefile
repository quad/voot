# VR Customization and 5.60 VR teaser driver Makefile
#
# $Id: Makefile,v 1.4 2002/08/04 05:48:05 quad Exp $

DRIVER_COMMON_DIR = ../common
DRIVER_TARGET = cust-plus
DRIVER_OBJS = $(DRIVER_COMMON_DIR)/crt0.o module.o customize.o
DRIVER_INCS = -I$(DRIVER_COMMON_DIR)
DRIVER_DEFS = -DDEBUG

all: clean libdriver-common.a $(DRIVER_TARGET).bin

libdriver-common.a:
	make -C $(DRIVER_COMMON_DIR)

include $(DRIVER_COMMON_DIR)/Makefile.driver
