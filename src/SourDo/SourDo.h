#pragma once

#include "Core/Tokenizer.h"

namespace srdo {
    struct ProgramFile
    {
        std::string name;
        std::string contents;
    };

    inline void compile(const ProgramFile& file)
    {
        Tokenizer tokenizer(file.name, file.contents);
        tokenizer.tokenize();
        auto tokens = tokenizer.getTokens();
        tokenizer.getErrors().logIssues();
        if(tokenizer.getErrors().isFatal())
        {
            return;
        }
        std::cout << tokens << std::endl;
    }
}