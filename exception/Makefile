LIBDREAM_BASE = ./libdream/

BIN = loader

# THIS MUST BE FIRST FOR PROPER .BIN LOADING
OBJS = crt0.o

# All others are second. ;-)
OBJS += loader.o
OBJS += vconsole.o gdrom.o

# LAST for easy debugging
OBJS += 2ndstage.o

TARGET=sh-elf
DCBASE=/home/scott/dcdev
CC=$(DCBASE)/bin/$(TARGET)-gcc -ml -O -m4-single-only -Wall
LD=$(DCBASE)/bin/$(TARGET)-ld
AS=$(DCBASE)/bin/$(TARGET)-as
AR=$(DCBASE)/bin/$(TARGET)-ar
OBJCOPY=$(DCBASE)/bin/$(TARGET)-objcopy

INCS = -I$(LIBDREAM_BASE)/include
LIBS = -L$(LIBDREAM_BASE)/lib -ldream

all: $(BIN).srec $(BIN).bin

$(BIN).srec: $(BIN).elf
	$(OBJCOPY) -O srec $(BIN).elf $(BIN).srec

$(BIN).bin: $(BIN).elf
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin

$(BIN).elf: $(OBJS)
	$(CC) -Wl,-Ttext,0x8C010000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)

2ndstage/2ndstage.bin:
	make -C 2ndstage 2ndstage.bin

2ndstage.c: bin2c 2ndstage/2ndstage.bin
	./bin2c 2ndstage/2ndstage.bin stage_two_bin 2ndstage.c
	echo "unsigned char stage_two_build_time[] = \"(main build at \" __TIME__ \" on \" __DATE__ \")\";" >> 2ndstage.c

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) -c $< -o $@

1ST_READ.BIN: $(BIN).elf scramble
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin
	./scramble $(BIN).bin 1ST_READ.BIN

scramble:
	gcc -o scramble tools/scramble/scramble.c

bin2c:
	gcc -o bin2c bin2c.c

clean:
	make -C 2ndstage clean
	-rm -f 2ndstage.c
	-rm -f *.o *.elf 1ST_READ.BIN *.bck $(EXTRA_CLEAN)

reallyclean: clean
	make -C 2ndstage clean
	-rm -f *.bin *.srec bin2c

run: $(BIN).bin
	dc-tool -p -x $(BIN).elf

force:
	-rm -f $(BIN).elf
	make
