#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


static GtkWidget *button, *input, *combo_box;
GtkWidget *window, *grid;

static sqlite3 *db;

const char *listeBudg[] = {"Alimentation", "Loisirs", "Dépacements", "Loyer"};
const int montantBudg[] = {100, 50, 20, 300};

void initBudg();

int bddConnect(){
  if (sqlite3_open("./the.db", &db)) {
    printf("Could not open the.db\n");
    exit(-1);
  }

  if (sqlite3_exec(db, "create table DEPENSES (idDep integer primary key autoincrement, montantDep flaot(7,2) not null, typeDep varchar(20), dateDep datetime default (STRFTIME('%d/%m/%Y', 'NOW','localtime')))", NULL, NULL, NULL)) {
     printf("Error executing sql statement\n");
  }
  else {
    printf("Table DEPENSES created\n");
  }

  if (sqlite3_exec(db, "create table BUDGETS (idBudg integer primary key autoincrement, montantBudg int not null, typeBudg varchar(20))", NULL, NULL, NULL)) {
   printf("Error executing sql statement\n");
}
else {
  printf("Table BUDGETS created\n");
  initBudg();
}

  if (sqlite3_exec(db, "create table DEPENSESRECURRENTE (idDepRecu integer primary key autoincrement, montantDepRecu int not null, typeDepRecu varchar(20), dateDepRecu datetime default (STRFTIME('%d/%m/%Y', 'NOW','localtime')))", NULL, NULL, NULL)) {
     printf("Error executing sql statement\n");
  }
  else {
    printf("Table DEPENSE RECURRENTE created\n");
  }

}

void printColumnValue(sqlite3_stmt* stmt, int col){
  int colType = sqlite3_column_type(stmt, col);

  switch(colType) {

    case SQLITE_INTEGER:
         printf("  %3d   ", sqlite3_column_int(stmt, col));
         break;

    case SQLITE_FLOAT:
         printf("  %6.2f", sqlite3_column_double(stmt, col));
         break;

    case SQLITE_TEXT:
         printf("  %-5s", sqlite3_column_text(stmt, col));
         break;

    case SQLITE_NULL:
         printf("  null");
         break;

    case SQLITE_BLOB:
         printf("  blob");
         break;
    }
}

void printDepenses(){
  printf("\n");
  printf("DEPENSES :\n");
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, "select * from depenses", -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  while (sqlite3_step(stmt) != SQLITE_DONE){
    for (int col = 0; col <= 3; col++) {
      printColumnValue(stmt, col);
    }
    printf("\n");
  }

  sqlite3_finalize(stmt);
}

void printBudgets(){
  printf("\n");
  printf("BUDGETS :\n");
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, "select * from budgets", -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  while (sqlite3_step(stmt) != SQLITE_DONE){
    for (int col = 0; col <= 2; col++) {
      printColumnValue(stmt, col);
    }
    printf("\n");
  }

  sqlite3_finalize(stmt);
}


void insertDepenses(GtkWidget *button){
  char request[80] = "insert into DEPENSES (montantDep, typeDep) values(";

  strcat(request, (char *)gtk_entry_get_text(GTK_ENTRY(input)));
  strcat(request,", '");
  strcat(request, (char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box)));
  strcat(request,"')");

  // printf("%s\n", request);

  if(sqlite3_exec(db, request, NULL, NULL, NULL)){
    printf("ERROR IN INSERTION\n");
  }else{
    printf("VALUES INSERT\n");
  }

  printBudgets();
  printDepenses();

  sqlite3_stmt *stmt;
  const char *t= "Alimentation";
  char a[80] = "select sum(montantDep) from depenses where typeDep = '";
  // strcat(a, );
  strcat(a, t);
  strcat(a, "'");
  printf("%s\n", a);
  if (sqlite3_prepare_v2(db, a, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  sqlite3_step(stmt);
  printf("SUM Alimentation:");
  printColumnValue(stmt, 0);
  printf("\n");
}

//insertion des budgets
void initBudg(){
  printf("RESET BUDGETS\n");

  for (int i = 0; i < sizeof(listeBudg) / sizeof(listeBudg[0]); i++) {
    char request[80] = "insert into BUDGETS (montantBudg, typeBudg) values(";
    char buf[30];
    sprintf(buf, "%d", montantBudg[i]);
    strcat(request, buf);
    strcat(request, ", '");
    strcat(request,listeBudg[i]);
    strcat(request,"')");
    printf("%s\n", request);
    if(sqlite3_exec(db, request, NULL, NULL, NULL)){
      printf("ERROR IN INSERTION\n");
    }else{
      printf("VALUES INSERT\n");
    }
  }
}


void calculBudget(int indexBudg) {



}

GtkWidget *displayBudgets(int index){
  sqlite3_stmt *stmt;
  char request[60] = "select * from budgets where idBudg = ";
  char bufIndex[10];
  sprintf(bufIndex, "%d", index);
  strcat(request, bufIndex);

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA\n");
    exit(-1);
  }

  sqlite3_step(stmt);

  char res[200] = "";
  char buf[30];
  strcat(res, sqlite3_column_text(stmt, 2));
  printf("TOTO\n" );
  sprintf(buf, "%.2f", sqlite3_column_double(stmt, 1));
  strcat(res, " : ");
  strcat(res, buf);

  return gtk_label_new (res);
}

//Creation de l'interface
void createWindow(int argc, char ** argv){
  gtk_init(&argc, &argv);

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  grid = gtk_grid_new();

  input = gtk_entry_new();
  gtk_grid_attach(GTK_GRID(grid), input, 0,0,1,1);

  button = gtk_button_new_with_label("Save");
  g_signal_connect(button, "clicked", G_CALLBACK(insertDepenses), NULL);
  gtk_grid_attach(GTK_GRID(grid), button, 1,0,1,1);

  for (int i = 1; i < 4; i++) {
    printf("%s\n", listeBudg[i]);
    GtkWidget *labelBudg = displayBudgets(i);
    gtk_grid_attach(GTK_GRID(grid), labelBudg, 10,0+i,1,1);

  }

  // Init de la comboBox des Budgets
  combo_box = gtk_combo_box_text_new();
  for (int i = 0; i < sizeof(listeBudg) / sizeof(listeBudg[0]); i++) {
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box), listeBudg[i]);
  }

  gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box), 0);
  gtk_grid_attach(GTK_GRID(grid), combo_box, 0  ,1,1,1);
  // g_signal_connect(combo_box, "changed", G_CALLBACK(insertDepenses), NULL);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_window_set_default_size(GTK_WINDOW (window), 955, 500);

  gtk_widget_show_all(window);
  gtk_main();
}


int main(int argc, char **argv){
  bddConnect(db);

  // createWindow(argc, argv);

}

// gcc displayData.c -o displayData -lsqlite3 -std=c99  `pkg-config --libs --cflags gtk+-3.0`
