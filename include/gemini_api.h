#ifndef GEMINI_API_H
#define GEMINI_API_H

#include "database.h"
#include <json-c/json.h>

int generate_medications(GeneratedMedication *medications, int count);
int generate_suppliers(GeneratedSupplier *suppliers, int count);

#endif