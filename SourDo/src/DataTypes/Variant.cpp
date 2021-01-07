#include "DataTypes/Variant.hpp"

#include <cmath>
#include <unordered_map>
#include <sstream>

namespace SourDo {
    std::ostream& operator<<(std::ostream& os, const Variant::Type& type)
    {
        static std::unordered_map<Variant::Type, const char*> strings = {
            {Variant::Type::Null, "null"},
            {Variant::Type::Int, "int"},
            {Variant::Type::Float, "float"},
        };
        os << strings[type];
        return os;
    }

    static std::string operator+(const std::string& string, const Variant::Type& type)
    {
        std::stringstream ss;
        ss << string << type;
        return ss.str();
    }

    Variant::Variant()
    {
        type = Type::Null;
        data_null = NullType();
    }

    Variant::Variant(const Variant& other)
    {
        type = other.type;
        position = other.position;
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
        position = other.position;
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

    VariantResult Variant::operator+(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return {Variant(data_int + other.data_int)};
            }
            else if(other.type == Type::Float)
            {
                return {Variant(data_int + other.data_float)};
            } 
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return {Variant(data_float + other.data_float)};
            }
            else if(other.type == Type::Int)
            {
                return {Variant(data_float + other.data_int)};
            }
        }
        return {NullType()};
    }

    VariantResult Variant::operator-(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return {Variant(data_int - other.data_int)};
            }
            else if(other.type == Type::Float)
            {
                return {Variant(data_int - other.data_float)};
            }
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return {Variant(data_float - other.data_float)};
            }
            else if(other.type == Type::Int)
            {
                return {Variant(data_float - other.data_int)};
            }
        }
        return {NullType()};
    }

    VariantResult Variant::operator*(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return {Variant(data_int * other.data_int)};
            }
            else if(other.type == Type::Float)
            {
                return {Variant(data_int * other.data_float)};
            }
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return {Variant(data_float * other.data_float)};
            }
            else if(other.type == Type::Int)
            {
                return {Variant(data_float * other.data_int)};
            }
        }
        return {NullType()};
    }

    VariantResult Variant::operator/(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                if(other.data_int == 0)
                {
                    return {NullType(), Error("Dividing a number by zero is not allowed", 
                            other.position)};
                }
                return {Variant(data_int / other.data_int)};
            }
            else if(other.type == Type::Float)
            {
                if(other.data_float == 0.0)
                {
                    return {NullType(), Error("Dividing a number by zero is not allowed", 
                            other.position)};
                }
                return {Variant(data_int / other.data_float)};
            }
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                if(other.data_float == 0)
                {
                    return {NullType(), Error("Dividing a number by zero is not allowed", 
                            other.position)};
                }
                return {Variant(data_float / other.data_float)};
            }
            else if(other.type == Type::Int)
            {
                if(other.data_int == 0)
                {
                    return {NullType(), Error("Dividing a number by zero is not allowed", 
                            other.position)};
                }
                return {Variant(data_float / other.data_int)};
            }
        }
        return {NullType()};
    }

    VariantResult Variant::power(const Variant& other)
    {
        if(type == Type::Int)
        {
            if(other.type == Type::Int)
            {
                return {Variant(std::pow(data_int, other.data_int))};
            }
            else if(other.type == Type::Float)
            {
                return {Variant(std::pow(data_int, other.data_float))};
            }
        }
        else if(type == Type::Float)
        {
            if(other.type == Type::Float)
            {
                return {Variant(std::pow(data_float, other.data_float))};
            }
            else if(other.type == Type::Int)
            {
                return {Variant(std::pow(data_float, other.data_int))};
            }
        }
        return {NullType()};
    }

    std::ostream& operator<<(std::ostream& os, const Variant& variant)
    {
        switch(variant.type)
        {
            case Variant::Type::Null:
                os << "null";
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
