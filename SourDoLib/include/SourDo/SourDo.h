#pragma once

typedef int SourDoBool;
typedef double SourDoNumber;

#define SOURDO_TRUE 1
#define SOURDO_FALSE 0

struct sourdo_Data;

sourdo_Data* sourdo_data_create();
void sourdo_data_destroy(sourdo_Data* data);

SourDoBool sourdo_is_number(sourdo_Data* data, int index);
SourDoBool sourdo_is_bool(sourdo_Data* data, int index);
SourDoBool sourdo_is_string(sourdo_Data* data, int index);
SourDoBool sourdo_is_null(sourdo_Data* data, int index);

SourDoNumber sourdo_to_number(sourdo_Data* data, int index);
SourDoBool sourdo_to_bool(sourdo_Data* data, int index);
const char* sourdo_to_string(sourdo_Data* data, int index);

void sourdo_push_number(sourdo_Data* data, SourDoNumber value);
void sourdo_push_bool(sourdo_Data* data, SourDoBool value);
void sourdo_push_string(sourdo_Data* data, const char* value);
void sourdo_push_null(sourdo_Data* data);

SourDoBool sourdo_get_global(sourdo_Data* data, const char* name);

void sourdo_pop(sourdo_Data* data);

SourDoBool sourdo_do_string(sourdo_Data* data, const char* string);
