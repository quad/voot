BIN = npclient

OBJS += client.o npc.o voot.o

CC = gcc -g -Wall

INCS =
LIBS = -lpthread -lreadline

all: $(BIN)

$(BIN): $(OBJS)
	$(CC) -o $(BIN) $(OBJS) $(LIBS)

%.o: %.c
	$(CC) $(INCS) -c $< -o $@

%.o: %.s
	$(CC) $(INCS) -c $< -o $@

clean:
	-rm -f *.o $(BIN) $(BIN).zip

mrproper: clean
	-rm -f *~ *.bak

package: mrproper
	(cd .. ; zip -r npclient/$(BIN).zip npclient/ ) 
