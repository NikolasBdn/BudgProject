
CC=gcc
CFLAGS=-Wall -fno-stack-protector  -lsqlite3 -std=c99  `pkg-config --libs --cflags gtk+-3.0
LDFLAGS=-Wall -fno-stack-protector  -lsqlite3 -std=c99  `pkg-config --libs --cflags gtk+-3.0
EXEC=budg

all: $(EXEC)

budg: main.o depense.o budget.o depense_recu.o
	$(CC) -o budg main.o depense.o budget.o depense_recu.o$(LDFLAGS)

main.o: main.c depense.h budget.h depense_recu.h
	$(CC) -o main.o -c main.c $(LDFLAGS)

depense.o: depense.c main.h budget.h depense_recu.h
	$(CC) -o depense.o -c depense.c $(LDFLAGS)

budget.o: budget.c depense.h main.h depense_recu.h
	$(CC) -o budget.o -c budget.c $(LDFLAGS)

depense_recu.o: depense_recu.c depense.h budget.h main.h
	$(CC) -o depense_recu.o -c depense_recu.c $(LDFLAGS)


clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
