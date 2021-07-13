#include "GarbageCollector.hpp"

#include <iostream>

#include "GlobalData.hpp"
#include "Datatypes/Function.hpp"


namespace sourdo
{
    std::vector<GCObject*> GarbageCollector::objects;

    void GarbageCollector::add_object(GCObject* object)
    {
        objects.emplace_back(object);
    }

    void GarbageCollector::collect_garbage(Data::Impl* data)
    {
        mark(data);
        sweep();
    }

    static void mark_class(ClassType* class_type);

    static void mark_gc_object(Value& ref);

    static void mark_object(Object* object)
    {
        object->marked = true;
        for(auto&[k, prop] : object->props)
        {
            mark_gc_object(prop.val);
        }
        mark_class(object->type);
    }

    static void mark_table(Table* table)
    {
        table->marked = true;
        for(auto&[k, v] : table->keys)
        {
            mark_gc_object(v);
        }
    }

    static void mark_class(ClassType* class_type)
    {
        class_type->marked = true;
        if(class_type->initializer)
        {
            class_type->initializer->marked = true;
        }

        for(auto&[k, method] : class_type->methods)
        {
            mark_gc_object(method.val);
        }

        for(auto&[k, setter] : class_type->setters)
        {
            mark_gc_object(setter.val);
        }

        for(auto&[k, getter] : class_type->getters)
        {
            mark_gc_object(getter.val);
        }

        for(auto&[k, method] : class_type->class_methods)
        {
            mark_gc_object(method.val);
        }
    }

    static void mark_cpp_object(CppObject* object)
    {
        object->marked = true;
        for(auto&[k, ref] : object->props)
        {
            mark_gc_object(ref.val);
        }
        mark_class(object->type);
    }

    static void mark_gc_object(Value& ref)
    {
        switch(ref.get_type())
        {
            case ValueType::SOURDO_FUNCTION:
                ref.to_sourdo_function()->marked = true;
                break;
            case ValueType::OBJECT:
                mark_object(ref.to_object());
                break;
            case ValueType::TABLE:
                mark_table(ref.to_table());
                break;
            case ValueType::CLASS_TYPE:
                mark_class(ref.to_class());
                break;
            case ValueType::CPP_OBJECT:
                mark_cpp_object(ref.to_cpp_object());
                break;
            default:
                break;
        }
    }

    void GarbageCollector::mark(Data::Impl* data)
    {
        while(data != nullptr)
        {
            for(auto& ref : GlobalData::references)
            {
                mark_gc_object(ref);
            }

            for(auto&[k, ref] : data->symbol_table)
            {
                mark_gc_object(ref.val);
            }

            for(auto& ref : data->stack)
            {
                mark_gc_object(ref);
            }
            data = data->parent;
        }
    }

    void GarbageCollector::sweep()
    {
        auto& obj = objects;
        auto it = objects.begin();
        while(it != objects.end())
        {
            if((*it)->marked)
            {
                (*it)->marked = false;
                it++;
            }
            else
            {
                delete (*it);
                it = objects.erase(it);
            }
        }
    }
} // namespace sourdo
