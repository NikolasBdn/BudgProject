#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <gtk/gtk.h>
#include <time.h>
#include "depense.h"
#include "main.h"

extern GtkWidget *inpt_dep, *combo_box_dep, *list;

extern sqlite3 *db;

void insertDepense(char *m, char *t){
  struct Depense dep;
  dep.montant = atof(m);
  snprintf(dep.type, sizeof(dep.type), "%s", t);


  time_t time_raw_format;
  struct tm * ptr_time;
  time ( &time_raw_format );
  ptr_time = localtime ( &time_raw_format );
  strftime(dep.date,50,"%d/%m/%Y",ptr_time);

  //Si le montant est > 0
  if (dep.montant > 0) {
    char request[80] = "insert into DEPENSES (montantDep, typeDep) values(";
    replacechar(m, ',', '.');
    strcat(request, m);
    strcat(request,", '");
    strcat(request, dep.type);
    strcat(request,"')");
    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : DEPENSE\n");
    }else{
      printf("INSERT : DEPENSE\n");
      displayDepense(list, dep);
      updateBudgetsMontant();

    }
  }
}


double getDepensesSumByType(char *type){
  sqlite3_stmt *stmt;
  char a[80] = "select sum(montantDep) from depenses where typeDep = '";
  strcat(a, type);
  strcat(a, "'");
  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  return sqlite3_column_double(stmt, 0);
}


int replacechar(char *str, char orig, char rep) {
    char *ix = str;
    int n = 0;
    while((ix = strchr(ix, orig)) != NULL) {
        *ix++ = rep;
        n++;
    }
    return n;
}
