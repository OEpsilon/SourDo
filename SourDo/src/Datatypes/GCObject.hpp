#pragma once

#include <cstddef>

namespace sourdo
{
    struct GCObject
    {
        virtual ~GCObject() = default;
        
        bool marked = true;

        static void* operator new(size_t size);
    };
} // namespace sourdo
