#pragma once

#include "Bytecode.hpp"
#include "../SourDoData.hpp"

#include <vector>
#include <optional>
#include <string>

namespace sourdo
{
    class VirtualMachine
    {
    public:
        std::optional<std::string> run_bytecode(const Bytecode& bytecode, Data::Impl* data);
    private:
        bool is_function = false;
    };
} // namespace sourdo
