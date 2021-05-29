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

    static void mark_table(Table* table);

    static void mark_class_type(ClassType* type)
    {
        type->marked = true;
        if(type->super)
        {
            mark_class_type(type->super);
        }
    }

    static void mark_object(Object* object)
    {
        object->marked = true;
        if(object->type)
        {
            object->type->marked = true;
        }

        for(auto& [k, symbol] : object->props)
        {
            if(symbol.get_type() == ValueType::OBJECT)
            {
                mark_object(symbol.to_object());
            }
            else if(symbol.get_type() == ValueType::TABLE)
            {
                mark_table(symbol.to_table());
            }
            else if(symbol.get_type() == ValueType::CPP_OBJECT)
            {
                symbol.to_cpp_object()->marked = true;
                mark_class_type(symbol.to_cpp_object()->type);
            }
            else if(symbol.get_type() == ValueType::SOURDO_FUNCTION)
            {
                symbol.to_sourdo_function()->marked = true;
            }
        }
    }

    static void mark_table(Table* table)
    {
        table->marked = true;
        for(auto& [k, symbol] : table->keys)
        {
            if(symbol.get_type() == ValueType::OBJECT)
            {
                mark_object(symbol.to_object());
            }
            else if(symbol.get_type() == ValueType::TABLE)
            {
                mark_table(symbol.to_table());
            }
            else if(symbol.get_type() == ValueType::CPP_OBJECT)
            {
                symbol.to_cpp_object()->marked = true;
                mark_class_type(symbol.to_cpp_object()->type);
            }
            else if(symbol.get_type() == ValueType::SOURDO_FUNCTION)
            {
                symbol.to_sourdo_function()->marked = true;
            }
        }
    }

    void GarbageCollector::mark(Data::Impl* data)
    {
        for(auto& ref : GlobalData::references)
        {
            if(ref.get_type() == ValueType::OBJECT)
            {
                mark_object(ref.to_object());
            }
            else if(ref.get_type() == ValueType::TABLE)
            {
                mark_table(ref.to_table());
            }
            else if(ref.get_type() == ValueType::CPP_OBJECT)
            {
                ref.to_cpp_object()->marked = true;
                mark_class_type(ref.to_cpp_object()->type);
            }
            else if(ref.get_type() == ValueType::SOURDO_FUNCTION)
            {
                ref.to_sourdo_function()->marked = true;
            }
        }

        Data::Impl* current_data = data;
        while(current_data != nullptr)
        {
            for(auto& [k, symbol] : current_data->symbol_table)
            {
                if(symbol.get_type() == ValueType::OBJECT)
                {
                    mark_object(symbol.to_object());
                }
                else if(symbol.get_type() == ValueType::TABLE)
                {
                    mark_table(symbol.to_table());
                }
                else if(symbol.get_type() == ValueType::CPP_OBJECT)
                {
                    symbol.to_cpp_object()->marked = true;
                    mark_class_type(symbol.to_cpp_object()->type);
                }
                else if(symbol.get_type() == ValueType::SOURDO_FUNCTION)
                {
                    symbol.to_sourdo_function()->marked = true;
                }
            }

            for(auto& symbol : current_data->stack)
            {
                if(symbol.get_type() == ValueType::OBJECT)
                {
                    mark_object(symbol.to_object());
                }
                else if(symbol.get_type() == ValueType::TABLE)
                {
                    mark_table(symbol.to_table());
                }
                else if(symbol.get_type() == ValueType::CPP_OBJECT)
                {
                    symbol.to_cpp_object()->marked = true;
                }
                else if(symbol.get_type() == ValueType::SOURDO_FUNCTION)
                {
                    symbol.to_sourdo_function()->marked = true;
                }
            }
            current_data = current_data->parent;
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
