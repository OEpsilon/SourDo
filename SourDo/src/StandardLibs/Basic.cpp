#include "SourDo/StandardLibs/Basic.hpp"

#include "SourDo/SourDo.hpp"
#include "SourDo/Errors.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>

namespace sourdo
{
    bool print(Data& data)
    {
        uint32_t arg_count = data.get_size();
        std::stringstream ss;
        ss << std::boolalpha << std::setprecision(14);
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
                case sourdo::ValueType::OBJECT:
                    ss << "[Object]";
                    break;
                case sourdo::ValueType::CPP_OBJECT:
                    data.push_string("[CppObject]");
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
        check_arg_count(data, 1);

        switch(data.get_value_type(1))
        {
            case ValueType::NUMBER:
            {
                std::stringstream ss;
                ss << std::boolalpha << std::setprecision(14) << data.value_to_number(1);
                data.push_string(ss.str());
                break;
            }
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
            case sourdo::ValueType::OBJECT:
                data.push_string("[Object]");
                break;
            case sourdo::ValueType::CPP_OBJECT:
                data.push_string("[CppObject]");
                break;
        }
        return true;
    }

    bool format(Data& data)
    {
        uint32_t arg_count = data.get_size();
        if(arg_count < 1)
        {
            data.error("Expected a format string as the first argument");
        }
        check_is_string(data, 1);

        std::string format_string = data.value_to_string(1);
        std::stringstream ss;
        ss << std::boolalpha << std::setprecision(14);
        for(int i = 0; i < format_string.length(); i++)
        {
            if(format_string[i] != '{')
            {
                ss << format_string[i];
            }
            else
            {
                i++;
                if(format_string[i] == '{')
                {
                    continue;
                }
                std::string index_str = "";
                while(i < format_string.length())
                {
                    if(format_string[i] == '}')
                    {
                        break;
                    }
                    index_str += format_string[i];
                    i++;
                }
                if(format_string[i] != '}')
                {
                    data.error("'{' in format string is not closed");
                }
                
                int index = std::stoi(index_str);
                if(index > arg_count)
                {
                    data.error("Index '" + index_str + "' in format string is greater than the number of format arguments");
                }
                else if(index < 1)
                {
                    data.error("Index '" + index_str + "' in format string is less than 1");
                }

                switch(data.get_value_type(index + 1))
                {
                    case sourdo::ValueType::_NULL:
                        ss << "null";
                        break;
                    case sourdo::ValueType::NUMBER:
                        ss << data.value_to_number(index + 1);
                        break;
                    case sourdo::ValueType::BOOL:
                        ss << data.value_to_bool(index + 1);
                        break;
                    case sourdo::ValueType::STRING:
                        ss << data.value_to_string(index + 1);
                        break;
                    case sourdo::ValueType::SOURDO_FUNCTION:
                        ss << "[SourdoFunction]";
                        break;
                    case sourdo::ValueType::CPP_FUNCTION:
                        ss << "[CppFunction]";
                        break;
                    case sourdo::ValueType::OBJECT:
                        ss << "[Object]";
                        break;
                    case sourdo::ValueType::CPP_OBJECT:
                        data.push_string("[CppObject]");
                        break;
                }
            }
        }

        data.push_string(ss.str());
        return true;
    }

    bool error(Data& data)
    {
        check_arg_count(data, 1);
        check_is_string(data, 1);
        data.error(data.value_to_string(1));
        return false;
    }

    bool assert_func(Data& data)
    {
        check_arg_count(data, 2);
        check_is_bool(data, 1);
        check_is_string(data, 2);
        if(!data.value_to_bool(1))
        {
            std::stringstream ss;
            ss << "Assertion failed: " << data.value_to_string(2);
            data.error(ss.str());
        }
        return false;
    }

    void load_lib_basic(Data& data)
    {
        data.create_value("print");
        data.push_cppfunction(print);
        data.set_value("print", true);

        data.create_value("to_string");
        data.push_cppfunction(to_string);
        data.set_value("to_string", true);

        data.create_value("format");
        data.push_cppfunction(format);
        data.set_value("format", true);

        data.create_value("error");
        data.push_cppfunction(error);
        data.set_value("error", true);

        data.create_value("assert");
        data.push_cppfunction(assert_func);
        data.set_value("assert", true);
    }
} // namespace sourdo
