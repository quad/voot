LIBDREAM_BASE = /home/scott/Projects/VOOT/prog/libdream/

BIN = ubc-2ndstage

# THIS MUST BE FIRST FOR PROPER .BIN LOADING
OBJS += crt0.o

# All others are second. ;-)
OBJS += disable.o regs.o ubc.o ubc-lowlevel.o
OBJS += serial.o

TARGET=sh-elf
DCBASE=/home/scott/dcdev
CC=$(DCBASE)/bin/$(TARGET)-gcc -ml -O -m4-single-only -Wall
LD=$(DCBASE)/bin/$(TARGET)-ld
AS=$(DCBASE)/bin/$(TARGET)-as
AR=$(DCBASE)/bin/$(TARGET)-ar
OBJCOPY=$(DCBASE)/bin/$(TARGET)-objcopy

INCS = -I$(LIBDREAM_BASE)/include
LIBS = -L$(LIBDREAM_BASE)/lib -ldream

#INCS =
#LIBS =

all: $(BIN).srec $(BIN).bin

$(BIN).srec: $(BIN).elf
	$(OBJCOPY) -O srec $(BIN).elf $(BIN).srec

$(BIN).bin: $(BIN).elf
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin

$(BIN).elf: $(OBJS)
#	$(CC) -static -Wl,-Ttext,0xAC008000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)
	$(CC) -static -Wl,-Tubc-2ndstage.lnk -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) -c $< -o $@

clean:
	-rm -f *.o *.elf 1ST_READ.BIN *.bck $(EXTRA_CLEAN)

reallyclean: clean
	-rm -f *.bin *.srec

run: $(BIN).bin
	dc-tool -p -x $(BIN).elf

force:
	-rm -f $(BIN).elf
	make
