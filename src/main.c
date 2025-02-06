#include "../include/gui.h"
#include "../include/database.h"
#include <gtk/gtk.h>

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    db_init();
    
    create_main_window();
    
    gtk_main();
    
    db_close();
    
    return 0;
}