#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <map>
#include <sstream>

#include "ConsoleColors.hpp"
#include "Datatypes/Value.hpp"

namespace sourdo {
    class Data::Impl
    {
    public:
        Data::Impl* parent = nullptr;

        // Used to keep temporary values.
        std::vector<sourdo::Value> stack;
        // Used to store named values.
        std::map<std::string, sourdo::Value> symbol_table;

        void set_symbol(const std::string& index, const sourdo::Value& value)
        {
            if(symbol_table.find(index) == symbol_table.end())
            {
                Data::Impl* current_parent = parent;
                while(current_parent != nullptr)
                {
                    if(current_parent->symbol_table.find(index) != parent->symbol_table.end())
                    {
                        current_parent->symbol_table[index] = value;
                        return;
                    }
                    current_parent = parent->parent;
                }
            }
            symbol_table[index] = value;
        }

        Value* get_symbol(const std::string& index)
        {
            if(symbol_table.find(index) == symbol_table.end())
            {
                if(parent)
                {
                    return parent->get_symbol(index);
                }
                return nullptr;
            }
            return &(symbol_table[index]);
        }

        sourdo::Value& index_stack(int index)
        {
            assert(index != 0);
            assert(std::abs(index) <= stack.size());

            if(index < 0)
            {
                return stack[stack.size() + index];
            }
            return stack[index - 1];
        }
    };
} // namespace sourdo
