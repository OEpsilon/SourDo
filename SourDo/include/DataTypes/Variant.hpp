#pragma once

#include "Errors/Error.hpp"

#include <ostream>
#include <optional>
#include <tuple>

namespace SourDo {
    class NullType
    {
    public:
        NullType() = default;
    };

    struct VariantResult;

    class Variant 
    {
    public:
    
        enum class Type
        {
            Null,
            Int,
            Float,
        };

        Variant();

        Variant(const Variant& other);
        Variant(NullType value);
        Variant(int value);
        Variant(double value);

        Variant& operator=(const Variant& other);
        Variant& operator=(NullType other);
        Variant& operator=(int other);
        Variant& operator=(double other);
        
        VariantResult operator+(const Variant& other);
        VariantResult operator-(const Variant& other);
        VariantResult operator*(const Variant& other);
        VariantResult operator/(const Variant& other);
        VariantResult power(const Variant& other);

        void set_position(const Position& pos)
        {
            position = pos;
        }

        Type get_type() const
        {
            return type;
        }

        friend std::ostream& operator<<(std::ostream& os, const Variant& variant);
    private:
        Type type;
        Position position;

        union
        {
            NullType data_null;
            int data_int;
            double data_float;
        };
    };

    struct VariantResult
    {
        Variant result;
        std::optional<Error> error;
    };

    std::ostream& operator<<(std::ostream& os, const Variant& variant);
    std::ostream& operator<<(std::ostream& os, const Variant::Type& type);
} // namespace SourDo
