#pragma once

#include "SourDo/SourDo.hpp"
#include "SourDo/Errors.hpp"

#include <string>

namespace sourdo
{
    inline bool string_length(Data& data)
    {
        sourdo::arg_count_error(data, 1);
        sourdo::is_string_error(data, 1);
        std::string self = data.value_to_string(1);
        data.push_number(self.size());
        return true;
    }
} // namespace sourdo
