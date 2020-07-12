#include <gtk/gtk.h>
#include <sqlite3.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include "main.h"
#include "budget.h"
#include "depense_recu.h"
#include "depense.h"


GtkWidget *bt_open_dialog_dep, *bt_open_dialog_budg, *bt_new_budg, *bt_save_dep;
GtkWidget *input_dep, *combo_box_dep, *check_box_recu;
GtkWidget *input_nom_budg, *input_montant_budg;
GtkWidget *window, *grid;
GtkWidget *labelBudg[20];
GtkWidget *box_all, *box_budg;
GtkWidget *list, *store;
GtkWidget *scrolled_window;
GtkWidget *dialog_dep, *dialog_budg;

GtkListStore *list_store_dep, *list_store_budg;
GtkTreeViewColumn *cx1, *cx2, *cx3, *cx4, *cx5, *cx6;
GtkCellRenderer *cr1, *cr2, *cr3, *cr4, *cr5, *cr6;

sqlite3 *db;
int nbBudgets;

//GOOD
void bddConnect(){
  if (sqlite3_open("the.db", &db)) {
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
  }


  if (sqlite3_exec(db, "create table DEPENSESRECURRENTE (idDepRecu integer primary key autoincrement, montantDepRecu int not null, typeDepRecu varchar(20), dateDepRecu datetime default (STRFTIME('%d/%m/%Y', 'NOW','localtime')))", NULL, NULL, NULL)) {
     printf("Error executing sql statement\n");
  }
  else {
    printf("Table DEPENSE RECURRENTE created\n");
  }
}

char *displayBudgets(struct Budget budg){
  static char res[200];
  sprintf(res, "%s", "");
  strcat(res, "<b>");
  char buf[50];
  strcat(res, budg.type);
  strcat(res, " : ");
  sprintf(buf, "%.2f", budg.montant - getDepensesSumByType(budg.type) );
  strcat(res, buf);
  strcat(res, " / ");

  sprintf(buf, "%.2f", budg.montant);
  strcat(res, buf);
  strcat(res, "</b>");
  return res;
}

enum {

  TYPE_COLUMN,
  MONTANT_COLUMN,
  DATE_COLUMN,
  N_COLUMNS
};

void init_list(GtkWidget *list) {

  GtkCellRenderer *renderer;
  GtkTreeViewColumn *column;
  GtkListStore *store;

  renderer = gtk_cell_renderer_text_new ();
  column = gtk_tree_view_column_new_with_attributes("Type",
          renderer, "text", TYPE_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  column = gtk_tree_view_column_new_with_attributes("Montant",
          renderer, "text", MONTANT_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  column = gtk_tree_view_column_new_with_attributes("Date",
          renderer, "text", DATE_COLUMN, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(list), column);

  store = gtk_list_store_new(N_COLUMNS,
                             G_TYPE_STRING,
                             G_TYPE_FLOAT,
                             G_TYPE_STRING);

  gtk_tree_view_set_model(GTK_TREE_VIEW(list),
      GTK_TREE_MODEL(store));

  g_object_unref(store);
}

void displayDepense(GtkWidget *list, struct Depense dep) {

  GtkListStore *store;
  GtkTreeIter iter;
  store = GTK_LIST_STORE(gtk_tree_view_get_model
      (GTK_TREE_VIEW(list)));

  gtk_list_store_append(store, &iter);

  float m = dep.montant;
  m = m*100;
  m = (float)((int)m);
  m = m/100;

  gtk_list_store_set(store, &iter,
                                  TYPE_COLUMN, dep.type,
                                  MONTANT_COLUMN, m,
                                  DATE_COLUMN, dep.date,
                                  -1);

}

void insertAllDepenses(){
  sqlite3_stmt *stmt;
  char request[60] = "select * from DEPENSES";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : getBudget\n");
    exit(-1);
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    struct Depense dep;
    dep.montant = sqlite3_column_double(stmt, 1) * 100;
    dep.montant = (float)((int) dep.montant);
    dep.montant = dep.montant/100;

    snprintf(dep.type, sizeof(dep.type), "%s", (char *)sqlite3_column_text(stmt, 2));
    snprintf(dep.date, sizeof(dep.date), "%s", (char *)sqlite3_column_text(stmt, 3));

    displayDepense(list, dep);
    // printf("%s : %.2f\n", sqlite3_column_text(stmt, 2), sqlite3_column_double(stmt, 0));
  }
}

//GOOD
void newBudget(GtkWidget *button, gpointer data){
  printf("NEW BUDGET\n");

  char *m = (char *)gtk_entry_get_text(GTK_ENTRY(input_montant_budg));
  replacechar(m, '.', ',');
  char *n = (char *)gtk_entry_get_text(GTK_ENTRY(input_nom_budg));

  insertBudg(m, n);
  gtk_entry_set_text(GTK_ENTRY(input_montant_budg), "");
  gtk_entry_set_text(GTK_ENTRY(input_nom_budg), "");
}

//GOOD
void newDepense(GtkWidget *button, gpointer data){
  printf("NEW DEPENSE\n");
  char *m = (char *)gtk_entry_get_text(GTK_ENTRY(input_dep));
  replacechar(m, '.', ',');
  char t[20];
  snprintf(t, sizeof(t), "%s", (char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box_dep)));

  // testRecu();

  if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_box_recu))){
    printf("DEPENSE RECURRENTE\n");
    insertDepenseRecu(m, t);
  }else{
    printf("DEPENSE\n");
    insertDepense(m, t);
  }

  gtk_entry_set_text(GTK_ENTRY(input_dep), "");
}

//GOOD
void vueDepenses(){
  GtkTreeIter iter;

  sqlite3_stmt *stmt;
  char request[60] = "select * from DEPENSES";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : vueDepenses\n");
    exit(-1);
  }
  gtk_list_store_clear(list_store_dep);

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    double montant = sqlite3_column_double(stmt, 1) * 100;
    montant = (float)((int) montant);
    montant = montant/100;
    char type[20];
    char date[20];
    snprintf(type, sizeof(type), "%s", (char *)sqlite3_column_text(stmt, 2));
    snprintf(date, sizeof(date), "%s", (char *)sqlite3_column_text(stmt, 3));
    gtk_list_store_append(list_store_dep, &iter);
    gtk_list_store_set(list_store_dep, &iter, 0, type, 1, montant, 2, date, -1);
  }
}

//GOOD
void vueBudgets(){
  GtkTreeIter iter;
  sqlite3_stmt *stmt;
  char request[60] = "select * from BUDGETS";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : vueBudgets\n");
    exit(-1);
  }
  gtk_list_store_clear(list_store_budg);
  gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo_box_dep));
  while (sqlite3_step(stmt) == SQLITE_ROW) {

    float montant = sqlite3_column_double(stmt, 1) ;
    montant = (float)((int) montant * 100) / 100;

    char type[20];
    snprintf(type, sizeof(type), "%s", (char *)sqlite3_column_text(stmt, 2));

    char montString[50];
    gcvt(getDepensesSumByType(type), 5, montString);
    strcat(montString, "/");

    char bufferMont[50];
    gcvt(montant, 5, bufferMont);
    strcat(montString, bufferMont);
    printf("%s\n", montString);

    int progress = (getDepensesSumByType(type) * 100) / montant;

    if(progress > 100)
      progress = 100;

    printf("%s : %f / %f  = %d\n",type, getDepensesSumByType(type), montant, progress );
    gtk_list_store_append(list_store_budg, &iter);
    gtk_list_store_set(list_store_budg, &iter, 0, type, 1, montString, 2, progress,   -1);

    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_dep), type);
  }
}

//GOOD
void  open_dialog_dep() {
  printf("OPEN DIALOG : ADD DEP \n");
  gtk_widget_show(dialog_dep);
}
//GOOD
void  hide_dialog_dep() {
  printf("HIDE DIALOG : ADD DEP \n");
  gtk_widget_hide(dialog_dep);
}

//GOOD
void  open_dialog_budg() {
  printf("OPEN DIALOG : ADD DEP \n");
  gtk_widget_show(dialog_budg);
}
//GOOD
void  hide_dialog_budg() {
  printf("HIDE DIALOG : ADD BUDG \n");
  gtk_widget_hide(dialog_budg);
}

//Creation de l'interface
void createWindow(int argc, char ** argv){
  GtkBuilder      *builder;

  gtk_init(&argc, &argv);

  builder = gtk_builder_new_from_file("glade1.glade");

  window = GTK_WIDGET(gtk_builder_get_object(builder, "window_main"));
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_builder_connect_signals(builder, NULL);

  bt_open_dialog_dep = GTK_WIDGET(gtk_builder_get_object(builder, "bt_dep"));
  bt_open_dialog_budg = GTK_WIDGET(gtk_builder_get_object(builder, "bt_budg"));

  dialog_dep = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_dep"));
  dialog_budg = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_budg"));

  combo_box_dep = GTK_WIDGET(gtk_builder_get_object(builder, "combo_box_dep"));
  input_dep = GTK_WIDGET(gtk_builder_get_object(builder, "entry_dep"));
  check_box_recu = GTK_WIDGET(gtk_builder_get_object(builder, "check_box_dep_recu"));
  bt_save_dep = GTK_WIDGET(gtk_builder_get_object(builder, "bt_save_dep"));
  list_store_dep = GTK_LIST_STORE(gtk_builder_get_object(builder, "historique_dep"));

  input_nom_budg = GTK_WIDGET(gtk_builder_get_object(builder, "entry_budg_nom"));
  input_montant_budg = GTK_WIDGET(gtk_builder_get_object(builder, "entry_budg_montant"));
  list_store_budg = GTK_LIST_STORE(gtk_builder_get_object(builder, "liste_budg"));


  vueDepenses();

  vueBudgets();

  // struct Input inpt_budg;
  // inpt_budg.m = input_montant_budg;
  // inpt_budg.t = input_nom_budg;
  // g_signal_connect(bt_new_budg, "clicked", G_CALLBACK(insertBudg), &inpt_budg);
  //
  // bt_save_dep = gtk_button_new_with_label("Nouvelle dépense");
  // check_box_recu = gtk_check_button_new_with_label ("Cette dépense est récurrente");
  // gtk_grid_attach(GTK_GRID(grid), check_box_recu, 0,8+getNbBudgets(),1,1);
  // // g_signal_connect(GTK_TOGGLE_BUTTON(check_box_recu), "toggled", G_CALLBACK(check_recu), &inpt_budg);
  //
  // GtkWidget *lab_dep = gtk_label_new("Depense:");
  // gtk_grid_attach(GTK_GRID(grid), lab_dep, 0,5+getNbBudgets(),1,1);
  //
  // input_dep = gtk_entry_new();
  // gtk_grid_attach(GTK_GRID(grid), input_dep, 0,6+getNbBudgets(),1,1);
  // gtk_entry_set_placeholder_text (GTK_ENTRY(input_dep), "Montant");
  //
  //
  // box_budg = gtk_box_new(TRUE, 0);
  //
  // for (int i = 0; i < getNbBudgets(); i++) {
  //   labelBudg[i] = gtk_label_new(displayBudgets(getBudget(i+1)));
  //   gtk_box_pack_start(GTK_BOX(box_budg), labelBudg[i], FALSE, FALSE, 0);
  //   gtk_label_set_use_markup(GTK_LABEL(labelBudg[i]), TRUE);
  //   gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_dep), getBudget(i+1).type);
  // }
  //
  // gtk_box_set_homogeneous (GTK_BOX(box_budg), TRUE);
  //
  // g_signal_connect(bt_save_dep, "clicked", G_CALLBACK(newDepense), NULL);
  // gtk_grid_attach(GTK_GRID(grid), bt_save_dep, 0,9+getNbBudgets(),1,1);
  //
  // gtk_combo_box_set_active (GTK_COMBO_BOX (combo_box_dep), 0);
  //
  // gtk_grid_attach(GTK_GRID(grid), combo_box_dep, 0,7+getNbBudgets(),1,1);
  //
  // box_all = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  //
  // gtk_box_pack_start(GTK_BOX(box_all), box_budg, FALSE, TRUE, 15);
  // gtk_box_pack_start(GTK_BOX(box_all), grid, FALSE, TRUE, 15);
  //
  //
  // scrolled_window = gtk_scrolled_window_new (NULL, NULL);
  // gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
  //                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  //
  // list = gtk_tree_view_new();
  //
  // gtk_container_add(GTK_CONTAINER(scrolled_window), list);
  // gtk_widget_show(list);
  //
  // gtk_box_pack_start(GTK_BOX(box_all), scrolled_window, TRUE, TRUE, 0);
  //
  //
  // init_list(list);//inti liste des depenses
  // insertAllDepenses();
  // paymentDepRecu();
  //
  // gtk_container_add(GTK_CONTAINER(window), box_all);
  // gtk_window_set_default_size(GTK_WINDOW (window), 900, 450);

  g_object_unref(builder);

  gtk_widget_show_all(window);
  gtk_main();
}

int main(int argc, char **argv){
  bddConnect(db);

  char jour[20];
  time_t time_raw_format;
  struct tm * ptr_time;
  time ( &time_raw_format );
  ptr_time = localtime ( &time_raw_format );
  strftime(jour ,50,"%d",ptr_time);
  printf("DATE : %s\n", jour);
  //si date egale premier du mois alors
  if (strcmp(jour, "12") == 0) {
    printf("PAYMENT TOUTES DEPENSES RECU !\n");
    paymentDepensesRecu();
  }
  createWindow(argc, argv);

}
