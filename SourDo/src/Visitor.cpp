#include "Interpreter.hpp"

#include <cmath>
#include <sstream>
#include <iostream>

#include "SourDoData.hpp"
#include "VisitorTypeFunctions/StringFunctions.hpp"
#include "GarbageCollector.hpp"

#include "SourDo/Errors.hpp"

namespace sourdo
{
    static std::ostream& operator<<(std::ostream& os, const ValueType& type)
    {
        switch(type)
        {
            case ValueType::NUMBER:
                os << "Number";
                break;
            case ValueType::BOOL:
                os << "Bool";
                break;
            case ValueType::STRING:
                os << "String";
                break;
            case ValueType::SOURDO_FUNCTION:
            case ValueType::CPP_FUNCTION:
                os << "Function";
                break;
            case ValueType::_NULL:
                os << "Null";
                break;
            case ValueType::OBJECT:
                os << "Object";
                break;
            case ValueType::CPP_OBJECT:
                os << "CppObject";
                break;
        }
        return os;
    }
    
    static VisitorReturn get_magic_method(const Value& left_value, const std::string& name, const Position& position)
    {
        VisitorReturn return_value;
        if(left_value.to_object()->keys.find(name) != left_value.to_object()->keys.end())
        {
            return_value.result = left_value.to_object()->keys[name];
            return return_value;
        }
        
        Value prototype_table = left_value.to_object()->keys["__prototype"];
        while(prototype_table.get_type() != ValueType::_NULL)
        {
            if(prototype_table.to_object()->keys.find(name) != prototype_table.to_object()->keys.end())
            {
                return_value.result = prototype_table.to_object()->keys[name];
                return return_value;
            }

            prototype_table = prototype_table.to_object()->keys["__prototype"];
        }

        std::stringstream ss;
        ss << position << "Object does not have magic method '" << name << "'";
        return_value.error_message = ss.str();
        return return_value;
    }
    
    static VisitorReturn call_function(Data::Impl* data, const Value& callee, const std::vector<Value>& arguments, const Position& position)
    {
        VisitorReturn return_value;
        if(callee.get_type() == ValueType::SOURDO_FUNCTION)
        {
            SourDoFunction* func_value = callee.to_sourdo_function();
            if(arguments.size() != func_value->parameters.size())
            {
                std::stringstream ss;
                ss << position << "Function being called expected " 
                            << func_value->parameters.size(); 
                // Grammar!
                if(func_value->parameters.size() == 1)
                {
                    ss << " argument but ";
                }
                else
                {
                    ss << " arguments but ";
                }

                ss << arguments.size();

                // More Grammar!
                if(arguments.size() == 1)
                {
                    ss << " was given";
                }
                else
                {
                    ss << " were given";
                }
                return_value.error_message = ss.str();
                return return_value;
            }

            Data func_scope;
            func_scope.get_impl()->parent = data;
            
            for(int i = 0; i < func_value->parameters.size(); i++)
            {
                func_scope.get_impl()->symbol_table[func_value->parameters[i]] = arguments[i];
            }

            return_value = visit_ast(func_scope.get_impl(), func_value->statements);
            if(return_value.error_message)
            {
                return return_value;
            }
            if(return_value.is_breaking)
            {
                std::stringstream ss;
                ss << return_value.break_position << "Cannot use 'break' outside of a loop";
                return_value.error_message = ss.str();
            }
            else if(return_value.is_continuing)
            {
                std::stringstream ss;
                ss << return_value.break_position << "Cannot use 'continue' outside of a loop";
                return_value.error_message = ss.str();
            }
            return_value.is_function_return = false;
        }
        else if(callee.get_type() == ValueType::CPP_FUNCTION)
        {
            CppFunction func_value = callee.to_cpp_function();

            Data func_scope;
            func_scope.get_impl()->parent = data;

            func_scope.get_impl()->stack.reserve(arguments.size());
            
            for(int i = 0; i < arguments.size(); i++)
            {
                func_scope.get_impl()->stack.push_back(arguments[i]);
            }

            try
            {
                if(func_value(func_scope))
                {
                    return_value.result = func_scope.get_impl()->index_stack(-1);
                }
                else
                {
                    return_value.result = Null();
                }
            }
            catch(const SourDoError& error)
            {
                std::stringstream ss;
                ss << position << error.what();
                return_value.error_message = ss.str();
            }
        }
        else
        {
            std::stringstream ss;
            ss << position << "Cannot call " << callee.get_type();
            return_value.error_message = ss.str();
        }
        return return_value;
    }

    static VisitorReturn perform_binary_operation(Data::Impl* data, const Value& left_value, const Value& right_value, Token::Type operation, const Position& position)
    {
        VisitorReturn return_value;
        if(left_value.get_type() == ValueType::CPP_OBJECT)
        {
            if(left_value.to_cpp_object()->prototype)
            {
                return perform_binary_operation(data, left_value.to_cpp_object()->prototype, 
                        right_value, operation, position);
            }
            std::stringstream ss;
            ss << position << "CppObject does not have an Object interface";
            return_value.error_message = ss.str();
        }

        switch(operation)
        {
            case Token::Type::ADD:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() + right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::STRING 
                        && right_value.get_type() == ValueType::STRING)
                {
                    return_value.result = left_value.to_string() + right_value.to_string();
                }
                else if(left_value.get_type() == ValueType::OBJECT)
                {
                    VisitorReturn add_method = get_magic_method(left_value, "__add", position);
                    if(add_method.error_message)
                    {
                        return add_method;
                    }
                    return_value = call_function(data, add_method.result, {left_value.to_object(), right_value}, position);
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform addition with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::SUB:
            {   
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() - right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::OBJECT)
                {
                    VisitorReturn sub_method = get_magic_method(left_value, "__sub", position);
                    if(sub_method.error_message)
                    {
                        return sub_method;
                    }
                    return_value = call_function(data, sub_method.result, {left_value.to_object(), right_value}, position);
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform subtraction with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::MUL:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() * right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::OBJECT)
                {
                    VisitorReturn mul_method = get_magic_method(left_value, "__mul", position);
                    if(mul_method.error_message)
                    {
                        return mul_method;
                    }
                    return_value = call_function(data, mul_method.result, {left_value.to_object(), right_value}, position);
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform multiplication with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::DIV:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    if(right_value.to_number() == 0)
                    {
                        std::stringstream ss;
                        ss << position << "Cannot divide a number by zero";
                        return_value.error_message = ss.str();
                        break;
                    }
                    return_value.result = left_value.to_number() / right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::OBJECT)
                {
                    VisitorReturn div_method = get_magic_method(left_value, "__div", position);
                    if(div_method.error_message)
                    {
                        return div_method;
                    }
                    return_value = call_function(data, div_method.result, {left_value.to_object(), right_value}, position);
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform division with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::POW:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = std::pow(left_value.to_number(), right_value.to_number());
                }
                else if(left_value.get_type() == ValueType::OBJECT)
                {
                    VisitorReturn add_method = get_magic_method(left_value, "__pow", position);
                    if(add_method.error_message)
                    {
                        return add_method;
                    }
                    return_value = call_function(data, add_method.result, {right_value}, position);
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform exponentiation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LESS_THAN:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() < right_value.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::GREATER_THAN:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() > right_value.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LESS_EQUAL:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() <= right_value.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::GREATER_EQUAL:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() >= right_value.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::EQUAL:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() == right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::BOOL 
                        && right_value.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.to_bool() == right_value.to_bool();
                }
                else if(left_value.get_type() == ValueType::STRING 
                        && right_value.get_type() == ValueType::STRING)
                {
                    return_value.result = left_value.to_string() == right_value.to_string();
                }
                else if(left_value.get_type() == ValueType::SOURDO_FUNCTION
                        && right_value.get_type() == ValueType::SOURDO_FUNCTION)
                {
                    return_value.result = left_value.to_sourdo_function() == right_value.to_sourdo_function();
                }
                else if(left_value.get_type() == ValueType::CPP_FUNCTION
                        && right_value.get_type() == ValueType::CPP_FUNCTION)
                {
                    return_value.result = left_value.to_cpp_function() == right_value.to_cpp_function();
                }
                else if(left_value.get_type() == ValueType::OBJECT
                        && right_value.get_type() == ValueType::OBJECT)
                {
                    return_value.result = left_value.to_object() == right_value.to_object();
                }
                else if(left_value.get_type() == ValueType::_NULL)
                {
                    switch(right_value.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                        case ValueType::OBJECT:
                        case ValueType::CPP_OBJECT:
                            return_value.result = false;
                            break;
                        case ValueType::_NULL:
                            return_value.result = true;
                            break;
                    }
                }
                else if(right_value.get_type() == ValueType::_NULL)
                {
                    switch(left_value.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                        case ValueType::OBJECT:
                        case ValueType::CPP_OBJECT:
                            return_value.result = false;
                            break;
                        case ValueType::_NULL:
                            return_value.result = true;
                            break;
                    }
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::NOT_EQUAL:
            {
                if(left_value.get_type() == ValueType::NUMBER 
                        && right_value.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.to_number() != right_value.to_number();
                }
                else if(left_value.get_type() == ValueType::BOOL 
                        && right_value.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.to_bool() != right_value.to_bool();
                }
                else if(left_value.get_type() == ValueType::STRING 
                        && right_value.get_type() == ValueType::STRING)
                {
                    return_value.result = left_value.to_string() != right_value.to_string();
                }
                else if(left_value.get_type() == ValueType::OBJECT
                        && right_value.get_type() == ValueType::OBJECT)
                {
                    return_value.result = left_value.to_object() != right_value.to_object();
                }
                else if(left_value.get_type() == ValueType::_NULL)
                {
                    switch(right_value.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                        case ValueType::OBJECT:
                        case ValueType::CPP_OBJECT:
                            return_value.result = true;
                            break;
                        case ValueType::_NULL:
                            return_value.result = false;
                            break;
                    }
                }
                else if(right_value.get_type() == ValueType::_NULL)
                {
                    switch(left_value.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                        case ValueType::OBJECT:
                        case ValueType::CPP_OBJECT:
                            return_value.result = true;
                            break;
                        case ValueType::_NULL:
                            return_value.result = false;
                            break;
                    }
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform comparison operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LOGIC_OR:
            {
                if(left_value.get_type() == ValueType::BOOL 
                        && right_value.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.to_bool() || right_value.to_bool();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform logical operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LOGIC_AND:
            {
                if(left_value.get_type() == ValueType::BOOL 
                        && right_value.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.to_bool() && right_value.to_bool();
                }
                else
                {
                    std::stringstream ss;
                    ss << position << "Cannot perform logical operation with types " << left_value.get_type() << " and " << right_value.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            default:
            {
                break;
            }
        }
        return return_value;
    }

    static VisitorReturn perform_index_operation(Data::Impl* data, const Value& base, const Value& attribute, const Position& base_position, const Position& attribute_position)
    {
        VisitorReturn return_value;
        
        if(base.get_type() == ValueType::STRING)
        {
            if(attribute.get_type() == ValueType::STRING)
            {
                if(attribute.to_string() == "length")
                {
                    return_value.result = string_length;
                }
                else
                {
                    std::stringstream ss;
                    ss << attribute_position << "'" << attribute.to_string() << "' does not exist in String";
                    return_value.error_message = ss.str();
                }
            }
            else if(attribute.get_type() == ValueType::NUMBER)
            {
                int num = attribute.to_number();
                if(num < 0)
                {
                    std::stringstream ss;
                    ss << attribute_position << "Index is less than 0";
                    return_value.error_message = ss.str();
                }
                else if(num > base.to_string().size())
                {
                    std::stringstream ss;
                    ss << attribute_position << "Index is greater than the string length";
                    return_value.error_message = ss.str();
                }
                else
                {
                    return_value.result = std::string(1, base.to_string()[num]);
                }
            }
            else
            {
                std::stringstream ss;
                ss << attribute_position << "Cannot index String with value of type " << attribute.get_type();
                return_value.error_message = ss.str();
            }
        }
        else if(base.get_type() == ValueType::OBJECT)
        {
            if(base.to_object()->keys.find(attribute) != base.to_object()->keys.end())
            {
                return_value.result = base.to_object()->keys[attribute];
            }
            else
            {
                Value prototype_table = base.to_object()->keys["__prototype"];
                while(prototype_table.get_type() != ValueType::_NULL)
                {
                    // We already confirmed the type of "__prototype" when it was set, so we can treat it as an object.
                    if(prototype_table.to_object()->keys.find(attribute) != prototype_table.to_object()->keys.end())
                    {
                        return_value.result = prototype_table.to_object()->keys[attribute];
                        return return_value;
                    }

                    prototype_table = prototype_table.to_object()->keys["__prototype"];
                }
                
                std::stringstream ss;
                ss << attribute_position << "Index does not exist in object";
                return_value.error_message = ss.str();
            }
        }
        else if(base.get_type() == ValueType::CPP_OBJECT)
        {
            if(base.to_cpp_object()->prototype)
            {
                return perform_index_operation(data, base.to_cpp_object()->prototype, 
                        attribute, base_position, attribute_position);
            }
            std::stringstream ss;
            ss << attribute_position << "CppObject does not have an Object interface";
            return_value.error_message = ss.str();
        }
        else
        {
            std::stringstream ss;
            ss << base_position << "Cannot index value of type " << base.get_type(); 
            return_value.error_message = ss.str();
        }
        return return_value;
    }

    static VisitorReturn visit_statement_list_node(Data::Impl* data, std::shared_ptr<StatementListNode> node)
    {
        VisitorReturn return_value;
        for(auto& stmt : node->statements)
        {
            VisitorReturn stmt_return = visit_ast(data, stmt);
            if(stmt_return.error_message || stmt_return.is_breaking 
                || stmt_return.is_continuing)
            {
                GarbageCollector::collect_garbage(data);
                return stmt_return;
            }
            else if(stmt_return.is_function_return)
            {
                return stmt_return;
            }
        }
        GarbageCollector::collect_garbage(data);
        return return_value;
    }

    static VisitorReturn visit_if_node(Data::Impl* data, std::shared_ptr<IfNode> node)
    {
        VisitorReturn return_value;
        for(auto& if_case : node->cases)
        {
            VisitorReturn condition = visit_ast(data, if_case.condition);
            if(condition.error_message)
            {
                return condition;
            }
            if(condition.result.get_type() != ValueType::BOOL)
            {
                std::stringstream ss;
                ss << if_case.condition->position << "condition does not result in a bool";
                return_value.error_message = ss.str();
                return return_value;
            }
            if(condition.result.to_bool())
            {
                Data if_scope;
                if_scope.get_impl()->parent = data;
                VisitorReturn statements = visit_ast(if_scope.get_impl(), if_case.statements);
                return statements;
            }
        }
        if(node->else_case)
        {
            Data else_scope;
            else_scope.get_impl()->parent = data;
            return_value = visit_ast(else_scope.get_impl(), node->else_case);
        }
        return return_value;
    }

    static VisitorReturn visit_for_node(Data::Impl* data, std::shared_ptr<ForNode> node)
    {
        VisitorReturn return_value;
        Data for_scope;
        for_scope.get_impl()->parent = data;
        VisitorReturn initializer = visit_ast(for_scope.get_impl(), node->initializer);
        if(initializer.error_message)
        {
            return initializer;
        }

        while(true)
        {
            VisitorReturn condition = visit_ast(for_scope.get_impl(), node->condition);
            if(condition.error_message)
            {
                return condition;
            }
            if(condition.result.get_type() != ValueType::BOOL)
            {
                std::stringstream ss;
                ss << node->condition->position << "Condition does not result in a bool";
                return_value.error_message = ss.str();
                break;
            }
            if(!condition.result.to_bool())
            {
                break;
            }

            VisitorReturn statements = visit_ast(for_scope.get_impl(), node->statements);
            if(statements.error_message || statements.is_function_return || statements.is_breaking)
            {
                statements.is_breaking = false;
                return statements;
            }

            VisitorReturn increment = visit_ast(for_scope.get_impl(), node->increment);
            if(increment.error_message)
            {
                return increment;
            }
        }

        return return_value;
    }

    static VisitorReturn visit_while_node(Data::Impl* data, std::shared_ptr<WhileNode> node)
    {
        VisitorReturn return_value;
        
        Data while_scope;
        while_scope.get_impl()->parent = data;

        while(true)
        {
            VisitorReturn condition = visit_ast(while_scope.get_impl(), node->condition);
            if(condition.error_message)
            {
                return condition;
            }

            if(condition.result.get_type() != ValueType::BOOL)
            {
                std::stringstream ss;
                ss << node->condition->position << "Condition does not result in a bool";
                return_value.error_message = ss.str();
                break;
            }

            if(!condition.result.to_bool())
            {
                break;
            }

            VisitorReturn statements = visit_ast(while_scope.get_impl(), node->statements);
            if(statements.error_message || statements.is_function_return || statements.is_breaking)
            {
                statements.is_breaking = false;
                return statements;
            }
        }

        return return_value;
    }

    static VisitorReturn visit_loop_node(Data::Impl* data, std::shared_ptr<LoopNode> node)
    {
        VisitorReturn return_value;

        Data loop_scope;
        loop_scope.get_impl()->parent = data;

        while(true)
        {
            VisitorReturn statements = visit_ast(loop_scope.get_impl(), node->statements);
            if(statements.error_message || statements.is_function_return || statements.is_breaking)
            {
                statements.is_breaking = false;
                return statements;
            }
        }

        return return_value;
    }

    static VisitorReturn visit_var_declaration_node(Data::Impl* data, std::shared_ptr<VarDeclarationNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn var_value;
        if(node->initializer)
        {
            var_value = visit_ast(data, node->initializer);
        }
        else
        {
            var_value.result = Null();
        }
        
        if(var_value.error_message)
        {
            return var_value;
        }

        if(data->symbol_table.find(node->name_tok.value) != data->symbol_table.end())
        {
            std::stringstream ss;
            ss << node->position << "'" << node->name_tok.value << "' is already defined";
            return_value.error_message = ss.str();
        }
        else
        {
            data->symbol_table[node->name_tok.value] = var_value.result;
            return_value.result = var_value.result;
        }

        return return_value;
    }

    static VisitorReturn visit_assignment_node(Data::Impl* data, std::shared_ptr<AssignmentNode> node)
    {
        auto do_assignment = [](Data::Impl* data, AssignmentNode::Operation op, const Position& position, Value& initial_value, Value& new_value) -> VisitorReturn
        {
            VisitorReturn return_value;
            switch(op)
            {
                case AssignmentNode::Operation::NONE:
                {
                    return_value.result = new_value;
                    break;
                }
                case AssignmentNode::Operation::ADD:
                {
                    return_value = perform_binary_operation(data, initial_value, new_value, Token::Type::ADD, position);
                    break;
                }
                case AssignmentNode::Operation::SUB:
                {
                    return_value = perform_binary_operation(data, initial_value, new_value, Token::Type::SUB, position);
                    break;
                }
                case AssignmentNode::Operation::MUL:
                {
                    return_value = perform_binary_operation(data, initial_value, new_value, Token::Type::MUL, position);
                    break;
                }
                case AssignmentNode::Operation::DIV:
                {
                    return_value = perform_binary_operation(data, initial_value, new_value, Token::Type::DIV, position);
                    break;
                }
            }
            return return_value;
        };

        VisitorReturn return_value;
        VisitorReturn new_value = visit_ast(data, node->new_value);
        if(new_value.error_message)
        {
            return new_value;
        }

        Value initial_value;

        if(node->assignee->type == Node::Type::INDEX_NODE)
        {
            auto index_node = std::static_pointer_cast<IndexNode>(node->assignee);
            VisitorReturn base = visit_ast(data, index_node->base);
            if(base.error_message)
            {
                return base;
            }

            VisitorReturn attribute = visit_ast(data, index_node->attribute);
            if(attribute.error_message)
            {
                return attribute;
            }

            if(base.result.get_type() == ValueType::STRING)
            {
                if(attribute.result.get_type() == ValueType::NUMBER)
                {
                    std::stringstream ss;
                    ss << index_node->position << "The result from indexing a string cannot be assigned to";
                    return_value.error_message = ss.str();
                }
                else
                {
                    std::stringstream ss;
                    ss << index_node->position << "Expected a number";
                    return_value.error_message = ss.str();
                }
                return return_value;
            }
            else if(base.result.get_type() == ValueType::OBJECT)
            {
                return_value = do_assignment(data, node->op, node->position, 
                        base.result.to_object()->keys[attribute.result], new_value.result);
                if(return_value.error_message)
                {
                    return return_value;
                }
                if(attribute.result == "__prototype" &&
                    (return_value.result.get_type() != ValueType::OBJECT 
                        && return_value.result.get_type() != ValueType::_NULL))
                {
                    std::stringstream ss;
                    ss << index_node->attribute->position << "Key '__prototype' must be of type Object or Null";
                    return_value.error_message = ss.str();
                    return return_value;
                }
                base.result.to_object()->keys[attribute.result] = return_value.result;
            }
            else
            {
                std::stringstream ss;
                ss << node->position << "Cannot index value of type " << base.result.get_type();
                return_value.error_message = ss.str();
            }
            return return_value;
        }
        
        Token name_tok = std::static_pointer_cast<IdentifierNode>(node->assignee)->name_tok;
        if(data->get_symbol(name_tok.value) == nullptr)
        {
            std::stringstream ss;
            ss << name_tok.position << "'" << name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            initial_value = *(data->get_symbol(name_tok.value));
            return_value = do_assignment(data, node->op, node->position, initial_value, new_value.result);
            if(return_value.error_message)
            {
                return return_value;
            }
            data->set_symbol(name_tok.value, return_value.result);
        }

        return return_value;
    }

    static VisitorReturn visit_func_node(Data::Impl* data, std::shared_ptr<FuncNode> node)
    {
        VisitorReturn return_value;
        
        std::vector<std::string> parameters;
        parameters.reserve(node->parameters.size());
        for(auto& param : node->parameters)
        {
            if(data->get_symbol(param) != nullptr)
            {
                std::stringstream ss;
                ss << node->position << "A value called '" << param << "' is already defined outside of the function";
                return_value.error_message = ss.str();
                return return_value;
            }
            if(std::find(parameters.begin(), parameters.end(), param) != parameters.end())
            {
                std::stringstream ss;
                ss << node->position << "A parameter called '" << param << "' is already defined in this function";
                return_value.error_message = ss.str();
                return return_value;
            }
            parameters.emplace_back(param);
        }
        return_value.result = new SourDoFunction(parameters, node->statements);

        return return_value;
    }

    static VisitorReturn visit_return_node(Data::Impl* data, std::shared_ptr<ReturnNode> node)
    {
        VisitorReturn return_value = visit_ast(data, node->return_value);
        return_value.is_function_return = true;
        return_value.break_position = node->position;
        return return_value;
    }

    static VisitorReturn visit_identifier_node(Data::Impl* data, std::shared_ptr<IdentifierNode> node)
    {
        VisitorReturn return_value;

        sourdo::Value* var_value = data->get_symbol(node->name_tok.value);
        if(var_value == nullptr)
        {
            std::stringstream ss;
            ss << node->position << "'" << node->name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            return_value.result = *var_value;
        }
        return return_value;
    }

    static VisitorReturn visit_unary_op_node(Data::Impl* data, std::shared_ptr<UnaryOpNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn operand_value = visit_ast(data, node->operand);
        if(operand_value.error_message)
        {
            return return_value;
        }

        switch(node->op_token.type)
        {
            case Token::Type::ADD:
            {
                if(operand_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = operand_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform unary '+' operation with type " << operand_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::SUB:
            {
                if(operand_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = -operand_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform unary '-' operation with type " << operand_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::LOGIC_NOT:
            {
                if(operand_value.result.get_type() == ValueType::BOOL)
                {
                    return_value.result = !operand_value.result.to_bool();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform logical negation with type " << operand_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            default:
            {
                break;
            }
        }

        return return_value;
    }
    
    static VisitorReturn visit_binary_op_node(Data::Impl* data, std::shared_ptr<BinaryOpNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn left_value = visit_ast(data, node->left_operand);
        if(left_value.error_message)
        {
            return left_value;
        }

        VisitorReturn right_value = visit_ast(data, node->right_operand);
        if(right_value.error_message)
        {
            return right_value;
        }

        return_value = perform_binary_operation(data, left_value.result, right_value.result, node->op_token.type, node->position);

        return return_value;
    }

    static VisitorReturn visit_call_node(Data::Impl* data, std::shared_ptr<CallNode> node)
    {
        VisitorReturn callee = visit_ast(data, node->callee);
        if(callee.error_message)
        {
            return callee;
        }

        std::vector<Value> arguments;
        arguments.reserve(node->arguments.size());
        for(auto& arg : node->arguments)
        {
            VisitorReturn arg_value = visit_ast(data, arg);
            if(arg_value.error_message)
            {
                return arg_value;
            }
            arguments.emplace_back(arg_value.result);
        }
        
        return call_function(data, callee.result, arguments, node->position);
    }

    static VisitorReturn visit_index_node(Data::Impl* data, std::shared_ptr<IndexNode> node)
    {
        VisitorReturn base = visit_ast(data, node->base);
        if(base.error_message)
        {
            return base;
        }
        VisitorReturn attribute = visit_ast(data, node->attribute);
        if(attribute.error_message)
        {
            return attribute;
        }

        return perform_index_operation(data, base.result, attribute.result, 
                node->base->position, node->attribute->position);
    }

    static VisitorReturn visit_index_call_node(Data::Impl* data, std::shared_ptr<IndexCallNode> node)
    {
        VisitorReturn return_value;

        VisitorReturn base_value = visit_ast(data, node->base);
        if(base_value.error_message)
        {
            return base_value;
        }

        VisitorReturn callee_value = visit_ast(data, node->callee);
        if(callee_value.error_message)
        {
            return callee_value;
        }

        VisitorReturn callee = perform_index_operation(data, base_value.result, callee_value.result, 
                node->base->position, node->callee->position);
        if(callee.error_message)
        {
            return callee;
        }
        
        std::vector<Value> arguments;
        arguments.reserve(node->arguments.size() + 1);
        arguments.emplace_back(base_value.result);
        for(auto& arg : node->arguments)
        {
            VisitorReturn arg_value = visit_ast(data, arg);
            if(arg_value.error_message)
            {
                return arg_value;
            }
            arguments.emplace_back(arg_value.result);
        }

        return call_function(data, callee.result, arguments, node->callee->position);
    }

    VisitorReturn visit_ast(Data::Impl* data, std::shared_ptr<Node> node)
    {
        VisitorReturn return_value;
        switch(node->type)
        {
            case Node::Type::STATEMENT_LIST_NODE:
            {
                return_value = visit_statement_list_node(data, std::static_pointer_cast<StatementListNode>(node));
                break;
            }
            case Node::Type::IF_NODE:
            {
                return_value = visit_if_node(data, std::static_pointer_cast<IfNode>(node));
                break;
            }
            case Node::Type::FOR_NODE:
            {
                return_value = visit_for_node(data, std::static_pointer_cast<ForNode>(node));
                break;
            }
            case Node::Type::WHILE_NODE:
            {
                return_value = visit_while_node(data, std::static_pointer_cast<WhileNode>(node));
                break;
            }
            case Node::Type::LOOP_NODE:
            {
                return_value = visit_loop_node(data, std::static_pointer_cast<LoopNode>(node));
                break;
            }
            case Node::Type::VAR_DECLARATION_NODE:
            {
                return_value = visit_var_declaration_node(data, std::static_pointer_cast<VarDeclarationNode>(node));
                break;
            }
            case Node::Type::ASSIGNMENT_NODE:
            {
                return_value = visit_assignment_node(data, std::static_pointer_cast<AssignmentNode>(node));
                break;
            }
            case Node::Type::FUNC_NODE:
            {
                return_value = visit_func_node(data, std::static_pointer_cast<FuncNode>(node));
                break;
            }
            case Node::Type::RETURN_NODE:
            {
                return_value = visit_return_node(data, std::static_pointer_cast<ReturnNode>(node));
                break;
            }
            case Node::Type::BREAK_NODE:
            {
                auto break_node = std::static_pointer_cast<ReturnNode>(node);
                return_value.is_breaking = true;
                return_value.break_position = break_node->position;
                break;
            }
            case Node::Type::CONTINUE_NODE:
            {
                auto continue_node = std::static_pointer_cast<ReturnNode>(node);
                return_value.is_continuing = true;
                return_value.break_position = continue_node->position;
                break;
            }
            case Node::Type::CALL_NODE:
            {
                return_value = visit_call_node(data, std::static_pointer_cast<CallNode>(node));
                break;
            }
            case Node::Type::INDEX_NODE:
            {
                return_value = visit_index_node(data, std::static_pointer_cast<IndexNode>(node));
                break;
            }
            case Node::Type::INDEX_CALL_NODE:
            {
                return_value = visit_index_call_node(data, std::static_pointer_cast<IndexCallNode>(node));
                break;
            }
            case Node::Type::UNARY_OP_NODE:
            {
                return_value = visit_unary_op_node(data, std::static_pointer_cast<UnaryOpNode>(node));
                break;
            }
            case Node::Type::BINARY_OP_NODE:
            {
                return_value = visit_binary_op_node(data, std::static_pointer_cast<BinaryOpNode>(node));
                break;
            }
            case Node::Type::NUMBER_NODE:
            {
                auto number_node = std::static_pointer_cast<NumberNode>(node);
                return_value.result = std::stod(number_node->value.value);
                break;
            }
            case Node::Type::STRING_NODE:
            {
                auto string_node = std::static_pointer_cast<StringNode>(node);
                return_value.result = string_node->value.value;
                break;
            }
            case Node::Type::BOOL_NODE:
            {
                auto bool_node = std::static_pointer_cast<BoolNode>(node);
                return_value.result = bool_node->value.value == "true";
                break;
            }
            case Node::Type::NULL_NODE:
            {
                return_value.result = Null(); 
                break;
            }
            case Node::Type::IDENTIFIER_NODE:
            {
                return_value = visit_identifier_node(data, std::static_pointer_cast<IdentifierNode>(node));
                break;
            }
            case Node::Type::OBJECT_LITERAL_NODE:
            {
                auto object_node = std::static_pointer_cast<ObjectLiteralNode>(node);
                std::unordered_map<Value, Value> keys;
                for(auto[k, v] : object_node->keys)
                {
                    VisitorReturn key = visit_ast(data, k);
                    if(key.error_message)
                    {
                        return key;
                    }

                    VisitorReturn value = visit_ast(data, v);
                    if(value.error_message)
                    {
                        return value;
                    }

                    if(key.result == "__prototype" &&
                        (value.result.get_type() != ValueType::OBJECT && value.result.get_type() != ValueType::_NULL))
                    {
                        std::stringstream ss;
                        ss << v->position << "Key '__prototype' must be of type Object or Null";
                        return_value.error_message = ss.str();
                        return return_value;
                    }

                    keys[key.result] = value.result;
                }
                if(keys.find("__prototype") == keys.end())
                {
                    keys["__prototype"] = Null();
                }
                return_value.result = new Object(std::move(keys));
                break;
            }
            default:
            {
                // Just warns me that I missed something.
                throw std::logic_error("A node hasn't been implemented yet!");
                break;
            }
        }
        return return_value;
    }
} // namespace sourdo
