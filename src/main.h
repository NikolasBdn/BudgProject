#ifndef BUDGETS_INCLUDED
#define BUDGETS_INCLUDED
#include "budget.h"
#include "depense.h"
#include "depense_recu.h"

extern sqlite3 *db;
extern int nbBudgets;
extern GtkWidget *input, *combo_box, *labelBudg[20], *list;

struct Input{
  GtkWidget *m, *t;
};

void bddConnect();
void addDepense(struct Depense dep);
void createWindow(int argc, char ** argv);
int main(int argc, char **argv);
char *displayBudgets(struct Budget budg);
void displayDepense(GtkWidget *list, struct Depense dep);
void updateBudgets(struct Budget);
#endif
