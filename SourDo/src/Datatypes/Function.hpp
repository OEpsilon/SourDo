#pragma once

#include "GCObject.hpp"
#include "../Bytecode/Bytecode.hpp"

namespace sourdo
{
    struct SourDoFunction : public GCObject
    {
        SourDoFunction(uint64_t parameter_count, const Bytecode& bytecode)
            : parameter_count(parameter_count), bytecode(bytecode)
        {
        }
        
        virtual ~SourDoFunction() = default;

        uint64_t parameter_count;
        Bytecode bytecode;

        void on_garbage_collected() final
        {
        }
    };
} // namespace sourdo
