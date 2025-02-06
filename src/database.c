#include "../include/database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // Add this line for strcat

sqlite3 *db = NULL;  // Define the database connection variable

void executerRequeteSQL(const char *requete) {
    char *messageErreur = NULL;
    if (sqlite3_exec(db, requete, 0, 0, &messageErreur) != SQLITE_OK) {
        printf("Erreur SQLite: %s\n", messageErreur);
        sqlite3_free(messageErreur);
    }
}

void db_init(void) {
    if (sqlite3_open("stock_medical.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
    
    // Create tables (your existing table creation code)
    const char *tables[] = {
        "CREATE TABLE IF NOT EXISTS Medicaments ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nom TEXT NOT NULL, "
        "quantite INTEGER NOT NULL CHECK(quantite >= 0), "
        "prix REAL NOT NULL CHECK(prix >= 0), "
        "date_expiration DATE);",

        "CREATE TABLE IF NOT EXISTS Fournisseurs ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nom TEXT NOT NULL, "
        "contact TEXT, "
        "adresse TEXT);",

        "CREATE TABLE IF NOT EXISTS Mouvements ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "id_medicament INTEGER, "
        "quantite INTEGER, "
        "operation TEXT, "
        "date_mouvement TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
        "FOREIGN KEY(id_medicament) REFERENCES Medicaments(id));"
    };

    for (int i = 0; i < sizeof(tables)/sizeof(tables[0]); i++) {
        char *err_msg = NULL;
        if (sqlite3_exec(db, tables[i], 0, 0, &err_msg) != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", err_msg);
            sqlite3_free(err_msg);
        }
    }
}

void db_close(void) {
    if (db != NULL) {
        sqlite3_close(db);
        db = NULL;
    }
}

void add_medication(const char* name, int quantity, double price, const char* expiry_date) {
    char query[256];
    snprintf(query, sizeof(query), 
             "INSERT INTO Medicaments (nom, quantite, prix, date_expiration) "
             "VALUES ('%s', %d, %.2f, '%s');",
             name, quantity, price, expiry_date);
    
    executerRequeteSQL(query);
}

void add_supplier(const char* name, const char* contact, const char* address) {
    char query[256];
    snprintf(query, sizeof(query), 
             "INSERT INTO Fournisseurs (nom, contact, adresse) "
             "VALUES ('%s', '%s', '%s');",
             name, contact, address);
    
    executerRequeteSQL(query);
}

void update_medication(int id, const char* name, int quantity, double price, const char* expiry_date) {
    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE Medicaments SET nom='%s', quantite=%d, prix=%.2f, date_expiration='%s' "
             "WHERE id=%d;",
             name, quantity, price, expiry_date, id);
    executerRequeteSQL(query);
}

void delete_medication(int id) {
    char query[128];
    snprintf(query, sizeof(query), "DELETE FROM Medicaments WHERE id=%d;", id);
    executerRequeteSQL(query);
}

void export_medications_to_csv(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "ID,Nom,Quantité,Prix,Date d'expiration\n");

    sqlite3_stmt* stmt;
    const char* query = "SELECT * FROM Medicaments;";
    
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

void update_supplier(int id, const char* name, const char* contact, const char* address) {
    char query[512];
    snprintf(query, sizeof(query),
             "UPDATE Fournisseurs SET nom='%s', contact='%s', adresse='%s' "
             "WHERE id=%d;",
             name, contact, address, id);
    executerRequeteSQL(query);
}

void delete_supplier(int id) {
    char query[128];
    snprintf(query, sizeof(query), "DELETE FROM Fournisseurs WHERE id=%d;", id);
    executerRequeteSQL(query);
}

void export_suppliers_to_csv(const char* filename) {
    FILE* file = fopen(filename, "w");
    if (!file) return;

    fprintf(file, "ID,Nom,Contact,Adresse\n");

    sqlite3_stmt* stmt;
    const char* query = "SELECT * FROM Fournisseurs;";
    
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

void search_medications(const char* query) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT * FROM Medicaments WHERE nom LIKE '%%%s%%' "
             "OR date_expiration LIKE '%%%s%%';",
             query, query);
    executerRequeteSQL(sql);
}

void search_suppliers(const char* query) {
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT * FROM Fournisseurs WHERE nom LIKE '%%%s%%' "
             "OR contact LIKE '%%%s%%' OR adresse LIKE '%%%s%%';",
             query, query, query);
    executerRequeteSQL(sql);
}

void advanced_search_medications(const MedicationSearchCriteria* criteria) {
    char query[1024] = "SELECT * FROM Medicaments WHERE 1=1";
    char condition[256];

    if (criteria->name_pattern) {
        snprintf(condition, sizeof(condition), " AND nom LIKE '%%%s%%'", criteria->name_pattern);
        strcat(query, condition);
    }
    if (criteria->min_quantity >= 0) {
        snprintf(condition, sizeof(condition), " AND quantite >= %d", criteria->min_quantity);
        strcat(query, condition);
    }
    if (criteria->max_quantity >= 0) {
        snprintf(condition, sizeof(condition), " AND quantite <= %d", criteria->max_quantity);
        strcat(query, condition);
    }
    if (criteria->min_price >= 0) {
        snprintf(condition, sizeof(condition), " AND prix >= %.2f", criteria->min_price);
        strcat(query, condition);
    }
    if (criteria->max_price >= 0) {
        snprintf(condition, sizeof(condition), " AND prix <= %.2f", criteria->max_price);
        strcat(query, condition);
    }
    if (criteria->expiry_before) {
        snprintf(condition, sizeof(condition), " AND date_expiration <= '%s'", criteria->expiry_before);
        strcat(query, condition);
    }
    if (criteria->expiry_after) {
        snprintf(condition, sizeof(condition), " AND date_expiration >= '%s'", criteria->expiry_after);
        strcat(query, condition);
    }

    strcat(query, ";");
    executerRequeteSQL(query);
}

void advanced_search_suppliers(const SupplierSearchCriteria* criteria) {
    char query[1024] = "SELECT * FROM Fournisseurs WHERE 1=1";
    char condition[256];

    if (criteria->name_pattern) {
        snprintf(condition, sizeof(condition), " AND nom LIKE '%%%s%%'", criteria->name_pattern);
        strcat(query, condition);
    }
    if (criteria->contact_pattern) {
        snprintf(condition, sizeof(condition), " AND contact LIKE '%%%s%%'", criteria->contact_pattern);
        strcat(query, condition);
    }
    if (criteria->address_pattern) {
        snprintf(condition, sizeof(condition), " AND adresse LIKE '%%%s%%'", criteria->address_pattern);
        strcat(query, condition);
    }

    strcat(query, ";");
    executerRequeteSQL(query);
}