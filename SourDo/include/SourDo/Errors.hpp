#pragma once

#include "SourDo.hpp"

namespace sourdo
{
    void arg_count_error(Data& data, uint32_t expected_count);

    void is_number_error(Data& data, int arg);
    void is_bool_error(Data& data, int arg);
    void is_string_error(Data& data, int arg);
    void is_function_error(Data& data, int arg);
    void is_null_error(Data& data, int arg);
} // namespace sourdo
