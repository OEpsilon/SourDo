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
} // namespace sourdo
