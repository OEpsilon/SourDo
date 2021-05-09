#include "GCObject.hpp"

#include "../GarbageCollector.hpp"

namespace sourdo
{
    void* GCObject::operator new(size_t size)
    {
        void* object = ::operator new(size);
        GarbageCollector::add_object((GCObject*)object);
        return object;
    }
} // namespace sourdo
