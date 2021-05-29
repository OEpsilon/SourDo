#include "Token.hpp"

#include <unordered_map>
#include <ostream>

namespace sourdo
{
    std::ostream& operator<<(std::ostream& os, Token::Type type)
    {
        static std::unordered_map<Token::Type, const char*> strings = {
            //  Type                            String
            {   Token::Type::NONE,              "NONE"              },
            {   Token::Type::BUILTIN_TYPE,      "BUILTIN_TYPE"      },


            {   Token::Type::VAR ,              "VAR"               },
            {   Token::Type::CONST ,            "CONST"             },
            {   Token::Type::FUNC ,             "FUNC"              },

            {   Token::Type::RETURN ,           "RETURN"            },
            {   Token::Type::BREAK ,            "BREAK"             },
            {   Token::Type::CONTINUE ,         "CONTINUE"          },

            {   Token::Type::IF ,               "IF"                },
            {   Token::Type::ELIF ,             "ELIF"              },
            {   Token::Type::ELSE ,             "ELSE"              },

            {   Token::Type::FOR ,              "FOR"               },
            {   Token::Type::WHILE ,            "WHILE"             },
            {   Token::Type::LOOP ,             "LOOP"              },

            {   Token::Type::THEN ,             "THEN"              },
            {   Token::Type::DO ,               "DO"                },
            {   Token::Type::END ,              "END"               },


            {   Token::Type::NUMBER_LITERAL,    "NUMBER_LITERAL"    },
            {   Token::Type::STRING_LITERAL,    "STRING_LITERAL"    },
            {   Token::Type::BOOL_TRUE,         "BOOL_TRUE"         },
            {   Token::Type::BOOL_FALSE,        "BOOL_FALSE"        },
            {   Token::Type::NULL_LITERAL,      "NULL_LITERAL"      },
            {   Token::Type::IDENTIFIER ,       "IDENTIFIER"        },

            {   Token::Type::ADD,               "ADD"               },
            {   Token::Type::SUB,               "SUB"               },
            {   Token::Type::MUL,               "MUL"               },
            {   Token::Type::DIV,               "DIV"               },
            {   Token::Type::POW,               "POW"               },

            {   Token::Type::LESS_THAN,         "LESS_THAN"         },
            {   Token::Type::GREATER_THAN,      "GREATER_THAN"      },
            {   Token::Type::LESS_EQUAL,        "LESS_EQUAL"        },
            {   Token::Type::GREATER_EQUAL,     "GREATER_EQUAL"     },
            {   Token::Type::EQUAL,             "EQUAL"             },
            {   Token::Type::NOT_EQUAL,         "NOT_EQUAL"         },

            {   Token::Type::IS,                "IS"                },
            {   Token::Type::OR,                "OR"                },
            {   Token::Type::AND,               "AND"               },
            {   Token::Type::NOT,               "NOT"               },

            {   Token::Type::COMMA,             "COMMA"             },
            {   Token::Type::DOT,               "DOT"               },
            {   Token::Type::COLON,             "COLON"             },

            {   Token::Type::ASSIGN,            "ASSIGN"            },
            {   Token::Type::ASSIGN_ADD,        "ASSIGN_ADD"        },
            {   Token::Type::ASSIGN_SUB,        "ASSIGN_SUB"        },
            {   Token::Type::ASSIGN_MUL,        "ASSIGN_MUL"        },
            {   Token::Type::ASSIGN_DIV,        "ASSIGN_DIV"        },

            {   Token::Type::LPAREN,            "LPAREN"            },
            {   Token::Type::RPAREN,            "RPAREN"            },
            {   Token::Type::LBRACKET,          "LBRACKET"          },
            {   Token::Type::RBRACKET,          "RBRACKET"          },
            {   Token::Type::LBRACE,            "LBRACE"            },
            {   Token::Type::RBRACE,            "RBRACE"            },

            {   Token::Type::NEW_LINE,          "NEW_LINE"          },
            {   Token::Type::TK_EOF,            "EOF"               },
        };

        return os << strings[type];
    }

    std::ostream& operator<<(std::ostream& os, const Token& token)
    {
        os << "[" << token.type;
        if(token.value != "")
        {
            os << ": " << token.value;  
        }
        os << "]";
        return os;
    }

    std::ostream& operator<<(std::ostream& os, const std::vector<Token>& tokens)
    {
        os << "[";
        for(int i = 0; i < tokens.size(); i++)
        {
            os << tokens[i];
            if(i < tokens.size() - 1)
            {
                os << ", ";
            }
        }
        os << "]";
        return os;
    }
} // namespace sourdo
