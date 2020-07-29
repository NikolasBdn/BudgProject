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
GtkCellRendererToggle *toggle; 

GtkCellRenderer *cell;

sqlite3 *db;

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

// static char **colors;

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;

//GOOD
void bddConnect(char *nomBdd){
  printf("BDD: %s\n", nomBdd);
  if (sqlite3_open(nomBdd, &db)) {
    printf("Could not open the.db\n");
    exit(-1);
  }

  if (sqlite3_exec(db, "create table BUDGETS (idBudg integer primary key autoincrement, montantBudg int not null, typeBudg varchar(20))", NULL, NULL, NULL)) {
      printf("Error executing sql statement\n");
  }
  else {
    printf("Table BUDGETS created\n");
  }

  if (sqlite3_exec(db, "create table DEPENSES (idDep integer primary key autoincrement, montantDep flaot(7,2) not null, idType integer, dateDep date default (strftime('%s', 'now')), FOREIGN KEY(idType) REFERENCES budgets(idBudg))", NULL, NULL, NULL)) {
    printf("Error executing sql statement\n");
  }
  else {
    printf("Table DEPENSES created\n");
  }

 
  if (sqlite3_exec(db, "create table DEPENSESRECURRENTE (idDepRecu integer primary key autoincrement, montantDepRecu int not null, idType integer, dernierPaiement varchar(10), FOREIGN KEY(idType) REFERENCES budgets(idBudg))", NULL, NULL, NULL)) {
    printf("Error executing sql statement\n");
  }
  else {
    printf("Table DEPENSE RECURRENTE created\n");
  }
}

void clear_surface (void){
  cairo_t *cr;
  cr = cairo_create (surface);
  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);
  cairo_destroy (cr);
}

//GOOD
void configure_event_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer data){
  if (surface)
  cairo_surface_destroy (surface);
  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
  CAIRO_CONTENT_COLOR,
  gtk_widget_get_allocated_width (widget),
  gtk_widget_get_allocated_height (widget));
  clear_surface ();
}

void genRandomColor(){
  char *alphanum = "0123456789ABCDEF";
  char **tabBudgColor = malloc (sizeof (char *) * 10);
  char s[7];
  int nbBudg;

  sqlite3_stmt *stmt;
  char request[200] = "SELECT count(*) from BUDGETS";

  if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
    printf("ERROR TO SELECT DATA : BUDGETS RANDOM COLOR\n");
    exit(-1);
  }
  sqlite3_step(stmt);
  nbBudg = sqlite3_column_int(stmt, 0);

  for (int i = 0; i < nbBudg; ++i){
    tabBudgColor[i] = malloc (7);
    s[0] = '#';
   for (int j = 1; j < 7; ++j) {
      s[j] = alphanum[rand() % 16];
    }
    // s[7]='/0';  
   strcpy(tabBudgColor[i], "#FF0000");//put s in tabBudgColor
  }
  
  // colors = tabBudgColor;
} 

void faireInterfaceSuivi(cairo_t *cr){
  GdkRGBA color;
  sqlite3_stmt *stmt;
  int b = 0;

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
    if ((288 + ((b + 1) * 10)) < 400) {
      cairo_rectangle (cr, 15, 298 + (b++ * 12), 15, 15);
      gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt, 0) - 1]);
      color.alpha = 0.5;
      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);

      cairo_rectangle (cr, 12, 290 + (b++ * 12), 15, 15);
      cairo_move_to (cr, 40, 290 + (b++ * 12));
    }else{
      cairo_rectangle (cr, 125, 150 + (b++ * 12), 15, 15);
      gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt, 0) - 1]);
      color.alpha = 0.5;
      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);

      cairo_rectangle (cr, 123, 145 + (b++ * 12), 15, 15);
      cairo_move_to (cr, 150, 145 + (b++ * 12));
    }
    cairo_set_source_rgba (cr, 200, 200, 200, 0.7);
    cairo_set_font_size (cr, 12);
    cairo_show_text (cr, (char *)sqlite3_column_text(stmt, 1));
    gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt, 0) - 1]);
    gdk_cairo_set_source_rgba (cr, &color);
    cairo_fill (cr);

    cairo_stroke (cr);
  }

  //information
  cairo_set_source_rgba (cr, 250, 250, 250, 1);
  cairo_rectangle (cr, 590, 300, 15, 15);
  cairo_fill (cr);

  cairo_move_to (cr, 610, 310);
  cairo_set_source_rgba (cr, 200, 200, 200, 0.7);
  cairo_set_font_size (cr, 12);
  cairo_show_text (cr, "Dépenses total dans un budget");

  cairo_set_source_rgba (cr, 250, 250, 250, 0.7);
  cairo_rectangle (cr, 590, 330, 15, 15);
  cairo_fill (cr);

  cairo_move_to (cr, 610, 340);
  cairo_set_source_rgba (cr, 200, 200, 200, 0.7);
  cairo_set_font_size (cr, 12);
  cairo_show_text (cr, "Montant total alloué à un budget");

  cairo_set_source_rgba (cr, 250, 250, 250, 0.4);
  cairo_rectangle (cr, 590, 360, 15, 15);
  cairo_fill (cr);

  cairo_move_to (cr, 610, 370);
  cairo_set_source_rgba (cr, 200, 200, 200, 0.7);
  cairo_set_font_size (cr, 12);
  cairo_show_text (cr, "Dépense(s) récurrente(s) prévue(s) dans un budget");

  cairo_stroke (cr);

  //graduation de montant par 50€
  int graduation = 50;
  for (int l = 1; l < 12; l++) {
    cairo_set_source_rgba (cr, 200, 200, 200, 0.5);
    cairo_move_to (cr, 35, 255 - graduation / 2 * l);
    cairo_line_to (cr, 40, 255 - graduation / 2 * l);
    char graduValeur[20];
    snprintf(graduValeur, sizeof(graduValeur), "%d", graduation * l);
    cairo_move_to (cr, 10  , 260 - graduation / 2 * l);
    cairo_set_font_size (cr, 12);
    cairo_show_text (cr, graduValeur);
  }

  cairo_set_source_rgba (cr, 200, 200, 200, 0.5);
  //abscisse
  cairo_move_to (cr, 0, 256);
  cairo_line_to (cr, 1600, 256);
  //ordonnée
  cairo_move_to (cr, 40, 5);
  cairo_line_to (cr, 40, 270);
  cairo_stroke (cr);
}

void affichageRecurentes(cairo_t *cr, int moisCourant, int nbMois, int nbBudgAffiche){
  GdkRGBA color;

  for (int i = moisCourant +1; i <= 12; i++) {
    nbMois++;
    sqlite3_stmt *stmt;
    char request[200] = "SELECT sum(montantDepRecu) ,idBudg from DEPENSESRECURRENTE INNER JOIN BUDGETS on idType = idBudg group by typeBudg";

    if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
      printf("ERROR TO SELECT DATA : DEPENSESRECURRENTE SUIVI\n");
      exit(-1);
    }
    int count = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {

      cairo_rectangle (cr, count++ * 20 + 52+ (nbMois * 45) + (nbBudgAffiche * 20) , 255, 14,  -(int)sqlite3_column_int(stmt, 0)/ 2);
      gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt, 1) - 1]);
      color.alpha = 0.3;

      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);
    }
    //Afficher graduations du mois ssi il y a des budget dans le mois
    if (count > 0) {
      int decalage;
      int numMois = i;
      char sMois[12];
      snprintf(sMois, sizeof(sMois), "%d", numMois);
      cairo_set_source_rgba (cr, 200, 200, 200, 0.5);

      //si nb budget pair plasser graduation du mois au millieu de deux budget si impair au centre du budget
      if (count % 2 == 0) {
        decalage = (count / 2) * 20 + 50 + (nbMois * 45 ) + ( nbBudgAffiche  * 20);
      }else{
        decalage = (count / 2) * 20 + 55 + (nbMois * 45) + ( nbBudgAffiche * 20);
      }

      cairo_move_to (cr, decalage, 257);
      cairo_line_to (cr, decalage, 262);

      cairo_move_to (cr, decalage - 4, 275);
      cairo_set_font_size (cr, 12);
      cairo_show_text (cr, sMois);
      cairo_stroke (cr);
    }
    nbBudgAffiche += count;
    moisCourant++;
  }
}

void dessinerSuivi(GtkWidget *widget, cairo_t *cr, gpointer data){
  GdkRGBA color;
  // genRandomColor();
  

  GtkStyleContext *context;
  context = gtk_widget_get_style_context (widget);

  faireInterfaceSuivi(cr);//Ceration axes, graduations et légendes

  sqlite3_stmt *stmt1;//recuperation premiere depense
  char request2[200] = "SELECT strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch')) from DEPENSES where idDep = 1 ";
  if (sqlite3_prepare_v2(db, request2, -1, &stmt1, NULL)) {
    printf("ERROR TO SELECT DATA : SUIVI\n");
    exit(-1);
  }

  sqlite3_step(stmt1);
  int moisCourant = 0;
  int nbBudgAfficherTotal = 0;

  //Pour chaque Mois
  for (int i = getDatePremiereDep(); i <= 12; i++) {
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
    char request[200] = "SELECT sum(montantDep), idBudg, montantBudg from BUDGETS INNER JOIN DEPENSES on idBudg = idType where strftime('%m', DATETIME(ROUND(dateDep), 'unixepoch') ) = '";

    strcat(request, nums);
    strcat(request,"' group by typeBudg");
    snprintf(nums, sizeof(nums), "%s", "");//on vide la var nums pour eviter que les chaines ne restent dedans

    if (sqlite3_prepare_v2(db, request, -1, &stmt2, NULL)) {
      printf("ERROR TO SELECT DATA : SUIVI\n");
      exit(-1);
    }

    int nbBudg = 0;
    while (sqlite3_step(stmt2) == SQLITE_ROW) {
      int budgMontant = (int)sqlite3_column_double(stmt2, 2);
      double depType = sqlite3_column_double(stmt2, 0);
      cairo_rectangle (cr, nbBudg++ * 22 + 50 + (moisCourant * 40) + (nbBudgAfficherTotal * 20), 255, 12,  -((depType + 1) / 2));

      gtk_style_context_get_color (context,
        gtk_style_context_get_state (context),
        &color);

      gdk_rgba_parse (&color, colorsTab [sqlite3_column_int(stmt2 , 1) - 1]);
      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);
      cairo_rectangle (cr, (nbBudg - 1) * 22 + 52 + (moisCourant * 40) + (nbBudgAfficherTotal * 20), 255, 16,  -((budgMontant + 1) / 2));
      gdk_rgba_parse (&color, colorsTab[sqlite3_column_int(stmt2 , 1) - 1]);
      color.alpha = 0.6;
      gdk_cairo_set_source_rgba (cr, &color);
      cairo_fill (cr);
    }

    //Afficher graduations du mois ssi il y a des budget dans le mois
    if (nbBudg > 0) {
      int decalage;

      int numMois = i;
      char sMois[12];
      snprintf(sMois, sizeof(sMois), "%d", numMois);
      cairo_set_source_rgba (cr, 200, 200, 200, 0.5);

      //si nb budget pair plasser graduation du mois au millieu de deux budget si impair au centre du budget
      if (nbBudg % 2 == 0) {
        decalage = (nbBudg / 2) * 20 + 50 + (moisCourant * 40) + ( nbBudgAfficherTotal  * 20);
      }else{
        decalage = (nbBudg / 2) * 20 + 55 + (moisCourant * 40) + ( nbBudgAfficherTotal  * 20);
      }

      cairo_move_to (cr, decalage, 257);
      cairo_line_to (cr, decalage, 262);
      cairo_move_to (cr, decalage - 4, 275);
      cairo_set_font_size (cr, 12);
      cairo_show_text (cr, sMois);
      cairo_stroke (cr);
    }
    nbBudgAfficherTotal += nbBudg;
    moisCourant++;
  }
  affichageRecurentes(cr, getDateDerniereDep(), getNbMois(), nbBudgAfficherTotal);
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

    char *m = (char *)gtk_entry_get_text(GTK_ENTRY(input_dep));
    replacechar(m, '.', ',');
    char t[20];
    snprintf(t, sizeof(t), "%s", (char *)gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_box_dep)));

    sqlite3_stmt *stmt;
    char buffId[30];

    char request[60] = "select idBudg from BUDGETS where typeBudg = '";
    strcat(request, t);
    strcat(request, "'");

    if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
      printf("ERROR TO SELECT DATA : vueDepenses\n");
      exit(-1);
    }

    sqlite3_step(stmt);

    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_box_recu))){
      printf("DEPENSE RECURRENTE\n");
      insertDepenseRecu(m, (int)sqlite3_column_int(stmt, 0));
    }else{
      printf("NEW DEPENSE\n");           
      insertDepense(m, (int)sqlite3_column_int(stmt, 0));
    }
  }

  //GOOD
  void vueDepenses(){
    GtkTreeIter iter;
    sqlite3_stmt *stmt;

    char request[80] = "select * from DEPENSES INNER JOIN BUDGETS on idType = idBudg order by dateDep";

    if (sqlite3_prepare_v2(db, request, -1, &stmt, NULL)) {
      printf("ERROR TO SELECT DATA : vueDepenses\n");
      exit(-1);
    }
    gtk_list_store_clear(list_store_dep);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
      int id = sqlite3_column_int(stmt, 0);
      double montant = sqlite3_column_double(stmt, 1);

      char type[20];
      snprintf(type, sizeof(type), "%s", (char *)sqlite3_column_text(stmt, 6));

      char date[20];

      time_t rawtime = atol((char *)sqlite3_column_text(stmt, 3));
      struct tm  ts;
      char       buf[20];
      ts = *localtime(&rawtime);
      strftime(buf, sizeof(buf), "%d/%m/%Y", &ts);
      snprintf(date, sizeof(date), "%s", buf);

      gtk_list_store_append(list_store_dep, &iter);
      gtk_list_store_set(list_store_dep, &iter, 0, type, 1, montant, 2, date, 3, FALSE, 4, id, -1);
    }
  }

  //GOOD Affichage des differents budgets en la Treeview
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
      int id = sqlite3_column_int(stmt, 0);

      float montant = sqlite3_column_double(stmt, 1) ;
      montant = (float)((int) montant * 100) / 100;

      int idBudg = sqlite3_column_int(stmt, 0);
      // snprintf(type, sizeof(type), "%s", (char *)sqlite3_column_text(stmt, 2));

      char montString[50];
      gcvt(getDepensesSumByType(idBudg), 5, montString);
      strcat(montString, "/");

      char bufferMont[50];
      gcvt(montant, 5, bufferMont);
      strcat(montString, bufferMont);
      // printf("%s\n", montString);

      total_dep += getDepensesSumByType(idBudg);
      total_budg += montant;
      int progress = (getDepensesSumByType(idBudg) * 100) / montant;
      char *type = (char *)sqlite3_column_text(stmt, 2);

      if(progress > 100)
        progress = 100;
      gtk_list_store_append(list_store_budg, &iter);
      gtk_list_store_set(list_store_budg, &iter, 0, type, 1, montString, 2, progress, 3, FALSE, 4, id, -1);

      gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_box_dep), type);
    }
    if((total_dep / total_budg) <= 0){
      gtk_progress_bar_set_fraction ((GtkProgressBar *)progress_bar_total, 0);
    }else
    {
      gtk_progress_bar_set_fraction ((GtkProgressBar *)progress_bar_total, (total_dep / total_budg));
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
    printf("OPEN DIALOG : SUIVI \n");
    gtk_widget_show(dialog_suivi);
  }

  //GOOD
  void  hide_dialog_suivi() {
    printf("HIDE DIALOG : SUIVI \n");
    gtk_widget_hide(dialog_suivi);
  }

  void testToggled(GtkCellRendererToggle *cell, gchar * path, GtkListStore * model){
    GtkTreeIter iter;
    gboolean active;

    active = gtk_cell_renderer_toggle_get_active (cell);

    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path);
// printf("PATH: %s\n", path);
    if (active) {
        gtk_list_store_set (model, &iter, 3, FALSE, -1);
    }
    else {
        gtk_list_store_set (model, &iter, 3, TRUE, -1);
    }
  }

  void supprRowDep(){
    GtkTreeIter iter;
    gint toggle;
    int idDepense;
    int tabSuppr[50];
    int count = 0;

    gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store_dep), &iter);
    gtk_tree_model_get (GTK_TREE_MODEL(list_store_dep), &iter, 3, &toggle, 4, &idDepense, -1);
    
   if (toggle){
      tabSuppr[count++] = idDepense;
    }

    while(gtk_tree_model_iter_next (GTK_TREE_MODEL(list_store_dep), &iter)){
      gtk_tree_model_get (GTK_TREE_MODEL(list_store_dep), &iter, 3, &toggle, 4, &idDepense, -1);
      if (toggle){
        tabSuppr[count++] = idDepense;
      }
    }

    for (int i = 0; i < count; ++i)
    { 
      deleteDepense(tabSuppr[i]);
    }
  }


  void supprRowBudg(){
    GtkTreeIter iter;
    gint toggle;
    int idBudget;
    int tabSuppr[50];
    int count = 0;

    gtk_tree_model_get_iter_first (GTK_TREE_MODEL(list_store_budg), &iter);
    gtk_tree_model_get (GTK_TREE_MODEL(list_store_budg), &iter, 3, &toggle, 4, &idBudget, -1);
  
   if (toggle){
      tabSuppr[count++] = idBudget;
    }

    while(gtk_tree_model_iter_next (GTK_TREE_MODEL(list_store_budg), &iter)){
      gtk_tree_model_get (GTK_TREE_MODEL(list_store_budg), &iter, 3, &toggle, 4, &idBudget, -1);
      if (toggle){
        tabSuppr[count++] = idBudget;
      }
    }
    printf("%ls\n", tabSuppr);
    for (int i = 0; i < count ; ++i)
    { 
      deleteBudg(tabSuppr[i]);
    }
  }

  void suppr(){
    supprRowDep();
    supprRowBudg();

    vueBudgets();
    vueDepenses();
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

   // GtkWidget *treeview_dep = GTK_WIDGET(gtk_builder_get_object(builder, "treeview_dep"));

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
    GtkWidget * bt_suppr = GTK_WIDGET(gtk_builder_get_object(builder, "bt_suppr"));
    gtk_widget_set_name (bt_close_dep,"bt_close");
    gtk_widget_set_name (bt_close_budg,"bt_close");
    gtk_widget_set_name (bt_close_suivi,"bt_close");
    gtk_widget_set_name (bt_suppr,"bt_close");

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

    // toggle = GTK_CELL_RENDERER_TOGGLE(gtk_builder_get_object (builder, "cr7"));
    // gtk_cell_renderer_toggle_set_activatable (toggle, TRUE);
    GtkTreeViewColumn *columnProgress = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx6"));
    gtk_tree_view_column_set_expand (columnProgress, TRUE);

    GtkTreeViewColumn *columnSupprDep = GTK_TREE_VIEW_COLUMN(gtk_builder_get_object(builder, "cx9"));
    gtk_tree_view_column_set_expand (columnSupprDep, TRUE);

    vueDepenses();

    vueBudgets();

    g_object_unref(builder);
    paymentDepensesRecu();

    gtk_widget_show_all(window);
    gtk_main();
  }

  int main(int argc, char **argv){
    bddConnect(argv[1]);
    createWindow(argc, argv);
    exit(0);
  }
