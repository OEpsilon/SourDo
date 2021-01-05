#pragma once

namespace SourDo {
    struct Position 
    {
        Position() = default;
        Position(int index, int line, int column)
            : index(index), line(line), column(column)
        {
        }

        int index, line, column;

        /**
         * Usually I don't put functions in structs, but it doesn't matter
         * in this case
         */
        void advance(char current_char)
        {
            index++;
            column++;

            if(current_char == '\n')
            {
                line++;
                column = 0;
            }
        }
    };
};
