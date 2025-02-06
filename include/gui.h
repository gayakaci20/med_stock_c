#ifndef GUI_H
#define GUI_H

#include <gtk/gtk.h>

void create_main_window(void);
void refresh_medication_list(void);
void refresh_supplier_list(void);

void show_add_medication_dialog(GtkWidget *parent);
void show_add_supplier_dialog(GtkWidget *parent);

void export_medications_dialog(GtkWidget *parent);
void export_suppliers_dialog(GtkWidget *parent);

void show_advanced_search_dialog(GtkWidget *parent);

void perform_simple_search(GtkWidget *button, GtkWidget *search_entry);

void show_generate_dialog(GtkWidget *button, gpointer user_data);
void show_edit_medication_dialog(GtkWidget *parent);
void show_edit_supplier_dialog(GtkWidget *parent);
void delete_selected_medication(GtkWidget *button, gpointer user_data);
void delete_selected_supplier(GtkWidget *button, gpointer user_data);

#endif