#include "SourDo/SourDo.hpp"
#include "SourDo/ConsoleColors.hpp"

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
        if(result.is_breaking)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.break_position << "Cannot use 'break' outside of a loop" << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }
        else if(result.is_continuing)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.break_position << "Cannot use 'continue' outside of a loop" << COLOR_DEFAULT << std::flush;
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
        if(result.is_breaking)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.break_position << "Cannot use 'break' outside of a loop" << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }
        else if(result.is_continuing)
        {
            std::stringstream ss;
            ss << COLOR_RED << result.break_position << "Cannot use 'continue' outside of a loop" << COLOR_DEFAULT << std::flush;
            push_string(ss.str());
            return Result::RUNTIME_ERROR;
        }

        return Result::SUCCESS;
    }
    
    ValueType Data::get_value_type(int index)
    {
        return impl->index_stack(index).get_type();
    }

    Number Data::value_to_number(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::NUMBER)
        {
            return 0;
        }
        return value.to_number();
    }

    bool Data::value_to_bool(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::BOOL)
        {
            return false;
        }
        return value.to_bool();
    }
    
    std::string Data::value_to_string(int index)
    {
        Value& value = impl->index_stack(index);
        if(value.get_type() != ValueType::STRING)
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

    void Data::push_cppfunction(const CppFunction& value)
    {
        impl->stack.push_back(value);
    }

    void Data::push_null()
    {
        impl->stack.push_back(Null());
    }

    Result Data::call_function(uint32_t arg_count, bool protected_mode_enabled)
    {
        Value func = impl->index_stack((-arg_count) - 1);
        if(func.get_type() != ValueType::SOURDO_FUNCTION && func.get_type() != ValueType::CPP_FUNCTION)
        {
            std::stringstream ss;
            ss << COLOR_RED << "Value is not a function" << COLOR_DEFAULT << std::flush;;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        remove(-arg_count - 1);
        std::vector<Value> args;
        args.reserve(arg_count);

        for(int i = -arg_count; i < 0; i++)
        {
            args.push_back(impl->index_stack(i));
            remove(i);
        }

        
        if(func.get_type() == ValueType::SOURDO_FUNCTION)
        {
            std::shared_ptr<SourDoFunction> func_value = func.to_sourdo_function();
            if(args.size() != func_value->parameters.size())
            {
                std::stringstream ss;
                ss << "Function being called expected " << func_value->parameters.size(); 
                
                if(func_value->parameters.size() == 1)
                {
                    ss << " argument but ";
                }
                else
                {
                    ss << " arguments but ";
                }

                ss << args.size();

                if(args.size() == 1)
                {
                    ss << " was given";
                }
                else
                {
                    ss << " were given";
                }
                if(protected_mode_enabled)
                {
                    push_string(ss.str());
                    return Result::RUNTIME_ERROR;
                }
                throw SourDoError(ss.str());
            }

            Data func_scope;
            func_scope.get_impl()->parent = impl;
            for(int i = 0; i < func_value->parameters.size(); i++)
            {
                func_scope.get_impl()->symbol_table[func_value->parameters[i]] = args[i];
            }
            visit_ast(func_scope.get_impl(), func_value->statements);
        }
        else
        {
            Data func_scope;
            func_scope.get_impl()->parent = impl;
            for(int i = 0; i < args.size(); i++)
            {
                func_scope.get_impl()->stack.push_back(args[i]);
            }
            CppFunction func_value = func.to_cpp_function();
            try
            {
                if(func_value(func_scope))
                {
                    impl->stack.push_back(func_scope.impl->index_stack(-1));
                }
                else
                {
                    push_null();
                }
            }
            catch(const SourDoError& error)
            {
                if(protected_mode_enabled)
                {
                    push_string(error.what());
                    return Result::RUNTIME_ERROR;
                }
                throw;
            }
        }
        return Result::SUCCESS;
    }

    void Data::create_value(const std::string& name)
    {
        impl->symbol_table[name] = Null();
    }

    Result Data::get_value(const std::string& name, bool protected_mode_enabled)
    {
        Value* value = impl->get_symbol(name);
        if(value == nullptr)
        {
            std::stringstream ss;
            ss << COLOR_RED << "'" << name << "' is undefined" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }

        impl->stack.push_back(*value);
        return Result::SUCCESS;
    }

    Result Data::set_value(const std::string& name, bool protected_mode_enabled)
    {
        bool result = impl->set_symbol(name, impl->index_stack(-1));
        pop();
        if(!result)
        {
            std::stringstream ss;
            ss << COLOR_RED << "'" << name << "' is undefined" << COLOR_DEFAULT << std::flush;
            if(protected_mode_enabled)
            {
                push_string(ss.str());
                return Result::RUNTIME_ERROR;
            }
            throw SourDoError(ss.str());
        }
        return Result::SUCCESS;
    }

    uint32_t Data::get_size()
    {
        return impl->stack.size();
    }

    void Data::remove(int index)
    {
        assert(index != 0);
        assert(std::abs(index) <= impl->stack.size());

        if(index > 0)
        {
            impl->stack.erase(impl->stack.begin() + (index - 1));
        }
        else
        {
            impl->stack.erase(impl->stack.begin() + (impl->stack.size() + index ) );
        }
    }

    void Data::pop()
    {
        impl->stack.pop_back();
    }

    void Data::error(const std::string& message)
    {
        throw SourDoError(message);
    }
} // namespace sourdo
