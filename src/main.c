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
GtkWidget *draw_area_suivi, *progress_bar_total;
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

void dessinerSuivi(GtkWidget *widget, cairo_t *cr, gpointer data){
  GdkRGBA color;

  GtkStyleContext *context;
    context = gtk_widget_get_style_context (widget);

  char colorsTab[11][8]={
    "#7CFC00",
    "#ED3811",
    "#8B20A8",
    "#F3F00A",
    "#0A94F3",
    "#F30AD3",
    "#F30A11",
    "#EFE9E9",
    "#2AFC08",
    "#15F3D7",
    "#4EE49E"
  };

  sqlite3_stmt *stmt;
  int b = 0;
  // char request[200] = "SELECT idDep, typeDep from DEPENSES group by typeDep";
  // if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
  //   printf("ERROR TO SELECT DATA : SUIVI\n");
  //   exit(-1);
  // }
  char request[200] = "SELECT idBudg, typeBudg from BUDGETS group by typeBudg order by idBudg";
  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : SUIVI\n");
    exit(-1);
  }

  /**
  Creation de l'interface du graphique: axes, graduations et legendes
  **/
  while (sqlite3_step(stmt) == SQLITE_ROW) {
    cairo_set_source_rgb (cr, 250, 250, 250);
    printf("VALUE: %d\n", (288 + ((b + 1) * 10)));
    if ((288 + ((b + 1) * 10)) < 400) {
      cairo_rectangle (cr, 10, 288 + (b++ * 10), 15, 15);
      cairo_move_to (cr, 30  , 290 + (b++ * 10));

    }else{
      cairo_rectangle (cr, 110, 288 + (b++ * 10), 15, 15);
      cairo_move_to (cr, 130, 290 + (b++ * 10));
    }
    cairo_set_font_size (cr, 12);
    cairo_show_text (cr, (char *)sqlite3_column_text(stmt, 1));


    gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt, 0) - 1]);
    gdk_cairo_set_source_rgba (cr, &color);
    cairo_fill (cr);

    cairo_stroke (cr);
  }


  cairo_set_source_rgba (cr, 200, 200, 200, 0.5);
  //abscisse
  cairo_move_to (cr, 0, 256);
  cairo_line_to (cr, 900, 256);
  //ordonnée
  cairo_move_to (cr, 40, 5);
  cairo_line_to (cr, 40, 270);

  //graduation de montant par 50€
  int graduation = 50;
  for (int l = 1; l < 12; l++) {
    cairo_move_to (cr, 35, 255 - graduation / 2 * l);
    cairo_line_to (cr, 40, 255 - graduation / 2 * l);
    char graduValeur[20];
    snprintf(graduValeur, sizeof(graduValeur), "%d", graduation * l);
    cairo_move_to (cr, 10  , 260 - graduation / 2 * l);
    cairo_set_font_size (cr, 12);
    cairo_show_text (cr, graduValeur);
  }

  sqlite3_stmt *stmt1;
  char request2[200] = "SELECT strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) from DEPENSES where idDep = 1 ";
  if (sqlite3_prepare_v2(db, request2, -1, &stmt1, NULL)) {
    printf("ERROR TO SELECT DATA : SUIVI\n");
    exit(-1);
  }

  sqlite3_step(stmt1);
  int h = 0;
  int colonnePrec = 0;

  for (int i = sqlite3_column_int(stmt1, 0); i < 12; i++) {
    char nums[10];
    char buffNum[3];
    if (i < 10) {
      strcat(nums, "0");
      snprintf(buffNum, sizeof(buffNum), "%d", i);
      strcat(nums, buffNum);
    }else{
      snprintf(nums, sizeof(nums), "%d", i);
    }

    sqlite3_stmt *stmt2;
    char request[200] = "SELECT typeBudg, sum(montantDep), dateDep, idBudg from BUDGETS INNER JOIN DEPENSES on typeBudg = typeDep where strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch') ) = '";

    strcat(request, nums);
    strcat(request,"' group by typeBudg");
    snprintf(nums, sizeof(nums), "%s", "");//on vide la var nums pour eviter que les chaines ne restent dedans

    printf("%s\n", request);
    if (sqlite3_prepare_v2(db, request, -1, &stmt2, NULL)) {
      printf("ERROR TO SELECT DATA : SUIVI\n");
      exit(-1);
    }

    int j = 0;

    while (sqlite3_step(stmt2) == SQLITE_ROW) {
      cairo_stroke (cr);

      double depType = sqlite3_column_double(stmt2, 1);
      cairo_rectangle (cr, j++ * 16 + 50 + (h * 40 + (colonnePrec * 15)), 255, 10,  -((depType + 1) / 2));

      gtk_style_context_get_color (context,
        gtk_style_context_get_state (context),
        &color);

      gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt2, 3) - 1]);
      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);
    }

    //Afficher graduations du mois ssi il y a des budget dans le mois
    if (j > 0) {
      int decalage;

      int numMois = i;
      char sMois[12];
      snprintf(sMois, sizeof(sMois), "%d", numMois);
      cairo_set_source_rgba (cr, 200, 200, 200, 0.5);

      //si nb budget pair plasser graduation du mois au millieu de deux budget si impair au centre du budget
      if (j % 2 == 0) {
        decalage = (j / 2) * 15 + 50 + (h * 40 + ( colonnePrec  * 15));
      }else{
        decalage = (j / 2) * 15 + 55 + (h * 40 + ( colonnePrec  * 15));
      }

      cairo_move_to (cr, decalage, 257);
      cairo_line_to (cr, decalage, 262);

      cairo_move_to (cr, decalage - 4, 275);
      cairo_set_font_size (cr, 12);
      cairo_show_text (cr, sMois);
      cairo_stroke (cr);
    }
    printf("MOIS %d : %d\n", i, j);
    colonnePrec += j;
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

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_box_recu))){
      printf("DEPENSE RECURRENTE\n");
      insertDepenseRecu(m, t);
    }else{
      printf("DEPENSE\n");
      insertDepense(m, t);
    }
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
      time_t rawtime = atol((char *)sqlite3_column_text(stmt, 3));
      struct tm  ts;
      char       buf[20];
      ts = *localtime(&rawtime);
      strftime(buf, sizeof(buf), "%d/%m/%Y", &ts);
      printf("%s\n", buf);

      snprintf(type, sizeof(type), "%s", (char *)sqlite3_column_text(stmt, 2));
      snprintf(date, sizeof(date), "%s", buf);
      gtk_list_store_append(list_store_dep, &iter);
      gtk_list_store_set(list_store_dep, &iter, 0, type, 1, montant, 2, date, -1);
    }
  }

  //GOOD
  void vueBudgets(){
    GtkTreeIter iter;
    sqlite3_stmt *stmt;
    float total_dep = 0;
    float total_budg = 0;

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
      // printf("%s\n", montString);

      total_dep += getDepensesSumByType(type);
      total_budg += montant;
      int progress = (getDepensesSumByType(type) * 100) / montant;

      if(progress > 100)
        progress = 100;

      // printf("%s : %f / %f  = %d\n",type, getDepensesSumByType(type), montant, progress );
      gtk_list_store_append(list_store_budg, &iter);
      gtk_list_store_set(list_store_budg, &iter, 0, type, 1, montString, 2, progress,   -1);

      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_dep), type);
    }
    printf("PROGRESS BAR: %f\n", (total_dep / total_budg) );
    gtk_progress_bar_set_fraction ((GtkProgressBar *)progress_bar_total, (total_dep / total_budg));

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
    printf("OPEN DIALOG : SUIVI \n");
    gtk_widget_show(dialog_suivi);
  }

  //GOOD
  void  hide_dialog_suivi() {
    printf("HIDE DIALOG : SUIVI \n");
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

    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                               GTK_STYLE_PROVIDER(cssProvider),
                               GTK_STYLE_PROVIDER_PRIORITY_USER);

   GtkWidget *titre_budg = GTK_WIDGET(gtk_builder_get_object(builder, "label_budg"));
   GtkWidget *titre_mes_budg = GTK_WIDGET(gtk_builder_get_object(builder, "label_mes_budg"));
   GtkWidget *titre_dep = GTK_WIDGET(gtk_builder_get_object(builder, "label_dep"));
   //CSS
   gtk_widget_set_name (titre_budg,"titre");
   gtk_widget_set_name (titre_mes_budg,"titre");
   gtk_widget_set_name (titre_dep,"titre");


    bt_open_dialog_dep = GTK_WIDGET(gtk_builder_get_object(builder, "bt_dep"));
    bt_open_dialog_budg = GTK_WIDGET(gtk_builder_get_object(builder, "bt_budg"));
    bt_open_dialog_suivie = GTK_WIDGET(gtk_builder_get_object(builder, "bt_suivi"));
    progress_bar_total = GTK_WIDGET(gtk_builder_get_object(builder, "progress_total"));
    //CSS
    gtk_widget_set_name(bt_open_dialog_dep, "bt_ok");
    gtk_widget_set_name(progress_bar_total, "progressbar_total");
    //CSS
    GtkWidget * bt_close_dep = GTK_WIDGET(gtk_builder_get_object(builder, "bt_close_dep"));
    GtkWidget * bt_close_budg = GTK_WIDGET(gtk_builder_get_object(builder, "bt_close_budg"));
    GtkWidget * bt_close_suivi = GTK_WIDGET(gtk_builder_get_object(builder, "bt_close_suivi"));
    gtk_widget_set_name (bt_close_dep,"bt_close");
    gtk_widget_set_name (bt_close_budg,"bt_close");
    gtk_widget_set_name (bt_close_suivi,"bt_close");

    //CSS
    GtkWidget * bt_save_dep = GTK_WIDGET(gtk_builder_get_object(builder, "bt_save_dep"));
    GtkWidget * bt_save_budg = GTK_WIDGET(gtk_builder_get_object(builder, "bt_save_budg"));
    gtk_widget_set_name (bt_save_dep,"bt_ok");
    gtk_widget_set_name (bt_save_budg, "bt_ok");

    dialog_dep = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_dep"));
    dialog_budg = GTK_WIDGET(gtk_builder_get_object(builder, "dialog_budg"));
    dialog_suivi = GTK_WIDGET(gtk_builder_get_object(builder, "window_suivi"));

    gtk_window_set_position(GTK_WINDOW(dialog_suivi), GTK_WIN_POS_CENTER);

    //CSS
    gtk_widget_set_name (bt_close_dep,"bt_close");
    gtk_widget_set_name (bt_close_budg,"bt_close");
    gtk_widget_set_name (bt_close_suivi,"bt_close");
    draw_area_suivi = GTK_WIDGET(gtk_builder_get_object(builder, "draw_suivi"));
    // g_signal_connect(G_OBJECT(draw_area_suivi), "expose_event", G_CALLBACK(dessinerSuivi), NULL);

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
    // printf("DATE : %s\n", jour);
    //si date egale premier du mois alors
    if (strcmp(jour, "13") == 0) {
      printf("PAYMENT TOUTES DEPENSES RECU !\n");
      paymentDepensesRecu();
    }
    createWindow(argc, argv);
  }
