#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include "../include/gemini_api.h"
#include "../include/config.h"


#define GEMINI_API_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent"

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct json_object **json = (struct json_object **)userp;
    
    *json = json_tokener_parse(contents);
    return realsize;
}

static int call_gemini_api(const char *prompt, struct json_object **response) {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char auth_header[256];
    char post_data[1024];

    curl = curl_easy_init();
    if (!curl) return 0;

    snprintf(auth_header, sizeof(auth_header), "Authorization: Bearer %s", GEMINI_API_KEY);
    snprintf(post_data, sizeof(post_data), 
             "{\"contents\":[{\"parts\":[{\"text\":\"%s\"}]}]}", prompt);

    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, auth_header);

    curl_easy_setopt(curl, CURLOPT_URL, GEMINI_API_URL);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, response);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    return (res == CURLE_OK);
}

static const char *MEDICATIONS[] = {
    "Doliprane", "Efferalgan", "Spasfon", "Advil", "Smecta", 
    "Voltarene", "Imodium", "Aspegic", "Rhinadvil", "Gaviscon",
    "Nurofen", "Maalox", "Strepsils", "Biseptine", "Dacryum"
};

static const char *SUPPLIERS[] = {
    "Alliance Healthcare France",
    "OCP Répartition",
    "CERP Rouen",
    "Phoenix Pharma",
    "CERP Rhin Med Alsace",
    "Eurapharma",
    "CERP Bretagne Atlantique"
};

int generate_medications(GeneratedMedication *medications, int count) {
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int med_index = rand() % (sizeof(MEDICATIONS) / sizeof(MEDICATIONS[0]));
        strncpy(medications[i].name, MEDICATIONS[med_index], sizeof(medications[i].name) - 1);
        
        medications[i].quantity = 50 + (rand() % 451); // 50-500
        medications[i].price = 3.50 + ((rand() % 2150) / 100.0); // 3.50-25.00
        
        int year = 2024 + (rand() % 3);
        int month = 1 + (rand() % 12);
        int day = 1 + (rand() % 28);
        snprintf(medications[i].expiry_date, sizeof(medications[i].expiry_date),
                "%04d-%02d-%02d", year, month, day);
    }
    return 1;
}

int generate_suppliers(GeneratedSupplier *suppliers, int count) {
    srand(time(NULL));
    for (int i = 0; i < count; i++) {
        int sup_index = rand() % (sizeof(SUPPLIERS) / sizeof(SUPPLIERS[0]));
        strncpy(suppliers[i].name, SUPPLIERS[sup_index], sizeof(suppliers[i].name) - 1);
        
        snprintf(suppliers[i].contact, sizeof(suppliers[i].contact),
                "+33 %d %02d %02d %02d %02d",
                1 + (rand() % 9),
                rand() % 100,
                rand() % 100,
                rand() % 100,
                rand() % 100);
        
        static const char *CITIES[] = {"Paris", "Lyon", "Marseille", "Bordeaux", "Lille"};
        static const char *STREETS[] = {"Rue de la Pharmacie", "Avenue des Médicaments", "Boulevard de la Santé"};
        
        int city_index = rand() % (sizeof(CITIES) / sizeof(CITIES[0]));
        int street_index = rand() % (sizeof(STREETS) / sizeof(STREETS[0]));
        int street_num = 1 + (rand() % 200);
        int postal = 10000 + (rand() % 90000);
        
        snprintf(suppliers[i].address, sizeof(suppliers[i].address),
                "%d %s, %05d %s",
                street_num, STREETS[street_index],
                postal, CITIES[city_index]);
    }
    return 1;
}