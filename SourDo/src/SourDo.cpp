#include "SourDo/SourDo.hpp"

#include <string>
#include <iostream>
#include <fstream>
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

    SourDoBool sourdo_is_number(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::NUMBER);
    }

    SourDoBool sourdo_is_bool(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        return (sourdo_index_stack(data, index).get_type() == sourdo::Value::Type::BOOL);
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

    SourDoNumber sourdo_to_number(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value& value = sourdo_index_stack(data, index);
        if(value.get_type() != sourdo::Value::Type::NUMBER)
        {
            return 0;
        }
        return value.to_number();
    }

    SourDoBool sourdo_to_bool(sourdo_Data* data, int index)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value& value = sourdo_index_stack(data, index);
        if(value.get_type() != sourdo::Value::Type::BOOL)
        {
            return false;
        }
        return value.to_bool();
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

    void sourdo_push_number(sourdo_Data* data, SourDoNumber value)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(value);
    }

    void sourdo_push_bool(sourdo_Data* data, SourDoBool value)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.push_back(bool(value));
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

    SourDoBool sourdo_get_value(sourdo_Data* data, const char* name)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo::Value* value = sourdo_get_symbol(data, name);
        if(value == nullptr)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << "Variable '" << name << "' is undefined" << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        data->stack.push_back(*value);
        return SOURDO_TRUE;
    }

    void sourdo_set_value(sourdo_Data* data, const char* name)
    {
        SOURDO_DATA_NOT_NULL();
        sourdo_set_symbol(data, name, sourdo_index_stack(data, -1));
    }

    void sourdo_pop(sourdo_Data* data)
    {
        SOURDO_DATA_NOT_NULL();
        data->stack.pop_back();
    }

    SourDoBool sourdo_do_string(sourdo_Data* data, const char* string)
    {
        SOURDO_DATA_NOT_NULL();
        auto[tokens, tok_error] = sourdo::tokenize_string(string, string);
        if(tok_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << tok_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        sourdo::Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << parse_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        auto[result, visit_error] = sourdo::visit_ast(data, ast);
        if(visit_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << visit_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }
        data->stack.push_back(result);

        return SOURDO_TRUE;
    }
    
    SourDoBool sourdo_do_file(sourdo_Data* data, const char* file_path)
    {
        SOURDO_DATA_NOT_NULL();
        std::ifstream file;
        file.open(file_path);
        if(!file.is_open())
        {
            std::stringstream ss;
            ss << "Could not open file '" << file_path << "'";
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        std::stringstream file_text;
        file_text << file.rdbuf();

        file.close();


        auto[tokens, tok_error] = sourdo::tokenize_string(file_text.str(), file_path);
        if(tok_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << tok_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        sourdo::Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << parse_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }

        auto[result, visit_error] = sourdo::visit_ast(data, ast);
        if(visit_error)
        {
            std::stringstream ss;
            ss << sourdo::COLOR_RED << visit_error.value() << sourdo::COLOR_DEFAULT << std::flush;
            sourdo_push_string(data, ss.str().c_str());
            return SOURDO_FALSE;
        }
        data->stack.push_back(result);

        return SOURDO_TRUE;
    }

}
