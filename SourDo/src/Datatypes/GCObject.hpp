#pragma once

#include <cstddef>

#include "SourDo/SourDo.hpp"

namespace sourdo
{
    struct GCObject
    {
        virtual ~GCObject() = default;
        
        bool marked = true;

        static void* operator new(size_t size);

        virtual void on_garbage_collected(Data::Impl* data) = 0;
    };
} // namespace sourdo
