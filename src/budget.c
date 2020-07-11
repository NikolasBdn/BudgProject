#include <gtk/gtk.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "budget.h"
#include "depense.h"
#include "main.h"
#include "depense_recu.h"

extern sqlite3 *db;
extern GtkWidget *labelBudg[20];

int budgUnique(char *type){
  sqlite3_stmt *stmt;
  char request[60] = "select count(*) from budgets where typeBudg = '";
  strcat(request, type);
  strcat(request, "'");

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : getBudget\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  return sqlite3_column_int(stmt, 0) - 1;
}


void insertBudg(char *montant, char *type){
  replacechar(montant, '.', ',');

  struct Budget budg;
  budg.montant = atof(montant);
  snprintf(budg.type, sizeof(budg.type), "%s", type);
  printf("INSERT BUDG : %s : %s\n", type, montant);
    //Si montant > 0 et si le budg n'existe pas encore
  if (budg.montant > 0 && budgUnique(budg.type)) {

    char request[80] = "insert into BUDGETS (montantBudg, typeBudg) values(";
    replacechar(montant, ',', '.');
    strcat(request, montant);
    strcat(request,", '");
    strcat(request, budg.type);
    strcat(request,"')");

    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : BUDGET\n");
    }else{
      printf("INSERT BUDGET : %s : %s\n", montant, type);
      // updateBudgets();
      vueBudgets();
    }
  }else{
    printf("PAS UNIQUE: %s\n", budg.type);
  }

}


struct Budget getBudget(int index){
  sqlite3_stmt *stmt;
  char request[60] = "select * from budgets where idBudg = ";
  char bufIndex[10];
  sprintf(bufIndex, "%d", index);
  strcat(request, bufIndex);

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : getBudget\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  struct Budget res;
  res.montant = sqlite3_column_double(stmt, 1);

  snprintf(res.type, sizeof(res.type), "%s", (char *)sqlite3_column_text(stmt, 2));
  return res;
}

struct Budget getBudgetByType(char *type){
  sqlite3_stmt *stmt;
  char request[60] = "select * from budgets where typeBudg = '";
  strcat(request, type);
  strcat(request, "'");

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : getBudget\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  struct Budget res;
  res.montant = sqlite3_column_double(stmt, 1);

  snprintf(res.type, sizeof(res.type), "%s", (char *)sqlite3_column_text(stmt, 2));
  return res;
}


int getNbBudgets(){
  sqlite3_stmt *stmt;
  char request[60] = "select count(*) from budgets ";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : getBudget\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  return sqlite3_column_int(stmt, 0);
}
