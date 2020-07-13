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


GtkWidget *bt_open_dialog_dep, *bt_open_dialog_budg, *bt_open_dialog_suivie, *bt_new_budg, *bt_save_dep;
GtkWidget *input_dep, *combo_box_dep, *check_box_recu;
GtkWidget *input_nom_budg, *input_montant_budg;
GtkWidget *window, *grid;
GtkWidget *labelBudg[20];
GtkWidget *box_all, *box_budg;
GtkWidget *list, *store;
GtkWidget *scrolled_window;
GtkWidget *dialog_dep, *dialog_budg, *dialog_suivi;

GtkListStore *list_store_dep, *list_store_budg;
GtkTreeViewColumn *cx1, *cx2, *cx3, *cx4, *cx5, *cx6;
GtkCellRenderer *cr1, *cr2, *cr3, *cr4, *cr5, *cr6;

sqlite3 *db;
int nbBudgets;


/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;


//GOOD
void bddConnect(){
  if (sqlite3_open("the.db", &db)) {
    printf("Could not open the.db\n");
    exit(-1);
  }

  if (sqlite3_exec(db, "create table DEPENSES (idDep integer primary key autoincrement, montantDep flaot(7,2) not null, typeDep varchar(20), dateDep date default (strftime('%s', 'now')))", NULL, NULL, NULL)) {
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


  if (sqlite3_exec(db, "create table DEPENSESRECURRENTE (idDepRecu integer primary key autoincrement, montantDepRecu int not null, typeDepRecu varchar(20), dateDepRecu date default (STRFTIME('%d/%m/%Y', 'NOW','localtime')))", NULL, NULL, NULL)) {
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


static void clear_surface (void){
cairo_t *cr;
cr = cairo_create (surface);
cairo_set_source_rgb (cr, 1, 1, 1);
cairo_paint (cr);
cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
void configure_event_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
if (surface)
cairo_surface_destroy (surface);

surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
CAIRO_CONTENT_COLOR,
gtk_widget_get_allocated_width (widget),
gtk_widget_get_allocated_height (widget));

/* Initialize the surface to white */
clear_surface ();
}


void dessinerSuiviProto(GtkWidget *widget, cairo_t *cr, gpointer data){

sqlite3_stmt *stmt;
char request[60] = "select * from budgets";


if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
  printf("ERROR TO SELECT DATA : getBudget\n");
  exit(-1);
}


guint width, height;
GdkRGBA color;
GtkStyleContext *context;

context = gtk_widget_get_style_context (widget);

width = gtk_widget_get_allocated_width (widget);
height = gtk_widget_get_allocated_height (widget);


int i = 0;
while(sqlite3_step(stmt) == SQLITE_ROW){
  double depType = getDepensesSumByType((char *)sqlite3_column_text(stmt, 2));
  printf("%s = %f\n",sqlite3_column_text(stmt, 2), depType);

  cairo_rectangle (cr, 10 + ( i++ * 50), 250, 40, 1 - (depType / 2));

  gtk_style_context_get_color (context,
    gtk_style_context_get_state (context),
    &color);
    color.red = 200;
    gdk_cairo_set_source_rgba (cr, &color);
    cairo_fill (cr);
  }
}

void dessinerSuivi(GtkWidget *widget, cairo_t *cr, gpointer data){

  sqlite3_stmt *stmt;
  char request[200] = "SELECT strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch') ) from DEPENSES where idDep = 1 ";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : SUIVI\n");
    exit(-1);
  }

  sqlite3_step(stmt);
    int h = 0;


  for (int i = sqlite3_column_int(stmt, 0); i < 12; i++) {
    char nums[10];
    snprintf(nums, sizeof(nums), "%d", i);

    sqlite3_stmt *stmt2;
    char request[200] = "SELECT typeBudg, sum(montantDep), dateDep from BUDGETS INNER JOIN DEPENSES on typeBudg = typeDep where strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch') ) = '0";
    strcat(request, nums);
    strcat(request,"' group by typeBudg");

    // printf("%s\n", request);
    if (sqlite3_prepare_v2(db, request, -1, &stmt2, NULL)) {
      printf("ERROR TO SELECT DATA : SUIVI\n");
      exit(-1);
    }

    guint width, height;
    GdkRGBA color;
    GtkStyleContext *context;

    context = gtk_widget_get_style_context (widget);

    width = gtk_widget_get_allocated_width (widget);
    height = gtk_widget_get_allocated_height (widget);

    int j = 0;

    while (sqlite3_step(stmt2) == SQLITE_ROW) {

      // printf("SQL ROW : %d %d\n", SQLITE_ROW, sqlite3_step(stmt2));
        double depType = sqlite3_column_double(stmt2, 1);
        printf("%s = %f\n",sqlite3_column_text(stmt2, 0), depType);
printf("H %d\n", h);
        cairo_rectangle (cr, 10 + (j++ * 15) + (h * 50) , 250, 10, 1 - (depType / 2));

        gtk_style_context_get_color (context,
                                     gtk_style_context_get_state (context),
                                     &color);
        color.red = 200;
        gdk_cairo_set_source_rgba (cr, &color);
        cairo_fill (cr);
    }

    h++;
  }
}


  //GOOD
  void newBudget(GtkWidget *button, gpointer data){
    printf("NEW BUDGET\n");

    char *m = (char *)gtk_entry_get_text(GTK_ENTRY(input_montant_budg));
    replacechar(m, '.', ',');
    char *n = (char *)gtk_entry_get_text(GTK_ENTRY(input_nom_budg));

    insertBudg(m, n);
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
    // gtk_entry_set_text(GTK_ENTRY(input_dep), "");
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

  //GOOD
  void  open_dialog_suivi() {
    printf("OPEN DIALOG : ADD SUIVI \n");
    gtk_widget_show(dialog_suivi);
  }

  //GOOD
  void  hide_dialog_suivi() {
    printf("HIDE DIALOG : ADD BUDG \n");
    gtk_widget_hide(dialog_suivi);
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
    bt_open_dialog_suivie = GTK_WIDGET(gtk_builder_get_object(builder, "bt_suivi"));

    dialog_dep = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_dep"));
    dialog_budg = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_budg"));
    dialog_suivi = GTK_WIDGET(gtk_builder_get_object(builder, "window_suivi"));
    gtk_window_set_position(GTK_WINDOW(dialog_suivi), GTK_WIN_POS_CENTER);

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
    if (strcmp(jour, "13") == 0) {
      printf("PAYMENT TOUTES DEPENSES RECU !\n");
      paymentDepensesRecu();
    }
    createWindow(argc, argv);
  }
