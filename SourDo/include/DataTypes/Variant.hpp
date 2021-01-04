#pragma once

#include <ostream>

namespace SourDo {
    class NullType
    {
    public:
        NullType() = default;
    };

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
    public:
        Variant operator+(const Variant& other);
        Variant operator-(const Variant& other);

        friend std::ostream& operator<<(std::ostream& os, const Variant& variant);
    private:
        Type type;

        union
        {
            NullType data_null;
            int data_int;
            double data_float;
        };
    };

    std::ostream& operator<<(std::ostream& os, const Variant& variant);
} // namespace SourDo
