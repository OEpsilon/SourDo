#include "Value.hpp"

namespace sourdo
{
    Value::Value()
    {
        type = Type::_NULL;
        value = Null();
    }

    Value::Value(Null new_value)
    {
        type = Type::_NULL;
        value = new_value;
    }

    Value::Value(int new_value)
    {
        type = Type::INT;
        value = new_value;
    }

    Value::Value(float new_value)
    {
        type = Type::FLOAT;
        value = new_value;
    }
    
    Value::Value(const std::string& new_value)
    {
        type = Type::STRING;
        value = new_value;
    }

    Value::Value(const Value& new_value)
    {
        type = new_value.type;
        value = new_value.value;
    }

    Value::Value(Value&& new_value)
    {
        type = new_value.type;
        value = new_value.value;

        new_value.type = Type::_NULL;
        new_value.value = Null();
    }

    Value& Value::operator=(const Value& new_value)
    {
        type = new_value.type;
        value = new_value.value;
        return *this;
    }
    
    Value& Value::operator=(Value&& new_value)
    {
        type = new_value.type;
        value = new_value.value;

        new_value.type = Type::_NULL;
        new_value.value = Null();
        return *this;
    }

    Value& Value::operator=(Null new_value)
    {
        type = Type::_NULL;
        value = new_value;
        return *this;
    }
    
    Value& Value::operator=(int new_value)
    {
        type = Type::INT;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(float new_value)
    {
        type = Type::FLOAT;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(const std::string& new_value)
    {
        type = Type::STRING;
        value = new_value;
        return *this;
    }

    Value::Type Value::get_type()
    {
        return type;
    }
} // namespace sourdo
