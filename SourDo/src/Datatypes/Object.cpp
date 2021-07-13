#include "Value.hpp"

#include "Value.hpp"
#include "../SourDoData.hpp"
#include "Function.hpp"

#include <sstream>

namespace sourdo
{
    void Object::on_garbage_collected(Data::Impl* data)
    {
        Value* sym = find_method("__gc");
        if(sym)
        {
            if(sym->get_type() == ValueType::SOURDO_FUNCTION
                || sym->get_type() == ValueType::CPP_FUNCTION)
            {
                Data scope;
                scope.get_impl()->parent = data;
                scope.get_impl()->stack.emplace_back(*sym);
                scope.get_impl()->stack.emplace_back(this);
                scope.call_function(1, true);
            }
        }
    }
} // namespace sourdo
