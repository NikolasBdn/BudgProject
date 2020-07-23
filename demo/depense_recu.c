#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <gtk/gtk.h>
#include <time.h>
#include "depense_recu.h"
#include "depense.h"
#include "main.h"
#include "budget.h"

extern sqlite3 *db;


void insertDepenseRecu(char *m, char *t){
  struct Depense dep;
  dep.montant = atof(m);
  snprintf(dep.type, sizeof(dep.type), "%s", t);

  //Si le montant est > 0
  if (dep.montant > 0) {
    char request[80] = "insert into DEPENSESRECURRENTE (montantDepRecu, typeDepRecu) values(";
    replacechar(m, ',', '.');
    strcat(request, m);
    strcat(request,", '");
    strcat(request, dep.type);
    strcat(request,"')");
    printf("%s\n", request);
    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : DEPENSE RECURRENTE\n");
    }else{
      printf("INSERT : DEPENSE RECURRENTE\n");
      insertDepense(m, t);//insertion de la premiere depense à la creation de la dépense recurrente
      // displayDepense(list, dep);
      // updateBudgetsMontant();
    }
  }
}


void testRecu(){
  sqlite3_stmt *stmt;
  char a[80] = "select * from DEPENSESRECURRENTE";

  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    printf("DEPENSES RECURRENTE:\n");
    printf("%s : %f\n",   sqlite3_column_text(stmt, 2),   sqlite3_column_double(stmt, 2));
 }
  return;
}


void paymentDepensesRecu(){
    sqlite3_stmt *stmt;
  char a[80] = "select * from DEPENSESRECURRENTE";

  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    char buff[30];
    gcvt(sqlite3_column_double(stmt, 1), 7, buff);
    insertDepense(buff, (char *)sqlite3_column_text(stmt, 2));
 }
}
