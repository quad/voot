include ../Makefile.config

INCS = ../include/

BIN = libdc
OBJS = bswap.o exception.o net.o rtl8139c.o serial.o system.o util.o

all: build-src

build-src: $(BIN).a

$(BIN).a: $(OBJS)
	$(AR) rcs $@ $(OBJFILES)
	-cp $@ ../lib/

%.o: %.c
	$(CC) -I $(INCS) -c $< -o $@

%.o: %.s
	$(CC) -I $(INCS) -c $< -o $@

clean:
	rm -f *.o *.bak *~

reallyclean:
	rm -f *.elf *.srec *.bin

force: reallyclean
	$(MAKE)
