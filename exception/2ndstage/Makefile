BIN = 2ndstage

include ../Makefile.config

# FIRST - for proper .bin entry point
OBJS += crt0.o

# SECOND - as close to bootstrap as possible
OBJS += exception.o exception-lowlevel.o system.o asic.o

# THIRD - loading code, also required
OBJS += main.o warez_load.o

# All the others...
OBJS += util.o serial.o gamedata.o printf.o biosfont.o assert.o
OBJS += rtl8139c.o bswap.o net.o biudp.o voot.o dumpio.o
OBJS += heartbeat.o

# Ill-fated game-smart SCIF emulation logic
#OBJS += trap.o

# Ill-fated ADX patch
#OBJS += cdda_patch.o
#OBJS += adx_tlk2.obj adx_tlk2_patch.o sj_mem.obj

# SCIF Emulation layer v2
OBJS += scif_emu.o

# Customization Reverse Helper
OBJS += customize.o

# Controller Access
OBJS += controller.o

# VMU Access
OBJS += vmu.o

all: $(BIN).srec $(BIN).bin

$(BIN).srec: $(BIN).elf
	$(OBJCOPY) -O srec $(BIN).elf $(BIN).srec

$(BIN).bin: $(BIN).elf
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin

$(BIN).elf: $(OBJS)
#	$(CC) -static -Wl,-Ttext,0x8C008000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)
	$(CC) -static -Wl,-Ttext,0x8C004000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) -c $< -o $@

clean:
	-rm -f *.o *.elf $(EXTRA_CLEAN)

mrproper: clean
	-rm -f *.bin *.srec
	-rm -f *~
	-rm -f DEADJOE

force:
	-rm -f $(BIN).elf
	make
