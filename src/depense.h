#ifndef DEPENSES_H
#define DEPENSES_H
#include <gtk/gtk.h>

struct Depense{
  float montant;
  char type[50];
  char date[50];
};

void insertDepense(char *m, char *t);
void printDepenses();
double getDepensesSumByType(char *type);
void printColumnValue(sqlite3_stmt* stmt, int col);
int replacechar(char *str, char orig, char rep);
void checkDepensesRecu();
int replacechar(char *str, char orig, char rep);
#endif
