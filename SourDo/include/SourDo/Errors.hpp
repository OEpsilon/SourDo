#pragma once

#include "SourDo.hpp"

namespace sourdo
{
    void check_arg_count(Data& data, uint32_t expected_count);

    void check_is_number(Data& data, int arg);
    void check_is_bool(Data& data, int arg);
    void check_is_string(Data& data, int arg);
    void check_is_function(Data& data, int arg);
    void check_is_null(Data& data, int arg);
    void check_is_object(Data& data, int arg);
} // namespace sourdo
