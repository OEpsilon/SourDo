#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <memory>
#include <variant>
#include <string>

namespace sourdo 
{
    struct StatementListNode;
    
    struct Null
    {
    };

    struct SourDoFunction
    {
        SourDoFunction(std::vector<std::string> parameters, std::shared_ptr<StatementListNode> statements)
            : parameters(parameters), statements(statements)
        {
        }
        std::vector<std::string> parameters;
        std::shared_ptr<StatementListNode> statements;
    };

    class Value
    {
    public:
        Value();
        Value(Null new_value);
        Value(double new_value);
        Value(bool new_value);
        Value(const std::string& new_value);
        Value(std::shared_ptr<SourDoFunction> new_value);
        Value(const CppFunction& new_value);
        
        Value(const Value& new_value);
        Value(Value&& new_value);

        Value& operator=(const Value& new_value);
        Value& operator=(Value&& new_value);

        Value& operator=(Null new_value);
        Value& operator=(double new_value);
        Value& operator=(bool new_value);
        Value& operator=(const std::string& new_value);
        Value& operator=(std::shared_ptr<SourDoFunction> new_value);
        Value& operator=(const CppFunction& new_value);

        ValueType get_type();

        double& to_number() 
        { 
            return std::get<double>(value);
        }
        
        bool& to_bool()
        {
            return std::get<bool>(value); 
        }

        std::string& to_string() 
        { 
            if(type == ValueType::NUMBER)
            {
                value = std::to_string(std::get<double>(value));
                return std::get<std::string>(value);
            }
            return std::get<std::string>(value); 
        }

        std::shared_ptr<SourDoFunction>& to_sourdo_function()
        {
            return std::get<std::shared_ptr<SourDoFunction>>(value);
        }

        CppFunction& to_cpp_function()
        {
            return std::get<CppFunction>(value);
        }
    private:
        ValueType type;

        std::variant<Null, double, bool, std::string, std::shared_ptr<SourDoFunction>, CppFunction> value;
    };

} // namespace SourDo