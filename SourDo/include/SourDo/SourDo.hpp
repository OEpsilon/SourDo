#pragma once

#include <string>
#include <exception>

namespace sourdo
{
    using SourDoError = std::runtime_error;

    enum class Result
    {
        SUCCESS = 0,
        RUNTIME_ERROR,
    };

    using Number = double;
    
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

        bool value_is_number(int index);
        bool value_is_bool(int index);
        bool value_is_string(int index);

        /**
         * @brief Checks if a value on the stack is null.
         * 
         * @param index The index of the value on the stack to check.
         * 
         * @returns Returns true if the value is null.
         */
        bool value_is_null(int index);

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
         * @brief Pushes 'null' onto the top of the stack.
         */
        void push_null();

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
         */
        void set_value(const std::string& name);

        /**
         * @brief Pops the value on the top of the stack.
         */
        void pop();

        /**
         * @brief Not for use outside of library code.
         */
        Impl* get_impl() { return impl; }
    private:
        Impl* impl;
    };
}
