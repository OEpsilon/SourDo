#include "SourDo/Errors.hpp"

#include <sstream>

namespace sourdo
{
    void check_arg_count(Data& data, uint32_t expected_count)
    {
        if(data.get_size() != expected_count) 
        {
            std::stringstream ss;
            ss << "Function being called expected " 
                        << expected_count;
            // Grammar!
            if(expected_count == 1)
            {
                ss << " argument but ";
            }
            else
            {
                ss << " arguments but ";
            }

            ss << data.get_size();

            // More Grammar!
            if(data.get_size() == 1)
            {
                ss << " was given";
            }
            else
            {
                ss << " were given";
            }
            data.error(ss.str());
        }
    }
    
    void check_is_number(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::NUMBER)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected a number";
            data.error(ss.str());
        }
    }

    void check_is_bool(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::BOOL)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected a bool";
            data.error(ss.str());
        }
    }

    void check_is_string(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::STRING)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected a string";
            data.error(ss.str());
        }
    }

    void check_is_function(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::SOURDO_FUNCTION 
            || data.get_value_type(arg) != ValueType::CPP_FUNCTION)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected a function";
            data.error(ss.str());
        }
    }

    void check_is_null(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::_NULL)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected null";
            data.error(ss.str());
        }
    }

    void check_is_object(Data& data, int arg)
    {
        if(data.get_value_type(arg) != ValueType::OBJECT)
        {
            std::stringstream ss;
            ss << "Argument #" << arg << ": Expected an object";
            data.error(ss.str());
        }
    }
} // namespace sourdo
