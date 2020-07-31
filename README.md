Par Nicolas BAUDON
Gestionnaire de budgets

Complier :
gcc -Wall src/main.c src/budget.c src/depense.c src/depense_recu.c -o src/main.o  -fno-stack-protector  -lsqlite3 -std=c99  `pkg-config --libs --cflags gtk+-3.0`
