# VR Customization and 5.60 VR Test driver Makefile
#
# $Id: Makefile,v 1.2 2002/06/12 04:41:34 quad Exp $

DRIVER_COMMON_DIR = ../common
DRIVER_TARGET = debug
DRIVER_OBJS = $(DRIVER_COMMON_DIR)/crt0.o module.o
DRIVER_INCS = -I$(DRIVER_COMMON_DIR)
#DRIVER_COMMON_OPTS = DEBUG=1

all: clean libdriver-common.a $(DRIVER_TARGET).bin

libdriver-common.a:
	make -C $(DRIVER_COMMON_DIR) $(DRIVER_COMMON_OPTS)

include $(DRIVER_COMMON_DIR)/Makefile.driver
