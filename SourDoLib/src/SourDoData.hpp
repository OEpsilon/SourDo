#pragma once

#include <vector>

#include "Datatypes/Value.hpp"

extern "C" {
    struct sourdo_Data
    {
        // Used to keep temporary values.
        std::vector<sourdo::Value> stack;
    };
}
