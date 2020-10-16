#pragma once

#include "Datatypes/Token.h"
#include "Datatypes/Issues.h"

namespace srdo {
    class Tokenizer 
    {
    public:
        Tokenizer(const std::string& name,const std::string& text);
        const std::vector<Token>& getTokens() const;
        const IssuePack<Error>& getErrors() const;
        void tokenize();
        void clearTokens();
    private:
        std::vector<Token> tokenList;
        IssuePack<Error> errors;
        std::string text;
        char currentChar;
        Position pos;
    private:
        void advance();
        void makeNumber();
        void makeIdentifier();
    };
}