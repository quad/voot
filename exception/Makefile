BIN = loader

include Makefile.config

# THIS MUST BE FIRST FOR PROPER .BIN LOADING
OBJS = crt0.o

# All others are second. ;-)
OBJS += loader.o
OBJS += vconsole.o gdrom.o

# LAST for easy debugging
OBJS += 2ndstage.o

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
	echo "unsigned char stage_two_build_time[] = \"(main built at \" __TIME__ \" on \" __DATE__ \")\";" >> 2ndstage.c

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) -c $< -o $@

1ST_READ.BIN: $(BIN).elf scramble
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin
	./scramble $(BIN).bin 1ST_READ.BIN

IP.BIN: makeip
	./makeip ip.txt IP.BIN

np-voot-slave.iso: 1ST_READ.BIN IP.BIN
	mkisofs -l -V NPVOOTSLAVE -C 0,11702 -o temp.iso 1ST_READ.BIN IP.BIN
	( cat IP.BIN ; dd if=temp.iso bs=2048 skip=16 ) > $@

data.iso-inzip: np-voot-slave.iso
	cp np-voot-slave.iso np-voot-slave/data.iso

np-voot-slave.zip: data.iso-inzip
	zip -r np-voot-slave.zip np-voot-slave/ -x np-voot-slave/CVS/*

scramble:
	gcc -o scramble tools/scramble/scramble.c

makeip:
	gcc -o makeip tools/makeip/makeip.c

bin2c:
	gcc -o bin2c bin2c.c

clean:
	make -C 2ndstage clean
	-rm -f 2ndstage.c
	-rm -f *.o *.elf *.bck $(EXTRA_CLEAN)
	-rm -f temp.iso
	-rm -f 1ST_READ.BIN IP.BIN np-voot-slave.iso np-voot-slave.zip
	-rm -f np-voot-slave/data.iso

mrproper: clean
	make -C 2ndstage mrproper
	-rm -f *.bin *.srec
	-rm -f *~
	-rm -f scramble makeip bin2c

run: $(BIN).bin
	dc-tool -n -x $(BIN).elf

force:
	-rm -f $(BIN).elf
	make
