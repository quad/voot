BIN = 2ndstage

# FIRST - for proper .bin entry point
OBJS += crt0.o

# SECOND - as close to bootstrap as possible
OBJS += exception.o exception-lowlevel.o system.o asic.o

# THIRD - loading code, also required
OBJS += main.o warez_load.o

# All the others...
OBJS += util.o serial.o gamedata.o printf.o biosfont.o
OBJS += rtl8139c.o net.o bswap.o biudp.o
OBJS += heartbeat.o trap.o voot.o

TARGET=sh-elf
DCBASE=/home/scott/dcdev
#CC=$(DCBASE)/bin/$(TARGET)-gcc -ml -m4-single-only -g -Wall
CC=$(DCBASE)/bin/$(TARGET)-gcc -ml -m4-single-only -Os -Wall
LD=$(DCBASE)/bin/$(TARGET)-ld
AS=$(DCBASE)/bin/$(TARGET)-as
AR=$(DCBASE)/bin/$(TARGET)-ar
OBJCOPY=$(DCBASE)/bin/$(TARGET)-objcopy

#INCS =
#LIBS =

all: $(BIN).srec $(BIN).bin

$(BIN).srec: $(BIN).elf
	$(OBJCOPY) -O srec $(BIN).elf $(BIN).srec

$(BIN).bin: $(BIN).elf
	$(OBJCOPY) -O binary $(BIN).elf $(BIN).bin

$(BIN).elf: $(OBJS)
#	$(CC) -static -Wl,-Ttext,0xAC008000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)
	$(CC) -static -Wl,-Ttext,0xAC004000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)

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
