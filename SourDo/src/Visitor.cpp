#include "Interpreter.hpp"

#include <cmath>
#include <sstream>
#include <iostream>

#include "SourDoData.hpp"

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
                os << "Function";
                break;
            case ValueType::CPP_FUNCTION:
                os << "CppFunction";
                break;
            case ValueType::_NULL:
                os << "Null";
                break;
        }
        return os;
    }

    static VisitorReturn visit_statement_list_node(Data::Impl* data, std::shared_ptr<StatementListNode> node)
    {
        VisitorReturn return_value;
        for(auto& stmt : node->statements)
        {
            VisitorReturn stmt_return = visit_ast(data, stmt);
            if(stmt_return.error_message || stmt_return.is_function_return)
            {
                return stmt_return;
            }
        }
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

        if(data->get_symbol(node->name_tok.value) != nullptr)
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

    static VisitorReturn visit_var_assignment_node(Data::Impl* data, std::shared_ptr<VarAssignmentNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn new_value = visit_ast(data, node->new_value);
        if(new_value.error_message)
        {
            return new_value;
        }

        if(data->get_symbol(node->name_tok.value) == nullptr)
        {
            std::stringstream ss;
            ss << node->position << "'" << node->name_tok.value << "' is not defined";
            return_value.error_message = ss.str();
        }
        else
        {
            data->set_symbol(node->name_tok.value, new_value.result);
            return_value.result = data->symbol_table[node->name_tok.value];
        }

        return return_value;
    }

    static VisitorReturn visit_func_declaration_node(Data::Impl* data, std::shared_ptr<FuncDeclarationNode> node)
    {
        VisitorReturn return_value;
        
        if(data->get_symbol(node->name.value) != nullptr)
        {
            std::stringstream ss;
            ss << node->position << "'" << node->name.value << "' is already defined";
            return_value.error_message = ss.str();
        }
        else
        {
            std::vector<std::string> parameters;
            parameters.reserve(node->parameters.size());
            for(auto& param : node->parameters)
            {
                if(data->get_symbol(param.value) != nullptr)
                {
                    std::stringstream ss;
                    ss << node->position << "A value called '" << param.value << "' is already defined outside of the function";
                    return_value.error_message = ss.str();
                    return return_value;
                }
                if(std::find(parameters.begin(), parameters.end(), param.value) != parameters.end())
                {
                    std::stringstream ss;
                    ss << node->position << "A parameter called '" << param.value << "' is already defined in this function";
                    return_value.error_message = ss.str();
                    return return_value;
                }
                parameters.emplace_back(param.value);
            }

            data->symbol_table[node->name.value] = std::make_shared<SourDoFunction>(parameters, node->statements);
        }

        return return_value;
    }

    static VisitorReturn visit_return_node(Data::Impl* data, std::shared_ptr<ReturnNode> node)
    {
        VisitorReturn return_value = visit_ast(data, node->return_value);
        return_value.is_function_return = true;

        return return_value;
    }

    static VisitorReturn visit_var_access_node(Data::Impl* data, std::shared_ptr<VarAccessNode> node)
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

        switch(node->op_token.type)
        {
            case Token::Type::ADD:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() + right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform addition with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::SUB:
            {   
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() - right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform subtraction with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::MUL:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() * right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform multiplication with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::DIV:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    if(right_value.result.to_number() == 0)
                    {
                        std::stringstream ss;
                        ss << node->position << "Cannot divide a number by zero";
                        return_value.error_message = ss.str();
                        break;
                    }
                    return_value.result = left_value.result.to_number() / right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform division with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }
                
                break;
            }
            case Token::Type::POW:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = std::pow(left_value.result.to_number(), right_value.result.to_number());
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform exponentiation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LESS_THAN:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() < right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::GREATER_THAN:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() > right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LESS_EQUAL:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() <= right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::GREATER_EQUAL:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() >= right_value.result.to_number();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::EQUAL:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() == right_value.result.to_number();
                }
                else if(left_value.result.get_type() == ValueType::BOOL 
                        && right_value.result.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.result.to_bool() == right_value.result.to_bool();
                }
                else if(left_value.result.get_type() == ValueType::STRING 
                        && right_value.result.get_type() == ValueType::STRING)
                {
                    return_value.result = left_value.result.to_string() == right_value.result.to_string();
                }
                else if(left_value.result.get_type() == ValueType::SOURDO_FUNCTION
                        && right_value.result.get_type() == ValueType::SOURDO_FUNCTION)
                {
                    return_value.result = left_value.result.to_sourdo_function() == right_value.result.to_sourdo_function();
                }
                else if(left_value.result.get_type() == ValueType::CPP_FUNCTION
                        && right_value.result.get_type() == ValueType::CPP_FUNCTION)
                {
                    return_value.result = left_value.result.to_cpp_function() == right_value.result.to_cpp_function();
                }
                else if(left_value.result.get_type() == ValueType::_NULL)
                {
                    switch(right_value.result.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                            return_value.result = false;
                            break;
                        case ValueType::_NULL:
                            return_value.result = true;
                            break;
                    }
                }
                else if(right_value.result.get_type() == ValueType::_NULL)
                {
                    switch(left_value.result.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
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
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::NOT_EQUAL:
            {
                if(left_value.result.get_type() == ValueType::NUMBER 
                        && right_value.result.get_type() == ValueType::NUMBER)
                {
                    return_value.result = left_value.result.to_number() != right_value.result.to_number();
                }
                else if(left_value.result.get_type() == ValueType::BOOL 
                        && right_value.result.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.result.to_bool() != right_value.result.to_bool();
                }
                else if(left_value.result.get_type() == ValueType::STRING 
                        && right_value.result.get_type() == ValueType::STRING)
                {
                    return_value.result = left_value.result.to_string() != right_value.result.to_string();
                }
                else if(left_value.result.get_type() == ValueType::_NULL)
                {
                    switch(right_value.result.get_type())
                    {
                        case ValueType::NUMBER:
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
                            return_value.result = true;
                            break;
                        case ValueType::_NULL:
                            return_value.result = false;
                            break;
                    }
                }
                else if(right_value.result.get_type() == ValueType::_NULL)
                {
                    switch(left_value.result.get_type())
                    {
                        case ValueType::NUMBER:
                        
                        case ValueType::BOOL:
                        case ValueType::STRING:
                        case ValueType::SOURDO_FUNCTION:
                        case ValueType::CPP_FUNCTION:
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
                    ss << node->position << "Cannot perform comparison operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LOGIC_OR:
            {
                if(left_value.result.get_type() == ValueType::BOOL 
                        && right_value.result.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.result.to_bool() || right_value.result.to_bool();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform logical operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
                    return_value.error_message = ss.str();
                }

                break;
            }
            case Token::Type::LOGIC_AND:
            {
                if(left_value.result.get_type() == ValueType::BOOL 
                        && right_value.result.get_type() == ValueType::BOOL)
                {
                    return_value.result = left_value.result.to_bool() && right_value.result.to_bool();
                }
                else
                {
                    std::stringstream ss;
                    ss << node->position << "Cannot perform logical operation with types " << left_value.result.get_type() << " and " << right_value.result.get_type();
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

    static VisitorReturn visit_call_node(Data::Impl* data, std::shared_ptr<CallNode> node)
    {
        VisitorReturn return_value;
        VisitorReturn callee = visit_ast(data, node->callee);
        if(callee.error_message)
        {
            return callee;
        }

        if(callee.result.get_type() == ValueType::SOURDO_FUNCTION)
        {
            std::shared_ptr<SourDoFunction> func_value = callee.result.to_sourdo_function();
            if(node->arguments.size() != func_value->parameters.size())
            {
                std::stringstream ss;
                ss << node->position << "Function being called expected " 
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

                ss << node->arguments.size();

                // More Grammar!
                if(node->arguments.size() == 1)
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
                VisitorReturn arg = visit_ast(data, node->arguments[i]);
                if(arg.error_message)
                {
                    return arg;
                }

                func_scope.get_impl()->symbol_table[func_value->parameters[i]] = arg.result;
            }

            return_value = visit_ast(func_scope.get_impl(), func_value->statements);
        }
        else if(callee.result.get_type() == ValueType::CPP_FUNCTION)
        {
            CppFunction func_value = callee.result.to_cpp_function();

            Data func_scope;
            func_scope.get_impl()->parent = data;

            func_scope.get_impl()->stack.reserve(node->arguments.size());
            
            for(int i = 0; i < node->arguments.size(); i++)
            {
                VisitorReturn arg = visit_ast(data, node->arguments[i]);
                if(arg.error_message)
                {
                    return arg;
                }

                func_scope.get_impl()->stack.push_back(arg.result);
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
                ss << node->position << error.what();
                return_value.error_message = ss.str();
            }
        }
        else
        {
            std::stringstream ss;
            ss << node->position << "Value being called is not a function";
            return_value.error_message = ss.str();
        }
        
        return return_value;
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
            case Node::Type::VAR_DECLARATION_NODE:
            {
                return_value = visit_var_declaration_node(data, std::static_pointer_cast<VarDeclarationNode>(node));
                break;
            }
            case Node::Type::VAR_ASSIGNMENT_NODE:
            {
                return_value = visit_var_assignment_node(data, std::static_pointer_cast<VarAssignmentNode>(node));
                break;
            }
            case Node::Type::FUNC_DECLARATION_NODE:
            {
                return_value = visit_func_declaration_node(data, std::static_pointer_cast<FuncDeclarationNode>(node));
                break;
            }
            case Node::Type::RETURN_NODE:
            {
                return_value = visit_return_node(data, std::static_pointer_cast<ReturnNode>(node));
                break;
            }
            case Node::Type::VAR_ACCESS_NODE:
            {
                return_value = visit_var_access_node(data, std::static_pointer_cast<VarAccessNode>(node));
                break;
            }
            case Node::Type::CALL_NODE:
            {
                return_value = visit_call_node(data, std::static_pointer_cast<CallNode>(node));
                break;
            }
            case Node::Type::NUMBER_VALUE_NODE:
            {
                auto number_node = std::static_pointer_cast<NumberValueNode>(node);
                return_value.result = std::stod(number_node->value.value);
                break;
            }
            case Node::Type::BOOL_VALUE_NODE:
            {
                auto bool_node = std::static_pointer_cast<BoolValueNode>(node);
                return_value.result = bool_node->value.value == "true";
                break;
            }
            case Node::Type::NULL_VALUE_NODE:
            {
                return_value.result = Null(); 
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
