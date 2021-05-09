#pragma once

#include <vector>

#include "Datatypes/Value.hpp"

namespace sourdo
{
    class GlobalData
    {
    public:
        static std::vector<Value> references;
    };

    bool check_value_type(const Value& value, const std::string& name);
} // namespace sourdo
