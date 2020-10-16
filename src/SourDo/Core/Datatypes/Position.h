#pragma once

namespace srdo {
    class Position
    {
    public:
        Position(const std::string& fn, const std::string& ftxt, 
                    int ln, int col, int idx)
            : fn(fn), ftxt(ftxt), ln(ln), col(col), idx(idx)
        {
        }

        std::string fn, ftxt;
        unsigned int ln, col, idx;
        
        void advance(char currentChar)
        {
            idx++;
            col++;
            if(currentChar == '\n')
            {
                ln++;
                col = 0;
            }
        }
    };
}