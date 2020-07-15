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

  printf("DEPENSES : %s\n", dep.type);
  //Si le montant est > 0 et type != null
  if (dep.montant > 0 && strcmp(dep.type, "(null)") != 0) {
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
      vueBudgets();
      vueDepenses();
    }
  }
}


double getDepensesSumByType(char *type){

 time_t timer;
  struct tm* tm_info;
  char mois[3];
  time(&timer);
  tm_info = localtime(&timer);
  strftime(mois, 3, "%m", tm_info);
  printf("%s\n", mois);

  sqlite3_stmt *stmt;
  char a[150] = "select sum(montantDep) from depenses where typeDep = '";
  strcat(a, type);
  strcat(a, "'");

  strcat(a, " and strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) = '");
  strcat(a, mois);
  strcat(a, "'");
  printf("%s\n", a);
  char request[150] = "select sum(montantDep) from depenses where typeDep = 'Alimentation' and strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) = '08'";

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
