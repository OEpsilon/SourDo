#include "Value.hpp"

namespace sourdo
{
    Value::Value()
    {
        type = ValueType::_NULL;
        value = Null();
    }

    Value::Value(Null new_value)
    {
        type = ValueType::_NULL;
        value = new_value;
    }

    Value::Value(double new_value)
    {
        type = ValueType::NUMBER;
        value = new_value;
    }

    Value::Value(bool new_value)
    {
        type = ValueType::BOOL;
        value = new_value;
    }
    
    Value::Value(const std::string& new_value)
    {
        type = ValueType::STRING;
        value = new_value;
    }

    Value::Value(const char* new_value)
    {
        type = ValueType::STRING;
        value = new_value;
    }

    Value::Value(SourDoFunction* new_value)
    {
        type = ValueType::SOURDO_FUNCTION;
        value = new_value;
    }

    Value::Value(const CppFunction& new_value)
    {
        type = ValueType::CPP_FUNCTION;
        value = new_value;
    }

    Value::Value(Object* new_value)
    {
        type = ValueType::OBJECT;
        value = new_value;
    }
    
    Value::Value(CppObject* new_value)
    {
        type = ValueType::CPP_OBJECT;
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

        new_value.type = ValueType::_NULL;
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

        new_value.type = ValueType::_NULL;
        new_value.value = Null();
        return *this;
    }

    Value& Value::operator=(Null new_value)
    {
        type = ValueType::_NULL;
        value = new_value;
        return *this;
    }
    
    Value& Value::operator=(double new_value)
    {
        type = ValueType::NUMBER;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(bool new_value)
    {
        type = ValueType::BOOL;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(const std::string& new_value)
    {
        type = ValueType::STRING;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(const char* new_value)
    {
        type = ValueType::STRING;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(SourDoFunction* new_value)
    {
        type = ValueType::SOURDO_FUNCTION;
        value = new_value;
        return *this;
    }
    
    Value& Value::operator=(const CppFunction& new_value)
    {
        type = ValueType::CPP_FUNCTION;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(Object* new_value)
    {
        type = ValueType::OBJECT;
        value = new_value;
        return *this;
    }

    Value& Value::operator=(CppObject* new_value)
    {
        type = ValueType::CPP_OBJECT;
        value = new_value;
        return *this;
    }

    bool Value::operator==(const Value& other) const
    {
        return value == other.value;
    }

    bool Value::operator!=(const Value& other) const
    {
        return value != other.value;
    }

    ValueType Value::get_type() const
    {
        return type;
    }
} // namespace sourdo
