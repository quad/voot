include ../Makefile.config

INCS = ../include/

BIN = libdc
OBJS = bswap.o exception.o net.o rtl8139c.o serial.o system.o util.o

all: $(BIN).a

$(BIN).a: $(OBJS)
#	$(CC) -static -Wl,-Ttext,0xAC004000 -nostartfiles -o $(BIN).elf $(OBJS) $(LIBS)

%.o: %.c
	$(CC) -I $(INCS) -c $< -o $@

%.o: %.s
	$(CC) -I $(INCS) -c $< -o $@

clean:
	rm *.o *.bak *~

force:
	-rm -f $(BIN).elf
	make
