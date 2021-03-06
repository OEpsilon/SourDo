#pragma once

#include <string>
#include <functional>
#include <exception>

namespace sourdo
{
    using SourDoError = std::runtime_error;

    enum class Result
    {
        SUCCESS = 0,
        RUNTIME_ERROR,
    };

    enum class ValueType
    {
        _NULL,
        NUMBER,
        BOOL,
        STRING,
        SOURDO_FUNCTION,
        CPP_FUNCTION,
        VALUE_REF,
        TABLE,
        OBJECT,
        CLASS_TYPE,
        CPP_OBJECT,
    };
    
    class Data;

    using Number = double;
    using CppFunction = bool(*)(Data&);
    using GCRef = int;
    struct SourDoFunction;
    
    /**
     * @brief Represents and holds the data of a scope in a SourDo Program. 
     *      
     * @note The highest scope is called 'module' and is controlled by the library user.
     */
    class Data
    {
    public:
        class Impl;
    
        Data();
        ~Data();
        
        Result do_string(const std::string& string);
        Result do_file(const std::string& file_path);

        void* create_cpp_object(size_t size);
        void* check_cpp_object(int index, const std::string& name);
        void* test_cpp_object(int index, const std::string& name);
        void set_cpp_object_type(int index, const std::string& name);

        GCRef create_ref(int index);
        void remove_ref(GCRef ref);
        void push_ref(GCRef ref);
        bool is_ref_valid(GCRef ref);

        /**
         * @brief Pushes onto the stack a new table. 
         */
        void create_table();

        /**
         * @brief Pushes onto the stack the value at the given key in the table. 
         * The new value is the value on the top of the stack, and
         * the key is the value just below top of the stack. Both are popped afterwards.
         * 
         * @param object_index The index that the table is located at.
         * 
         * @param protected_mode_enabled If true, returns an error code instead of throwing an exception.
         * 
         * @throws SourDoError Thrown if there is an error when calling the function and 'protected_mode_enabled' is false.
         */
        Result table_set(int object_index, bool protected_mode_enabled = false);
        
        /**
         * @brief Pushes onto the stack the value at the given key in the table. 
         * The key is the value on the top of the stack and is popped afterwards.
         * 
         * @param object_index The index that the table is located at.
         * 
         * @param protected_mode_enabled If true, returns an error code instead of throwing an exception.
         * 
         * @throws SourDoError Thrown if there is an error when calling the function and 'protected_mode_enabled' is false.
         */
        Result table_get(int object_index, bool protected_mode_enabled = false);

        /**
         * @brief Gets the type of the value at the given index. 
         * 
         * @param index The index of the value on the stack to check.
         */
        ValueType get_value_type(int index);

        /**
         * @brief Converts a value to a number.
         * @param index The index of the value on the stack to be converted.
         * 
         * @returns Returns the value converted to a number if the value is a number. Otherwise '0' is returned.
         */
        Number value_to_number(int index);

        /**
         * @brief Converts a value to a bool.
         * @param index The index of the value on the stack to be converted.
         * 
         * @returns Returns the value converted to a bool if the value is a bool. Otherwise 'false' is returned.
         */
        bool value_to_bool(int index);

        /**
         * @brief Converts the value at 'index' to a string. If the value is not a string, then an empty string is returned.
         */
        std::string value_to_string(int index);
        
        /**
         * @brief Pushes 'value' onto the top of the stack.
         */
        void push_number(Number value);

        /**
         * @brief Pushes 'value' onto the top of the stack.
         */
        void push_bool(bool value);

        /**
         * @brief Pushes 'value' onto the top of the stack.
         */
        void push_string(const std::string& value);

        /**
         * @brief Pushes 'value' onto the top of the stack.
         */
        void push_cppfunction(const CppFunction& value);

        /**
         * @brief Pushes 'null' onto the top of the stack.
         */
        void push_null();

        /**
         * @brief Pops 'arg_count' number of values off the top of the stack and calls the function below the arguments while popping it from the stack.
         * 
         * @param arg_count The number of arguments that are passed to the function.
         * @param protected_mode_enabled If true, returns an error code instead of throwing an exception.
         * 
         * @throws SourDoError Thrown if there is an error when calling the function and 'protected_mode_enabled' is false.
         */
        Result call_function(uint32_t arg_count, bool protected_mode_enabled = false);

        uint32_t get_sourdo_func_param_count(int index);

        /**
         * @brief Creates a value in the symbol table if it doesn't already exist.
         * 
         * @param name The name of the value to be returned.
         * 
         */
        void create_value(const std::string& name);

        /**
         * @brief Creates a value in the symbol table if it doesn't already exist.
         *      If the value is newly created, then it is made constant. 
         * 
         * @param name The name of the value to be returned.
         * 
         */
        void create_constant(const std::string& name);

        /**
         * @brief Pushes a value in the symbol table onto the top of the stack.
         * 
         * @param name The name of the value to be returned.
         * @param protected_mode_enabled If true, returns an error code instead of throwing an exception.
         * 
         * @throws SourDoError Thrown if 'name' is not in the symbol table and 'protected_mode_enabled' is false.
         */
        Result get_value(const std::string& name, bool protected_mode_enabled = false);

        /**
         * @brief Pops the value on the top of the stack and sets it to the object in the symbol table called 'name', creating it if not present.
         * 
         * @param name The name of the value to be set
         * 
         * @param protected_mode_enabled If true, returns an error code instead of throwing an exception.
         * 
         * @throws SourDoError Thrown if 'name' is not in the symbol table and 'protected_mode_enabled' is false.
         */
        Result set_value(const std::string& name, bool protected_mode_enabled = false);

        /**
         * @brief Returns the size of the stack.
         */
        uint32_t get_size();

        /**
         * @brief Removes the value at the given index
         */
        void remove(int index);

        /**
         * @brief Pops the value on the top of the stack.
         */
        void pop();

        /**
         * @brief Equivalent to 'throw SourDoError(message)'.
         */
        void error(const std::string& message);

        /**
         * @brief Not for use outside of library code.
         */
        Impl* get_impl() { return impl; }
    private:
        Impl* impl;
    };
}
