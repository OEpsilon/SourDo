#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <map>
#include <sstream>

#include "ConsoleColors.hpp"
#include "Datatypes/Value.hpp"

extern "C" {
    struct sourdo_Data
    {
        sourdo_Data* parent = nullptr;
        // Used to keep temporary values.
        std::vector<sourdo::Value> stack;
        // Used to store named values.
        std::map<std::string, sourdo::Value> symbol_table;
    };

    inline void sourdo_set_symbol(sourdo_Data* data, const std::string& index, const sourdo::Value& value)
    {
        /*if(data->symbol_table.find(index) == data->symbol_table.end() && data->parent)
        {
            sourdo_set_symbol(data->parent, index, value);
            return;
        }*/
        if(data->symbol_table.find(index) == data->symbol_table.end())
        {
            sourdo_Data* parent = data->parent;
            while(parent != nullptr)
            {
                if(parent->symbol_table.find(index) != parent->symbol_table.end())
                {
                    parent->symbol_table[index] = value;
                }
            }
        }
        data->symbol_table[index] = value;
    }

    inline sourdo::Value* sourdo_get_symbol(sourdo_Data* data, const std::string& index)
    {
        if(data->symbol_table.find(index) == data->symbol_table.end())
        {
            if(data->parent)
            {
                return sourdo_get_symbol(data->parent, index);
            }
            return nullptr;
        }
        return &(data->symbol_table[index]);
    }
}
