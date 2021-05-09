#pragma once

#include "SourDoData.hpp"

#include <vector>

namespace sourdo
{
    class GarbageCollector
    {
    public:
        static void add_object(GCObject* object);
        static void collect_garbage(Data::Impl* data);
    private:
        static std::vector<GCObject*> objects;

        static void mark(Data::Impl* data);
        static void sweep();
    };
} // namespace sourdo
