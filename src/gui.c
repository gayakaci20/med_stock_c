#include "../include/gui.h"
#include "../include/database.h"
#include <gtk/gtk.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <time.h>
#include "../include/gemini_api.h"

extern sqlite3 *db;  

static GtkWidget *main_window;
static GtkWidget *medication_list;
static GtkWidget *supplier_list;
static GtkWidget *notebook;  
static gboolean on_button_pressed(GtkWidget *treeview, GdkEventButton *event, gpointer user_data);

static void setup_medication_list(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    store = gtk_list_store_new(5, G_TYPE_INT, G_TYPE_STRING, G_TYPE_INT, G_TYPE_DOUBLE, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(medication_list), GTK_TREE_MODEL(store));
    g_object_unref(store);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(medication_list), column);

    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(medication_list), column);

    column = gtk_tree_view_column_new_with_attributes("Quantité", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(medication_list), column);

    column = gtk_tree_view_column_new_with_attributes("Prix", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(medication_list), column);

    column = gtk_tree_view_column_new_with_attributes("Date Expiration", renderer, "text", 4, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(medication_list), column);
}
static void show_medication_popup_menu(GtkWidget *treeview, GdkEventButton *event) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Modifier");
    GtkWidget *delete_item = gtk_menu_item_new_with_label("Supprimer");

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), edit_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_item);

    g_signal_connect(edit_item, "activate", G_CALLBACK(show_edit_medication_dialog), main_window);
    g_signal_connect(delete_item, "activate", G_CALLBACK(delete_selected_medication), NULL);

    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
}

static void setup_supplier_list(void) {
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    store = gtk_list_store_new(4, G_TYPE_INT, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(supplier_list), GTK_TREE_MODEL(store));
    g_object_unref(store);

    renderer = gtk_cell_renderer_text_new();
    column = gtk_tree_view_column_new_with_attributes("ID", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(supplier_list), column);

    column = gtk_tree_view_column_new_with_attributes("Nom", renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(supplier_list), column);

    column = gtk_tree_view_column_new_with_attributes("Contact", renderer, "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(supplier_list), column);

    column = gtk_tree_view_column_new_with_attributes("Adresse", renderer, "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(supplier_list), column);
}
static void show_supplier_popup_menu(GtkWidget *treeview, GdkEventButton *event) {
    GtkWidget *menu = gtk_menu_new();
    GtkWidget *edit_item = gtk_menu_item_new_with_label("Modifier");
    GtkWidget *delete_item = gtk_menu_item_new_with_label("Supprimer");

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), edit_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), delete_item);

    g_signal_connect(edit_item, "activate", G_CALLBACK(show_edit_supplier_dialog), main_window);
    g_signal_connect(delete_item, "activate", G_CALLBACK(delete_selected_supplier), NULL);

    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), (GdkEvent*)event);
}

void refresh_medication_list(void) {
    sqlite3_stmt *stmt;
    GtkListStore *store;
    const char *query = "SELECT * FROM Medicaments;";

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(medication_list)));
    gtk_list_store_clear(store);

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            GtkTreeIter iter;
            char price_str[32];
            snprintf(price_str, sizeof(price_str), "%.2f", sqlite3_column_double(stmt, 3));
            
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                             0, sqlite3_column_int(stmt, 0),
                             1, sqlite3_column_text(stmt, 1),
                             2, sqlite3_column_int(stmt, 2),
                             3, atof(price_str),  // Utilise la chaîne formatée
                             4, sqlite3_column_text(stmt, 4),
                             -1);
        }
        sqlite3_finalize(stmt);
    }
}

void create_main_window(void) {
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Gestionnaire de Stock Médical");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 800, 600);
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(main_window), main_box);

    GtkWidget *title_label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title_label), "<b>Gestionnaire de Stock Médical</b>");
    gtk_box_pack_start(GTK_BOX(main_box), title_label, FALSE, FALSE, 10);

    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start(GTK_BOX(main_box), toolbar, FALSE, FALSE, 0);

    GtkWidget *search_entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(search_entry), "Rechercher par nom...");
    GtkToolItem *search_item = gtk_tool_item_new();
    gtk_container_add(GTK_CONTAINER(search_item), search_entry);
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), search_item, -1);

    GtkToolItem *btn_search_simple = gtk_tool_button_new(NULL, "Rechercher");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_search_simple, -1);
    g_signal_connect(btn_search_simple, "clicked", G_CALLBACK(perform_simple_search), search_entry);

    GtkToolItem *separator0 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator0, -1);

    GtkToolItem *btn_add_med = gtk_tool_button_new(NULL, "Ajouter Médicament");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_add_med, -1);
    g_signal_connect(btn_add_med, "clicked", G_CALLBACK(show_add_medication_dialog), main_window);

    GtkToolItem *btn_add_sup = gtk_tool_button_new(NULL, "Ajouter Fournisseur");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_add_sup, -1);
    g_signal_connect(btn_add_sup, "clicked", G_CALLBACK(show_add_supplier_dialog), main_window);

    GtkToolItem *separator1 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator1, -1);

    GtkToolItem *btn_export_med = gtk_tool_button_new(NULL, "Exporter Médicaments");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_export_med, -1);
    g_signal_connect(btn_export_med, "clicked", G_CALLBACK(export_medications_dialog), main_window);

    GtkToolItem *btn_export_sup = gtk_tool_button_new(NULL, "Exporter Fournisseurs");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_export_sup, -1);
    g_signal_connect(btn_export_sup, "clicked", G_CALLBACK(export_suppliers_dialog), main_window);

    GtkToolItem *separator2 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator2, -1);

    GtkToolItem *btn_search = gtk_tool_button_new(NULL, "Recherche Avancée");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_search, -1);
    g_signal_connect(btn_search, "clicked", G_CALLBACK(show_advanced_search_dialog), main_window);

    // Ajout des nouveaux boutons
    GtkToolItem *separator3 = gtk_separator_tool_item_new();
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), separator3, -1);

    GtkToolItem *btn_generate = gtk_tool_button_new(NULL, "Générer avec IA");
    gtk_toolbar_insert(GTK_TOOLBAR(toolbar), btn_generate, -1);
    g_signal_connect(btn_generate, "clicked", G_CALLBACK(show_generate_dialog), main_window);

    // Création du notebook (onglets)
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(main_box), notebook, TRUE, TRUE, 0);

    // Create medication page
    GtkWidget *med_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    medication_list = gtk_tree_view_new();
    setup_medication_list();  
    GtkWidget *med_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(med_scroll), medication_list);
    gtk_box_pack_start(GTK_BOX(med_page), med_scroll, TRUE, TRUE, 0);

    // Create medication buttons
    GtkWidget *med_button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(med_page), med_button_box, FALSE, FALSE, 5);

    GtkWidget *edit_med_button = gtk_button_new_with_label("Modifier");
    GtkWidget *delete_med_button = gtk_button_new_with_label("Supprimer");
    gtk_container_add(GTK_CONTAINER(med_button_box), edit_med_button);
    gtk_container_add(GTK_CONTAINER(med_button_box), delete_med_button);

    g_signal_connect(edit_med_button, "clicked", G_CALLBACK(show_edit_medication_dialog), main_window);
    g_signal_connect(delete_med_button, "clicked", G_CALLBACK(delete_selected_medication), NULL);

    // Create supplier page
    GtkWidget *sup_page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    supplier_list = gtk_tree_view_new();
    setup_supplier_list();  
    GtkWidget *sup_scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(sup_scroll), supplier_list);
    gtk_box_pack_start(GTK_BOX(sup_page), sup_scroll, TRUE, TRUE, 0);

    // Create supplier buttons
    GtkWidget *sup_button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_pack_start(GTK_BOX(sup_page), sup_button_box, FALSE, FALSE, 5);

    GtkWidget *edit_sup_button = gtk_button_new_with_label("Modifier");
    GtkWidget *delete_sup_button = gtk_button_new_with_label("Supprimer");
    gtk_container_add(GTK_CONTAINER(sup_button_box), edit_sup_button);
    gtk_container_add(GTK_CONTAINER(sup_button_box), delete_sup_button);

    g_signal_connect(edit_sup_button, "clicked", G_CALLBACK(show_edit_supplier_dialog), main_window);
    g_signal_connect(delete_sup_button, "clicked", G_CALLBACK(delete_selected_supplier), NULL);

    // Add pages to notebook
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), med_page, gtk_label_new("Médicaments"));
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sup_page, gtk_label_new("Fournisseurs"));

    // Connect right-click events
    g_signal_connect(medication_list, "button-press-event", G_CALLBACK(on_button_pressed), GINT_TO_POINTER(0));
    g_signal_connect(supplier_list, "button-press-event", G_CALLBACK(on_button_pressed), GINT_TO_POINTER(1));

    // Refresh lists
    refresh_medication_list();
    refresh_supplier_list();

    gtk_widget_show_all(main_window);
}

void show_add_medication_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Medication",
                                                   GTK_WINDOW(parent),
                                                   GTK_DIALOG_MODAL,
                                                   "Cancel",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Add",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    GtkWidget *name_label = gtk_label_new("Name:");
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *quantity_label = gtk_label_new("Quantity:");
    GtkWidget *quantity_entry = gtk_entry_new();
    GtkWidget *price_label = gtk_label_new("Price:");
    GtkWidget *price_entry = gtk_entry_new();
    GtkWidget *expiry_label = gtk_label_new("Expiry Date (YYYY-MM-DD):");
    GtkWidget *expiry_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), quantity_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), quantity_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), price_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), price_entry, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), expiry_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), expiry_entry, 1, 3, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        int quantity = atoi(gtk_entry_get_text(GTK_ENTRY(quantity_entry)));
        double price = atof(gtk_entry_get_text(GTK_ENTRY(price_entry)));
        const char *expiry = gtk_entry_get_text(GTK_ENTRY(expiry_entry));
        
        add_medication(name, quantity, price, expiry);
        refresh_medication_list();
    }

    gtk_widget_destroy(dialog);
}

void show_add_supplier_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Add Supplier",
                                                   GTK_WINDOW(parent),
                                                   GTK_DIALOG_MODAL,
                                                   "Cancel",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Add",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    GtkWidget *name_label = gtk_label_new("Name:");
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *contact_label = gtk_label_new("Contact:");
    GtkWidget *contact_entry = gtk_entry_new();
    GtkWidget *address_label = gtk_label_new("Address:");
    GtkWidget *address_entry = gtk_entry_new();

    gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), contact_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), contact_entry, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), address_label, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), address_entry, 1, 2, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        const char *name = gtk_entry_get_text(GTK_ENTRY(name_entry));
        const char *contact = gtk_entry_get_text(GTK_ENTRY(contact_entry));
        const char *address = gtk_entry_get_text(GTK_ENTRY(address_entry));
        
        add_supplier(name, contact, address);
        refresh_supplier_list();
    }

    gtk_widget_destroy(dialog);
}

void show_edit_medication_dialog(GtkWidget *parent) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(medication_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gchar *name;
        gint quantity;
        gdouble price;
        gchar *expiry;

        gtk_tree_model_get(model, &iter,
                          0, &id,
                          1, &name,
                          2, &quantity,
                          3, &price,
                          4, &expiry,
                          -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier Médicament",
                                                       GTK_WINDOW(parent),
                                                       GTK_DIALOG_MODAL,
                                                       "Annuler",
                                                       GTK_RESPONSE_CANCEL,
                                                       "Enregistrer",
                                                       GTK_RESPONSE_ACCEPT,
                                                       NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(content_area), grid);
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

        GtkWidget *name_label = gtk_label_new("Nom:");
        GtkWidget *name_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(name_entry), name);

        GtkWidget *quantity_label = gtk_label_new("Quantité:");
        GtkWidget *quantity_entry = gtk_entry_new();
        char qty_str[32];
        sprintf(qty_str, "%d", quantity);
        gtk_entry_set_text(GTK_ENTRY(quantity_entry), qty_str);

        GtkWidget *price_label = gtk_label_new("Prix:");
        GtkWidget *price_entry = gtk_entry_new();
        char price_str[32];
        sprintf(price_str, "%.2f", price);
        gtk_entry_set_text(GTK_ENTRY(price_entry), price_str);

        GtkWidget *expiry_label = gtk_label_new("Date d'expiration (YYYY-MM-DD):");
        GtkWidget *expiry_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(expiry_entry), expiry);

        gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), quantity_label, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), quantity_entry, 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), price_label, 0, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), price_entry, 1, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), expiry_label, 0, 3, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), expiry_entry, 1, 3, 1, 1);

        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            const char *new_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
            int new_quantity = atoi(gtk_entry_get_text(GTK_ENTRY(quantity_entry)));
            double new_price = atof(gtk_entry_get_text(GTK_ENTRY(price_entry)));
            const char *new_expiry = gtk_entry_get_text(GTK_ENTRY(expiry_entry));

            char query[512];
            snprintf(query, sizeof(query),
                    "UPDATE Medicaments SET nom='%s', quantite=%d, prix=%.2f, date_expiration='%s' WHERE id=%d;",
                    new_name, new_quantity, new_price, new_expiry, id);

            char *err_msg = NULL;
            if (sqlite3_exec(db, query, NULL, NULL, &err_msg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
            }

            refresh_medication_list();
        }

        g_free(name);
        g_free(expiry);
        gtk_widget_destroy(dialog);
    }
}

void show_edit_supplier_dialog(GtkWidget *parent) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(supplier_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gchar *name;
        gchar *contact;
        gchar *address;

        gtk_tree_model_get(model, &iter,
                          0, &id,
                          1, &name,
                          2, &contact,
                          3, &address,
                          -1);

        GtkWidget *dialog = gtk_dialog_new_with_buttons("Modifier Fournisseur",
                                                       GTK_WINDOW(parent),
                                                       GTK_DIALOG_MODAL,
                                                       "Annuler",
                                                       GTK_RESPONSE_CANCEL,
                                                       "Enregistrer",
                                                       GTK_RESPONSE_ACCEPT,
                                                       NULL);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        GtkWidget *grid = gtk_grid_new();
        gtk_container_add(GTK_CONTAINER(content_area), grid);
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
        gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

        GtkWidget *name_label = gtk_label_new("Nom:");
        GtkWidget *name_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(name_entry), name);

        GtkWidget *contact_label = gtk_label_new("Contact:");
        GtkWidget *contact_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(contact_entry), contact);

        GtkWidget *address_label = gtk_label_new("Adresse:");
        GtkWidget *address_entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(address_entry), address);

        gtk_grid_attach(GTK_GRID(grid), name_label, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), name_entry, 1, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), contact_label, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), contact_entry, 1, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), address_label, 0, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), address_entry, 1, 2, 1, 1);

        gtk_widget_show_all(dialog);

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
            const char *new_name = gtk_entry_get_text(GTK_ENTRY(name_entry));
            const char *new_contact = gtk_entry_get_text(GTK_ENTRY(contact_entry));
            const char *new_address = gtk_entry_get_text(GTK_ENTRY(address_entry));

            char query[512];
            snprintf(query, sizeof(query),
                    "UPDATE Fournisseurs SET nom='%s', contact='%s', adresse='%s' WHERE id=%d;",
                    new_name, new_contact, new_address, id);

            char *err_msg = NULL;
            if (sqlite3_exec(db, query, NULL, NULL, &err_msg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
            }

            refresh_supplier_list();
        }

        g_free(name);
        g_free(contact);
        g_free(address);
        gtk_widget_destroy(dialog);
    }
}

void delete_selected_medication(GtkWidget *button, gpointer user_data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(medication_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Voulez-vous vraiment supprimer ce médicament ?");

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
            char query[256];
            snprintf(query, sizeof(query), "DELETE FROM Medicaments WHERE id=%d;", id);

            char *err_msg = NULL;
            if (sqlite3_exec(db, query, NULL, NULL, &err_msg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
            }

            refresh_medication_list();
        }

        gtk_widget_destroy(dialog);
    }
}

void delete_selected_supplier(GtkWidget *button, gpointer user_data) {
    GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(supplier_list));
    GtkTreeModel *model;
    GtkTreeIter iter;

    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gint id;
        gtk_tree_model_get(model, &iter, 0, &id, -1);

        GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(main_window),
                                                 GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_QUESTION,
                                                 GTK_BUTTONS_YES_NO,
                                                 "Voulez-vous vraiment supprimer ce fournisseur ?");

        if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_YES) {
            char query[256];
            snprintf(query, sizeof(query), "DELETE FROM Fournisseurs WHERE id=%d;", id);

            char *err_msg = NULL;
            if (sqlite3_exec(db, query, NULL, NULL, &err_msg) != SQLITE_OK) {
                fprintf(stderr, "SQL error: %s\n", err_msg);
                sqlite3_free(err_msg);
            }

            refresh_supplier_list();
        }

        gtk_widget_destroy(dialog);
    }
}
// Add these functions before the last closing brace

void refresh_supplier_list(void) {
    sqlite3_stmt *stmt;
    GtkListStore *store;
    const char *query = "SELECT * FROM Fournisseurs;";

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(supplier_list)));
    gtk_list_store_clear(store);

    if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            GtkTreeIter iter;
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter,
                             0, sqlite3_column_int(stmt, 0),
                             1, sqlite3_column_text(stmt, 1),
                             2, sqlite3_column_text(stmt, 2),
                             3, sqlite3_column_text(stmt, 3),
                             -1);
        }
        sqlite3_finalize(stmt);
    }
}

void perform_simple_search(GtkWidget *button, GtkWidget *search_entry) {
    const char *search_term = gtk_entry_get_text(GTK_ENTRY(search_entry));
    int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
    
    if (current_page == 0) {  // Medications page
        char query[512];
        snprintf(query, sizeof(query),
                "SELECT * FROM Medicaments WHERE nom LIKE '%%%s%%';",
                search_term);
        sqlite3_stmt *stmt;
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(medication_list)));
        gtk_list_store_clear(store);

        if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                 0, sqlite3_column_int(stmt, 0),
                                 1, sqlite3_column_text(stmt, 1),
                                 2, sqlite3_column_int(stmt, 2),
                                 3, sqlite3_column_double(stmt, 3),
                                 4, sqlite3_column_text(stmt, 4),
                                 -1);
            }
            sqlite3_finalize(stmt);
        }
    } else {  // Suppliers page
        char query[512];
        snprintf(query, sizeof(query),
                "SELECT * FROM Fournisseurs WHERE nom LIKE '%%%s%%';",
                search_term);
        sqlite3_stmt *stmt;
        GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(supplier_list)));
        gtk_list_store_clear(store);

        if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                GtkTreeIter iter;
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter,
                                 0, sqlite3_column_int(stmt, 0),
                                 1, sqlite3_column_text(stmt, 1),
                                 2, sqlite3_column_text(stmt, 2),
                                 3, sqlite3_column_text(stmt, 3),
                                 -1);
            }
            sqlite3_finalize(stmt);
        }
    }
}

void export_medications_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export Medications",
                                                   GTK_WINDOW(parent),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Save",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "ID,Nom,Quantité,Prix,Date Expiration\n");
            
            sqlite3_stmt *stmt;
            const char *query = "SELECT * FROM Medicaments;";
            
            if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    fprintf(file, "%d,%s,%d,%.2f,%s\n",
                           sqlite3_column_int(stmt, 0),
                           sqlite3_column_text(stmt, 1),
                           sqlite3_column_int(stmt, 2),
                           sqlite3_column_double(stmt, 3),
                           sqlite3_column_text(stmt, 4));
                }
                sqlite3_finalize(stmt);
            }
            fclose(file);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void export_suppliers_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_file_chooser_dialog_new("Export Suppliers",
                                                   GTK_WINDOW(parent),
                                                   GTK_FILE_CHOOSER_ACTION_SAVE,
                                                   "Cancel",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Save",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        FILE *file = fopen(filename, "w");
        if (file) {
            fprintf(file, "ID,Nom,Contact,Adresse\n");
            
            sqlite3_stmt *stmt;
            const char *query = "SELECT * FROM Fournisseurs;";
            
            if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    fprintf(file, "%d,%s,%s,%s\n",
                           sqlite3_column_int(stmt, 0),
                           sqlite3_column_text(stmt, 1),
                           sqlite3_column_text(stmt, 2),
                           sqlite3_column_text(stmt, 3));
                }
                sqlite3_finalize(stmt);
            }
            fclose(file);
        }
        g_free(filename);
    }
    gtk_widget_destroy(dialog);
}

void show_advanced_search_dialog(GtkWidget *parent) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Advanced Search",
                                                   GTK_WINDOW(parent),
                                                   GTK_DIALOG_MODAL,
                                                   "Cancel",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Search",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *notebook = gtk_notebook_new();
    gtk_container_add(GTK_CONTAINER(content_area), notebook);

    // Medications search page
    GtkWidget *med_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(med_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(med_grid), 5);

    GtkWidget *name_label = gtk_label_new("Name pattern:");
    GtkWidget *name_entry = gtk_entry_new();
    GtkWidget *min_qty_label = gtk_label_new("Min quantity:");
    GtkWidget *min_qty_entry = gtk_entry_new();
    GtkWidget *max_qty_label = gtk_label_new("Max quantity:");
    GtkWidget *max_qty_entry = gtk_entry_new();
    GtkWidget *min_price_label = gtk_label_new("Min price:");
    GtkWidget *min_price_entry = gtk_entry_new();
    GtkWidget *max_price_label = gtk_label_new("Max price:");
    GtkWidget *max_price_entry = gtk_entry_new();
    GtkWidget *exp_after_label = gtk_label_new("Expires after:");
    GtkWidget *exp_after_entry = gtk_entry_new();
    GtkWidget *exp_before_label = gtk_label_new("Expires before:");
    GtkWidget *exp_before_entry = gtk_entry_new();

    int row = 0;
    gtk_grid_attach(GTK_GRID(med_grid), name_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), name_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), min_qty_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), min_qty_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), max_qty_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), max_qty_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), min_price_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), min_price_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), max_price_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), max_price_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), exp_after_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), exp_after_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), exp_before_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(med_grid), exp_before_entry, 1, row++, 1, 1);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), med_grid, 
                           gtk_label_new("Medications"));

    // Suppliers search page
    GtkWidget *sup_grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(sup_grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(sup_grid), 5);

    GtkWidget *sup_name_label = gtk_label_new("Name:");
    GtkWidget *sup_name_entry = gtk_entry_new();
    GtkWidget *contact_label = gtk_label_new("Contact:");
    GtkWidget *contact_entry = gtk_entry_new();
    GtkWidget *address_label = gtk_label_new("Address:");
    GtkWidget *address_entry = gtk_entry_new();

    row = 0;
    gtk_grid_attach(GTK_GRID(sup_grid), sup_name_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(sup_grid), sup_name_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(sup_grid), contact_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(sup_grid), contact_entry, 1, row++, 1, 1);
    gtk_grid_attach(GTK_GRID(sup_grid), address_label, 0, row, 1, 1);
    gtk_grid_attach(GTK_GRID(sup_grid), address_entry, 1, row++, 1, 1);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), sup_grid, 
                           gtk_label_new("Suppliers"));

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        int current_page = gtk_notebook_get_current_page(GTK_NOTEBOOK(notebook));
        
        if (current_page == 0) {  // Medications
            char query[1024];
            snprintf(query, sizeof(query),
                    "SELECT * FROM Medicaments WHERE nom LIKE '%%%s%%'",
                    gtk_entry_get_text(GTK_ENTRY(name_entry)));

            const char *min_qty = gtk_entry_get_text(GTK_ENTRY(min_qty_entry));
            if (strlen(min_qty) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND quantite >= %d", atoi(min_qty));
                strcat(query, temp);
            }

            const char *max_qty = gtk_entry_get_text(GTK_ENTRY(max_qty_entry));
            if (strlen(max_qty) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND quantite <= %d", atoi(max_qty));
                strcat(query, temp);
            }

            const char *min_price = gtk_entry_get_text(GTK_ENTRY(min_price_entry));
            if (strlen(min_price) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND prix >= %.2f", atof(min_price));
                strcat(query, temp);
            }

            const char *max_price = gtk_entry_get_text(GTK_ENTRY(max_price_entry));
            if (strlen(max_price) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND prix <= %.2f", atof(max_price));
                strcat(query, temp);
            }

            const char *exp_after = gtk_entry_get_text(GTK_ENTRY(exp_after_entry));
            if (strlen(exp_after) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND date_expiration >= '%s'", exp_after);
                strcat(query, temp);
            }

            const char *exp_before = gtk_entry_get_text(GTK_ENTRY(exp_before_entry));
            if (strlen(exp_before) > 0) {
                char temp[128];
                snprintf(temp, sizeof(temp), " AND date_expiration <= '%s'", exp_before);
                strcat(query, temp);
            }

            strcat(query, ";");

            sqlite3_stmt *stmt;
            GtkListStore *store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(medication_list)));
            gtk_list_store_clear(store);
            if (sqlite3_prepare_v2(db, query, -1, &stmt, 0) == SQLITE_OK) {
                while (sqlite3_step(stmt) == SQLITE_ROW) {
                    GtkTreeIter iter;
                    gtk_list_store_append(store, &iter);
                    gtk_list_store_set(store, &iter,
                                     0, sqlite3_column_int(stmt, 0),
                                     1, sqlite3_column_text(stmt, 1),
                                     2, sqlite3_column_int(stmt, 2),
                                     3, sqlite3_column_double(stmt, 3),
                                     4, sqlite3_column_text(stmt, 4),
                                     -1);
                }
            }
        }
    }
}
static gboolean on_button_pressed(GtkWidget *treeview, GdkEventButton *event, gpointer user_data) {
    if (event->type == GDK_BUTTON_PRESS && event->button == 3) { // Right click
        GtkTreeSelection *selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        GtkTreePath *path;
        
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), event->x, event->y, &path, NULL, NULL, NULL)) {
            gtk_tree_selection_select_path(selection, path);
            gtk_tree_path_free(path);
            
            if (GPOINTER_TO_INT(user_data) == 0) {
                show_medication_popup_menu(treeview, event);
            } else {
                show_supplier_popup_menu(treeview, event);
            }
            
            return TRUE;
        }
    }
    return FALSE;
}
void show_generate_dialog(GtkWidget *button, gpointer user_data) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Générer avec IA",
                                                   GTK_WINDOW(main_window),
                                                   GTK_DIALOG_MODAL,
                                                   "Annuler",
                                                   GTK_RESPONSE_CANCEL,
                                                   "Générer",
                                                   GTK_RESPONSE_ACCEPT,
                                                   NULL);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *grid = gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);

    GtkWidget *type_label = gtk_label_new("Type:");
    GtkWidget *type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Médicaments");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(type_combo), "Fournisseurs");
    gtk_combo_box_set_active(GTK_COMBO_BOX(type_combo), 0);

    GtkWidget *count_label = gtk_label_new("Nombre d'éléments:");
    GtkWidget *count_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(count_entry), "5");

    gtk_grid_attach(GTK_GRID(grid), type_label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), type_combo, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), count_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), count_entry, 1, 1, 1, 1);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        int type = gtk_combo_box_get_active(GTK_COMBO_BOX(type_combo));
        int count = atoi(gtk_entry_get_text(GTK_ENTRY(count_entry)));

        if (type == 0) {  // Médicaments
            GeneratedMedication *medications = malloc(count * sizeof(GeneratedMedication));
            if (generate_medications(medications, count)) {
                for (int i = 0; i < count; i++) {
                    add_medication(medications[i].name, 
                                 medications[i].quantity,
                                 medications[i].price,
                                 medications[i].expiry_date);
                }
                refresh_medication_list();
            }
            free(medications);
        } else {  // Fournisseurs
            GeneratedSupplier *suppliers = malloc(count * sizeof(GeneratedSupplier));
            if (generate_suppliers(suppliers, count)) {
                for (int i = 0; i < count; i++) {
                    add_supplier(suppliers[i].name,
                               suppliers[i].contact,
                               suppliers[i].address);
                }
                refresh_supplier_list();
            }
            free(suppliers);
        }
    }

    gtk_widget_destroy(dialog);
}

