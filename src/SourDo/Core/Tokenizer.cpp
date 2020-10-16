#include "Tokenizer.h"

namespace srdo {
    Tokenizer::Tokenizer(const std::string& name, const std::string& text)
        : text(text), currentChar(text[0]), pos(name, text, 0, 0, 0), errors("", 40)
    {
    }

    const std::vector<Token>& Tokenizer::getTokens() const
    {
        return tokenList;
    }

    const IssuePack<Error>& Tokenizer::getErrors() const
    {
        return errors;
    }

    void Tokenizer::clearTokens()
    {
        tokenList.clear();
    }

    void Tokenizer::tokenize()
    {
        while(currentChar != '\0')
        {
            if(std::isspace(currentChar))
            {
                advance();
            }
            else if(isalpha(currentChar) || currentChar == '_')
            {
                makeIdentifier();
            }
            else if(isdigit(currentChar))
            {
                makeNumber();
            }
            else
            {
                switch(currentChar)
                {
                    case '+':
                        tokenList.emplace_back(pos, TType::Plus);
                        break;
                    case '-':
                        tokenList.emplace_back(pos, TType::Minus);
                        break;
                    case '*':
                        advance();
                        if(currentChar == '*')
                        {
                            tokenList.emplace_back(pos, TType::Pow);
                            advance();
                        }
                        tokenList.emplace_back(pos, TType::Mul);
                        continue;
                        break;
                    case '/':
                        tokenList.emplace_back(pos, TType::Div);
                        break;
                    case '=':
                        tokenList.emplace_back(pos, TType::Assign);
                        break;
                    case '(':
                        tokenList.emplace_back(pos, TType::LParen);
                        break;
                    case ')':
                        tokenList.emplace_back(pos, TType::RParen);
                        break;
                    case '<':
                        tokenList.emplace_back(pos, TType::LAngle);
                        break;
                    case '>':
                        tokenList.emplace_back(pos, TType::RAngle);
                        break;
                    case '{':
                        tokenList.emplace_back(pos, TType::LCurly);
                        break;
                    case '}':
                        tokenList.emplace_back(pos, TType::RCurly);
                        break;
                    case '[':
                        tokenList.emplace_back(pos, TType::LSquare);
                        break;
                    case ']':
                        tokenList.emplace_back(pos, TType::RSquare);
                        break;
                    case ';':
                        tokenList.emplace_back(pos, TType::Terminal);
                        break;
                    default:
                    {
                        std::string strChar(1, currentChar);
                        errors.emplaceIssue("Unexpected character '" + strChar + "'", pos);
                        break;
                    }
                }
                advance();
            }
        }
        tokenList.emplace_back(pos, TType::Eof);
    }

    void Tokenizer::advance()
    {
        pos.advance(currentChar);
        currentChar = text[pos.idx];
    }

    void Tokenizer::makeNumber()
    {
        std::string numStr = "";
        bool containsPoint = false;
        bool isFloat = false;
        do
        {
            if(isdigit(currentChar))
            {
                numStr += currentChar;
            }
            else if (currentChar == '.')
            {
                if(containsPoint)
                {
                    advance();
                    break;
                }
                containsPoint = true;
                numStr += currentChar;
            }
            else
            {
                isFloat = true;
                advance();
                break;
            }
            advance();
        } 
        while(currentChar != '\0' && (isdigit(currentChar) 
                || currentChar == '.' 
                || currentChar == 'f'));
        
        if(containsPoint)
        {
            if(isFloat)
            {
                tokenList.emplace_back(pos, TType::Float, std::stof(numStr));
            }
            else
            {
                tokenList.emplace_back(pos, TType::Double, std::stod(numStr));
            }
        }
        else
        {
            tokenList.emplace_back(pos, TType::Int, std::stoi(numStr));
        }
    }

    void Tokenizer::makeIdentifier()
    {
        std::string str = "";
        do
        {
            str += currentChar;
            advance();
        }
        while(currentChar != '\0' && (std::isalnum(currentChar) || currentChar == '_'));


        if(std::find(KEYWORDS.begin(), KEYWORDS.end(), str) != KEYWORDS.end())
        {
            tokenList.emplace_back(pos, TType::Keyword, str);
        }
        else
        {
            tokenList.emplace_back(pos, TType::Id, str);
        }
    }
}