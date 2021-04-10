#include "SourDo/SourDo.hpp"

#include <string>
#include <iostream>
#include <vector>
#include <cmath>

#include "SourDoData.hpp"

#include "Datatypes/Value.hpp"

#include "Interpreter.hpp"
#include "ConsoleColors.hpp"

extern "C" {
    #define SOURDO_DATA_NOT_NULL() assert(data != nullptr);

    static sourdo::Value& sourdo_index_stack(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();

        assert(index != 0);
        assert(std::abs(index) <= data->stack.size());

        if(index < 0)
        {
            return data->stack[data->stack.size() + index];
        }
        return data->stack[index - 1];
    }

    sourdo_Data* sourdo_data_create()
    {
        return new sourdo_Data();
    }

    void sourdo_data_destroy(sourdo_Data* data)
    {
        delete data;
    }

    SourDoBool sourdo_is_int(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::INT);
    }
    
    SourDoBool sourdo_is_float(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::FLOAT);
    }

    SourDoBool sourdo_is_string(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::STRING);
    }
    
    SourDoBool sourdo_is_null(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::_NULL);
    }

    int sourdo_to_int(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value& value = sourdo_index_stack(data, index);
        if(value.get_type() == sourdo::Value::Type::FLOAT)
        {
            return value.to_float();
        }
        if(value.get_type() != sourdo::Value::Type::INT)
        {
            return 0;
        }
        return value.to_int();
    }
    
    float sourdo_to_float(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value& value = sourdo_index_stack(data, index);
        if(value.get_type() == sourdo::Value::Type::INT)
        {
            return value.to_int();
        }
        else if(value.get_type() != sourdo::Value::Type::FLOAT)
        {
            return 0.0;
        }
        return value.to_float();
    }
    
    const char* sourdo_to_string(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value& value = sourdo_index_stack(data, index);
        if(value.get_type() != sourdo::Value::Type::STRING)
        {
            return "";
        }
        return value.to_string().c_str();
    }

    void sourdo_push_int(sourdo_Data* data, int value)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(value);
    }

    void sourdo_push_float(sourdo_Data* data, float value)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(value);
    }

    void sourdo_push_string(sourdo_Data* data, const char* value)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(std::string(value));
    }

    void sourdo_push_null(sourdo_Data* data)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(sourdo::Null());
    }

    void sourdo_pop(sourdo_Data* data)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.pop_back();
    }


    SourDoBool sourdo_do_string(sourdo_Data* data, const char* string)
    {
        auto[tokens, tok_error] = sourdo::tokenize_string(string);
        if(tok_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << tok_error.value() << sourdo::COLOR_DEFAULT << std::endl;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }
        //std::cout << tokens << std::endl;
        sourdo::Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << parse_error.value() << sourdo::COLOR_DEFAULT << std::endl;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }
        std::cout << ast << std::endl;
        auto[result, visit_error] = sourdo::visit_ast(data, ast);
        if(visit_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << visit_error.value() << sourdo::COLOR_DEFAULT << std::endl;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }
        data->stack.push_back(result);

        return SOURDO_TRUE;
    }

}
