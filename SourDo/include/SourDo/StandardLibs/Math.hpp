#pragma once

namespace sourdo
{
    class Data;
    
    bool max(Data& data);
    bool min(Data& data);
    bool clamp(Data& data);
    bool abs(Data& data);

    bool acos(Data& data);
    bool csin(Data& data);
    bool atan(Data& data);
    
    bool cos(Data& data);
    bool sin(Data& data);
    bool tan(Data& data);

    bool cosh(Data& data);
    bool sinh(Data& data);
    bool tanh(Data& data);

    bool deg2reg(Data& data);
    bool rad2deg(Data& data);

    bool floor(Data& data);
    bool ceil(Data& data);
    bool sqrt(Data& data);

    void load_lib_math(Data& data);
} // namespace sourdo
