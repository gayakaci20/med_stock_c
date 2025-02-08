#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

extern sqlite3 *db;

void db_init(void);
void db_close(void);
void executerRequeteSQL(const char *requete);

void add_medication(const char* name, int quantity, double price, const char* expiry_date);
void update_medication(int id, const char* name, int quantity, double price, const char* expiry_date);
void delete_medication(int id);
void list_medications(void);
void list_expired_medications(void);
void search_medications(const char* query);
void export_medications_to_csv(const char* filename);

void add_supplier(const char* name, const char* contact, const char* address);
void update_supplier(int id, const char* name, const char* contact, const char* address);
void delete_supplier(int id);
void list_suppliers(void);
void search_suppliers(const char* query);
void export_suppliers_to_csv(const char* filename);
void assign_supplier_to_medication(int med_id, int supplier_id);

typedef struct {
    char* name_pattern;
    int min_quantity;
    int max_quantity;
    double min_price;
    double max_price;
    char* expiry_before;
    char* expiry_after;
} MedicationSearchCriteria;

typedef struct {
    char* name_pattern;
    char* contact_pattern;
    char* address_pattern;
} SupplierSearchCriteria;

void advanced_search_medications(const MedicationSearchCriteria* criteria);
void advanced_search_suppliers(const SupplierSearchCriteria* criteria);

typedef struct {
    char name[256];
    int quantity;
    double price;
    char expiry_date[11];
} GeneratedMedication;

typedef struct {
    char name[256];
    char contact[32];
    char address[512];
} GeneratedSupplier;

int generate_medications(GeneratedMedication *medications, int count);
int generate_suppliers(GeneratedSupplier *suppliers, int count);

#endif