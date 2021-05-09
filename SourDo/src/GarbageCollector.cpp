#include "GarbageCollector.hpp"

#include <iostream>

#include "GlobalData.hpp"

namespace sourdo
{
    std::vector<GCObject*> GarbageCollector::objects;

    static void print_objects(const std::string& message, const std::vector<GCObject*>& objects)
    {
        std::cout << message << "\n{\n";
        for(auto obj : objects)
        {
            std::cout << std::boolalpha << "    [" << obj << "]: " << "{.marked = " << obj->marked << "},\n";
        }
        std::cout << "}\n\n";
    }

    void GarbageCollector::add_object(GCObject* object)
    {
        objects.emplace_back(object);
    }

    void GarbageCollector::collect_garbage(Data::Impl* data)
    {
        mark(data);
        //print_objects("After mark: ", objects);
        sweep();
        //print_objects("After sweep: ", objects);
    }

    static void mark_child_objects(Object* object)
    {
        for(auto& [k, symbol] : object->keys)
        {
            if(symbol.get_type() == ValueType::OBJECT)
            {
                symbol.to_object()->marked = true;
                mark_child_objects(symbol.to_object());
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
                ref.to_object()->marked = true;
                mark_child_objects(ref.to_object());
            }
            else if(ref.get_type() == ValueType::CPP_OBJECT)
            {
                ref.to_cpp_object()->marked = true;
                if(ref.to_cpp_object()->prototype)
                {
                    mark_child_objects(ref.to_cpp_object()->prototype);
                }
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
                    symbol.to_object()->marked = true;
                    mark_child_objects(symbol.to_object());
                }
                else if(symbol.get_type() == ValueType::CPP_OBJECT)
                {
                    symbol.to_cpp_object()->marked = true;
                    if(symbol.to_cpp_object()->prototype)
                    {
                        symbol.to_cpp_object()->prototype->marked = true;
                        mark_child_objects(symbol.to_cpp_object()->prototype);
                    }
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
                    symbol.to_object()->marked = true;
                    mark_child_objects(symbol.to_object());
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
