#pragma once

#include "GCObject.hpp"
#include "../Bytecode/Bytecode.hpp"

namespace sourdo
{
    struct SourDoFunction : public GCObject
    {
        SourDoFunction(uint64_t parameter_count, const std::optional<std::string>& class_context, const Bytecode& bytecode)
            : parameter_count(parameter_count), class_context(class_context), bytecode(bytecode)
        {
        }
        
        virtual ~SourDoFunction() = default;

        uint64_t parameter_count;
        std::optional<std::string> class_context;
        Bytecode bytecode;

        void on_garbage_collected(Data::Impl* data) final
        {
        }
    };
} // namespace sourdo
