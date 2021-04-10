#pragma once

typedef int SourDoBool;

#define SOURDO_TRUE 1
#define SOURDO_FALSE 0

struct sourdo_Data;

sourdo_Data* sourdo_data_create();
void sourdo_data_destroy(sourdo_Data* data);

SourDoBool sourdo_is_int(sourdo_Data* data, int index);
SourDoBool sourdo_is_float(sourdo_Data* data, int index);
SourDoBool sourdo_is_string(sourdo_Data* data, int index);
SourDoBool sourdo_is_null(sourdo_Data* data, int index);

int sourdo_to_int(sourdo_Data* data, int index);
float sourdo_to_float(sourdo_Data* data, int index);
const char* sourdo_to_string(sourdo_Data* data, int index);

void sourdo_push_int(sourdo_Data* data, int value);
void sourdo_push_float(sourdo_Data* data, float value);
void sourdo_push_string(sourdo_Data* data, const char* value);
void sourdo_push_null(sourdo_Data* data);

void sourdo_pop(sourdo_Data* data);

SourDoBool sourdo_do_string(sourdo_Data* data, const char* string);
