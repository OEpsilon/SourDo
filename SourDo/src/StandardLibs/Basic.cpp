#include "SourDo/StandardLibs/Basic.hpp"

#include "SourDo/SourDo.hpp"
#include "SourDo/Errors.hpp"

#include <iostream>
#include <sstream>

namespace sourdo
{
    bool print(Data& data)
    {
        uint32_t arg_count = data.get_size();
        std::stringstream ss;
        ss << std::boolalpha << std::fixed;
        for(int i = 1; i <= arg_count; i++)
        {
            switch(data.get_value_type(i))
            {
                case sourdo::ValueType::_NULL:
                    ss << "null";
                    break;
                case sourdo::ValueType::NUMBER:
                    ss << data.value_to_number(i);
                    break;
                case sourdo::ValueType::BOOL:
                    ss << data.value_to_bool(i);
                    break;
                case sourdo::ValueType::STRING:
                    ss << data.value_to_string(i);
                    break;
                case sourdo::ValueType::SOURDO_FUNCTION:
                    ss << "[SourdoFunction]";
                    break;
                case sourdo::ValueType::CPP_FUNCTION:
                    ss << "[CppFunction]";
                    break;
            }
            if(i < arg_count)
            {
                ss << " ";
            }
        }
        ss << std::endl;
        std::cout << ss.str();
        return false;
    }

    bool to_string(Data& data)
    {
        arg_count_error(data, 1);

        switch(data.get_value_type(1))
        {
            case ValueType::NUMBER:
                data.push_string(std::to_string(data.value_to_number(1)));
                break;
            case ValueType::BOOL:
            {
                std::stringstream ss;
                ss << std::boolalpha << data.value_to_bool(1);
                data.push_string(ss.str());
                break;
            }
            case ValueType::STRING:
                data.push_string(data.value_to_string(1));
                break;
            case ValueType::SOURDO_FUNCTION:
                data.push_string("[SourdoFunction]");
                break;
            case ValueType::CPP_FUNCTION:
                data.push_string("[CppFunction]");
                break;
            case ValueType::_NULL:
                data.push_string("null");
                break;
        }
        return true;
    }

    void load_lib_basic(Data& data)
    {
        data.create_value("print");
        data.push_cppfunction(print);
        data.set_value("print", true);

        data.create_value("to_string");
        data.push_cppfunction(to_string);
        data.set_value("to_string", true);
    }
} // namespace sourdo
