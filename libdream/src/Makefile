include ../Makefile.config

INCS=-I../include
OBJFILES  = libdream.o serial.o font.o spu.o
OBJFILES += video.o ta.o biosfont.o
OBJFILES += maple.o controller.o keyboard.o vmu.o mouse.o
OBJFILES += cdrom.o fs_iso9660.o
OBJFILES += timer.o

all: libdream.a

libdream.a: $(OBJFILES)
	$(AR) rcs $@ $(OBJFILES)
	-cp $@ ../lib/

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(AS) -little $< -o $@

clean:
	-rm -f *.o *.a *.bck


