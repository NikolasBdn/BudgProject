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

void insertDepense(char *m, int idB){
  struct Depense dep;
  char buffId[10];
  dep.montant = atof(m);

  printf("DEPENSES : %s\n", dep.type);
  //Si le montant est > 0 et type != null
  if (dep.montant > 0 && strcmp(dep.type, "(null)") != 0) {
    char request[80] = "insert into DEPENSES (montantDep, idType) values(";
    replacechar(m, ',', '.');
    strcat(request, m);
    strcat(request,", '");
    snprintf(buffId, sizeof(buffId), "%d", idB);
    strcat(request, buffId);
    strcat(request,"')");
    printf("REQUEST: %s", request);
    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION : DEPENSE\n");
    }else{
      printf("INSERT : DEPENSE\n");
      vueBudgets();
      vueDepenses();
    }
  }
}

void deleteDepense(int id){
  char buffId[10];
  printf("SUPPRIMER %d\n", id);
  char request[80] = "DELETE from DEPENSES where idDep =";
  snprintf(buffId, sizeof(buffId), "%d", id);
  strcat(request, buffId);
  printf("%s\n", request);

  if(sqlite3_exec(db, request, NULL, NULL, NULL)){
    printf("ERROR IN DELETE : DEPENSE\n");
  }
}

double getDepensesSumByType(int idB){

 time_t timer;
  struct tm* tm_info;
  char mois[3];
  time(&timer);
  tm_info = localtime(&timer);
  strftime(mois, 3, "%m", tm_info);
  printf("%s\n", mois);

  sqlite3_stmt *stmt;
  char buffId[10];

  char a[150] = "select sum(montantDep) from depenses where idType = '";
  snprintf(buffId, sizeof(buffId), "%d", idB);
  strcat(a, buffId);
  strcat(a, "'");

  strcat(a, " and strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) = '");
  strcat(a, mois);
  strcat(a, "'");
  // printf("%s\n", a);
  // char request[150] = "select sum(montantDep) from depenses where typeDep = 'Alimentation' and strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) = '08'";

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

int getDateDerniereDep(){
  sqlite3_stmt *stmt;
  char a[150] = "select  strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) from depenses order by dateDep DESC";

  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  printf("LAST DEP: %d\n", sqlite3_column_int(stmt, 0));
  return sqlite3_column_int(stmt, 0);
}

int getDatePremiereDep(){
  sqlite3_stmt *stmt;
  char a[150] = "select  strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) from depenses order by strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch'))";

  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  printf("FISt DEP: %d\n", sqlite3_column_int(stmt, 0));
  return sqlite3_column_int(stmt, 0);
}


int getNbMois(){
  int nb = 0;
    sqlite3_stmt *stmt;
    char a[150] = "SELECT * from DEPENSES GROUP by strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch'))";

    if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
      printf("ERROR TO SELECT DATA\n");
      exit(-1);
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
      nb++;
    }

    printf("NB MOIS DEP: %d\n", nb);
    return nb;
}
