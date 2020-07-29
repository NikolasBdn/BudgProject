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
  printf("budg UNIQUE ?: %d\n", sqlite3_column_int(stmt, 0));

  return sqlite3_column_int(stmt, 0) - 1;
}


void insertBudg(char *montant, char *type){
  replacechar(montant, '.', ',');

  struct Budget budg;
  budg.montant = atof(montant);
  snprintf(budg.type, sizeof(budg.type), "%s", type);
  printf("INSERT BUDG : %s : %s\n", type, montant);
    //Si montant > 0 et si le budg n'existe pas encore
  if (budg.montant > 0 && budgUnique(budg.type) && strcmp(budg.type, "") != 0) {

    char request[80] = "insert into BUDGETS (montantBudg, typeBudg) values(";
    replacechar(montant, ',', '.');
    strcat(request, montant);
    strcat(request,", '");
    strcat(request, budg.type);
    strcat(request,"')");

    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : BUDGET\n");
    }else{
      // printf("INSERT BUDGET : %s : %s %d\n", montant, type, budgUnique(budg.type) );
      // updateBudgets();
      vueBudgets();
    }
  }else{
    // printf("PAS UNIQUE: %s %d\n", budg.type, budgUnique(budg.type));
  }
}


void deleteBudg(int id){
  sqlite3_stmt *stmt;
  char buffId[10];
  char request2[80] = "select idBudg from BUDGETS where idBudg =";
  snprintf(buffId, sizeof(buffId), "%d", id);
  strcat(request2, buffId);
  printf("%s\n", request2);

  sqlite3_prepare_v2(db, request2, -1, &stmt, NULL);
  sqlite3_step(stmt);
   
  printf("SUPPRIMER %d\n", id);
  char request1[80] = "DELETE from BUDGETS where idBudg =";
  snprintf(buffId, sizeof(buffId), "%d", id);
  strcat(request1, buffId);
  printf("request1: %s\n", request1);

  if(sqlite3_exec(db, request1, NULL, NULL, NULL)){
    printf("ERROR IN DELETE : BUDGETS\n");
  }
 

  // suppression des dépenses associées au budg supprimé
  char *buffType = (char *)sqlite3_column_text(stmt, 0);
  printf("BUFFTYPE: %s\n", buffType);
  char request3[80] = "DELETE from DEPENSES where idType =";
  // snprintf(buffType, sizeof(buffType), "%d", id);
  strcat(request3, "'");
  strcat(request3, buffType);
  strcat(request3, "'");
  printf("request3: %s\n", request3);

  if(sqlite3_exec(db, request3, NULL, NULL, NULL)){
    printf("ERROR IN DELETE : DEPENSES DE BUDGETS\n");
  }
   // suppression des dépenses récurrentes associées au budg supprimé
  printf("BUFFTYPE: %s\n", buffType);
  char request4[80] = "DELETE from DEPENSESRECURRENTE where idType =";
  // snprintf(buffType, sizeof(buffType), "%d", id);
  strcat(request4, "'");
  strcat(request4, buffType);
  strcat(request4, "'");
  printf("request3: %s\n", request3);

  if(sqlite3_exec(db, request4, NULL, NULL, NULL)){
    printf("ERROR IN DELETE : DEPENSESRECURRENTES DE BUDGETS\n");
  }
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
