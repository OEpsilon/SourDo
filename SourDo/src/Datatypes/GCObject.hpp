#pragma once

#include <cstddef>

namespace sourdo
{
    struct GCObject
    {
        virtual ~GCObject() = default;
        
        bool marked = true;

        static void* operator new(size_t size);

        virtual void on_garbage_collected() = 0;
    };
} // namespace sourdo
