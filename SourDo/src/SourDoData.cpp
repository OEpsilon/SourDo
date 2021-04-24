#include "SourDo/SourDo.hpp"

#include "SourDoData.hpp"
#include "Interpreter.hpp"

#include <fstream>

namespace sourdo
{
    Data::Data()
    {
        impl = new Impl();
    }

    Data::~Data()
    {
        delete impl;
    }

    bool Data::value_is_number(int index)
    {
        return (impl->index_stack(index).get_type() == Value::Type::NUMBER);
    }

    bool Data::value_is_bool(int index)
    {
        return (impl->index_stack(index).get_type() == Value::Type::BOOL);
    }

    bool Data::value_is_string(int index)
    {
        return (impl->index_stack(index).get_type() == Value::Type::STRING);
    }
    
    bool Data::value_is_null(int index)
    {
        return (impl->index_stack(index).get_type() == Value::Type::_NULL);
    }

    Number Data::value_to_number(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != Value::Type::NUMBER)
        {
            return 0;
        }
        return value.to_number();
    }

    bool Data::value_to_bool(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != Value::Type::BOOL)
        {
            return false;
        }
        return value.to_bool();
    }
    
    std::string Data::value_to_string(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != Value::Type::STRING)
        {
            return "";
        }
        return value.to_string().c_str();
    }

    void Data::push_number(Number value)
    {
        impl->stack.push_back(value);
    }

    void Data::push_bool(bool value)
    {
        impl->stack.push_back(bool(value));
    }

    void Data::push_string(const std::string& value)
    {
        impl->stack.push_back(value);
    }

    void Data::push_null()
    {
        impl->stack.push_back(Null());
    }

    Result Data::get_value(const std::string& name, bool protected_mode_enabled)
    {
        Value* value = impl->get_symbol(name);
        if(value == nullptr)
        {
            if(protected_mode_enabled)
            {
                std::stringstream ss;
                ss << COLOR_RED << "Variable '" << name << "' is undefined" << COLOR_DEFAULT << std::flush;
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(name);
        }

        impl->stack.push_back(*value);
        return Result::SUCCESS;
    }

    void Data::set_value(const std::string& name)
    {
        impl->set_symbol(name, impl->index_stack(-1));
    }

    void Data::pop()
    {
        impl->stack.pop_back();
    }

    Result Data::do_string(const std::string& string)
    {
        auto[tokens, tok_error] = tokenize_string(string, string);
        if(tok_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << tok_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << parse_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        VisitorReturn result = visit_ast(impl, ast);
        if(result.error_message)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.error_message.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }
        impl->stack.push_back(result.result);

        return Result::SUCCESS;
    }
    
    Result Data::do_file(const std::string& file_path)
    {
        std::ifstream file;
        file.open(file_path);
        if(!file.is_open())
        {
            std::stringstream ss;
            ss << "Could not open file '" << file_path << "'";
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        std::stringstream file_text;
        file_text << file.rdbuf();

        file.close();


        auto[tokens, tok_error] = tokenize_string(file_text.str(), file_path);
        if(tok_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << tok_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        Parser parser;
        auto[ast, parse_error] = parser.parse_tokens(tokens);
        if(parse_error)
        {
            std::stringstream ss;
            ss << COLOR_RED << parse_error.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        VisitorReturn result = visit_ast(impl, ast);
        if(result.error_message)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.error_message.value() << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        return Result::SUCCESS;
    }
} // namespace sourdo
