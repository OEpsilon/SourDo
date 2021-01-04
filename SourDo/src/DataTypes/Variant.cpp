#include "DataTypes/Variant.hpp"

namespace SourDo {
    Variant::Variant()
    {
        type = Type::Null;
        data_null = NullType();
    }

    Variant::Variant(const Variant& other)
    {
        type = other.type;
        switch(other.type)
        {
            case Type::Null:
                data_null = other.data_null;
                break;
            case Type::Int:
                data_int = other.data_int;
                break;
            case Type::Float:
                data_float = other.data_float;
                break;
        }
    }

    Variant::Variant(NullType value)
    {
        type = Type::Null;
        data_null = value;
    }

    Variant::Variant(int value)
    {
        type = Type::Int;
        data_int = value;
    }

    Variant::Variant(double value)
    {
        type = Type::Float;
        data_float = value;
    }

    Variant& Variant::operator=(const Variant& other)
    {
        type = other.type;
        switch(other.type)
        {
            case Type::Null:
                data_null = other.data_null;
                break;
            case Type::Int:
                data_int = other.data_int;
                break;
            case Type::Float:
                data_float = other.data_float;
                break;
        }
        return *this;
    }

    Variant& Variant::operator=(NullType other)
    {
        type = Type::Null;
        data_null = other;
        return *this;
    }

    Variant& Variant::operator=(int other)
    {
        type = Type::Int;
        data_int = other;
        return *this;
    }

    Variant& Variant::operator=(double other)
    {
        type = Type::Float;
        data_float = other;
        return *this;
    }

    Variant Variant::operator+(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return Variant(data_int + other.data_int);
            }
            else if(other.type == Type::Float)
            {
                return Variant(data_int + other.data_float);
            } 
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return Variant(data_float + other.data_float);
            }
            else if(other.type == Type::Int)
            {
                return Variant(data_float + other.data_int);
            }
        }
        return Variant();
    }

    Variant Variant::operator-(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return Variant(data_int - other.data_int);
            }
            else if(other.type == Type::Float)
            {
                return Variant(data_int - other.data_float);
            }
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return Variant(data_float - other.data_float);
            }
            else if(other.type == Type::Int)
            {
                return Variant(data_float - other.data_int);
            }
        }
        return Variant();
    }

    std::ostream& operator<<(std::ostream& os, const Variant& variant)
    {
        switch(variant.type)
        {
            case Variant::Type::Null:
                os << variant.data_null;
                break;
            case Variant::Type::Int:
                os << variant.data_int;
                break;
            case Variant::Type::Float:
                os << variant.data_float;
                break;
        }
        return os;
    }
}
