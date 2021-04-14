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
        // Used to keep temporary values.
        std::vector<sourdo::Value> stack;
        // Used to store named values.
        std::map<std::string, sourdo::Value> symbol_table;
    };

}
