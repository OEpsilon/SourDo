#pragma once

#include "SourDo/SourDo.hpp"
#include "../SourDoData.hpp"
#include "SourDo/Errors.hpp"

#include <string>

namespace sourdo
{
    inline bool table_has(Data& data)
    {
        sourdo::check_arg_count(data, 2);
        sourdo::check_is_table(data, 1);
        Value self = data.get_impl()->index_stack(1);
        Value key = data.get_impl()->index_stack(2);
        data.push_bool(self.to_table()->keys.find(key) != self.to_table()->keys.end());
        return true;
    }
} // namespace sourdo
