#include <gtk/gtk.h>


void appendValues(GtkListStore *list){
    GtkTreeIter gti;

    gtk_list_store_append (list, &gti);
    gtk_list_store_set (list, &gti, 0, TRUE, -1);
    }

GtkListStore *
create_model ()
{
    // GtkTreeIter gti;
    GtkListStore *gls;

    gls = gtk_list_store_new (1, G_TYPE_BOOLEAN);

    return gls;
}

void
check (GtkCellRendererToggle * cell, gchar * path, GtkListStore * model)
{
    GtkTreeIter iter;
    gboolean active;

    active = gtk_cell_renderer_toggle_get_active (cell);
printf("%d\n", active);
    gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (model), &iter, path);

    if (active) {
        // gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(cell), 0, 0);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, FALSE, -1);
    }
    else {
        // gtk_cell_renderer_set_alignment(GTK_CELL_RENDERER(cell), 0.5, 0.5);
        gtk_list_store_set (GTK_LIST_STORE (model), &iter, 0, TRUE, -1);
    }
}



int
main (int argc, char *argv[])
{

    GtkTreeViewColumn *column1;
    GtkWidget *window_main;
    GtkWidget *treeview;
    GtkCellRenderer *cell;
    GtkListStore *store;


    gtk_init (&argc, &argv);
   
        store = create_model ();
    
    //window
    window_main = gtk_window_new (GTK_WINDOW_TOPLEVEL);

    //tree view
    treeview = gtk_tree_view_new_with_model (GTK_TREE_MODEL (store));

    //cell_renderer_toggle
    cell = gtk_cell_renderer_toggle_new ();

    column1 =
        gtk_tree_view_column_new_with_attributes ("test",
                                                  cell, "active", 0, NULL);

    gtk_tree_view_append_column (GTK_TREE_VIEW (treeview), column1); 

    g_signal_connect (cell, "toggled", G_CALLBACK (check), store);

    for (int i = 0; i < 3; ++i)
    {
        appendValues(store);
    }

    g_signal_connect (window_main, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);

    gtk_window_set_default_size (GTK_WINDOW(window_main),
                                 300,
                                 200);
    gtk_container_add (GTK_CONTAINER (window_main), treeview);
    gtk_widget_show_all (window_main);
    gtk_main ();

    return 0;
}