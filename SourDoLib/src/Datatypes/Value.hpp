#pragma once

#include <variant>
#include <string>

namespace sourdo 
{
    struct Null
    {
    };

    class Value
    {
    public:
        enum class Type
        {
            _NULL,
            INT,
            FLOAT,
            STRING,
        };

        Value();
        Value(Null new_value);
        Value(int new_value);
        Value(float new_value);
        Value(const std::string& new_value);
        
        Value(const Value& new_value);
        Value(Value&& new_value);

        Value& operator=(const Value& new_value);
        Value& operator=(Value&& new_value);

        Value& operator=(Null new_value);
        Value& operator=(int new_value);
        Value& operator=(float new_value);
        Value& operator=(const std::string& new_value);

        Type get_type();

        int& to_int() 
        { 
            return std::get<int>(value); 
        }

        float& to_float() 
        { 
            return std::get<float>(value); 
        }

        std::string& to_string() 
        { 
            if(type == Type::INT)
            {
                value = std::to_string(std::get<int>(value));
                return std::get<std::string>(value);
            }
            return std::get<std::string>(value); 
        }
    private:
        Type type;

        std::variant<Null, int, float, std::string> value;
    };

} // namespace SourDo