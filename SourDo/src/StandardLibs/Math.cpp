#include "SourDo/StandardLibs/Math.hpp"

#include "SourDo/SourDo.hpp"

#include "SourDo/Errors.hpp"

#include <cmath>

namespace sourdo
{
    bool max(Data& data)
    {
        arg_count_error(data, 2);
        is_number_error(data, 1);
        is_number_error(data, 2);
        data.push_number(std::max(data.value_to_number(1), data.value_to_number(2)) );
        return true;
    }

    bool min(Data& data)
    {
        arg_count_error(data, 2);
        is_number_error(data, 1);
        is_number_error(data, 2);
        data.push_number(std::min(data.value_to_number(1), data.value_to_number(2)) );
        return true;
    }

    bool clamp(Data& data)
    {
        arg_count_error(data, 3);
        is_number_error(data, 1);
        is_number_error(data, 2);
        is_number_error(data, 3);
        data.push_number(std::clamp(data.value_to_number(1), data.value_to_number(2), data.value_to_number(3)) );
        return true;
    }

    bool abs(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::abs(data.value_to_number(1)));
        return true;
    }

    bool acos(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::acos(data.value_to_number(1)));
        return true;
    }

    bool asin(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::asin(data.value_to_number(1)));
        return true;
    }

    bool atan(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::atan(data.value_to_number(1)));
        return true;
    }
    
    bool cos(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::cos(data.value_to_number(1)));
        return true;
    }

    bool sin(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::sin(data.value_to_number(1)));
        return true;
    }

    bool tan(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::tan(data.value_to_number(1)));
        return true;
    }

    bool cosh(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::cosh(data.value_to_number(1)));
        return true;
    }

    bool sinh(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::sinh(data.value_to_number(1)));
        return true;
    }

    bool tanh(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::tanh(data.value_to_number(1)));
        return true;
    }

    bool deg2rad(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(data.value_to_number(1) * (M_PI / 180) );
        return true;
    }

    bool rad2deg(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(data.value_to_number(1) * 180 * M_PI );
        return true;
    }

    bool floor(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::floor(data.value_to_number(1)));
        return true;
    }

    bool ceil(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::ceil(data.value_to_number(1)));
        return true;
    }

    bool sqrt(Data& data)
    {
        arg_count_error(data, 1);
        is_number_error(data, 1);
        data.push_number(std::sqrt(data.value_to_number(1)));
        return true;
    }

    void load_lib_math(Data& data)
    {
        data.create_value("pi");
        data.push_number(M_PI);
        data.set_value("pi", true);

        data.create_value("max");
        data.push_cppfunction(max);
        data.set_value("max", true);

        data.create_value("min");
        data.push_cppfunction(min);
        data.set_value("min", true);

        data.create_value("clamp");
        data.push_cppfunction(clamp);
        data.set_value("clamp", true);

        data.create_value("abs");
        data.push_cppfunction(abs);
        data.set_value("abs", true);

        data.create_value("abs");
        data.push_cppfunction(abs);
        data.set_value("abs", true);

        data.create_value("acos");
        data.push_cppfunction(acos);
        data.set_value("acos", true);

        data.create_value("asin");
        data.push_cppfunction(asin);
        data.set_value("asin", true);

        data.create_value("atan");
        data.push_cppfunction(atan);
        data.set_value("atan", true);

        data.create_value("cos");
        data.push_cppfunction(cos);
        data.set_value("cos", true);

        data.create_value("sin");
        data.push_cppfunction(sin);
        data.set_value("sin", true);

        data.create_value("tan");
        data.push_cppfunction(tan);
        data.set_value("tan", true);

        data.create_value("cosh");
        data.push_cppfunction(cosh);
        data.set_value("cosh", true);

        data.create_value("sinh");
        data.push_cppfunction(sinh);
        data.set_value("sinh", true);

        data.create_value("tanh");
        data.push_cppfunction(tanh);
        data.set_value("tanh", true);

        data.create_value("deg2rad");
        data.push_cppfunction(deg2rad);
        data.set_value("deg2rad", true);

        data.create_value("rad2deg");
        data.push_cppfunction(rad2deg);
        data.set_value("rad2deg", true);

        data.create_value("floor");
        data.push_cppfunction(floor);
        data.set_value("floor", true);

        data.create_value("ceil");
        data.push_cppfunction(ceil);
        data.set_value("ceil", true);

        data.create_value("sqrt");
        data.push_cppfunction(sqrt);
        data.set_value("sqrt", true);
    }
} // namespace sourdo

