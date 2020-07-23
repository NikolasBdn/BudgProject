
#ifndef BUDGET_H
#define BUDGET_H

struct Budget{
  float montant;
  char type[100];
};

void insertBudg(char *montant, char *type);
struct Budget getBudget(int index);
struct Budget getBudgetByType(char *type);

int getNbBudgets();
void updateBudgetsMontant();
#endif
