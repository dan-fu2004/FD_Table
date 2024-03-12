CC = gcc
CFLAGS = -Wall -g
OBJ = FD.o functions.o
EXEC = FD

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^

FD.o: FD.c functions.h
	$(CC) -c $(CFLAGS) $<

functions.o: functions.c functions.h
	$(CC) -c $(CFLAGS) $<

clean:
	rm -f $(OBJ) $(EXEC)
