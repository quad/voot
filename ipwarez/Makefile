BINA = boot1
BINB = bootstrap
OBJS = crt0.o go.o disable.o warez_load.o

include ./Makefile.config

INCS=
LIBS=

all: $(BINA).bin $(BINB).bin

%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

$(BINA).elf: $(BINA).o
	$(CC) -Wl,-Ttext,8c00b800 -nostdlib -nostartfiles -o $@ $(BINA).o $(LIBS)

$(BINB).elf: $(OBJS)
	$(CC) -Wl,-Ttext,8c00e000 -nostdlib -nostartfiles -o $@ $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -fomit-frame-pointer $(INCS) -c $< -o $@

%.o: %.s
	$(AS) -little $(INCS) $< -o $@

clean:
	-rm -f *.o *.elf 1ST_READ.BIN *.bck $(EXTRA_CLEAN)

reallyclean: clean
	-rm -f *.bin *.srec

force:
	-rm -f $(BIN).elf
	make

