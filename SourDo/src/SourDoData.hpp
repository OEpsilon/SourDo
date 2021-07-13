#pragma once

#include "SourDo/SourDo.hpp"

#include <vector>
#include <optional>
#include <map>
#include <sstream>

#include "Datatypes/Value.hpp"

namespace sourdo {
    struct Symbol
    {
        Symbol()
        {
        }

        Symbol(bool readonly, Value val)
            : readonly(readonly), val(val)
        {
        }
        bool readonly = false;
        Value val;
    };

    enum class SetSymbolResult
    {
        SUCCESS,
        SYM_NOT_FOUND,
        SYM_READONLY,
    };

    class Data::Impl
    {
    public:
        Data::Impl* parent = nullptr;

        // Used to keep temporary values.
        std::vector<Value> stack;
        // Used to store named values.
        std::map<std::string, Symbol> symbol_table;

        SetSymbolResult set_symbol(const std::string& index, const Value& value)
        {
            if(symbol_table.find(index) == symbol_table.end())
            {
                Data::Impl* current_parent = parent;
                while(current_parent != nullptr)
                {
                    if(current_parent->symbol_table.find(index) != parent->symbol_table.end())
                    {
                        if(symbol_table[index].readonly)
                        {
                            return SetSymbolResult::SYM_READONLY;
                        }
                        current_parent->symbol_table[index].val = value;

                        return SetSymbolResult::SUCCESS;
                    }
                    current_parent = parent->parent;
                }
                return SetSymbolResult::SYM_NOT_FOUND;
            }
            if(symbol_table[index].readonly)
            {
                return SetSymbolResult::SYM_READONLY;
            }

            symbol_table[index].val = value;
            return SetSymbolResult::SUCCESS;
        }

        std::optional<Value> get_symbol(const std::string& index)
        {
            if(symbol_table.find(index) == symbol_table.end())
            {
                if(parent)
                {
                    return parent->get_symbol(index);
                }
                return {};
            }
            return symbol_table[index].val;
        }

        Value& index_stack(int index)
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
