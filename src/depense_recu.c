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


void insertDepenseRecu(char *m, int idB){
  struct Depense dep;
  char buffId[10];
  dep.montant = atof(m);

  time_t timer;
  struct tm* tm_info;
  char mois[3];
  time(&timer);
  tm_info = localtime(&timer);
  strftime(mois, 3, "%m", tm_info);
  printf("DEPENCE RECU: %s %d", m, idB);
  //Si le montant est > 0
  if (dep.montant > 0) {
    char request[90] = "insert into DEPENSESRECURRENTE (dernierPaiement, montantDepRecu, idType) values('";
    replacechar(m, ',', '.');
    strcat(request, mois);
    strcat(request, "', ");
    strcat(request, m);
    strcat(request,", '");
    snprintf(buffId, sizeof(buffId), "%d", idB);
    strcat(request, buffId);
    strcat(request,"')");
    printf("%s\n", request);
    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : DEPENSE RECURRENTE\n");
    }else{
      printf("INSERT : DEPENSE RECURRENTE\n");
      insertDepense(m, idB);//insertion de la premiere depense à la creation de la dépense recurrente
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
    // printf("%s : %f\n",   sqlite3_column_text(stmt, 2),   sqlite3_column_double(stmt, 2));
 }
  return;
}


void paiementDepensesRecu(){
  sqlite3_stmt *stmt;
  char a[80] = "select * from DEPENSESRECURRENTE";

  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  // char *mois = "08";
  
  time_t timer;
  struct tm* tm_info;
  char mois[3];
  time(&timer);
  tm_info = localtime(&timer);
  strftime(mois, 3, "%m", tm_info);
  printf("%s\n", mois);


  while (sqlite3_step(stmt) == SQLITE_ROW) {
    printf("LOLO: %d %s\n",sqlite3_column_int(stmt, 0), (char *)sqlite3_column_text(stmt,3));
    if (strcmp((char *)sqlite3_column_text(stmt,3), mois) != 0) {
      printf("PAIMENT RECU\n");
      char buff2[30];


      gcvt(sqlite3_column_double(stmt, 1), 7, buff2);
      insertDepense(buff2, sqlite3_column_int(stmt, 2));

      sqlite3_stmt *stmt2;
      char b[80] = "UPDATE DEPENSESRECURRENTE set dernierPaiement ='";
      strcat(b, mois);
      strcat(b, "' where idDepRecu = ");
      strcat(b, (char *)sqlite3_column_text(stmt, 0));
      printf("%s\n", b);

      if (sqlite3_prepare_v2(db, b, -1, &stmt2, NULL)) {
        printf("ERROR UPDATE DEPENSESRECURRENTE\n");
        exit(-1);
      }

      sqlite3_step(stmt2);
    }else
    {
      printf("NON PAIEMENT RECU\n");
    }
    
 }
}
