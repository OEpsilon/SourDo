#pragma once

namespace sourdo
{
    class Data;
    
    bool print(Data& data);
    bool to_string(Data& data);
    bool format(Data& data);

    void load_lib_basic(Data& data);
} // namespace sourdo
