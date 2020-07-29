#ifndef DEPENSES_H
#define DEPENSES_H
#include <gtk/gtk.h>

struct Depense{
  float montant;
  char type[50];
  char date[50];
};

void insertDepense(char *m, int id);
void printDepenses();
double getDepensesSumByType(int id);
void printColumnValue(sqlite3_stmt* stmt, int col);
int replacechar(char *str, char orig, char rep);
void checkDepensesRecu();
int replacechar(char *str, char orig, char rep);
int getDateDerniereDep();
int getNbMois();
void deleteDepense(int id);
int getDatePremiereDep();
#endif
