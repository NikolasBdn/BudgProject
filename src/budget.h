
#ifndef BUDGET_H
#define BUDGET_H

struct Budget{
  float montant;
  char type[100];
};

void insertBudg(GtkWidget *button, gpointer data);
struct Budget getBudget(int index);
struct Budget getBudgetByType(char *type);

int getNbBudgets();
void updateBudgetsMontant();
#endif
